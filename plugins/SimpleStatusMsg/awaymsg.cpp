/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2010 Miranda ICQ/IM project,
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
#include "commonheaders.h"
#include "simplestatusmsg.h"

static HANDLE hAwayMsgMenuItem;
static HANDLE hCopyMsgMenuItem;
static HANDLE hGoToURLMenuItem;
static HANDLE hWindowList;
static HANDLE hWindowList2;

static char *StrNormNewlineA(char *szStr)
{
	if (szStr == NULL) return NULL;

	int nCR = 0;
	for (int i = 0; szStr[i]; i++)
		if (szStr[i] != 0x0D && szStr[i + 1] == 0x0A) nCR++;

	if (!nCR) return mir_strdup(szStr);

	char *szNewStr = (char*)mir_alloc(lstrlenA(szStr) + nCR + 1), *pszStr = szNewStr;
	while (*szStr)
	{
		if (*szStr == 0x0A)
			*pszStr++ = 0x0D;
		*pszStr++ = *szStr++;
	}
	*pszStr++ = 0;

	return szNewStr;
}

#ifdef _UNICODE
static TCHAR *StrNormNewline(TCHAR *tszStr)
{
	if (tszStr == NULL) return NULL;

	int nCR = 0;
	for (int i = 0; tszStr[i]; i++)
		if (tszStr[i] != 0x0D && tszStr[i + 1] == 0x0A) nCR++;

	if (!nCR) return mir_tstrdup(tszStr);

	TCHAR *tszNewStr = (TCHAR*)mir_alloc((lstrlen(tszStr) + nCR + 1) * sizeof(TCHAR)), *ptszStr = tszNewStr;
	while (*tszStr)
	{
		if (*tszStr == 0x0A)
			*ptszStr++ = 0x0D;
		*ptszStr++ = *tszStr++;
	}
	*ptszStr++ = 0;

	return tszNewStr;
}
#endif

struct AwayMsgDlgData
{
	HANDLE hContact;
	HANDLE hSeq;
	HANDLE hAwayMsgEvent;
};

#define HM_AWAYMSG  (WM_USER + 10)

static INT_PTR CALLBACK ReadAwayMsgDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	AwayMsgDlgData *dat = (AwayMsgDlgData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (message)
	{
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			dat = (AwayMsgDlgData*)mir_alloc(sizeof(AwayMsgDlgData));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);

			dat->hContact = (HANDLE)lParam;
			dat->hSeq = (HANDLE)CallContactService(dat->hContact, PSS_GETAWAYMSG, 0, 0);
			dat->hAwayMsgEvent = dat->hSeq ? HookEventMessage(ME_PROTO_ACK, hwndDlg, HM_AWAYMSG) : NULL;
			WindowList_Add(hWindowList, hwndDlg, dat->hContact);
			{
				TCHAR str[256], format[128];
				TCHAR *contactName = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)dat->hContact, GCDNF_TCHAR);
				char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)dat->hContact, 0);
				WORD dwStatus = DBGetContactSettingWord(dat->hContact, szProto, "Status", ID_STATUS_OFFLINE);
				TCHAR *status = (TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, dwStatus, GSMDF_TCHAR);

				GetWindowText(hwndDlg, format, SIZEOF(format));
				mir_sntprintf(str, SIZEOF(str), format, status, contactName);
				SetWindowText(hwndDlg, str);
				if (dat->hSeq)
				{
					GetDlgItemText(hwndDlg, IDC_RETRIEVING, format, SIZEOF(format));
					mir_sntprintf(str, SIZEOF(str), format, status);
				}
				else
				{
					mir_sntprintf(str, SIZEOF(str), TranslateT("Failed to retrieve %s message."), status);
					SetDlgItemText(hwndDlg, IDOK, TranslateT("&Close"));
				}
				SetDlgItemText(hwndDlg, IDC_RETRIEVING, str);
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedProtoIcon(szProto, dwStatus));
				SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadSkinnedProtoIcon(szProto, dwStatus));
				EnableWindow(GetDlgItem(hwndDlg, IDC_COPY), FALSE);
			}
			Utils_RestoreWindowPosition(hwndDlg, (HANDLE)lParam, "SRAway", "AwayMsgDlg");
			return TRUE;

		case HM_AWAYMSG:
		{
			ACKDATA *ack = (ACKDATA*)lParam;
			if (ack->hContact != dat->hContact || ack->type != ACKTYPE_AWAYMSG) break;
			if (ack->result != ACKRESULT_SUCCESS) break;
			if (dat->hAwayMsgEvent && ack->hProcess == dat->hSeq) { UnhookEvent(dat->hAwayMsgEvent); dat->hAwayMsgEvent = NULL; }

#ifdef _UNICODE
			DBVARIANT dbv;
			bool unicode = !DBGetContactSetting(dat->hContact, "CList", "StatusMsg", &dbv) && 
				(dbv.type == DBVT_UTF8 || dbv.type == DBVT_WCHAR);
			DBFreeVariant(&dbv);
			if (unicode)
			{
				DBGetContactSettingWString(dat->hContact, "CList", "StatusMsg", &dbv);
				TCHAR *tszMsg = StrNormNewline(dbv.pwszVal);
				SetDlgItemText(hwndDlg, IDC_MSG, tszMsg);
				mir_free(tszMsg);
				DBFreeVariant(&dbv);
			}
			else
#endif
			{
				char *szMsg = StrNormNewlineA((char *)ack->lParam);
				SetDlgItemTextA(hwndDlg, IDC_MSG, szMsg);
				mir_free(szMsg);
			}

			if (ack->lParam && *((char*)ack->lParam) != '\0') EnableWindow(GetDlgItem(hwndDlg, IDC_COPY), TRUE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_RETRIEVING), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_MSG), SW_SHOW);
			SetDlgItemText(hwndDlg, IDOK, TranslateT("&Close"));
			break;
		}

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
				case IDOK:
					DestroyWindow(hwndDlg);
					break;

				case IDC_COPY:
					if (!OpenClipboard(hwndDlg)) break;
					if (EmptyClipboard())
					{
						TCHAR msg[1024];
						int len = GetDlgItemText(hwndDlg, IDC_MSG, msg, SIZEOF(msg));
						if (len)
						{
							LPTSTR  lptstrCopy;
							HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(TCHAR));
							if (hglbCopy == NULL)
							{
								CloseClipboard();
								break;
							}
							lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
							memcpy(lptstrCopy, msg, len * sizeof(TCHAR));
							lptstrCopy[len] = (TCHAR)0;
							GlobalUnlock(hglbCopy);
#ifdef _UNICODE
							SetClipboardData(CF_UNICODETEXT, hglbCopy);
#else
							SetClipboardData(CF_TEXT, hglbCopy);
#endif
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
			if (dat->hAwayMsgEvent) UnhookEvent(dat->hAwayMsgEvent);
			Utils_SaveWindowPosition(hwndDlg, dat->hContact, "SRAway", "AwayMsgDlg");
			WindowList_Remove(hWindowList, hwndDlg);
			CallService(MS_SKIN2_RELEASEICON, (WPARAM)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)NULL), 0);
			CallService(MS_SKIN2_RELEASEICON, (WPARAM)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)NULL), 0);
			mir_free(dat);
			break;
	}
	return FALSE;
}

static INT_PTR GetMessageCommand(WPARAM wParam, LPARAM)
{
	if (HWND hwnd = WindowList_Find(hWindowList, (HANDLE)wParam))
	{
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}
	else CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_READAWAYMSG), NULL, ReadAwayMsgDlgProc, wParam);
	return 0;
}

static INT_PTR CALLBACK CopyAwayMsgDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	AwayMsgDlgData *dat = (AwayMsgDlgData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (message)
	{
		case WM_INITDIALOG:
		{
			TCHAR str[256], format[128];
			TCHAR *contactName;

			TranslateDialogDefault(hwndDlg);
			dat = (AwayMsgDlgData*)mir_alloc(sizeof(AwayMsgDlgData));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);

			dat->hContact = (HANDLE)lParam;
			dat->hSeq = (HANDLE)CallContactService(dat->hContact, PSS_GETAWAYMSG, 0, 0);
			dat->hAwayMsgEvent = dat->hSeq ? HookEventMessage(ME_PROTO_ACK, hwndDlg, HM_AWAYMSG) : NULL;
			WindowList_Add(hWindowList2, hwndDlg, dat->hContact);
			contactName = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)dat->hContact, GCDNF_TCHAR);
			GetWindowText(hwndDlg, format, SIZEOF(format));
			mir_sntprintf(str, SIZEOF(str), format, contactName);
			SetWindowText(hwndDlg, str);
			if (!dat->hSeq) DestroyWindow(hwndDlg);
			return TRUE;
		}

		case HM_AWAYMSG:
		{	
			ACKDATA *ack = (ACKDATA*)lParam;
			if (ack->hContact != dat->hContact || ack->type != ACKTYPE_AWAYMSG) { DestroyWindow(hwndDlg); break; }
			if (ack->result != ACKRESULT_SUCCESS) { DestroyWindow(hwndDlg); break; }
			if (dat->hAwayMsgEvent && ack->hProcess == dat->hSeq) { UnhookEvent(dat->hAwayMsgEvent); dat->hAwayMsgEvent = NULL; }

			if (!OpenClipboard(hwndDlg)) { DestroyWindow(hwndDlg); break; }
			if (EmptyClipboard())
			{
				TCHAR msg[1024];
				int len;
#ifdef _UNICODE
				DBVARIANT dbv;
				bool unicode = !DBGetContactSetting(dat->hContact, "CList", "StatusMsg", &dbv) && 
					(dbv.type == DBVT_UTF8 || dbv.type == DBVT_WCHAR);
				DBFreeVariant(&dbv);
				if (unicode)
				{
					DBGetContactSettingWString(dat->hContact, "CList", "StatusMsg", &dbv);
					TCHAR *tszMsg = StrNormNewline(dbv.pwszVal);
					mir_sntprintf(msg, SIZEOF(msg), _T("%s"), tszMsg);
					mir_free(tszMsg);
					DBFreeVariant(&dbv);
				}
				else
#endif
				{
					char *szMsg = StrNormNewlineA((char *)ack->lParam);
					mir_sntprintf(msg, SIZEOF(msg), _T("%hs"), szMsg);
					mir_free(szMsg);
				}
				len = lstrlen(msg);

				if (len)
				{
					LPTSTR  lptstrCopy;
					HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(TCHAR));
					if (hglbCopy == NULL)
					{
						CloseClipboard();
						DestroyWindow(hwndDlg);
						break;
					}
					lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
					memcpy(lptstrCopy, msg, len * sizeof(TCHAR));
					lptstrCopy[len] = (TCHAR)0;
					GlobalUnlock(hglbCopy);
#ifdef _UNICODE
					SetClipboardData(CF_UNICODETEXT, hglbCopy);
#else
					SetClipboardData(CF_TEXT, hglbCopy);
#endif
				}
			}
			CloseClipboard();
			DestroyWindow(hwndDlg);
			break;
		}

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
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
	if (HWND hwnd = WindowList_Find(hWindowList2, (HANDLE)wParam))
	{
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}
	else CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_COPY), NULL, CopyAwayMsgDlgProc, wParam);
	return 0;
}

static char *StrFindURL(char *pszStr)
{
	char *pszURL = NULL;

	if (pszStr != NULL && *pszStr != '\0')
	{
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

static INT_PTR GoToURLMsgCommand(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;
	char *szMsg;

#ifdef _UNICODE
	int unicode = !DBGetContactSetting((HANDLE)wParam, "CList", "StatusMsg", &dbv) && (dbv.type == DBVT_UTF8 || dbv.type == DBVT_WCHAR);
	DBFreeVariant(&dbv);
	if (unicode)
	{
		DBGetContactSettingWString((HANDLE)wParam, "CList", "StatusMsg", &dbv);
		szMsg = mir_u2a(dbv.pwszVal);
	}
	else
#endif
	{
		DBGetContactSettingString((HANDLE)wParam, "CList", "StatusMsg", &dbv);
		szMsg = mir_strdup(dbv.pszVal);
	}
	DBFreeVariant(&dbv);

	char *szURL = StrFindURL(szMsg);
	if (szURL != NULL)
	{
		int i;
		for (i = 0; szURL[i] != ' ' && szURL[i] != '\n' && szURL[i] != '\r' &&
			szURL[i] != '\t' && szURL[i] != '\0'; i++);

		char *szMsgURL = (char *)mir_alloc(i + 1);
		if (szMsgURL)
		{
			lstrcpynA(szMsgURL, szURL, i + 1);
			CallService(MS_UTILS_OPENURL, (WPARAM)1, (LPARAM)szMsgURL);
			mir_free(szMsgURL);
		}
	}
	mir_free(szMsg);

	return 0;
}

static int AwayMsgPreBuildMenu(WPARAM wParam, LPARAM lParam)
{
	CLISTMENUITEM clmi = {0};
	TCHAR str[128];
	char *szProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
	int iHidden = szProto ? DBGetContactSettingByte((HANDLE)wParam, szProto, "ChatRoom", 0) : 0;
	char *szMsg;
	int iStatus;

	clmi.cbSize = sizeof(clmi);
	clmi.flags = CMIM_FLAGS | CMIF_HIDDEN | CMIF_TCHAR;

	if (!iHidden)
	{
		iHidden = 1;
		iStatus = DBGetContactSettingWord((HANDLE)wParam, szProto, "Status", ID_STATUS_OFFLINE);
		if (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1,0) & PF1_MODEMSGRECV)
		{
			if (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3,0) & Proto_Status2Flag(iStatus == ID_STATUS_OFFLINE ? ID_STATUS_INVISIBLE : iStatus))
			{
				iHidden = 0;
				clmi.flags = CMIM_FLAGS | CMIM_NAME | CMIM_ICON | CMIF_TCHAR;
				clmi.hIcon = LoadSkinnedProtoIcon(szProto, iStatus);
				mir_sntprintf(str, SIZEOF(str), TranslateT("Re&ad %s Message"), (TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, iStatus, GSMDF_TCHAR));
				clmi.ptszName = str;
			}
		}
	}
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hAwayMsgMenuItem, (LPARAM)&clmi);
	CallService(MS_SKIN2_RELEASEICON, (WPARAM)clmi.hIcon, (LPARAM)0);
	clmi.flags = CMIM_FLAGS | CMIF_HIDDEN | CMIF_TCHAR;

	if (!iHidden)
	{
		DBVARIANT dbv;
#ifdef _UNICODE
		int unicode = !DBGetContactSetting((HANDLE)wParam, "CList", "StatusMsg", &dbv) && (dbv.type == DBVT_UTF8 || dbv.type == DBVT_WCHAR);
		DBFreeVariant(&dbv);
		if (unicode)
		{
			DBGetContactSettingWString((HANDLE)wParam, "CList", "StatusMsg", &dbv);
			szMsg = mir_u2a(dbv.pwszVal);
		}
		else
#endif
		{
			DBGetContactSettingString((HANDLE)wParam, "CList", "StatusMsg", &dbv);
			szMsg = mir_strdup(dbv.pszVal);
		}
		DBFreeVariant(&dbv);

		if (DBGetContactSettingByte(NULL, "SimpleStatusMsg", "ShowCopy", 1) && szMsg && *szMsg != '\0')
		{
			clmi.flags = CMIM_FLAGS | CMIM_NAME | CMIF_TCHAR;
			mir_sntprintf(str, SIZEOF(str), TranslateT("Copy %s Message"), (TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, iStatus, GSMDF_TCHAR));
			clmi.ptszName = str;
		}
	}
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hCopyMsgMenuItem, (LPARAM)&clmi);
	clmi.flags = CMIM_FLAGS | CMIF_HIDDEN | CMIF_TCHAR;

	if (!iHidden)
	{
		if (DBGetContactSettingByte(NULL, "SimpleStatusMsg", "ShowGoToURL", 1) && StrFindURL(szMsg) != NULL)
		{
			clmi.flags = CMIM_FLAGS | CMIM_NAME | CMIF_TCHAR;
			mir_sntprintf(str, SIZEOF(str), TranslateT("&Go to URL in %s Message"), (TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, iStatus, GSMDF_TCHAR));
			clmi.ptszName = str;
		}
		mir_free(szMsg);
	}
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hGoToURLMenuItem, (LPARAM)&clmi);

	return 0;
}

int AwayMsgPreShutdown(void)
{
	if (hWindowList) WindowList_BroadcastAsync(hWindowList, WM_CLOSE, 0, 0);
	if (hWindowList2) WindowList_BroadcastAsync(hWindowList2, WM_CLOSE, 0, 0);

	return 0;
}

int LoadAwayMsgModule(void)
{
	CLISTMENUITEM mi = {0};

	hWindowList = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	hWindowList2 = (HANDLE)CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	mi.cbSize = sizeof(mi);
	mi.flags = CMIF_TCHAR;

	CreateServiceFunctionEx(MS_AWAYMSG_SHOWAWAYMSG, GetMessageCommand);
	mi.position = -2000005000;
	mi.ptszName = LPGENT("Re&ad Away Message");
	mi.pszService = MS_AWAYMSG_SHOWAWAYMSG;
	hAwayMsgMenuItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	mi.flags |= CMIF_ICONFROMICOLIB;
	CreateServiceFunctionEx(MS_SIMPLESTATUSMSG_COPYMSG, CopyAwayMsgCommand);
	mi.position = -2000006000;
	mi.icolibItem = GetIconHandle(IDI_COPY);
	mi.ptszName = LPGENT("Copy Away Message");
	mi.pszService = MS_SIMPLESTATUSMSG_COPYMSG;
	hCopyMsgMenuItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	CreateServiceFunctionEx(MS_SIMPLESTATUSMSG_GOTOURLMSG, GoToURLMsgCommand);
	mi.position = -2000007000;
	mi.icolibItem = GetIconHandle(IDI_GOTOURL);
	mi.ptszName = LPGENT("&Go to URL in Away Message");
	mi.pszService = MS_SIMPLESTATUSMSG_GOTOURLMSG;
	hGoToURLMenuItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	HookEventEx(ME_CLIST_PREBUILDCONTACTMENU, AwayMsgPreBuildMenu);

	// Deprecated SimpleAway services
	CreateServiceFunctionEx(MS_SA_COPYAWAYMSG, CopyAwayMsgCommand);
	CreateServiceFunctionEx(MS_SA_GOTOURLMSG, GoToURLMsgCommand);

	return 0;
}
