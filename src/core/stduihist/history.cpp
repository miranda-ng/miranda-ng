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

#define SUMMARY     0
#define DETAIL      1
#define DM_FINDNEXT  (WM_USER+10)
#define DM_HREBUILD  (WM_USER+11)

static INT_PTR CALLBACK DlgProcHistory(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcHistoryFind(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static HANDLE hWindowList = 0;
static HGENMENU hContactMenu = 0;

/////////////////////////////////////////////////////////////////////////////////////////
// Fills the events list

static void GetMessageDescription(DBEVENTINFO *dbei, TCHAR* buf, int cbBuf)
{
	TCHAR* msg = DbGetEventTextT(dbei, CP_ACP);
    _tcsncpy(buf, msg ? msg : TranslateT("Invalid message"), cbBuf);
    buf[ cbBuf-1 ] = 0;
	mir_free(msg);
}

static void GetUrlDescription(DBEVENTINFO *dbei, TCHAR* buf, int cbBuf)
{
	int len = dbei->cbBlob;
	if (len >= cbBuf)
		len = cbBuf-1;

	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)dbei->pBlob, len, buf, cbBuf);
	buf[ len ] = 0;

	if (len < cbBuf-3)
		_tcscat(buf, _T("\r\n"));
}

static void GetFileDescription(DBEVENTINFO *dbei, TCHAR* buf, int cbBuf)
{
	int len = dbei->cbBlob - sizeof(DWORD);
	if (len >= cbBuf)
		len = cbBuf-1;

	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)dbei->pBlob + sizeof(DWORD), len, buf, cbBuf);
	buf[ len ] = 0;

	if (len < cbBuf-3)
		_tcscat(buf, _T("\r\n"));
}

static void GetObjectDescription(DBEVENTINFO *dbei, TCHAR* str, int cbStr)
{
	switch(dbei->eventType) {
	case EVENTTYPE_MESSAGE:
		GetMessageDescription(dbei, str, cbStr);
		break;

	case EVENTTYPE_URL:
		GetUrlDescription(dbei, str, cbStr);
		break;

	case EVENTTYPE_FILE:
		GetFileDescription(dbei, str, cbStr);
		break;

	default:
		DBEVENTTYPEDESCR *et = (DBEVENTTYPEDESCR*)CallService(MS_DB_EVENT_GETTYPE, (WPARAM)dbei->szModule, (LPARAM)dbei->eventType);
		if (et && (et->flags & DETF_HISTORY))
			GetMessageDescription(dbei, str, cbStr);
		else
			*str = 0;
}	}

static void GetObjectSummary(DBEVENTINFO *dbei, TCHAR* str, int cbStr)
{
	TCHAR* pszSrc, *pszTmp = NULL;

	switch(dbei->eventType) {
	case EVENTTYPE_MESSAGE:
		if (dbei->flags & DBEF_SENT)   pszSrc = TranslateT("Outgoing message");
		else                             pszSrc = TranslateT("Incoming message");
		break;

	case EVENTTYPE_URL:
		if (dbei->flags & DBEF_SENT)   pszSrc = TranslateT("Outgoing URL");
      else                             pszSrc = TranslateT("Incoming URL");
		break;

	case EVENTTYPE_FILE:
		if (dbei->flags & DBEF_SENT)   pszSrc = TranslateT("Outgoing file");
		else                             pszSrc = TranslateT("Incoming file");
		break;

	default:
		DBEVENTTYPEDESCR* et = (DBEVENTTYPEDESCR*)CallService(MS_DB_EVENT_GETTYPE, (WPARAM)dbei->szModule, (LPARAM)dbei->eventType);
		if (et && (et->flags & DETF_HISTORY)) {
			pszTmp = mir_a2t(et->descr);
			pszSrc = TranslateTS(pszTmp);
			break;
		}
		*str = 0;
		return;
	}

	_tcsncpy(str, (const TCHAR*)pszSrc, cbStr);
	str[cbStr-1] = 0;
	mir_free(pszTmp);
}

typedef struct {
	MCONTACT hContact;
	HWND hwnd;
} THistoryThread;

static void FillHistoryThread(void* param)
{
	Thread_SetName("HistoryWindow::FillHistoryThread");

	THistoryThread *hInfo = (THistoryThread*)param;
	HWND hwndList = GetDlgItem(hInfo->hwnd, IDC_LIST);

	SendDlgItemMessage(hInfo->hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
	int i = db_event_count(hInfo->hContact);
	SendDlgItemMessage(hInfo->hwnd, IDC_LIST, LB_INITSTORAGE, i, i*40);

	DBEVENTINFO dbei = { sizeof(dbei) };
	int oldBlobSize = 0;
	HANDLE hDbEvent = db_event_last(hInfo->hContact);

	while (hDbEvent != NULL) {
		if ( !IsWindow(hInfo->hwnd))
			break;
		int newBlobSize = db_event_getBlobSize(hDbEvent);
		if (newBlobSize > oldBlobSize) {
			dbei.pBlob = (PBYTE)mir_realloc(dbei.pBlob, newBlobSize);
			oldBlobSize = newBlobSize;
		}
		dbei.cbBlob = oldBlobSize;
		db_event_get(hDbEvent, &dbei);

		TCHAR str[200], eventText[256], strdatetime[64];
		GetObjectSummary(&dbei, str, SIZEOF(str));
		if (str[0]) {
			tmi.printTimeStamp(NULL, dbei.timestamp, _T("d t"), strdatetime, SIZEOF(strdatetime), 0);
			mir_sntprintf(eventText, SIZEOF(eventText), _T("%s: %s"), strdatetime, str);
			i = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)eventText);
			SendMessage(hwndList, LB_SETITEMDATA, i, (LPARAM)hDbEvent);
		}
		hDbEvent = db_event_prev(hInfo->hContact, hDbEvent);
	}
	mir_free(dbei.pBlob);

	SendDlgItemMessage(hInfo->hwnd, IDC_LIST, LB_SETCURSEL, 0, 0);
	SendMessage(hInfo->hwnd, WM_COMMAND, MAKEWPARAM(IDC_LIST, LBN_SELCHANGE), 0);
	EnableWindow(GetDlgItem(hInfo->hwnd, IDC_LIST), TRUE);
	mir_free(hInfo);
}

static int HistoryDlgResizer(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch(urc->wId) {
	case IDC_LIST:
		return RD_ANCHORX_WIDTH|RD_ANCHORY_HEIGHT;
	case IDC_EDIT:
		return RD_ANCHORX_WIDTH|RD_ANCHORY_BOTTOM;
	case IDC_FIND:
	case IDC_DELETEHISTORY:
		return RD_ANCHORX_LEFT|RD_ANCHORY_BOTTOM;
	case IDOK:
		return RD_ANCHORX_RIGHT|RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT|RD_ANCHORY_TOP;
}

static INT_PTR CALLBACK DlgProcHistory(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
		hContact = lParam;
		WindowList_Add(hWindowList, hwndDlg, hContact);
		Utils_RestoreWindowPosition(hwndDlg, hContact, "History", "");
		{
			TCHAR* contactName, str[200];
			contactName = pcli->pfnGetContactDisplayName(hContact, 0);
			mir_sntprintf(str, SIZEOF(str), TranslateT("History for %s"), contactName);
			SetWindowText(hwndDlg, str);
		}
		Window_SetIcon_IcoLib(hwndDlg, SKINICON_OTHER_HISTORY);
		SendMessage(hwndDlg, DM_HREBUILD, 0, 0);
		return TRUE;

	case DM_HREBUILD:
		{
			THistoryThread* hInfo = (THistoryThread*)mir_alloc(sizeof(THistoryThread));
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIST), FALSE);
			hInfo->hContact = hContact;
			hInfo->hwnd = hwndDlg;
			forkthread(FillHistoryThread, 0, hInfo);
		}
		return TRUE;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hwndDlg);
		Utils_SaveWindowPosition(hwndDlg, hContact, "History", "");
		WindowList_Remove(hWindowList, hwndDlg);
		return TRUE;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 300;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 230;

	case WM_SIZE:
		{
			UTILRESIZEDIALOG urd = {0};
			urd.cbSize = sizeof(urd);
			urd.hwndDlg = hwndDlg;
			urd.hInstance = hInst;
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_HISTORY);
			urd.lParam = 0;
			urd.pfnResizer = HistoryDlgResizer;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);
			return TRUE;
		}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return TRUE;

		case IDC_FIND:
			ShowWindow(CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_HISTORY_FIND), hwndDlg, DlgProcHistoryFind, (LPARAM)hwndDlg), SW_SHOW);
			return TRUE;

		case IDC_DELETEHISTORY:
			{
				HANDLE hDbevent;
				int index = SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETCURSEL, 0, 0);
				if (index == LB_ERR)
					break;

				if (MessageBox(hwndDlg, TranslateT("Are you sure you want to delete this history item?"), TranslateT("Delete history"), MB_YESNO|MB_ICONQUESTION) == IDYES) {
					hDbevent = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETITEMDATA, index, 0);
					db_event_delete(hContact, hDbevent);
					SendMessage(hwndDlg, DM_HREBUILD, 0, 0);
				}
				return TRUE;
			}
		case IDC_LIST:
			if (HIWORD(wParam) == LBN_SELCHANGE) {
				int sel = SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETCURSEL, 0, 0);
				if (sel == LB_ERR) { EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEHISTORY), FALSE); break; }
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEHISTORY), TRUE);
				TCHAR *contactName = pcli->pfnGetContactDisplayName(hContact, 0);
				HANDLE hDbEvent = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETITEMDATA, sel, 0);

				DBEVENTINFO dbei = { sizeof(dbei) };
				dbei.cbBlob = db_event_getBlobSize(hDbEvent);
				if ((int)dbei.cbBlob != -1) {
					dbei.pBlob = (PBYTE)mir_alloc(dbei.cbBlob);
					if (db_event_get(hDbEvent, &dbei) == 0) {
						TCHAR str[8192];
						GetObjectDescription(&dbei, str, SIZEOF(str));
						if (str[0])
							SetDlgItemText(hwndDlg, IDC_EDIT, str);
					}
					mir_free(dbei.pBlob);
				}
			}
			return TRUE;
		}
		break;
	case DM_FINDNEXT:
		{
			int index = SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETCURSEL, 0, 0);
			if (index == LB_ERR)
				break;

			DBEVENTINFO dbei = { sizeof(dbei) };
			int oldBlobSize = 0;
			HANDLE hDbEventStart = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETITEMDATA, index, 0);

			for (;;) {
				HANDLE hDbEvent = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETITEMDATA, ++index, 0);
				if (hDbEvent == (HANDLE)LB_ERR) {
					index = -1;
					continue;
				}
				if (hDbEvent == hDbEventStart)
					break;

				int newBlobSize = db_event_getBlobSize(hDbEvent);
				if (newBlobSize>oldBlobSize) {
					dbei.pBlob = (PBYTE)mir_realloc(dbei.pBlob, newBlobSize);
					oldBlobSize = newBlobSize;
				}
				dbei.cbBlob = oldBlobSize;
				db_event_get(hDbEvent, &dbei);

				TCHAR str[1024];
				GetObjectDescription(&dbei, str, SIZEOF(str));
				if (str[0]) {
					CharUpperBuff(str, lstrlen(str));
					if (_tcsstr(str, (const TCHAR*)lParam) != NULL) {
						SendDlgItemMessage(hwndDlg, IDC_LIST, LB_SETCURSEL, index, 0);
						SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_LIST, LBN_SELCHANGE), 0);
						break;
			}	}	}

			mir_free(dbei.pBlob);
			break;
		}
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcHistoryFind(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)lParam);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case IDOK://find Next
			{
				TCHAR str[128];
				HWND hwndParent = (HWND)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
				GetDlgItemText(hwndDlg, IDC_FINDWHAT, str, SIZEOF(str));
				CharUpperBuff(str, lstrlen(str));
				SendMessage(hwndParent, DM_FINDNEXT, 0, (LPARAM)str);
				return TRUE;
			}
			case IDCANCEL:
				DestroyWindow(hwndDlg);
				return TRUE;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR UserHistoryCommand(WPARAM wParam, LPARAM)
{
	HWND hwnd = WindowList_Find(hWindowList, wParam);
	if (hwnd) {
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
		return 0;
	}
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_HISTORY), NULL, DlgProcHistory, wParam);
	return 0;
}

static int HistoryContactDelete(WPARAM wParam, LPARAM)
{
	HWND hwnd = WindowList_Find(hWindowList, wParam);
	if (hwnd != NULL)
		DestroyWindow(hwnd);
	return 0;
}

static int PrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	Menu_ShowItem(hContactMenu, db_event_last(hContact) != NULL);
	return 0;
}

static int PreShutdownHistoryModule(WPARAM, LPARAM)
{
	if (hWindowList) {
		WindowList_Broadcast(hWindowList, WM_DESTROY, 0, 0);
		WindowList_Destroy(hWindowList);
	}
	return 0;
}

int LoadHistoryModule(void)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 1000090000;
	mi.icolibItem = GetSkinIconHandle(SKINICON_OTHER_HISTORY);
	mi.pszName = LPGEN("View &history");
	mi.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
	hContactMenu = Menu_AddContactMenuItem(&mi);

	CreateServiceFunction(MS_HISTORY_SHOWCONTACTHISTORY, UserHistoryCommand);
	hWindowList = WindowList_Create();
	HookEvent(ME_DB_CONTACT_DELETED, HistoryContactDelete);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdownHistoryModule);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);
	return 0;
}
