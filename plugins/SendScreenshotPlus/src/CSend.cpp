/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,

This file is part of Send Screenshot Plus, a Miranda IM plugin.
Copyright (c) 2010 Ing.U.Horn

Parts of this file based on original sorce code
(c) 2004-2006 Sérgio Vieira Rolanski (portet from Borland C++)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "global.h"

//---------------------------------------------------------------------------
CSend::CSend(HWND /*Owner*/, MCONTACT hContact, bool bAsync, bool bSilent) :
	m_bDeleteAfterSend(false),
	m_bAsync(bAsync),
	m_bSilent(bSilent),
	m_pszFile(NULL),
	m_pszFileDesc(NULL),
	m_pszSendTyp(NULL),
	m_pszProto(NULL),
//	m_hContact(hContact), // initialized below
	m_EnableItem(0),
	m_ChatRoom(0),
//	m_PFflag(0),
	m_cbEventMsg(0),
	m_szEventMsg(NULL),
	m_hSend(0),
	m_hOnSend(0),
	m_ErrorMsg(NULL),
	m_ErrorTitle(NULL)
{
	SetContact(hContact);
}

CSend::~CSend(){
	mir_free(m_pszFile);
	mir_free(m_pszFileDesc);
	mir_free(m_szEventMsg);
	mir_free(m_ErrorMsg);
	mir_free(m_ErrorTitle);
	if(m_hOnSend) UnhookEvent(m_hOnSend);
}

//---------------------------------------------------------------------------
void CSend::SetContact(MCONTACT hContact) {
	m_hContact = hContact;
	if(hContact){
		m_pszProto = GetContactProto(hContact);
		m_ChatRoom = db_get_b(hContact, m_pszProto, "ChatRoom", 0);
		/*
		m_PFflag = hasCap(PF1_URLSEND);
		m_PFflag = hasCap(PF1_CHAT);
		m_PFflag = hasCap(PF1_IMSEND);// */
	}
}

//---------------------------------------------------------------------------
/*bool	CSend::hasCap(unsigned int Flag) {
	return (Flag & CallContactService(m_hContact, PS_GETCAPS, PFLAGNUM_1, NULL)) == Flag;
}// */

//---------------------------------------------------------------------------
INT_PTR CALLBACK CSend::ResultDialogProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam){
	switch(uMsg){
	case WM_INITDIALOG:{
		SendMessage(hwndDlg,WM_SETICON,ICON_BIG,(LPARAM)Skin_GetIcon(ICO_COMMON_SSWINDOW1,1));
		CSend* self=(CSend*)lParam;
		TCHAR* tmp=mir_tstrdup(TranslateT("Resulting URL from\n"));
		mir_tcsadd(tmp,self->m_pszSendTyp);
		SetDlgItemText(hwndDlg,IDC_HEADERBAR,tmp);
		mir_free(tmp);
		SendMessage(GetDlgItem(hwndDlg,IDC_HEADERBAR),WM_SETICON,ICON_BIG,(LPARAM)Skin_GetIcon(ICO_COMMON_ARROWR,1));
		SetDlgItemText(hwndDlg,ID_edtURL,self->m_ErrorTitle);
		if(self->m_pszFileDesc)
			SetDlgItemText(hwndDlg,ID_bvlDesc,self->m_pszFileDesc);
		SendMessage(GetDlgItem(hwndDlg,IDOK),BUTTONSETDEFAULT,1,NULL);
		SendMessage(GetDlgItem(hwndDlg,IDOK),BM_SETIMAGE,IMAGE_ICON,(LPARAM)Skin_GetIcon(ICO_BTN_EDIT));
		SendMessage(GetDlgItem(hwndDlg,IDCANCEL),BM_SETIMAGE,IMAGE_ICON,(LPARAM)Skin_GetIcon(ICO_BTN_CANCEL));
		return TRUE;}
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:{
			TCHAR tmp[1024];
			size_t len=GetDlgItemText(hwndDlg,ID_edtURL,tmp,1024);
			int retries=3;
			do{
				if(!OpenClipboard(hwndDlg)){
					Sleep(100);
					continue;
				}
				EmptyClipboard();
				HGLOBAL clipbuffer=GlobalAlloc(GMEM_MOVEABLE, len*sizeof(TCHAR)+sizeof(TCHAR));
				TCHAR* tmp2=(TCHAR*)GlobalLock(clipbuffer);
				mir_tcsncpy(tmp2,tmp,len+1); tmp2[len]='\0';
				GlobalUnlock(clipbuffer);
				SetClipboardData(CF_UNICODETEXT,clipbuffer);
				CloseClipboard();
				break;
			}while(--retries);
			DestroyWindow(hwndDlg);
			return TRUE;}
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return TRUE;
		}
	}
	return FALSE;
}
void CSend::svcSendMsgExit(const char* szMessage) {
	if(m_bSilent){
		Exit(ACKRESULT_SUCCESS); return;
	}
	if(!m_hContact){
		mir_free(m_ErrorTitle), m_ErrorTitle=mir_a2t(szMessage);
		DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_UResultForm),0, ResultDialogProc,(LPARAM)this);
		Exit(ACKRESULT_SUCCESS); return;
	}
	if(m_ChatRoom){
		TCHAR* tmp = mir_a2t(szMessage);
		if (m_pszFileDesc) {
			mir_tcsadd(tmp, _T("\r\n"));
			mir_tcsadd(tmp, m_pszFileDesc);
		}
		GC_INFO gci = {0};
		int res = GC_RESULT_NOSESSION;
		int cnt = (int)CallService(MS_GC_GETSESSIONCOUNT, 0, (LPARAM)m_pszProto);

		//loop on all gc session to get the right (save) ptszID for the chatroom from m_hContact
		gci.pszModule = m_pszProto;
		for (int i = 0; i < cnt ; i++ ) {
			gci.iItem = i;
			gci.Flags = GCF_BYINDEX | GCF_HCONTACT | GCF_ID;
			CallService(MS_GC_GETINFO, 0, (LPARAM)&gci);
			if (gci.hContact == m_hContact) {
				GCDEST gcd = { m_pszProto, gci.pszID, GC_EVENT_SENDMESSAGE };
				GCEVENT gce = { sizeof(gce), &gcd };
				gce.bIsMe = TRUE;
				gce.dwFlags = GCEF_ADDTOLOG;
				gce.ptszText = tmp;
				gce.time = time(NULL);

				//* returns 0 on success or error code on failure
				res = 200 + (int)CallService(MS_GC_EVENT, 0, (LPARAM)&gce);
				break;
			}
		}
		Exit(res); return;
	}else{
		mir_freeAndNil(m_szEventMsg);
		m_cbEventMsg=lstrlenA(szMessage)+1;
		m_szEventMsg=(char*)mir_realloc(m_szEventMsg, sizeof(char)*m_cbEventMsg);
		ZeroMemory(m_szEventMsg, m_cbEventMsg);
		lstrcpyA(m_szEventMsg,szMessage);
		if (m_pszFileDesc && m_pszFileDesc[0] != NULL) {
			char *temp = mir_t2a(m_pszFileDesc);
			mir_stradd(m_szEventMsg, "\r\n");
			mir_stradd(m_szEventMsg, temp);
			m_cbEventMsg = lstrlenA(m_szEventMsg)+1;
			mir_free(temp);
		}
		//create a HookEventObj on ME_PROTO_ACK
		if (!m_hOnSend) {
			m_hOnSend = HookEventObj(ME_PROTO_ACK, OnSend, this);
		}
		//start PSS_MESSAGE service
		m_hSend = (HANDLE)CallContactService(m_hContact, PSS_MESSAGE, NULL, (LPARAM)m_szEventMsg);
		// check we actually got an ft handle back from the protocol
		if (!m_hSend) {
			Unhook();
			Error(SS_ERR_INIT, m_pszSendTyp);
			Exit(ACKRESULT_FAILED); return;
		}
	}
}

void CSend::svcSendFileExit() {
//szMessage should be encoded as the File followed by the description, the
//separator being a single nul (\0). If there is no description, do not forget
//to end the File with two nuls.
	if(m_bSilent){
		Exit(ACKRESULT_SUCCESS); return;
	}
	if(!m_hContact){
		Error(LPGENT("%s requires a valid contact!"), m_pszSendTyp);
		Exit(ACKRESULT_FAILED); return;
	}
	mir_freeAndNil(m_szEventMsg);
	char* szFile = mir_t2a(m_pszFile);
	m_cbEventMsg=lstrlenA(szFile)+2;
	m_szEventMsg=(char*)mir_realloc(m_szEventMsg, sizeof(char)*m_cbEventMsg);
	ZeroMemory(m_szEventMsg, m_cbEventMsg);
	lstrcpyA(m_szEventMsg,szFile);
	if (m_pszFileDesc && m_pszFileDesc[0] != NULL) {
		char* temp = mir_t2a(m_pszFileDesc);
		m_cbEventMsg += lstrlenA(temp);
		m_szEventMsg=(char*)mir_realloc(m_szEventMsg, sizeof(char)*m_cbEventMsg);
		lstrcpyA(m_szEventMsg+lstrlenA(szFile)+1,temp);
		m_szEventMsg[m_cbEventMsg-1] = 0;
		mir_free(temp);
	}
	mir_free(szFile);

	//create a HookEventObj on ME_PROTO_ACK
	if (!m_hOnSend) {
		m_hOnSend = HookEventObj(ME_PROTO_ACK, OnSend, this);
	}

	// Start miranda PSS_FILE based on mir ver (T)
	TCHAR* ppFile[2]={0,0};
	TCHAR* pDesc = mir_tstrdup(m_pszFileDesc);
	ppFile[0] = mir_tstrdup (m_pszFile);
	ppFile[1] = NULL;
	m_hSend = (HANDLE)CallContactService(m_hContact, PSS_FILET, (WPARAM)pDesc, (LPARAM)ppFile);
	mir_free(pDesc);
	mir_free(ppFile[0]);

	// check we actually got an ft handle back from the protocol
	if (!m_hSend) {
		Unhook();
		Error(SS_ERR_INIT, m_pszSendTyp);
		Exit(ACKRESULT_FAILED); return;
	}
}

//---------------------------------------------------------------------------
int CSend::OnSend(void *obj, WPARAM wParam, LPARAM lParam){
	CSend* self=(CSend*)obj;
	ACKDATA *ack=(ACKDATA*)lParam;
	if(ack->hProcess!= self->m_hSend) return 0;
		/*	if(dat->waitingForAcceptance) {
				SetTimer(hwndDlg,1,1000,NULL);
				dat->waitingForAcceptance=0;
			}
		*/

	switch(ack->result) {
	case ACKRESULT_INITIALISING:	//SetFtStatus(hwndDlg, LPGENT("Initialising..."), FTS_TEXT); break;
	case ACKRESULT_CONNECTING:		//SetFtStatus(hwndDlg, LPGENT("Connecting..."), FTS_TEXT); break;
	case ACKRESULT_CONNECTPROXY:	//SetFtStatus(hwndDlg, LPGENT("Connecting to proxy..."), FTS_TEXT); break;
	case ACKRESULT_LISTENING:		//SetFtStatus(hwndDlg, LPGENT("Waiting for connection..."), FTS_TEXT); break;
	case ACKRESULT_CONNECTED:		//SetFtStatus(hwndDlg, LPGENT("Connected"), FTS_TEXT); break;
	case ACKRESULT_SENTREQUEST:		//SetFtStatus(hwndDlg, LPGENT("Decision sent"), FTS_TEXT); break;
	case ACKRESULT_NEXTFILE:		//SetFtStatus(hwndDlg, LPGENT("Moving to next file..."), FTS_TEXT);
	case ACKRESULT_FILERESUME:		//
	case ACKRESULT_DATA:			//transfer is on progress
		break;
	case ACKRESULT_DENIED:
		self->Unhook();
		self->Exit(ack->result);
		break;
	case ACKRESULT_FAILED:
		self->Unhook();
		self->Exit(ack->result);
		//type=ACKTYPE_MESSAGE, result=success/failure, (char*)lParam=error message or NULL.
		//type=ACKTYPE_URL, result=success/failure, (char*)lParam=error message or NULL.
		//type=ACKTYPE_FILE, result=ACKRESULT_FAILED then lParam=(LPARAM)(const char*)szReason
		break;
	case ACKRESULT_SUCCESS:
		self->Unhook();
		switch(ack->type) {
		case ACKTYPE_CHAT:
			break;
		case ACKTYPE_MESSAGE:
			self->DB_EventAdd((WORD)EVENTTYPE_MESSAGE);
			break;
		case ACKTYPE_URL:
			self->DB_EventAdd((WORD)EVENTTYPE_URL);
			break;
		case ACKTYPE_FILE:
			self->m_szEventMsg = (char*) mir_realloc(self->m_szEventMsg, sizeof(DWORD) + self->m_cbEventMsg);
			memmove(self->m_szEventMsg+sizeof(DWORD), self->m_szEventMsg, self->m_cbEventMsg);
			self->m_cbEventMsg += sizeof(DWORD);
			self->DB_EventAdd((WORD)EVENTTYPE_FILE);
			break;
		}
		self->Exit(ack->result);
		break;
	}
	return 0;
}

void CSend::DB_EventAdd(WORD EventType)
{
	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = m_pszProto;
	dbei.eventType = EventType;
	dbei.flags = DBEF_SENT;
	dbei.timestamp = time(NULL);
	dbei.flags |= DBEF_UTF;
	dbei.cbBlob= m_cbEventMsg;
	dbei.pBlob = (PBYTE)m_szEventMsg;
	db_event_add(m_hContact, &dbei);
}

//---------------------------------------------------------------------------
void CSend::Error(LPCTSTR pszFormat, ...) {
	TCHAR tszMsg[MAX_SECONDLINE];

	mir_sntprintf(tszMsg, SIZEOF(tszMsg),_T("%s - %s") ,_T(MODNAME), TranslateT("Error"));
	mir_free(m_ErrorTitle), m_ErrorTitle = mir_tstrdup(tszMsg);

	va_list vl;
	va_start(vl, pszFormat);
	mir_vsntprintf(tszMsg, SIZEOF(tszMsg), TranslateTS(pszFormat), vl);
	va_end(vl);
	mir_free(m_ErrorMsg), m_ErrorMsg = mir_tstrdup(tszMsg);

	ZeroMemory(&m_box, sizeof(MSGBOX));
	m_box.cbSize		= sizeof(MSGBOX);
	m_box.hParent		= NULL;
	m_box.hiLogo		= Skin_GetIcon(ICO_COMMON_SSWINDOW1);
	m_box.hiMsg			= NULL;
	m_box.ptszTitle		= m_ErrorTitle;
	m_box.ptszMsg		= m_ErrorMsg;
	m_box.uType			= MB_OK|MB_ICON_ERROR;
}

//---------------------------------------------------------------------------
void CSend::Exit(unsigned int Result) {
	if(!m_bSilent){
		bool err = true;
		switch(Result) {
			case ACKRESULT_SUCCESS:
			case GC_RESULT_SUCCESS:
				SkinPlaySound("FileDone");
				err = false;
				break;
			case ACKRESULT_DENIED:
				SkinPlaySound("FileDenied");
				Error(_T("%s (%i):\nFile transfer denied."),TranslateTS(m_pszSendTyp),Result);
				MsgBoxService(NULL, (LPARAM)&m_box);
				err = false;
				break;
			case GC_RESULT_WRONGVER:	//.You appear to be using the wrong version of GC API.
				Error(_T("%s (%i):\nYou appear to be using the wrong version of GC API"),TranslateT("GCHAT error"),Result);
				break;
			case GC_RESULT_ERROR:		// An internal GC error occurred.
				Error(_T("%s (%i):\nAn internal GC error occurred."),TranslateT("GCHAT error"),Result);
				break;
			case GC_RESULT_NOSESSION:	// contact has no open GC session
				Error(_T("%s (%i):\nContact has no open GC session."),TranslateT("GCHAT error"),Result);
				break;
			case ACKRESULT_FAILED:
			default:
				break;
		}
		if(err){
			SkinPlaySound("FileFailed");
			if(m_ErrorMsg) MsgBoxService(NULL, (LPARAM)&m_box);
			else MsgErr(NULL, LPGENT("An unknown error has occurred."));
		}
	}
	if(m_pszFile && *m_pszFile && m_bDeleteAfterSend && m_EnableItem&SS_DLG_DELETEAFTERSSEND) {
		DeleteFile(m_pszFile), m_pszFile=NULL;
	}
	if(m_bAsync)
		delete this;/// deletes derived class since destructor is virtual (which also auto-calls base dtor)
}
