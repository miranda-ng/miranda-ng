/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (с) 2012-15 Miranda NG project (http://miranda-ng.org),
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
#define CSEND_DIALOG 8800

//---------------------------------------------------------------------------
CSend::CSend(HWND /*Owner*/, MCONTACT hContact, bool bAsync, bool bSilent) :
	m_bDeleteAfterSend(false),
	m_bAsync(bAsync),
	m_bSilent(bSilent),
	m_pszFile(NULL),
	m_pszFileDesc(NULL),
	m_URL(NULL),
	m_URLthumb(NULL),
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
	mir_free(m_URL);
	mir_free(m_URLthumb);
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
		SendMessage(hwndDlg,WM_SETICON,ICON_BIG,(LPARAM)GetIcon(ICO_MAIN));
		CSend* self=(CSend*)lParam;
		TCHAR* tmp=mir_tstrdup(TranslateT("Resulting URL from\n"));
		mir_tstradd(tmp,self->m_pszSendTyp);
		SetDlgItemText(hwndDlg,IDC_HEADERBAR,tmp);
		mir_free(tmp);
		SendDlgItemMessage(hwndDlg,IDC_HEADERBAR,WM_SETICON,ICON_BIG,(LPARAM)GetIconBtn(ICO_BTN_ARROWR));
		SetDlgItemTextA(hwndDlg,ID_edtURL,self->m_URL);
		if(self->m_URLthumb){
			SetDlgItemTextA(hwndDlg,ID_edtURLthumb,self->m_URLthumb);
		}else{
			SetDlgItemTextA(hwndDlg,ID_edtURLthumb,"-");
			for(int i=ID_btnThumbCopy; i<=ID_edtURLthumb; ++i){
				EnableWindow(GetDlgItem(hwndDlg,i),FALSE);
			}
		}
		if(!self->m_pszFileDesc)
			SetDlgItemText(hwndDlg,ID_bvlDesc,self->m_ErrorTitle);
		else
			SetDlgItemText(hwndDlg,ID_bvlDesc,self->m_pszFileDesc);
		SendDlgItemMessage(hwndDlg,IDOK,BM_SETIMAGE,IMAGE_ICON,(LPARAM)GetIconBtn(ICO_BTN_COPY));
		SendDlgItemMessage(hwndDlg,IDOK,BUTTONTRANSLATE,0,0);
		SendDlgItemMessage(hwndDlg,IDCANCEL,BM_SETIMAGE,IMAGE_ICON,(LPARAM)GetIconBtn(ICO_BTN_CANCEL));
		SendDlgItemMessage(hwndDlg,IDCANCEL,BUTTONTRANSLATE,0,0);
		for(int i=ID_btnCopy; i<=ID_btnThumbBBC2; ++i){
			SendDlgItemMessage(hwndDlg,i,BUTTONSETASTHEMEDBTN,0,0);
			SendDlgItemMessage(hwndDlg,i,BUTTONSETASFLATBTN,1,0);
			switch(i){
			case ID_btnCopy:
			case ID_btnThumbCopy:
				SendDlgItemMessage(hwndDlg,i,BM_SETIMAGE,IMAGE_ICON,(LPARAM)GetIconBtn(ICO_BTN_COPY));
				SendDlgItemMessage(hwndDlg,i,BUTTONADDTOOLTIP,(WPARAM)LPGENT("Copy"),BATF_TCHAR);
				break;
			case ID_btnBBC:
			case ID_btnThumbBBC:
				SendDlgItemMessage(hwndDlg,i,BM_SETIMAGE,IMAGE_ICON,(LPARAM)GetIconBtn(ICO_BTN_BBC));
				SendDlgItemMessage(hwndDlg,i,BUTTONADDTOOLTIP,(WPARAM)LPGENT("Copy BBCode"),BATF_TCHAR);
				break;
			default:
				SendDlgItemMessage(hwndDlg,i,BM_SETIMAGE,IMAGE_ICON,(LPARAM)GetIconBtn(ICO_BTN_BBCLNK));
				SendDlgItemMessage(hwndDlg,i,BUTTONADDTOOLTIP,(WPARAM)LPGENT("Copy BBCode w/ link"),BATF_TCHAR);
			}
		}
		TranslateDialogDefault(hwndDlg);
		return TRUE;}
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
		case ID_btnCopy:
		case ID_btnThumbCopy:
		case ID_btnBBC:
		case ID_btnThumbBBC:
		case ID_btnThumbBBC2:{
			TCHAR tmp[2048];
			int edtID=ID_edtURL;
			int bbc=0;
			switch(LOWORD(wParam)){
			case ID_btnThumbBBC2: ++bbc;
			case ID_btnThumbBBC: ++bbc;
			case ID_btnThumbCopy:
				edtID=ID_edtURLthumb;
				break;
			case ID_btnBBC: ++bbc;
				break;
			}
			size_t len;
			if(bbc){
				if(bbc==1){
					memcpy(tmp,_T("[img]"),5*sizeof(TCHAR)); len=5;
					len+=GetDlgItemText(hwndDlg,edtID,tmp+len,2048-11);
					memcpy(tmp+len,_T("[/img]"),7*sizeof(TCHAR)); len+=7;
				}else{
					memcpy(tmp,_T("[url="),5*sizeof(TCHAR)); len=5;
					len+=GetDlgItemText(hwndDlg,ID_edtURL,tmp+len,1024);
					memcpy(tmp+len,_T("][img]"),6*sizeof(TCHAR)); len+=6;
					len+=GetDlgItemText(hwndDlg,edtID,tmp+len,1024);
					memcpy(tmp+len,_T("[/img][/url]"),13*sizeof(TCHAR)); len+=12;
				}
			}else
				len=GetDlgItemText(hwndDlg,edtID,tmp,SIZEOF(tmp));
			int retries=3;
			do{
				if(!OpenClipboard(hwndDlg)){
					Sleep(100);
					continue;
				}
				EmptyClipboard();
				HGLOBAL clipbuffer=GlobalAlloc(GMEM_MOVEABLE, len*sizeof(TCHAR)+sizeof(TCHAR));
				TCHAR* tmp2=(TCHAR*)GlobalLock(clipbuffer);
				mir_tstrncpy(tmp2,tmp,len+1); tmp2[len]='\0';
				GlobalUnlock(clipbuffer);
				SetClipboardData(CF_UNICODETEXT,clipbuffer);
				CloseClipboard();
				break;
			}while(--retries);
			if(LOWORD(wParam)==IDOK)
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
		if(!m_pszFileDesc)
			m_pszFileDesc=mir_a2t(szMessage);
		Exit(CSEND_DIALOG); return;
	}
	if(m_ChatRoom){
		TCHAR* tmp = mir_a2t(szMessage);
		if (m_pszFileDesc) {
			mir_tstradd(tmp, _T("\r\n"));
			mir_tstradd(tmp, m_pszFileDesc);
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
		m_cbEventMsg = (DWORD)mir_strlen(szMessage)+1;
		m_szEventMsg = (char*)mir_realloc(m_szEventMsg, (sizeof(char) * m_cbEventMsg));
		memset(m_szEventMsg, 0, (sizeof(char) * m_cbEventMsg));
		mir_strcpy(m_szEventMsg,szMessage);
		if (m_pszFileDesc && m_pszFileDesc[0] != NULL) {
			char *temp = mir_t2a(m_pszFileDesc);
			mir_stradd(m_szEventMsg, "\r\n");
			mir_stradd(m_szEventMsg, temp);
			m_cbEventMsg = (DWORD)mir_strlen(m_szEventMsg)+1;
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
	m_cbEventMsg = (DWORD)mir_strlen(szFile)+2;
	m_szEventMsg=(char*)mir_realloc(m_szEventMsg, (sizeof(char) * m_cbEventMsg));
	memset(m_szEventMsg, 0, (sizeof(char) * m_cbEventMsg));
	mir_strcpy(m_szEventMsg,szFile);
	if (m_pszFileDesc && m_pszFileDesc[0] != NULL) {
		char* temp = mir_t2a(m_pszFileDesc);
		m_cbEventMsg += (DWORD)mir_strlen(temp);
		m_szEventMsg=(char*)mir_realloc(m_szEventMsg, sizeof(char)*m_cbEventMsg);
		mir_strcpy(m_szEventMsg+mir_strlen(szFile)+1,temp);
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

	mir_sntprintf(tszMsg, SIZEOF(tszMsg),_T("%s - %s") ,_T(SZ_SENDSS), TranslateT("Error"));
	mir_free(m_ErrorTitle), m_ErrorTitle = mir_tstrdup(tszMsg);

	va_list vl;
	va_start(vl, pszFormat);
	mir_vsntprintf(tszMsg, SIZEOF(tszMsg), TranslateTS(pszFormat), vl);
	va_end(vl);
	mir_free(m_ErrorMsg), m_ErrorMsg = mir_tstrdup(tszMsg);

	memset(&m_box, 0, sizeof(MSGBOX));
	m_box.cbSize		= sizeof(MSGBOX);
	m_box.hParent		= NULL;
	m_box.hiLogo		= GetIcon(ICO_MAIN);
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
			case CSEND_DIALOG:
				SkinPlaySound("FileDone");
				DialogBoxParam(g_hSendSS, MAKEINTRESOURCE(IDD_UResultForm),0, ResultDialogProc,(LPARAM)this);
				err = false;
				break;
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

/// helper functions used for HTTP uploads
//---------------------------------------------------------------------------
#define snprintf _snprintf
const char* CSend::GetHTMLContent(char* str, const char* startTag, const char* endTag) {
	char* begin=strstr(str,startTag);
	if(!begin) return NULL;
	begin+=mir_strlen(startTag)-1;
	for(; *begin!='>' && *begin; ++begin);
	if(*begin){
		char* end=strstr(++begin,endTag);
		if(end) *end=0;
	}
	return begin;
}
int JSON_ParseData_(const char** jsondata,size_t jsonlen,const char** rawdata){
	const char* c=*jsondata;
	const char* jsonend=*jsondata+jsonlen;
	int len=0;
	*rawdata=NULL;
	if(c==jsonend)
		return 0;
	if(*c=='{'){ // scope (object)
		*rawdata=c;
		do{
			if(*c=='{') ++len;
			else if(*c=='}') --len;
			if(++c==jsonend)
				return 0;
		}while(len>0);
		len=c-*rawdata;
		if(*c==',') ++c;
	}else if(*c=='"' || *c=='\''){ // string
		char needle=*c;
		if(++c==jsonend)
			return 0;
		*rawdata=c;
		do{
			if(c==jsonend || (*c=='\\' && ++c==jsonend))
				return 0;
		}while(*c++!=needle);
		len=c-*rawdata-1;
		if(*c==',') ++c;
	}else{ // other
		for(*rawdata=c; c<jsonend && *c++!=','; );
		len=c-*rawdata;
		if(c[-1]==',') --len;
	}
	*jsondata=c;
	return len;
}
int JSON_Get_(const char* json, size_t jsonlen, const char* variable, const char** value) {
	char needle[32];
	const char* needlechild;
	char var[32];
	char* tmp;
	const char* c;
	const char* jsonend=json+jsonlen;
	/// get needle
	if(!jsonlen || *json!='{')
		return 0;
	for(tmp=needle,c=*variable=='['?variable+1:variable; *c!='[' && *c!=']'; ++c){
		if(c==jsonend)
			return 0;
		if(tmp<needle+sizeof(needle)-1) *tmp++=*c;
	}
	*tmp='\0';
	/// get child needle (if any)
	if(*c==']') ++c;
	if(c==jsonend)
		return 0;
	needlechild=c;
	/// parse JSON
	for(c=json+1; c<jsonend && (*c=='"' || *c=='\''); ){
		for(++c,tmp=var; c<jsonend && (*c!='"' && *c!='\''); ++c)
			if(tmp<var+sizeof(var)-1) *tmp++=*c;
		*tmp='\0';
		if(c+2>=jsonend || *++c!=':') break;
		/// read data
		++c;
		if(!mir_strcmp(var,needle)){
			int datalen=JSON_ParseData_(&c,jsonend-c,value);
			if(!datalen)
				return 0;
			if(*needlechild && **value=='{'){ // we need a child value, parse child object
				return JSON_Get_(*value,datalen,needlechild,value);
			}
			return datalen;
		}else{
			JSON_ParseData_(&c,jsonend-c,value);
		}
	}
	*value=NULL;
	return 0;
}
int CSend::GetJSONString(const char* json, size_t jsonlen, const char* variable, char* value, size_t valuesize) {
	if(!jsonlen || !valuesize)
		return 0;
	const char* rawvalue;
	int rawlen=JSON_Get_(json,jsonlen,variable,&rawvalue);
	if(rawlen){
		size_t out=0;
		--valuesize;
		/// copy & parse escape sequences
		for(int in=0; in<rawlen && out<valuesize; ++in,++out){
			if(rawvalue[in]=='\\'){
				if(++in==rawlen)
					break;
				switch(rawvalue[in]){
				case 's': value[out]=' '; break;
				case 't': value[out]='\t'; break;
				case 'n': value[out]='\n'; break;
				case 'r': value[out]='\r'; break;
				default: value[out]=rawvalue[in];
				}
				continue;
			}
			value[out]=rawvalue[in];
		}
		value[out]='\0';
		return 1;
	}
	*value='\0';
	return 0;
}
int CSend::GetJSONInteger(const char* json, size_t jsonlen, const char* variable,int defvalue) {
	const char* rawvalue;
	int rawlen=JSON_Get_(json,jsonlen,variable,&rawvalue);
	if(rawlen){
		defvalue=0;
		for(int offset=0; offset<rawlen; ++offset){
			if(rawvalue[offset]<'0' || rawvalue[offset]>'9') break;
			defvalue*=10;
			defvalue+=rawvalue[offset]-'0';
		}
	}
	return defvalue;
}
bool CSend::GetJSONBool(const char* json, size_t jsonlen, const char* variable) {
	const char* rawvalue;
	int rawlen=JSON_Get_(json,jsonlen,variable,&rawvalue);
	if(rawlen){
		if(rawlen==4 && !memcmp(rawvalue,"true",4))
			return true;
		if(*rawvalue>'0' && *rawvalue<='9')
			return true;
	}
	return false;
}

static void HTTPFormAppendData(NETLIBHTTPREQUEST* nlhr, size_t* dataMax, char** dataPos, const char* data,size_t len){
	nlhr->dataLength=(*dataPos-nlhr->pData);
	if(nlhr->dataLength+len >= *dataMax){
		*dataPos=nlhr->pData;
		*dataMax+=0x1000+0x1000*(len>>12);
		nlhr->pData=(char*)mir_realloc(nlhr->pData,*dataMax);
		if(!nlhr->pData) mir_free(*dataPos);
		*dataPos=nlhr->pData;
		if(!*dataPos)
			return;
		*dataPos+=nlhr->dataLength;
	}
	if(data){
		memcpy(*dataPos,data,sizeof(char)*len); *dataPos+=len;
		nlhr->dataLength += (int)len; // not necessary
	}
}
void CSend::HTTPFormDestroy(NETLIBHTTPREQUEST* nlhr)
{
	mir_free(nlhr->headers[0].szValue), nlhr->headers[0].szValue=NULL;
	mir_free(nlhr->headers), nlhr->headers=NULL;
	mir_free(nlhr->pData), nlhr->pData=NULL;
}
int CSend::HTTPFormCreate(NETLIBHTTPREQUEST* nlhr,int requestType,char* url,HTTPFormData* frm,size_t frmNum)
{
	char boundary[16];
	memcpy(boundary,"--M461C/",8);
	{
		union{
			DWORD num;
			unsigned char cr[4];
		};num=GetTickCount()^0x8000;
		for(int i=0; i<4; ++i){
			unsigned char chcode=cr[i]>>4;
			boundary[8+i*2]=(chcode<0x0a ? '0' : 'a'-0x0a)+chcode;
			chcode=cr[i]&0x0f;
			boundary[9+i*2]=(chcode<0x0a ? '0' : 'a'-0x0a)+chcode;
		}
	}
	nlhr->cbSize		=sizeof(NETLIBHTTPREQUEST);
	nlhr->requestType	=requestType;
	nlhr->flags			=NLHRF_HTTP11;
	if(!strncmp(url,"https://",8)) nlhr->flags|=NLHRF_SSL;
	nlhr->szUrl			=url;
	nlhr->headersCount	=3;
	for(HTTPFormData* iter=frm,* end=frm+frmNum; iter!=end; ++iter){
		if(!(iter->flags&HTTPFF_HEADER)) break;
		++nlhr->headersCount;
	}
	nlhr->headers		=(NETLIBHTTPHEADER*)mir_alloc(sizeof(NETLIBHTTPHEADER)*nlhr->headersCount);
	{
		char* contenttype=(char*)mir_alloc(sizeof(char)*(30+1+sizeof(boundary)));
		memcpy(contenttype,"multipart/form-data; boundary=",30);
		memcpy(contenttype+30,boundary,sizeof(boundary));
		contenttype[30+sizeof(boundary)]='\0';
		nlhr->headers[0].szName		="Content-Type";
		nlhr->headers[0].szValue	=contenttype;
		nlhr->headers[1].szName		="User-Agent";
		nlhr->headers[1].szValue	=__USER_AGENT_STRING;
		nlhr->headers[2].szName		="Accept-Language";
		nlhr->headers[2].szValue	="en-us,en;q=0.8";
		int i=3;
		for(HTTPFormData* iter=frm,* end=frm+frmNum; iter!=end; ++iter){
			if(!(iter->flags&HTTPFF_HEADER)) break;
			nlhr->headers[i].szName=(char*)iter->name;
			nlhr->headers[i++].szValue=(char*)iter->value_str;
		}
	}
	char* dataPos=nlhr->pData;
	size_t dataMax=0;
	for(HTTPFormData* iter=frm,* end=frm+frmNum; iter!=end; ++iter){
		if(iter->flags&HTTPFF_HEADER) continue;
		HTTPFormAppendData(nlhr,&dataMax,&dataPos,NULL,2+sizeof(boundary)+40);
		memset(dataPos,'-',2); dataPos+=2;
		memcpy(dataPos,boundary,sizeof(boundary)); dataPos+=sizeof(boundary);
		memcpy(dataPos,"\r\nContent-Disposition: form-data; name=\"",40); dataPos+=40;
		size_t namelen=mir_strlen(iter->name), valuelen;
		if(!(iter->flags&HTTPFF_INT))
			valuelen=mir_strlen(iter->value_str);
		if(iter->flags&HTTPFF_FILE){
			const char* filename	=strrchr(iter->value_str,'\\');
			if(!filename) filename	=strrchr(iter->value_str,'/');
			if(!filename) filename	=iter->value_str;
			else ++filename;
			valuelen=mir_strlen(filename);
			HTTPFormAppendData(nlhr,&dataMax,&dataPos,NULL,namelen+13+valuelen+17);
			memcpy(dataPos,iter->name,namelen); dataPos+=namelen;
			memcpy(dataPos,"\"; filename=\"",13); dataPos+=13;
			memcpy(dataPos,filename,valuelen); dataPos+=valuelen;
			memcpy(dataPos,"\"\r\nContent-Type: ",17); dataPos+=17;
			/// add mime type
			const char* mime="application/octet-stream";
			const char* fileext=strrchr(filename,'.');
			if(fileext){
				if(!mir_strcmp(fileext,".jpg") || !mir_strcmp(fileext,".jpeg") || !mir_strcmp(fileext,".jpe"))
					mime="image/jpeg";
				else if(!mir_strcmp(fileext,".bmp"))
					mime="image/bmp";
				else if(!mir_strcmp(fileext,".png"))
					mime="image/png";
				else if(!mir_strcmp(fileext,".gif"))
					mime="image/gif";
				else if(!mir_strcmp(fileext,".tif") || !mir_strcmp(fileext,".tiff"))
					mime="image/tiff";
			}
			HTTPFormAppendData(nlhr,&dataMax,&dataPos,mime,mir_strlen(mime));
			HTTPFormAppendData(nlhr,&dataMax,&dataPos,"\r\n\r\n",4);
			/// add file content
			size_t filesize=0;
			FILE* fp=fopen(iter->value_str,"rb");
			if(fp){
				fseek(fp,0,SEEK_END);
				filesize=ftell(fp); fseek(fp,0,SEEK_SET);
				HTTPFormAppendData(nlhr,&dataMax,&dataPos,NULL,filesize+2);
				if(fread(dataPos,1,filesize,fp)!=filesize){
					fclose(fp), fp=NULL;
				}
			}
			if(!fp){
				HTTPFormDestroy(nlhr);
				Error(_T("Error occurred when opening local file.\nAborting file upload..."));
				Exit(ACKRESULT_FAILED);
				return 1;
			}else
				fclose(fp);
			dataPos+=filesize;
			memcpy(dataPos,"\r\n",2); dataPos+=2;
		}else if(iter->flags&HTTPFF_8BIT){
			HTTPFormAppendData(nlhr,&dataMax,&dataPos,NULL,namelen+38+valuelen+2);
			memcpy(dataPos,iter->name,namelen); dataPos+=namelen;
			memcpy(dataPos,"\"\r\nContent-Transfer-Encoding: 8bit\r\n\r\n",38); dataPos+=38;
			memcpy(dataPos,iter->value_str,valuelen); dataPos+=valuelen;
			memcpy(dataPos,"\r\n",2); dataPos+=2;
		}else if(iter->flags&HTTPFF_INT){
			HTTPFormAppendData(nlhr,&dataMax,&dataPos,NULL,namelen+5+17/*max numbers*/+2);
			memcpy(dataPos,iter->name,namelen); dataPos+=namelen;
			memcpy(dataPos,"\"\r\n\r\n",5); dataPos+=5;
			int ret=snprintf(dataPos,17,"%Id",iter->value_int);
			if(ret<17 && ret>0) dataPos+=ret;
			memcpy(dataPos,"\r\n",2); dataPos+=2;
		}else{
			HTTPFormAppendData(nlhr,&dataMax,&dataPos,NULL,namelen+5+valuelen+2);
			memcpy(dataPos,iter->name,namelen); dataPos+=namelen;
			memcpy(dataPos,"\"\r\n\r\n",5); dataPos+=5;
			memcpy(dataPos,iter->value_str,valuelen); dataPos+=valuelen;
			memcpy(dataPos,"\r\n",2); dataPos+=2;
		}
	}
	HTTPFormAppendData(nlhr,&dataMax,&dataPos,NULL,2+sizeof(boundary)+4);
	memset(dataPos,'-',2); dataPos+=2;
	memcpy(dataPos,boundary,sizeof(boundary)); dataPos+=sizeof(boundary);
	memcpy(dataPos,"--\r\n",4); dataPos+=4;
	nlhr->dataLength=dataPos-nlhr->pData;
	#ifdef _DEBUG /// print request content to "_sendss_tmp" file for debugging
	{
		FILE* fp=fopen("_sendss_tmp","wb");
		if(fp){
			fprintf(fp,"--Target-- %s\n",nlhr->szUrl);
			for(int i=0; i<nlhr->headersCount; ++i){
				fprintf(fp,"%s: %s\n",nlhr->headers[i].szName,nlhr->headers[i].szValue);
			}
			fprintf(fp,"\n\n");
			fwrite(nlhr->pData,1,nlhr->dataLength,fp);
			fclose(fp);
		}
	}
	#endif // _DEBUG
	return 0;
}
