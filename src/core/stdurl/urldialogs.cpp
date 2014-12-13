/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "commonheaders.h"
#include "url.h"

INT_PTR CALLBACK DlgProcUrlSend(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

extern HANDLE hUrlWindowList;

static void sttUpdateTitle(HWND hwndDlg, MCONTACT hContact)
{
	TCHAR newtitle[256], oldtitle[256];
	TCHAR *szStatus, *contactName, *pszNewTitleStart = TranslateT("Send URL to");
	char  *szProto;

	if (hContact) {
		szProto = GetContactProto(hContact);
		if (szProto) {
			CONTACTINFO ci;
			int hasName = 0;
			char buf[128];
			memset(&ci, 0, sizeof(ci));

			ci.cbSize = sizeof(ci);
			ci.hContact = hContact;
			ci.szProto = szProto;
			ci.dwFlag = CNF_UNIQUEID;
			if ( !CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
				switch(ci.type) {
				case CNFT_ASCIIZ:
					hasName = 1;
					strncpy_s(buf, (char*)ci.pszVal, _TRUNCATE);
					mir_free(ci.pszVal);
					break;
				case CNFT_DWORD:
					hasName = 1;
					mir_snprintf(buf, SIZEOF(buf), "%u", ci.dVal);
					break;
			}	}

			contactName = pcli->pfnGetContactDisplayName(hContact, 0);
			if (hasName)
				SetDlgItemTextA(hwndDlg, IDC_NAME, buf);
			else
				SetDlgItemText(hwndDlg, IDC_NAME, contactName);

			szStatus = pcli->pfnGetStatusModeDescription(szProto == NULL ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), 0);
			mir_sntprintf(newtitle, SIZEOF(newtitle), _T("%s %s (%s)"), pszNewTitleStart, contactName, szStatus);
		}
	}
	else mir_tstrncpy(newtitle, pszNewTitleStart, SIZEOF(newtitle));

	GetWindowText(hwndDlg, oldtitle, SIZEOF(oldtitle));

	if (mir_tstrcmp(newtitle, oldtitle))	   //swt() flickers even if the title hasn't actually changed
		SetWindowText(hwndDlg, newtitle);
}

INT_PTR CALLBACK DlgProcUrlRecv(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct UrlRcvData *dat = (struct UrlRcvData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		Window_SetIcon_IcoLib(hwndDlg, SKINICON_EVENT_URL);
		Button_SetIcon_IcoLib(hwndDlg, IDC_ADD, SKINICON_OTHER_ADDCONTACT, LPGEN("Add contact permanently to list"));
		Button_SetIcon_IcoLib(hwndDlg, IDC_DETAILS, SKINICON_OTHER_USERDETAILS, LPGEN("View user's details"));
		Button_SetIcon_IcoLib(hwndDlg, IDC_HISTORY, SKINICON_OTHER_HISTORY, LPGEN("View user's history"));
		Button_SetIcon_IcoLib(hwndDlg, IDC_USERMENU, SKINICON_OTHER_DOWNARROW, LPGEN("User menu"));

		dat = (struct UrlRcvData*)mir_alloc(sizeof(struct UrlRcvData));
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);

		dat->hContact = ((CLISTEVENT*)lParam)->hContact;
		dat->hDbEvent = ((CLISTEVENT*)lParam)->hDbEvent;

		WindowList_Add(hUrlWindowList, hwndDlg, dat->hContact);
		{
			DBEVENTINFO dbei = { sizeof(dbei) };
			dbei.cbBlob = db_event_getBlobSize(dat->hDbEvent);
			dbei.pBlob = (PBYTE)mir_alloc(dbei.cbBlob);
			db_event_get(dat->hDbEvent, &dbei);
			SetDlgItemTextA(hwndDlg, IDC_URL, (char*)dbei.pBlob);
			SetDlgItemTextA(hwndDlg, IDC_MSG, (char*)dbei.pBlob+mir_strlen((char*)dbei.pBlob)+1);
			mir_free(dbei.pBlob);

			db_event_markRead(dat->hContact, dat->hDbEvent);

			TCHAR *contactName = pcli->pfnGetContactDisplayName(dat->hContact, 0), msg[128];
			mir_sntprintf(msg, SIZEOF(msg), TranslateT("URL from %s"), contactName);
			SetWindowText(hwndDlg, msg);
			SetDlgItemText(hwndDlg, IDC_FROM, contactName);
			SendDlgItemMessage(hwndDlg, IDOK, BUTTONSETARROW, 1, 0);

			TCHAR str[128];
			tmi.printTimeStamp(NULL, dbei.timestamp, _T("t d"), str, SIZEOF(str), 0);
			SetDlgItemText(hwndDlg, IDC_DATE, str);
		}

		// From message dlg
		if ( !db_get_b(dat->hContact, "CList", "NotOnList", 0))
			ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), SW_HIDE);

		SendMessage(hwndDlg, DM_UPDATETITLE, 0, 0);
		// From message dlg end

		Utils_RestoreWindowPositionNoSize(hwndDlg, NULL, "SRUrl", "recv");
		return TRUE;

	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->hwndItem == GetDlgItem(hwndDlg, IDC_PROTOCOL)) {
				char *szProto;

				szProto = GetContactProto(dat->hContact);
				if (szProto) {
					HICON hIcon;

					hIcon = (HICON)CallProtoService(szProto, PS_LOADICON, PLI_PROTOCOL|PLIF_SMALL, 0);
					if (hIcon) {
						DrawIconEx(dis->hDC, dis->rcItem.left, dis->rcItem.top, hIcon, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, NULL, DI_NORMAL);
						DestroyIcon(hIcon);
		}	}	}	}
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

	case DM_UPDATETITLE:
		sttUpdateTitle(hwndDlg, dat->hContact);
		break;

	case WM_COMMAND:
		if (dat)
			if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)dat->hContact))
				break;
		switch(LOWORD(wParam)) {
		case IDOK:
			{	HMENU hMenu, hSubMenu;
				RECT rc;
				char url[256];

				hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXT));
				hSubMenu = GetSubMenu(hMenu, 6);
				TranslateMenu(hSubMenu);
				GetWindowRect((HWND)lParam, &rc);
				GetDlgItemTextA(hwndDlg, IDC_URL, url, SIZEOF(url));
				switch(TrackPopupMenu(hSubMenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, hwndDlg, NULL)) {
					case IDM_OPENNEW:
						CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW, (LPARAM)url);
						break;
					case IDM_OPENEXISTING:
						CallService(MS_UTILS_OPENURL, 0, (LPARAM)url);
						break;
					case IDM_COPYLINK:
					{	HGLOBAL hData;
						if ( !OpenClipboard(hwndDlg)) break;
						EmptyClipboard();
						hData = GlobalAlloc(GMEM_MOVEABLE, mir_strlen(url)+1);
						mir_strcpy((char*)GlobalLock(hData), url);
						GlobalUnlock(hData);
						SetClipboardData(CF_TEXT, hData);
						CloseClipboard();
						break;
					}
				}
                DestroyMenu(hMenu);
			}
			return TRUE;

		case IDC_USERMENU:
			{
				RECT rc;
				HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)dat->hContact, 0);
				GetWindowRect(GetDlgItem(hwndDlg, IDC_USERMENU), &rc);
				TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, hwndDlg, NULL);
				DestroyMenu(hMenu);
			}
			break;

		case IDC_HISTORY:
			CallService(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM)dat->hContact, 0);
			break;

		case IDC_DETAILS:
			CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)dat->hContact, 0);
			break;

		case IDC_ADD:
			{
				ADDCONTACTSTRUCT acs = {0};
				acs.hContact = dat->hContact;
				acs.handleType = HANDLE_CONTACT;
				CallService(MS_ADDCONTACT_SHOW, (WPARAM)hwndDlg, (LPARAM)&acs);
			}
			if ( !db_get_b(dat->hContact, "CList", "NotOnList", 0)) {
				ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
			}
			break;

		case IDC_REPLY:
			CallService(MS_MSG_SENDMESSAGE, (WPARAM)dat->hContact, 0);
			//fall through
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return TRUE;
		}
		break;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hwndDlg);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_ADD);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_DETAILS);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_HISTORY);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_USERMENU);

		WindowList_Remove(hUrlWindowList, hwndDlg);
		mir_free(dat);
		Utils_SaveWindowPosition(hwndDlg, NULL, "SRUrl", "recv");
		break;
	}
	return FALSE;
}

static int ddeAcked, ddeData;
static ATOM hSzDdeData;
static HWND hwndDde;
static HGLOBAL hGlobalDdeData;
static LRESULT DdeMessage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UINT_PTR hSzItem;

	switch(msg) {
	case WM_DDE_ACK:
		ddeAcked = 1;
		hwndDde = (HWND)wParam;
		break;

	case WM_DDE_DATA:
		UnpackDDElParam(msg, lParam, (PUINT_PTR)&hGlobalDdeData, (PUINT_PTR)&hSzItem);
		ddeData = 1;
		if (hGlobalDdeData) {
			DDEDATA* data = (DDEDATA*)GlobalLock(hGlobalDdeData);
			if (data->fAckReq) {
				DDEACK ack = {0};
				PostMessage((HWND)wParam, WM_DDE_ACK, (WPARAM)hwndDlg, PackDDElParam(WM_DDE_ACK, *(PUINT)&ack, hSzItem));
			}
			else GlobalDeleteAtom((ATOM)hSzItem);
			GlobalUnlock(hGlobalDdeData);
		}
		else GlobalDeleteAtom((ATOM)hSzItem);
		break;
	}
	return 0;
}

static HGLOBAL DoDdeRequest(const char *szItemName, HWND hwndDlg)
{
	ATOM hSzItemName;
	DWORD timeoutTick, thisTick;
	MSG msg;

	hSzItemName = GlobalAddAtomA(szItemName);
	if ( !PostMessage(hwndDde, WM_DDE_REQUEST, (WPARAM)hwndDlg, MAKELPARAM(CF_TEXT, hSzItemName))) {
		GlobalDeleteAtom(hSzItemName);
		return NULL;
	}
	timeoutTick = GetTickCount()+5000;
	ddeData = 0; ddeAcked = 0;
	do {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (ddeData || ddeAcked) break;
		thisTick = GetTickCount();
		if (thisTick>timeoutTick) break;
	}
		while (MsgWaitForMultipleObjects(0, NULL, FALSE, timeoutTick-thisTick, QS_ALLINPUT) == WAIT_OBJECT_0);

	if ( !ddeData) {
		GlobalDeleteAtom(hSzItemName);
		return NULL;
	}

	return hGlobalDdeData;
}

static void FreeDdeRequestData(HGLOBAL hData)
{
	DDEDATA *data;
	data = (DDEDATA*)GlobalLock(hData);
	if (data->fRelease) {
		GlobalUnlock(hData);
		GlobalFree(hData);
	}
	else GlobalUnlock(hData);
}

static void AddBrowserPageToCombo(char *url, HWND hwndCombo)
{
	char *title, *frame, *end;

	if (url[0] != '"') return;
	url++;
	title = strchr(url, '"');
	if (title == NULL) return;
	*title = '\0'; title++;
	if (*title) {
		title+=2;
		frame = strchr(title, '"');
		if (frame == NULL) return;
		*frame = '\0'; frame++;
		if (*frame) {
			frame+=2;
			end = strchr(frame, '"');
			if (end == NULL) return;
			*end = '\0';
		}
		else frame = NULL;
	}
	else title = frame = NULL;
	if (frame == NULL || *frame == 0) {
		char *szItemData;
		int i;
		char szExistingUrl[1024];

		for (i = SendMessage(hwndCombo, CB_GETCOUNT, 0, 0)-1;i>=0;i--) {
			if (SendMessage(hwndCombo, CB_GETLBTEXTLEN, i, 0) >= SIZEOF(szExistingUrl))
				continue;
			SendMessageA(hwndCombo, CB_GETLBTEXT, i, (LPARAM)szExistingUrl);
			if ( !mir_strcmp(szExistingUrl, url)) return;
		}
		i = SendMessageA(hwndCombo, CB_ADDSTRING, 0, (LPARAM)url);
		szItemData = mir_strdup(title);
		SendMessage(hwndCombo, CB_SETITEMDATA, i, (LPARAM)szItemData);
	}
}

//see Q160957 and http://developer.netscape.com/docs/manuals/communicator/DDE/index.htm
static void GetOpenBrowserUrlsForBrowser(const char *szBrowser, HWND hwndDlg, HWND hwndCombo)
{
	ATOM hSzBrowser, hSzTopic;
	int windowCount, i;
	DWORD *windowId;
	DWORD dwResult;
	HGLOBAL hData;
	DDEDATA *data;
	int dataLength;

	hSzBrowser = GlobalAddAtomA(szBrowser);

	hSzTopic = GlobalAddAtomA("WWW_ListWindows");
	ddeAcked = 0;
	if ( !SendMessageTimeout(HWND_BROADCAST, WM_DDE_INITIATE, (WPARAM)hwndDlg, MAKELPARAM(hSzBrowser, hSzTopic), SMTO_ABORTIFHUNG|SMTO_NORMAL, DDEMESSAGETIMEOUT, (PDWORD_PTR)&dwResult)
	   || !ddeAcked) {
		GlobalDeleteAtom(hSzTopic);
		GlobalDeleteAtom(hSzBrowser);
		return;
	}
	hData = DoDdeRequest("WWW_ListWindows", hwndDlg);
	if (hData == NULL) {
		GlobalDeleteAtom(hSzTopic);
		GlobalDeleteAtom(hSzBrowser);
		return;
	}
	dataLength = GlobalSize(hData)-offsetof(DDEDATA, Value);
	data = (DDEDATA*)GlobalLock(hData);
	windowCount = dataLength/sizeof(DWORD);
	windowId = (PDWORD)mir_alloc(sizeof(DWORD)*windowCount);
	memcpy(windowId, data->Value, windowCount*sizeof(DWORD));
	GlobalUnlock(hData);
	FreeDdeRequestData(hData);
	PostMessage(hwndDde, WM_DDE_TERMINATE, (WPARAM)hwndDlg, 0);
	GlobalDeleteAtom(hSzTopic);

	hSzTopic = GlobalAddAtomA("WWW_GetWindowInfo");
	ddeAcked = 0;
	if ( !SendMessageTimeout(HWND_BROADCAST, WM_DDE_INITIATE, (WPARAM)hwndDlg, MAKELPARAM(hSzBrowser, hSzTopic), SMTO_ABORTIFHUNG|SMTO_NORMAL, DDEMESSAGETIMEOUT, (PDWORD_PTR)&dwResult)
	   || !ddeAcked) {
		GlobalDeleteAtom(hSzTopic);
		GlobalDeleteAtom(hSzBrowser);
		mir_free(windowId);
		return;
	}
	for (i=0;i<windowCount;i++) {
		if (windowId[i] == 0) break;
		{	char str[16];
			mir_snprintf(str, SIZEOF(str), "%d", windowId[i]);
			hData = DoDdeRequest(str, hwndDlg);
		}
		if (hData != NULL) {
			dataLength = GlobalSize(hData)-offsetof(DDEDATA, Value);
			data = (DDEDATA*)GlobalLock(hData);
			AddBrowserPageToCombo((char*)data->Value, hwndCombo);
			GlobalUnlock(hData);
			FreeDdeRequestData(hData);
		}
	}
	PostMessage(hwndDde, WM_DDE_TERMINATE, (WPARAM)hwndDlg, 0);
	GlobalDeleteAtom(hSzTopic);
	GlobalDeleteAtom(hSzBrowser);
	mir_free(windowId);
}

static void GetOpenBrowserUrls(HWND hwndDlg, HWND hwndCombo)
{
	GetOpenBrowserUrlsForBrowser("opera", hwndDlg, hwndCombo);
	GetOpenBrowserUrlsForBrowser("netscape", hwndDlg, hwndCombo);
	GetOpenBrowserUrlsForBrowser("iexplore", hwndDlg, hwndCombo);
}

static LRESULT CALLBACK SendEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_CHAR:
		if (wParam == '\n' && GetKeyState(VK_CONTROL)&0x8000) {
			PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
			return 0;
		}
		break;
	case WM_SYSCHAR:
		if ((wParam == 's' || wParam == 'S') && GetKeyState(VK_MENU)&0x8000) {
			PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
			return 0;
		}
		break;
	}
	return mir_callNextSubclass(hwnd, SendEditSubclassProc, msg, wParam, lParam);
}

INT_PTR CALLBACK DlgProcUrlSend(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct UrlSendData* dat = (struct UrlSendData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		Window_SetIcon_IcoLib(hwndDlg, SKINICON_EVENT_URL);
		Button_SetIcon_IcoLib(hwndDlg, IDC_ADD, SKINICON_OTHER_ADDCONTACT, LPGEN("Add contact permanently to list"));
		Button_SetIcon_IcoLib(hwndDlg, IDC_DETAILS, SKINICON_OTHER_USERDETAILS, LPGEN("View user's details"));
		Button_SetIcon_IcoLib(hwndDlg, IDC_HISTORY, SKINICON_OTHER_HISTORY, LPGEN("View user's history"));
		Button_SetIcon_IcoLib(hwndDlg, IDC_USERMENU, SKINICON_OTHER_DOWNARROW, LPGEN("User menu"));

		SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_LIMITTEXT, 450, 0);
		dat = (struct UrlSendData*)mir_alloc(sizeof(struct UrlSendData));
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
		dat->hContact = lParam;
		dat->hAckEvent = NULL;
		dat->hSendId = NULL;
		dat->sendBuffer = NULL;

		WindowList_Add(hUrlWindowList, hwndDlg, dat->hContact);
		{
			TCHAR *str = pcli->pfnGetContactDisplayName(dat->hContact, 0);
			SetDlgItemText(hwndDlg, IDC_NAME, str);
		}

		GetOpenBrowserUrls(hwndDlg, GetDlgItem(hwndDlg, IDC_URLS));
		SendDlgItemMessage(hwndDlg, IDC_URLS, CB_SETCURSEL, 0, 0);
		if (SendDlgItemMessage(hwndDlg, IDC_URLS, CB_GETCOUNT, 0, 0))SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_URLS, CBN_SELCHANGE), 0);
		EnableWindow(GetDlgItem(hwndDlg, IDOK), (SendDlgItemMessage(hwndDlg, IDC_URLS, CB_GETCURSEL, 0, 0) == CB_ERR)?FALSE:TRUE);
		Utils_RestoreWindowPositionNoSize(hwndDlg, NULL, "SRUrl", "send");
		mir_subclassWindow( GetDlgItem(hwndDlg, IDC_MESSAGE), SendEditSubclassProc);
		mir_subclassWindow( GetWindow(GetDlgItem(hwndDlg, IDC_URLS), GW_CHILD), SendEditSubclassProc);

		// From message dlg
		if ( !db_get_b(dat->hContact, "CList", "NotOnList", 0))
			ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), SW_HIDE);

		SendMessage(hwndDlg, DM_UPDATETITLE, 0, 0);
		// From message dlg end
		return TRUE;

	case WM_DDE_DATA:
	case WM_DDE_ACK:
		return DdeMessage(hwndDlg, msg, wParam, lParam);

	case WM_TIMER:
		if (wParam == 0) {
			//ICQ sendurl timed out
			KillTimer(hwndDlg, 0);
			MessageBox(hwndDlg, TranslateT("Send timed out"), _T(""), MB_OK);
			EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_URLS), TRUE);
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETREADONLY, FALSE, 0);
		}
		break;

	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->hwndItem == GetDlgItem(hwndDlg, IDC_PROTOCOL)) {
				char *szProto = GetContactProto(dat->hContact);
				if (szProto) {
					HICON hIcon = (HICON)CallProtoService(szProto, PS_LOADICON, PLI_PROTOCOL|PLIF_SMALL, 0);
					if (hIcon) {
						DrawIconEx(dis->hDC, dis->rcItem.left, dis->rcItem.top, hIcon, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, NULL, DI_NORMAL);
						DestroyIcon(hIcon);
		}	}	}	}
		return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);

	case DM_UPDATETITLE:
		sttUpdateTitle(hwndDlg, dat->hContact);
		break;

	case WM_COMMAND:
		if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)dat->hContact))
			break;

		switch (LOWORD(wParam)) {
		case IDOK:
			{
				char *body, *url;
				int bodySize, urlSize;

				urlSize = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_URLS))+1;
				url = (char*)mir_alloc(urlSize);
				GetDlgItemTextA(hwndDlg, IDC_URLS, url, urlSize);
				if (url[0] == 0) {
					mir_free(url);
					break;
				}
				bodySize = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE))+1;
				body = (char*)mir_alloc(bodySize);
				GetDlgItemTextA(hwndDlg, IDC_MESSAGE, body, bodySize);

				dat->sendBuffer = (char*)mir_realloc(dat->sendBuffer, mir_strlen(url)+mir_strlen(body)+2);
				mir_strcpy(dat->sendBuffer, url);
				mir_strcpy(dat->sendBuffer+mir_strlen(url)+1, body);
				dat->hAckEvent = HookEventMessage(ME_PROTO_ACK, hwndDlg, HM_EVENTSENT);
				dat->hSendId = (HANDLE)CallContactService(dat->hContact, PSS_URL, 0, (LPARAM)dat->sendBuffer);
				mir_free(url);
				mir_free(body);

				//create a timeout timer
				SetTimer(hwndDlg, 0, TIMEOUT_URLSEND, NULL);
				EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_URLS), FALSE);
				SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETREADONLY, TRUE, 0);

				return TRUE;
			}

		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return TRUE;

		case IDC_URLS:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				int i, urlSize;
				char *title;
				i = SendDlgItemMessage(hwndDlg, IDC_URLS, CB_GETCURSEL, 0, 0);
				title = (char*)SendDlgItemMessage(hwndDlg, IDC_URLS, CB_GETITEMDATA, (WPARAM)i, 0);
				SetDlgItemTextA(hwndDlg, IDC_MESSAGE, title);
				urlSize = SendDlgItemMessage(hwndDlg, IDC_URLS, CB_GETLBTEXTLEN, (WPARAM)i, 0);
				EnableWindow(GetDlgItem(hwndDlg, IDOK), (urlSize>0));
			}
			else if (HIWORD(wParam) == CBN_EDITCHANGE) {
				int urlSize = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_URLS));
				EnableWindow(GetDlgItem(hwndDlg, IDOK), (urlSize>0));
			}
			break;
		case IDC_USERMENU:
			{
				RECT rc;
				HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)dat->hContact, 0);
				GetWindowRect(GetDlgItem(hwndDlg, IDC_USERMENU), &rc);
				TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, hwndDlg, NULL);
				DestroyMenu(hMenu);
			}
			break;

		case IDC_HISTORY:
			CallService(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM)dat->hContact, 0);
			break;

		case IDC_DETAILS:
			CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)dat->hContact, 0);
			break;

		case IDC_ADD:
			ADDCONTACTSTRUCT acs = {0};
			acs.hContact = dat->hContact;
			acs.handleType = HANDLE_CONTACT;
			acs.szProto = 0;
			CallService(MS_ADDCONTACT_SHOW, (WPARAM)hwndDlg, (LPARAM)&acs);

			if ( !db_get_b(dat->hContact, "CList", "NotOnList", 0))
				ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
		}
		break;

	case HM_EVENTSENT:
		{
			ACKDATA *ack = (ACKDATA*)lParam;
			if (ack->hProcess != dat->hSendId) break;
			if (ack->hContact != dat->hContact) break;
			if (ack->type != ACKTYPE_URL || ack->result != ACKRESULT_SUCCESS) break;

			DBEVENTINFO dbei = { sizeof(dbei) };
			dbei.eventType = EVENTTYPE_URL;
			dbei.flags = DBEF_SENT;
			dbei.szModule = GetContactProto(dat->hContact);
			dbei.timestamp = time(NULL);
			dbei.cbBlob = (DWORD)(strlen(dat->sendBuffer)+strlen(dat->sendBuffer+strlen(dat->sendBuffer)+1)+2);
			dbei.pBlob = (PBYTE)dat->sendBuffer;
			db_event_add(dat->hContact, &dbei);
			KillTimer(hwndDlg, 0);
			DestroyWindow(hwndDlg);
		}
		break;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hwndDlg);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_ADD);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_DETAILS);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_HISTORY);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_USERMENU);

		WindowList_Remove(hUrlWindowList, hwndDlg);
		if (dat->hAckEvent) UnhookEvent(dat->hAckEvent);
		if (dat->sendBuffer != NULL) mir_free(dat->sendBuffer);
		mir_free(dat);
		Utils_SaveWindowPosition(hwndDlg, NULL, "SRUrl", "send");

		for (int i = SendDlgItemMessage(hwndDlg, IDC_URLS, CB_GETCOUNT, 0, 0)-1;i>=0;i--)
			mir_free((char*)SendDlgItemMessage(hwndDlg, IDC_URLS, CB_GETITEMDATA, i, 0));
		break;
	}

	return FALSE;
}
