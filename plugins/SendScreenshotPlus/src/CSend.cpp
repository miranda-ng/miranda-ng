/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"
#define CSEND_DIALOG 8800

/////////////////////////////////////////////////////////////////////////////////////////

CSend::CSend(HWND /*Owner*/, MCONTACT hContact, bool bAsync, bool bSilent) :
	m_bDeleteAfterSend(false),
	m_bAsync(bAsync),
	m_bSilent(bSilent),
	m_pszFile(nullptr),
	m_pszFileDesc(nullptr),
	m_pszSendTyp(nullptr),
	m_pszProto(nullptr),
	m_EnableItem(0),
	m_ChatRoom(0),
	m_cbEventMsg(0),
	m_hSend(nullptr),
	m_hOnSend(nullptr),
	m_ErrorMsg(nullptr),
	m_ErrorTitle(nullptr)
{
	SetContact(hContact);
}

CSend::~CSend()
{
	mir_free(m_pszFile);
	mir_free(m_pszFileDesc);
	mir_free(m_ErrorMsg);
	mir_free(m_ErrorTitle);
	if (m_hOnSend) UnhookEvent(m_hOnSend);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSend::SetContact(MCONTACT hContact)
{
	m_hContact = hContact;
	if (hContact) {
		m_pszProto = Proto_GetBaseAccountName(hContact);
		m_ChatRoom = Contact::IsGroupChat(hContact, m_pszProto);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK CSend::ResultDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		Window_SetIcon_IcoLib(hwndDlg, GetIconHandle(ICO_MAIN));
		{
			CSend *self = (CSend*)lParam;
			SetDlgItemText(hwndDlg, IDC_HEADERBAR, CMStringW(TranslateT("Resulting URL from\n")) + self->m_pszSendTyp);

			SendDlgItemMessage(hwndDlg, IDC_HEADERBAR, WM_SETICON, ICON_BIG, (LPARAM)GetIconBtn(ICO_BTN_ARROWR));
			SetDlgItemTextA(hwndDlg, ID_edtURL, self->m_URL);
			if (self->m_URLthumb) {
				SetDlgItemTextA(hwndDlg, ID_edtURLthumb, self->m_URLthumb);
			}
			else {
				SetDlgItemTextA(hwndDlg, ID_edtURLthumb, "-");
				for (int i = ID_btnThumbCopy; i <= ID_edtURLthumb; ++i) {
					EnableWindow(GetDlgItem(hwndDlg, i), FALSE);
				}
			}
			if (!self->m_pszFileDesc)
				SetDlgItemText(hwndDlg, ID_bvlDesc, self->m_ErrorTitle);
			else
				SetDlgItemText(hwndDlg, ID_bvlDesc, self->m_pszFileDesc);
			SendDlgItemMessage(hwndDlg, IDOK, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIconBtn(ICO_BTN_COPY));
			SendDlgItemMessage(hwndDlg, IDOK, BUTTONTRANSLATE, 0, 0);
			SendDlgItemMessage(hwndDlg, IDCANCEL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIconBtn(ICO_BTN_CANCEL));
			SendDlgItemMessage(hwndDlg, IDCANCEL, BUTTONTRANSLATE, 0, 0);
			for (int i = ID_btnCopy; i <= ID_btnThumbBBC2; ++i) {
				SendDlgItemMessage(hwndDlg, i, BUTTONSETASTHEMEDBTN, 0, 0);
				SendDlgItemMessage(hwndDlg, i, BUTTONSETASFLATBTN, 1, 0);
				switch (i) {
				case ID_btnCopy:
				case ID_btnThumbCopy:
					SendDlgItemMessage(hwndDlg, i, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIconBtn(ICO_BTN_COPY));
					SendDlgItemMessage(hwndDlg, i, BUTTONADDTOOLTIP, (WPARAM)LPGENW("Copy"), BATF_UNICODE);
					break;
				case ID_btnBBC:
				case ID_btnThumbBBC:
					SendDlgItemMessage(hwndDlg, i, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIconBtn(ICO_BTN_BBC));
					SendDlgItemMessage(hwndDlg, i, BUTTONADDTOOLTIP, (WPARAM)LPGENW("Copy BBCode"), BATF_UNICODE);
					break;
				default:
					SendDlgItemMessage(hwndDlg, i, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIconBtn(ICO_BTN_BBCLNK));
					SendDlgItemMessage(hwndDlg, i, BUTTONADDTOOLTIP, (WPARAM)LPGENW("Copy BBCode w/ link"), BATF_UNICODE);
				}
			}
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return TRUE;

		case IDOK:
		case ID_btnCopy:
		case ID_btnThumbCopy:
		case ID_btnBBC:
		case ID_btnThumbBBC:
		case ID_btnThumbBBC2:
			wchar_t tmp[2048];
			int edtID = ID_edtURL;
			int bbc = 0;
			switch (LOWORD(wParam)) {
			case ID_btnThumbBBC2: ++bbc;
			case ID_btnThumbBBC: ++bbc;
			case ID_btnThumbCopy:
				edtID = ID_edtURLthumb;
				break;
			case ID_btnBBC: ++bbc;
				break;
			}
			size_t len;
			if (bbc) {
				if (bbc == 1) {
					memcpy(tmp, L"[img]", 5 * sizeof(wchar_t)); len = 5;
					len += GetDlgItemText(hwndDlg, edtID, tmp + len, 2048 - 11);
					memcpy(tmp + len, L"[/img]", 7 * sizeof(wchar_t)); len += 7;
				}
				else {
					memcpy(tmp, L"[url=", 5 * sizeof(wchar_t)); len = 5;
					len += GetDlgItemText(hwndDlg, ID_edtURL, tmp + len, 1024);
					memcpy(tmp + len, L"][img]", 6 * sizeof(wchar_t)); len += 6;
					len += GetDlgItemText(hwndDlg, edtID, tmp + len, 1024);
					memcpy(tmp + len, L"[/img][/url]", 13 * sizeof(wchar_t)); len += 12;
				}
			}
			else len = GetDlgItemText(hwndDlg, edtID, tmp, _countof(tmp));

			Utils_ClipboardCopy(MClipUnicode(CMStringW(tmp, (int)len + 1)));
			
			if (LOWORD(wParam) == IDOK)
				DestroyWindow(hwndDlg);
			return TRUE;
		}
	}
	return FALSE;
}

void CSend::svcSendMsgExit(const char* szMessage)
{
	if (m_bSilent) {
		Exit(ACKRESULT_SUCCESS);
		return;
	}
	
	if (!m_hContact) {
		if (!m_pszFileDesc)
			m_pszFileDesc = mir_a2u(szMessage);
		Exit(CSEND_DIALOG);
		return;
	}

	if (m_ChatRoom) {
		CMStringW tmp(szMessage);
		if (m_pszFileDesc) {
			tmp.Append(L"\r\n");
			tmp.Append(m_pszFileDesc);
		}
		Chat_SendUserMessage(m_pszProto, tmp);

		Exit(200);
		return;
	}

	m_szEventMsg = szMessage;
	if (m_pszFileDesc && m_pszFileDesc[0] != NULL) {
		m_szEventMsg.Append("\r\n");
		m_szEventMsg.Append(_T2A(m_pszFileDesc));
		m_cbEventMsg = m_szEventMsg.GetLength() + 1;
	}

	// create a HookEventObj on ME_PROTO_ACK
	if (!m_hOnSend)
		m_hOnSend = HookEventObj(ME_PROTO_ACK, OnSend, this);

	// start PSS_MESSAGE service
	m_hSend = (HANDLE)ProtoChainSend(m_hContact, PSS_MESSAGE, NULL, ptrA(mir_utf8encode(m_szEventMsg)));
		
	// check we actually got an ft handle back from the protocol
	if (!m_hSend) {
		Unhook();
		Error(SS_ERR_INIT, m_pszSendTyp);
		Exit(ACKRESULT_FAILED);
	}
}

void CSend::svcSendFileExit()
{
	// szMessage should be encoded as the File followed by the description, the
	// separator being a single nul (\0). If there is no description, do not forget
	// to end the File with two nuls.
	if (m_bSilent) {
		Exit(ACKRESULT_SUCCESS); return;
	}
	
	if (!m_hContact) {
		Error(LPGENW("%s requires a valid contact!"), m_pszSendTyp);
		Exit(ACKRESULT_FAILED); return;
	}
	
	m_szEventMsg = _T2A(m_pszFile);

	if (m_pszFileDesc && m_pszFileDesc[0] != NULL) {
		m_szEventMsg.AppendChar(0);
		m_szEventMsg.Append(_T2A(m_pszFileDesc));
	}
	
	m_cbEventMsg = m_szEventMsg.GetLength() + 1;

	// Сreate a HookEventObj on ME_PROTO_ACK
	if (!m_hOnSend) {
		m_hOnSend = HookEventObj(ME_PROTO_ACK, OnSend, this);
	}

	// Start miranda PSS_FILE based on mir ver (T)
	wchar_t* ppFile[2] = { nullptr, nullptr };
	wchar_t* pDesc = mir_wstrdup(m_pszFileDesc);
	ppFile[0] = mir_wstrdup(m_pszFile);
	ppFile[1] = nullptr;
	m_hSend = (HANDLE)ProtoChainSend(m_hContact, PSS_FILE, (WPARAM)pDesc, (LPARAM)ppFile);
	mir_free(pDesc);
	mir_free(ppFile[0]);

	// check we actually got an ft handle back from the protocol
	if (!m_hSend) {
		Unhook();
		Error(SS_ERR_INIT, m_pszSendTyp);
		Exit(ACKRESULT_FAILED); return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

int CSend::OnSend(void *obj, WPARAM, LPARAM lParam)
{
	CSend* self = (CSend*)obj;
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack->hProcess != self->m_hSend)
		return 0;

	switch (ack->result) {
	case ACKRESULT_INITIALISING: // SetFtStatus(hwndDlg, LPGENW("Initialising..."), FTS_TEXT); break;
	case ACKRESULT_CONNECTING:   // SetFtStatus(hwndDlg, LPGENW("Connecting..."), FTS_TEXT); break;
	case ACKRESULT_CONNECTPROXY: // SetFtStatus(hwndDlg, LPGENW("Connecting to proxy..."), FTS_TEXT); break;
	case ACKRESULT_LISTENING:    // SetFtStatus(hwndDlg, LPGENW("Waiting for connection..."), FTS_TEXT); break;
	case ACKRESULT_CONNECTED:    // SetFtStatus(hwndDlg, LPGENW("Connected"), FTS_TEXT); break;
	case ACKRESULT_SENTREQUEST:  // SetFtStatus(hwndDlg, LPGENW("Decision sent"), FTS_TEXT); break;
	case ACKRESULT_NEXTFILE:     // SetFtStatus(hwndDlg, LPGENW("Moving to next file..."), FTS_TEXT);
	case ACKRESULT_FILERESUME:
	case ACKRESULT_DATA:         // transfer is on progress
		break;
	case ACKRESULT_DENIED:
		self->Unhook();
		self->Exit(ack->result);
		break;
	case ACKRESULT_FAILED:
		self->Unhook();
		self->Exit(ack->result);
		// type=ACKTYPE_MESSAGE, result=success/failure, (char*)lParam=error message or NULL.
		// type=ACKTYPE_FILE, result=ACKRESULT_FAILED then lParam=(LPARAM)(const char*)szReason
		break;
	case ACKRESULT_SUCCESS:
		self->Unhook();
		switch (ack->type) {
		case ACKTYPE_CHAT:
			break;
		case ACKTYPE_MESSAGE:
			self->DB_EventAdd((uint16_t)EVENTTYPE_MESSAGE);
			break;
		case ACKTYPE_FILE:
			self->m_szEventMsg.Insert(0, "aaaa");
			self->m_cbEventMsg += sizeof(uint32_t);
			self->DB_EventAdd((uint16_t)EVENTTYPE_FILE);
			break;
		}
		self->Exit(ack->result);
		break;
	}
	return 0;
}

void CSend::DB_EventAdd(uint16_t EventType)
{
	DBEVENTINFO dbei = {};
	dbei.szModule = m_pszProto;
	dbei.eventType = EventType;
	dbei.flags = DBEF_SENT;
	dbei.iTimestamp = time(0);
	dbei.flags |= DBEF_UTF;
	dbei.cbBlob = m_cbEventMsg;
	dbei.pBlob = m_szEventMsg.GetBuffer();
	db_event_add(m_hContact, &dbei);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSend::Error(LPCTSTR pszFormat, ...)
{
	wchar_t tszMsg[MAX_SECONDLINE];

	mir_snwprintf(tszMsg, L"%s - %s", _A2W(MODULENAME), TranslateT("Error"));
	mir_free(m_ErrorTitle), m_ErrorTitle = mir_wstrdup(tszMsg);

	va_list vl;
	va_start(vl, pszFormat);
	mir_vsnwprintf(tszMsg, _countof(tszMsg), TranslateW(pszFormat), vl);
	va_end(vl);
	mir_free(m_ErrorMsg), m_ErrorMsg = mir_wstrdup(tszMsg);

	memset(&m_box, 0, sizeof(MSGBOX));
	m_box.cbSize = sizeof(MSGBOX);
	m_box.hParent = nullptr;
	m_box.hiLogo = GetIcon(ICO_MAIN);
	m_box.hiMsg = nullptr;
	m_box.ptszTitle = m_ErrorTitle;
	m_box.ptszMsg = m_ErrorMsg;
	m_box.uType = MB_OK | MB_ICON_ERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CSend::Exit(unsigned int Result)
{
	if (!m_bSilent) {
		bool err = true;
		switch (Result) {
		case CSEND_DIALOG:
			Skin_PlaySound("FileDone");
			DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_UResultForm), nullptr, ResultDialogProc, (LPARAM)this);
			err = false;
			break;
		case ACKRESULT_SUCCESS:
		case GC_RESULT_SUCCESS:
			Skin_PlaySound("FileDone");
			err = false;
			break;
		case ACKRESULT_DENIED:
			Skin_PlaySound("FileDenied");
			Error(L"%s (%i):\nFile transfer denied.", TranslateW(m_pszSendTyp), Result);
			MsgBoxService(NULL, (LPARAM)&m_box);
			err = false;
			break;
		case GC_RESULT_WRONGVER:	// You appear to be using the wrong version of GC API.
			Error(L"%s (%i):\nYou appear to be using the wrong version of GC API", TranslateT("GCHAT error"), Result);
			break;
		case GC_RESULT_ERROR:		// An internal GC error occurred.
			Error(L"%s (%i):\nAn internal GC error occurred.", TranslateT("GCHAT error"), Result);
			break;
		case GC_RESULT_NOSESSION:	// contact has no open GC session
			Error(L"%s (%i):\nContact has no open GC session.", TranslateT("GCHAT error"), Result);
			break;
		case ACKRESULT_FAILED:
		default:
			break;
		}
		if (err) {
			Skin_PlaySound("FileFailed");
			if (m_ErrorMsg) MsgBoxService(NULL, (LPARAM)&m_box);
			else MsgErr(nullptr, LPGENW("An unknown error has occurred."));
		}
	}
	if (m_pszFile && *m_pszFile && m_bDeleteAfterSend && m_EnableItem&SS_DLG_DELETEAFTERSSEND) {
		DeleteFile(m_pszFile), m_pszFile = nullptr;
	}
	if (m_bAsync)
		delete this; // deletes derived class since destructor is virtual (which also auto-calls base dtor)
}

/////////////////////////////////////////////////////////////////////////////////////////
// helper functions used for HTTP uploads

#define snprintf _snprintf

const char* CSend::GetHTMLContent(char* str, const char* startTag, const char* endTag)
{
	char* begin = strstr(str, startTag);
	if (!begin) return nullptr;
	begin += mir_strlen(startTag) - 1;
	for (; *begin != '>' && *begin; ++begin);
	if (*begin) {
		char* end = strstr(++begin, endTag);
		if (end) *end = 0;
	}
	return begin;
}

int CSend::HTTPFormCreate(MHttpRequest* nlhr, const char* url, HTTPFormData* frm, size_t frmNum)
{
	char boundary[16];
	strcpy(boundary, "--M461C/");
	{
		union
		{
			uint32_t num;
			unsigned char cr[4];
		}; num = GetTickCount() ^ 0x8000;
		for (int i = 0; i < 4; ++i) {
			unsigned char chcode = cr[i] >> 4;
			boundary[8 + i * 2] = (chcode < 0x0a ? '0' : 'a' - 0x0a) + chcode;
			chcode = cr[i] & 0x0f;
			boundary[9 + i * 2] = (chcode < 0x0a ? '0' : 'a' - 0x0a) + chcode;
		}
	}

	nlhr->flags = NLHRF_HTTP11;
	if (!strncmp(url, "https://", 8))
		nlhr->flags |= NLHRF_SSL;
	nlhr->m_szUrl = url;
	nlhr->AddHeader("Content-Type", CMStringA("multipart/form-data; boundary=") + boundary);
	nlhr->AddHeader("User-Agent", __USER_AGENT_STRING);
	nlhr->AddHeader("Accept-Language", "en-us,en;q=0.8");
	for (HTTPFormData* iter = frm, *end = frm + frmNum; iter != end; ++iter) {
		if (!(iter->flags & HTTPFF_HEADER))
			break;
		nlhr->AddHeader(iter->name, iter->value_str);
	}

	auto &str = nlhr->m_szParam;
	for (HTTPFormData *iter = frm, *end = frm + frmNum; iter != end; ++iter) {
		if (iter->flags & HTTPFF_HEADER)
			continue;
		
		str.AppendFormat("--%s", boundary);
		str.Append("\r\nContent-Disposition: form-data; name=\"");

		if (iter->flags & HTTPFF_FILE) {
			const char *filename = strrchr(iter->value_str, '\\');
			if (!filename) filename = strrchr(iter->value_str, '/');
			if (!filename) filename = iter->value_str;
			else ++filename;

			str.Append(iter->name);
			str.Append("\"; filename=\"");
			str.Append(filename);
			str.Append("\"\r\nContent-Type: ");

			/// add mime type
			const char *mime = "application/octet-stream";
			const char *fileext = strrchr(filename, '.');
			if (fileext) {
				if (!mir_strcmp(fileext, ".jpg") || !mir_strcmp(fileext, ".jpeg") || !mir_strcmp(fileext, ".jpe"))
					mime = "image/jpeg";
				else if (!mir_strcmp(fileext, ".bmp"))
					mime = "image/bmp";
				else if (!mir_strcmp(fileext, ".png"))
					mime = "image/png";
				else if (!mir_strcmp(fileext, ".gif"))
					mime = "image/gif";
				else if (!mir_strcmp(fileext, ".tif") || !mir_strcmp(fileext, ".tiff"))
					mime = "image/tiff";
			}
			str.Append(mime);
			str.Append("\r\n\r\n");
			
			/// add file content
			long filesize = 0;
			FILE *fp = fopen(iter->value_str, "rb");
			if (fp) {
				fseek(fp, 0, SEEK_END);
				filesize = ftell(fp); fseek(fp, 0, SEEK_SET);
				ptrA buf((char *)mir_alloc(filesize));
				if (fread(buf, 1, filesize, fp) != filesize) {
					str.Append(buf, filesize);
					fclose(fp), fp = nullptr;
				}
				else str.Append(buf, filesize);
			}
			if (!fp) {
				Error(L"Error occurred when opening local file.\nAborting file upload...");
				Exit(ACKRESULT_FAILED);
				return 1;
			}
			fclose(fp);
			str.Append("\r\n");
		}
		else if (iter->flags & HTTPFF_8BIT) {
			str.Append(iter->name);
			str.Append("\"\r\nContent-Transfer-Encoding: 8bit\r\n\r\n");
			str.Append(iter->value_str);
			str.Append("\r\n");
		}
		else if (iter->flags & HTTPFF_INT) {
			str.Append(iter->name);
			str.Append("\"\r\n\r\n");
			str.AppendFormat("%Id", iter->value_int);
			str.Append("\r\n");
		}
		else {
			str.Append(iter->name);
			str.Append("\"\r\n\r\n");
			str.Append(iter->value_str);
			str.Append("\r\n");
		}
	}

	str.AppendFormat("--%s--\r\n", boundary);
	return 0;
}
