/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
Copyright (c) 2000-10 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

Portions of this code modified for Simple Status Message plugin
Copyright (C) 2006-2011 Bartosz 'Dezeath' Bia³ek, (C) 2005 Harven

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "stdafx.h"

static HGENMENU hAwayMsgMenuItem, hCopyMsgMenuItem, hGoToURLMenuItem;
static MWindowList hWindowList, hWindowList2;

static wchar_t *StrNormNewline(wchar_t *tszStr)
{
	if (tszStr == NULL)
		return NULL;

	int nCR = 0;
	for (int i = 0; tszStr[i]; i++)
		if (tszStr[i] != 0x0D && tszStr[i + 1] == 0x0A)
			nCR++;

	if (!nCR)
		return mir_wstrdup(tszStr);

	wchar_t *tszNewStr = (wchar_t *)mir_alloc((mir_wstrlen(tszStr) + nCR + 1) * sizeof(wchar_t)), *ptszStr = tszNewStr;
	while (*tszStr) {
		if (*tszStr == 0x0A)
			*ptszStr++ = 0x0D;
		*ptszStr++ = *tszStr++;
	}
	*ptszStr++ = 0;

	return tszNewStr;
}


struct AwayMsgDlgData
{
	MCONTACT hContact;
	HANDLE hSeq;
	HANDLE hAwayMsgEvent;
};

#define HM_AWAYMSG  (WM_USER + 10)

static INT_PTR CALLBACK ReadAwayMsgDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	AwayMsgDlgData *dat = (AwayMsgDlgData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		dat = (AwayMsgDlgData *)mir_alloc(sizeof(AwayMsgDlgData));
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);

		dat->hContact = lParam;
		dat->hSeq = (HANDLE)ProtoChainSend(dat->hContact, PSS_GETAWAYMSG, 0, 0);
		dat->hAwayMsgEvent = dat->hSeq ? HookEventMessage(ME_PROTO_ACK, hwndDlg, HM_AWAYMSG) : NULL;
		WindowList_Add(hWindowList, hwndDlg, dat->hContact);
		{
			wchar_t str[256], format[128];
			wchar_t *contactName = (wchar_t *)pcli->pfnGetContactDisplayName(dat->hContact, 0);
			char *szProto = GetContactProto(dat->hContact);
			WORD dwStatus = db_get_w(dat->hContact, szProto, "Status", ID_STATUS_OFFLINE);
			wchar_t *status = pcli->pfnGetStatusModeDescription(dwStatus, 0);

			GetWindowText(hwndDlg, format, _countof(format));
			mir_snwprintf(str, format, status, contactName);
			SetWindowText(hwndDlg, str);
			if (dat->hSeq) {
				GetDlgItemText(hwndDlg, IDC_RETRIEVING, format, _countof(format));
				mir_snwprintf(str, format, status);
			}
			else {
				mir_snwprintf(str, TranslateT("Failed to retrieve %s message."), status);
				SetDlgItemText(hwndDlg, IDOK, TranslateT("&Close"));
			}
			SetDlgItemText(hwndDlg, IDC_RETRIEVING, str);
			Window_SetProtoIcon_IcoLib(hwndDlg, szProto, dwStatus);
			EnableWindow(GetDlgItem(hwndDlg, IDC_COPY), FALSE);
		}
		Utils_RestoreWindowPosition(hwndDlg, lParam, "SRAway", "AwayMsgDlg");
		return TRUE;

	case HM_AWAYMSG:
		{
			ACKDATA *ack = (ACKDATA *)lParam;
			if (ack->hContact != dat->hContact || ack->type != ACKTYPE_AWAYMSG)
				break;
			if (ack->result != ACKRESULT_SUCCESS)
				break;
			if (dat->hAwayMsgEvent && ack->hProcess == dat->hSeq) {
				UnhookEvent(dat->hAwayMsgEvent);
				dat->hAwayMsgEvent = NULL;
			}

			wchar_t *tszMsg = StrNormNewline((wchar_t *)ack->lParam);
			SetDlgItemText(hwndDlg, IDC_MSG, tszMsg);
			mir_free(tszMsg);

			if (ack->lParam && *((char *)ack->lParam) != '\0')
				EnableWindow(GetDlgItem(hwndDlg, IDC_COPY), TRUE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_RETRIEVING), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_MSG), SW_SHOW);
			SetDlgItemText(hwndDlg, IDOK, TranslateT("&Close"));
			break;
		}

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
		case IDOK:
			DestroyWindow(hwndDlg);
			break;

		case IDC_COPY:
			if (!OpenClipboard(hwndDlg))
				break;
			if (EmptyClipboard()) {
				wchar_t msg[1024];
				int len = GetDlgItemText(hwndDlg, IDC_MSG, msg, _countof(msg));
				if (len) {
					LPTSTR lptstrCopy;
					HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(wchar_t));
					if (hglbCopy == NULL) {
						CloseClipboard();
						break;
					}
					lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
					memcpy(lptstrCopy, msg, len * sizeof(wchar_t));
					lptstrCopy[len] = (wchar_t)0;
					GlobalUnlock(hglbCopy);

					SetClipboardData(CF_UNICODETEXT, hglbCopy);

				}
			}
			CloseClipboard();
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		if (dat->hAwayMsgEvent)
			UnhookEvent(dat->hAwayMsgEvent);
		Utils_SaveWindowPosition(hwndDlg, dat->hContact, "SRAway", "AwayMsgDlg");
		WindowList_Remove(hWindowList, hwndDlg);
		Window_FreeIcon_IcoLib(hwndDlg);
		mir_free(dat);
		break;
	}
	return FALSE;
}

static INT_PTR GetMessageCommand(WPARAM wParam, LPARAM)
{
	if (HWND hwnd = WindowList_Find(hWindowList, wParam)) {
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}
	else CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_READAWAYMSG), NULL, ReadAwayMsgDlgProc, wParam);
	return 0;
}

static INT_PTR CALLBACK CopyAwayMsgDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	AwayMsgDlgData *dat = (AwayMsgDlgData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			dat = (AwayMsgDlgData *)mir_alloc(sizeof(AwayMsgDlgData));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);

			dat->hContact = lParam;
			dat->hSeq = (HANDLE)ProtoChainSend(dat->hContact, PSS_GETAWAYMSG, 0, 0);
			dat->hAwayMsgEvent = dat->hSeq ? HookEventMessage(ME_PROTO_ACK, hwndDlg, HM_AWAYMSG) : NULL;
			WindowList_Add(hWindowList2, hwndDlg, dat->hContact);
			wchar_t *contactName = pcli->pfnGetContactDisplayName(dat->hContact, 0);
			wchar_t str[256], format[128];
			GetWindowText(hwndDlg, format, _countof(format));
			mir_snwprintf(str, format, contactName);
			SetWindowText(hwndDlg, str);
			if (!dat->hSeq)
				DestroyWindow(hwndDlg);
		}
		return TRUE;

	case HM_AWAYMSG:
		{
			ACKDATA *ack = (ACKDATA *)lParam;
			if (ack->hContact != dat->hContact || ack->type != ACKTYPE_AWAYMSG) {
				DestroyWindow(hwndDlg);
				break;
			}
			if (ack->result != ACKRESULT_SUCCESS) {
				DestroyWindow(hwndDlg);
				break;
			}
			if (dat->hAwayMsgEvent && ack->hProcess == dat->hSeq) {
				UnhookEvent(dat->hAwayMsgEvent);
				dat->hAwayMsgEvent = NULL;
			}

			if (!OpenClipboard(hwndDlg)) {
				DestroyWindow(hwndDlg);
				break;
			}
			if (EmptyClipboard()) {
				wchar_t msg[1024];
				wchar_t *tszMsg = StrNormNewline((wchar_t *)ack->lParam);
				wcsncpy_s(msg, tszMsg, _TRUNCATE);
				mir_free(tszMsg);
				size_t len = mir_wstrlen(msg);
				if (len) {
					LPTSTR lptstrCopy;
					HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(wchar_t));
					if (hglbCopy == NULL) {
						CloseClipboard();
						DestroyWindow(hwndDlg);
						break;
					}
					lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
					memcpy(lptstrCopy, msg, len * sizeof(wchar_t));
					lptstrCopy[len] = (wchar_t)0;
					GlobalUnlock(hglbCopy);

					SetClipboardData(CF_UNICODETEXT, hglbCopy);
				}
			}
			CloseClipboard();
			DestroyWindow(hwndDlg);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
		case IDOK:
			DestroyWindow(hwndDlg);
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		if (dat->hAwayMsgEvent) UnhookEvent(dat->hAwayMsgEvent);
		WindowList_Remove(hWindowList2, hwndDlg);
		mir_free(dat);
		break;
	}
	return FALSE;
}

static INT_PTR CopyAwayMsgCommand(WPARAM wParam, LPARAM)
{
	if (HWND hwnd = WindowList_Find(hWindowList2, wParam)) {
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}
	else
		CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_COPY), NULL, CopyAwayMsgDlgProc, wParam);
	return 0;
}

static char *StrFindURL(char *pszStr)
{
	char *pszURL = NULL;

	if (pszStr != NULL && *pszStr != '\0') {
		pszURL = strstr(pszStr, "www.");
		if (pszURL == NULL)
			pszURL = strstr(pszStr, "http://");
		if (pszURL == NULL)
			pszURL = strstr(pszStr, "https://");
		if (pszURL == NULL)
			pszURL = strstr(pszStr, "ftp://");
	}

	return pszURL;
}

static INT_PTR GoToURLMsgCommand(WPARAM wParam, LPARAM)
{
	ptrA szMsg(db_get_sa(wParam, "CList", "StatusMsg"));

	char *szURL = StrFindURL(szMsg);
	if (szURL != NULL) {
		int i;
		for (i = 0; szURL[i] != ' ' && szURL[i] != '\n' && szURL[i] != '\r' && szURL[i] != '\t' && szURL[i] != '\0'; i++);

		char *szMsgURL = (char *)mir_alloc(i + 1);
		if (szMsgURL) {
			mir_strncpy(szMsgURL, szURL, i + 1);
			Utils_OpenUrl(szMsgURL);
			mir_free(szMsgURL);
		}
	}

	return 0;
}

static int AwayMsgPreBuildMenu(WPARAM hContact, LPARAM)
{
	Menu_ShowItem(hCopyMsgMenuItem, false);
	Menu_ShowItem(hGoToURLMenuItem, false);
	Menu_ShowItem(hAwayMsgMenuItem, false);

	char *szProto = GetContactProto(hContact);
	if (szProto == NULL || db_get_b(hContact, szProto, "ChatRoom", 0))
		return 0;

	wchar_t str[128];
	int iStatus = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
	if (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGRECV) {
		if (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(iStatus == ID_STATUS_OFFLINE ? ID_STATUS_INVISIBLE : iStatus)) {
			HICON hIcon = Skin_LoadProtoIcon(szProto, iStatus);
			mir_snwprintf(str, TranslateT("Re&ad %s message"), pcli->pfnGetStatusModeDescription(iStatus, 0));
			Menu_ModifyItem(hAwayMsgMenuItem, str, hIcon, 0);
			IcoLib_ReleaseIcon(hIcon);

			ptrA szMsg(db_get_sa(hContact, "CList", "StatusMsg"));
			if (szMsg != NULL) {
				mir_snwprintf(str, TranslateT("Copy %s message"), pcli->pfnGetStatusModeDescription(iStatus, 0));
				Menu_ModifyItem(hCopyMsgMenuItem, str);

				if (StrFindURL(szMsg) != NULL) {
					mir_snwprintf(str, TranslateT("&Go to URL in %s message"), pcli->pfnGetStatusModeDescription(iStatus, 0));
					Menu_ModifyItem(hGoToURLMenuItem, str);
				}
			}
		}
	}

	return 0;
}

int AwayMsgPreShutdown(void)
{
	if (hWindowList) {
		WindowList_Broadcast(hWindowList, WM_CLOSE, 0, 0);
		WindowList_Destroy(hWindowList);
	}
	if (hWindowList2) {
		WindowList_Broadcast(hWindowList2, WM_CLOSE, 0, 0);
		WindowList_Destroy(hWindowList2);
	}
	return 0;
}

int LoadAwayMsgModule(void)
{
	hWindowList = WindowList_Create();
	hWindowList2 = WindowList_Create();

	CMenuItem mi;
	mi.flags = CMIF_UNICODE;

	SET_UID(mi, 0xd3282acc, 0x9ff1, 0x4ede, 0x8a, 0x1e, 0x36, 0x72, 0x3f, 0x44, 0x4f, 0x84);
	CreateServiceFunction(MS_AWAYMSG_SHOWAWAYMSG, GetMessageCommand);
	mi.position = -2000005000;
	mi.name.w = LPGENW("Re&ad Away message");
	mi.pszService = MS_AWAYMSG_SHOWAWAYMSG;
	hAwayMsgMenuItem = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x311124e9, 0xb477, 0x42ef, 0x84, 0xd2, 0xc, 0x6c, 0x50, 0x3f, 0x4a, 0x84);
	CreateServiceFunction(MS_SIMPLESTATUSMSG_COPYMSG, CopyAwayMsgCommand);
	mi.position = -2000006000;
	mi.hIcolibItem = GetIconHandle(IDI_COPY);
	mi.name.w = LPGENW("Copy Away message");
	mi.pszService = MS_SIMPLESTATUSMSG_COPYMSG;
	hCopyMsgMenuItem = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0xe2c75070, 0x455d, 0x455f, 0xbf, 0x53, 0x86, 0x64, 0xbc, 0x14, 0xa1, 0xbe);
	CreateServiceFunction(MS_SIMPLESTATUSMSG_GOTOURLMSG, GoToURLMsgCommand);
	mi.position = -2000007000;
	mi.hIcolibItem = GetIconHandle(IDI_GOTOURL);
	mi.name.w = LPGENW("&Go to URL in Away message");
	mi.pszService = MS_SIMPLESTATUSMSG_GOTOURLMSG;
	hGoToURLMenuItem = Menu_AddContactMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, AwayMsgPreBuildMenu);
	return 0;
}
