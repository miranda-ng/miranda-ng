/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

#include "stdafx.h"

#define SUMMARY     0
#define DETAIL      1
#define DM_FINDNEXT  (WM_USER+10)
#define DM_HREBUILD  (WM_USER+11)

static MWindowList hWindowList = nullptr;
static HGENMENU hContactMenu = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////
// Find dialog window procedure

static CMStringW g_savedSearch;

class CHistoryFindDlg : public CDlgBase
{
public:
	CHistoryFindDlg(HWND hwndParent) :
		CDlgBase(g_plugin, IDD_HISTORY_FIND)
	{
		SetParent(hwndParent);
	}

	bool OnInitDialog() override
	{
		SetDlgItemText(m_hwnd, IDC_FINDWHAT, g_savedSearch);
		return true;
	}

	bool OnApply() override
	{
		wchar_t str[128];
		GetDlgItemText(m_hwnd, IDC_FINDWHAT, str, _countof(str));
		g_savedSearch = str;

		CharUpperW(str);
		SendMessage(m_hwndParent, DM_FINDNEXT, 0, (LPARAM)str);
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Fills the events list

static void GetObjectSummary(DBEVENTINFO *dbei, wchar_t *str, int cbStr)
{
	wchar_t *pszSrc, *pszTmp = nullptr;

	switch (dbei->eventType) {
	case EVENTTYPE_MESSAGE:
		if (dbei->flags & DBEF_SENT) pszSrc = TranslateT("Outgoing message");
		else                         pszSrc = TranslateT("Incoming message");
		break;

	case EVENTTYPE_FILE:
		if (dbei->flags & DBEF_SENT) pszSrc = TranslateT("Outgoing file");
		else                         pszSrc = TranslateT("Incoming file");
		break;

	case EVENTTYPE_AUTHREQUEST:
		pszSrc = TranslateT("Authorization request");
		break;

	case EVENTTYPE_ADDED:
		pszSrc = TranslateT("Added event");
		break;

	default:
		DBEVENTTYPEDESCR *et = DbEvent_GetType(dbei->szModule, dbei->eventType);
		if (et && (et->flags & DETF_HISTORY)) {
			pszTmp = mir_a2u(et->descr);
			pszSrc = TranslateW(pszTmp);
			break;
		}
		*str = 0;
		return;
	}

	wcsncpy(str, (const wchar_t*)pszSrc, cbStr);
	str[cbStr-1] = 0;
	mir_free(pszTmp);
}

struct THistoryThread
{
	MCONTACT hContact;
	HWND hwnd;
};

static void FillHistoryThread(THistoryThread *hInfo)
{
	Thread_SetName("HistoryWindow::FillHistoryThread");

	HWND hwndList = GetDlgItem(hInfo->hwnd, IDC_LIST);

	SendDlgItemMessage(hInfo->hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
	int i = db_event_count(hInfo->hContact);
	SendDlgItemMessage(hInfo->hwnd, IDC_LIST, LB_INITSTORAGE, i, i * 40);

	DB::ECPTR pCursor(DB::EventsRev(hInfo->hContact));
	while (MEVENT hDbEvent = pCursor.FetchNext()) {
		if (!IsWindow(hInfo->hwnd))
			break;

		DBEVENTINFO dbei = {};
		db_event_get(hDbEvent, &dbei);

		wchar_t str[200], eventText[256], strdatetime[64];
		GetObjectSummary(&dbei, str, _countof(str));
		if (str[0]) {
			TimeZone_PrintTimeStamp(NULL, dbei.timestamp, L"d t", strdatetime, _countof(strdatetime), 0);
			mir_snwprintf(eventText, L"%s: %s", strdatetime, str);
			i = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)eventText);
			SendMessage(hwndList, LB_SETITEMDATA, i, (LPARAM)hDbEvent);
		}
	}

	SendDlgItemMessage(hInfo->hwnd, IDC_LIST, LB_SETCURSEL, 0, 0);
	SendMessage(hInfo->hwnd, WM_COMMAND, MAKEWPARAM(IDC_LIST, LBN_SELCHANGE), 0);
	EnableWindow(GetDlgItem(hInfo->hwnd, IDC_LIST), TRUE);
	mir_free(hInfo);
}

static int HistoryDlgResizer(HWND, LPARAM, UTILRESIZECONTROL *urc)
{
	switch (urc->wId) {
	case IDC_LIST:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;
	case IDC_EDIT:
		return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;
	case IDC_FIND:
	case IDC_DELETEHISTORY:
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
	case IDOK:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

static LRESULT CALLBACK HotkeyProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CHAR:
		if (wParam == 0x06) // Ctrl+F
			PostMessage(GetParent(hwndDlg), WM_COMMAND, IDC_FIND, 0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_F3) {
			wchar_t buf[200];
			wcsncpy_s(buf, g_savedSearch, _countof(buf)-1);
			CharUpperW(buf);
			SendMessage(GetParent(hwndDlg), DM_FINDNEXT, 0, LPARAM(buf));
		}
		else if (wParam == VK_DELETE) {
			PostMessage(GetParent(hwndDlg), WM_COMMAND, IDC_DELETEHISTORY, 0);
		}
		break;
	}

	return mir_callNextSubclass(hwndDlg, HotkeyProc, msg, wParam, lParam);
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

		if (hContact) {
			wchar_t str[200];
			mir_snwprintf(str, TranslateT("History for %s"), Clist_GetContactDisplayName(hContact));
			SetWindowText(hwndDlg, str);
		}
		else SetWindowText(hwndDlg, TranslateT("System history"));

		mir_subclassWindow(GetDlgItem(hwndDlg, IDC_LIST), HotkeyProc);
		mir_subclassWindow(GetDlgItem(hwndDlg, IDC_EDIT), HotkeyProc);
	
		Window_SetSkinIcon_IcoLib(hwndDlg, SKINICON_OTHER_HISTORY);
		SendMessage(hwndDlg, DM_HREBUILD, 0, 0);
		return TRUE;

	case DM_HREBUILD:
		{
			THistoryThread *hInfo = (THistoryThread *)mir_alloc(sizeof(THistoryThread));
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIST), FALSE);
			hInfo->hContact = hContact;
			hInfo->hwnd = hwndDlg;
			mir_forkThread<THistoryThread>(FillHistoryThread, hInfo);
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
		break;

	case WM_SIZE:
		Utils_ResizeDialog(hwndDlg, g_plugin.getInst(), MAKEINTRESOURCEA(IDD_HISTORY), HistoryDlgResizer);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return TRUE;

		case IDC_FIND:
			CHistoryFindDlg(hwndDlg).DoModal();
			return TRUE;

		case IDC_DELETEHISTORY:
			{
				int index = SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETCURSEL, 0, 0);
				if (index == LB_ERR)
					break;

				if (MessageBox(hwndDlg, TranslateT("Are you sure you want to delete this history item?"), TranslateT("Delete history"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
					MEVENT hDbevent = SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETITEMDATA, index, 0);
					db_event_delete(hDbevent);
					SendMessage(hwndDlg, DM_HREBUILD, 0, 0);
				}
			}
			return TRUE;

		case IDC_LIST:
			if (HIWORD(wParam) == LBN_SELCHANGE) {
				int sel = SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETCURSEL, 0, 0);
				if (sel == LB_ERR) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEHISTORY), FALSE);
					break;
				}
				
				EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEHISTORY), TRUE);
				MEVENT hDbEvent = SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETITEMDATA, sel, 0);

				DB::EventInfo dbei;
				dbei.cbBlob = -1;
				if (!db_event_get(hDbEvent, &dbei)) {
					ptrW wszDescr(DbEvent_GetTextW(&dbei, CP_ACP));
					if (wszDescr)
						SetDlgItemText(hwndDlg, IDC_EDIT, wszDescr);
				}
			}
			return TRUE;
		}
		break;

	case DM_FINDNEXT:
		int index = SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETCURSEL, 0, 0);
		if (index == LB_ERR)
			break;

		DBEVENTINFO dbei = {};
		int oldBlobSize = 0;
		MEVENT hDbEventStart = SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETITEMDATA, index, 0);

		for (;;) {
			MEVENT hDbEvent = SendDlgItemMessage(hwndDlg, IDC_LIST, LB_GETITEMDATA, ++index, 0);
			if (hDbEvent == LB_ERR) {
				index = -1;
				continue;
			}
			if (hDbEvent == hDbEventStart)
				break;

			int newBlobSize = db_event_getBlobSize(hDbEvent);
			if (newBlobSize > oldBlobSize) {
				dbei.pBlob = (uint8_t*)mir_realloc(dbei.pBlob, newBlobSize);
				oldBlobSize = newBlobSize;
			}
			dbei.cbBlob = oldBlobSize;
			db_event_get(hDbEvent, &dbei);

			ptrW wszDescr(DbEvent_GetTextW(&dbei, CP_ACP));
			if (wszDescr) {
				CharUpperW(wszDescr);
				if (wcsstr(wszDescr, (const wchar_t *)lParam) != nullptr) {
					SendDlgItemMessage(hwndDlg, IDC_LIST, LB_SETCURSEL, index, 0);
					SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_LIST, LBN_SELCHANGE), 0);
					break;
				}
			}
		}

		mir_free(dbei.pBlob);
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
	CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_HISTORY), NULL, DlgProcHistory, wParam);
	return 0;
}

static int HistoryContactDelete(WPARAM wParam, LPARAM)
{
	HWND hwnd = WindowList_Find(hWindowList, wParam);
	if (hwnd != nullptr)
		DestroyWindow(hwnd);
	return 0;
}

static int PrebuildContactMenu(WPARAM hContact, LPARAM)
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
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x28848d7a, 0x6995, 0x4799, 0x82, 0xd7, 0x18, 0x40, 0x3d, 0xe3, 0x71, 0xc4);
	mi.position = 1000090000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_HISTORY);
	mi.name.a = LPGEN("View &history");
	mi.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
	hContactMenu = Menu_AddContactMenuItem(&mi);

	CreateServiceFunction(MS_HISTORY_SHOWCONTACTHISTORY, UserHistoryCommand);
	hWindowList = WindowList_Create();
	HookEvent(ME_DB_CONTACT_DELETED, HistoryContactDelete);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdownHistoryModule);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);
	return 0;
}
