/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson
Copyright 2003-2009 Miranda ICQ/IM project,

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

#define UM_CHECKSTATECHANGE (WM_USER+100)

static HWND hPathTip = nullptr;

struct branch_t
{
	wchar_t  *szDescr;
	char     *szDBName;
	int       iMode;
	uint8_t      bDefault;
	HTREEITEM hItem;
};

static struct branch_t branch1[] = {
	{ LPGENW("Flash when someone speaks"), "FlashWindow", 0, 0, nullptr },
	{ LPGENW("Flash when a word is highlighted"), "FlashWindowHighlight", 0, 1, nullptr },
	{ LPGENW("Show chat nick list"), "ShowNicklist", 0, 1, nullptr },
	{ LPGENW("Show topic on your contact list (if supported)"), "TopicOnClist", 0, 0, nullptr },
	{ LPGENW("Do not play sounds when focused"), "SoundsFocus", 0, 0, nullptr },
	{ LPGENW("Do not pop up the window when joining a chat room"), "PopupOnJoin", 0, 0, nullptr },
	{ LPGENW("Show contact statuses (if supported)"), "ShowContactStatus", 0, 0, nullptr },
	{ LPGENW("Display contact status icon before role icon"), "ContactStatusFirst", 0, 0, nullptr },
	{ LPGENW("Add ':' to auto-completed names"), "AddColonToAutoComplete", 0, 1, nullptr }
};

static struct branch_t branch2[] = {
	{ LPGENW("Show icons"), "IconFlags", GC_EVENT_TOPIC | GC_EVENT_JOIN | GC_EVENT_QUIT |
	GC_EVENT_MESSAGE | GC_EVENT_ACTION | GC_EVENT_HIGHLIGHT | GC_EVENT_PART |
	GC_EVENT_KICK | GC_EVENT_NOTICE | GC_EVENT_NICK | GC_EVENT_INFORMATION | GC_EVENT_ADDSTATUS, 0, nullptr },
	{ LPGENW("Prefix all events with a timestamp"), "ShowTimeStamp", 0, 1, nullptr },
	{ LPGENW("Only prefix with timestamp if it has changed"), "ShowTimeStampIfChanged", 0, 0, nullptr },
	{ LPGENW("Timestamp has same color as the event"), "TimeStampEventColour", 0, 0, nullptr },
	{ LPGENW("Indent the second line of a message"), "LogIndentEnabled", 0, 1, nullptr },
	{ LPGENW("Limit user names to 20 characters"), "LogLimitNames", 0, 1, nullptr },
	{ LPGENW("Strip colors from messages"), "StripFormatting", 0, 0, nullptr },
	{ LPGENW("Enable 'event filter' for new rooms"), "FilterEnabled", 0, 0, nullptr }
};

static HTREEITEM InsertBranch(HWND hwndTree, wchar_t *pszDescr, BOOL bExpanded)
{
	TVINSERTSTRUCT tvis;
	tvis.hParent = nullptr;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_STATE;
	tvis.item.pszText = TranslateW(pszDescr);
	tvis.item.stateMask = bExpanded ? TVIS_STATEIMAGEMASK | TVIS_EXPANDED : TVIS_STATEIMAGEMASK;
	tvis.item.state = bExpanded ? INDEXTOSTATEIMAGEMASK(1) | TVIS_EXPANDED : INDEXTOSTATEIMAGEMASK(1);
	return TreeView_InsertItem(hwndTree, &tvis);
}

static void FillBranch(HWND hwndTree, HTREEITEM hParent, struct branch_t *branch, int nValues, uint32_t defaultval)
{
	int iState;

	TVINSERTSTRUCT tvis;
	tvis.hParent = hParent;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_STATE;
	for (int i = 0; i < nValues; i++) {
		tvis.item.pszText = TranslateW(branch[i].szDescr);
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		if (branch[i].iMode)
			iState = ((db_get_dw(0, CHAT_MODULE, branch[i].szDBName, defaultval)&branch[i].iMode)&branch[i].iMode) != 0 ? 2 : 1;
		else
			iState = db_get_b(0, CHAT_MODULE, branch[i].szDBName, branch[i].bDefault) != 0 ? 2 : 1;
		tvis.item.state = INDEXTOSTATEIMAGEMASK(iState);
		branch[i].hItem = TreeView_InsertItem(hwndTree, &tvis);
	}
}

static void SaveBranch(HWND hwndTree, struct branch_t *branch, int nValues)
{
	int iState = 0;

	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_STATE;
	for (int i = 0; i < nValues; i++) {
		tvi.hItem = branch[i].hItem;
		TreeView_GetItem(hwndTree, &tvi);
		uint8_t bChecked = ((tvi.state&TVIS_STATEIMAGEMASK) >> 12 == 1) ? 0 : 1;
		if (branch[i].iMode) {
			if (bChecked)
				iState |= branch[i].iMode;
			if (iState&GC_EVENT_ADDSTATUS)
				iState |= GC_EVENT_REMOVESTATUS;
			db_set_dw(0, CHAT_MODULE, branch[i].szDBName, (uint32_t)iState);
		}
		else db_set_b(0, CHAT_MODULE, branch[i].szDBName, bChecked);
	}
}

static void CheckHeading(HWND hwndTree, HTREEITEM hHeading)
{
	BOOL bChecked = TRUE;

	if (hHeading == nullptr)
		return;

	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_STATE;
	tvi.hItem = TreeView_GetNextItem(hwndTree, hHeading, TVGN_CHILD);
	while (tvi.hItem && bChecked) {
		if (tvi.hItem != branch1[0].hItem && tvi.hItem != branch1[1].hItem) {
			TreeView_GetItem(hwndTree, &tvi);
			if (((tvi.state&TVIS_STATEIMAGEMASK) >> 12 == 1))
				bChecked = FALSE;
		}
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
	tvi.stateMask = TVIS_STATEIMAGEMASK;
	tvi.state = INDEXTOSTATEIMAGEMASK(bChecked ? 2 : 1);
	tvi.hItem = hHeading;
	TreeView_SetItem(hwndTree, &tvi);
}

static void CheckBranches(HWND hwndTree, HTREEITEM hHeading)
{
	BOOL bChecked = TRUE;
	TVITEM tvi;

	if (hHeading == nullptr)
		return;

	tvi.mask = TVIF_HANDLE | TVIF_STATE;
	tvi.hItem = hHeading;
	TreeView_GetItem(hwndTree, &tvi);
	if (((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2))
		bChecked = FALSE;
	tvi.hItem = TreeView_GetNextItem(hwndTree, hHeading, TVGN_CHILD);
	tvi.stateMask = TVIS_STATEIMAGEMASK;
	while (tvi.hItem) {
		tvi.state = INDEXTOSTATEIMAGEMASK(bChecked ? 2 : 1);
		if (tvi.hItem != branch1[0].hItem && tvi.hItem != branch1[1].hItem)
			TreeView_SetItem(hwndTree, &tvi);
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
}

static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	char szDir[MAX_PATH];
	switch (uMsg) {
	case BFFM_INITIALIZED:
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;
	case BFFM_SELCHANGED:
		if (SHGetPathFromIDListA((LPITEMIDLIST)lp, szDir))
			SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szDir);
		break;
	}
	return 0;
}

#define OPT_FIXHEADINGS (WM_USER+1)

static INT_PTR CALLBACK DlgProcOptions1(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HTREEITEM hListHeading1 = nullptr;
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN2, UDM_SETRANGE, 0, MAKELONG(255, 10));
		SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN2, UDM_SETPOS, 0, MAKELONG(db_get_b(0, CHAT_MODULE, "NicklistRowDist", 12), 0));
		hListHeading1 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), TranslateT("Appearance and functionality of chat windows"), db_get_b(0, CHAT_MODULE, "Branch1Exp", 0) ? TRUE : FALSE);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading1, branch1, _countof(branch1), 0);
		SendMessage(hwndDlg, OPT_FIXHEADINGS, 0, 0);
		SetDlgItemText(hwndDlg, IDC_CHAT_GROUP, ptrW(Chat_GetGroup()));
		break;

	case OPT_FIXHEADINGS:
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading1);
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_CHAT_NICKROW || LOWORD(wParam) == IDC_CHAT_GROUP) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;

		if (lParam != 0)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case UM_CHECKSTATECHANGE:
		{
			TVITEM tvi = { 0 };
			tvi.mask = TVIF_HANDLE | TVIF_STATE;
			tvi.hItem = (HTREEITEM)lParam;
			TreeView_GetItem((HWND)wParam, &tvi);
			if (tvi.hItem == hListHeading1)
				CheckBranches(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading1);
			else
				PostMessage(hwndDlg, OPT_FIXHEADINGS, 0, 0);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_CHAT_CHECKBOXES:
			if (((LPNMHDR)lParam)->code == NM_CLICK) {
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti))
					if (hti.flags&TVHT_ONITEMSTATEICON) {
						SendMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom, (LPARAM)hti.hItem);
					}

			}
			else if (((LPNMHDR)lParam)->code == TVN_KEYDOWN) {
				if (((LPNMTVKEYDOWN)lParam)->wVKey == VK_SPACE)
					SendMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom,
					(LPARAM)TreeView_GetSelection(((LPNMHDR)lParam)->hwndFrom));
			}
			break;

		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_WIZFINISH:
				g_chatApi.ReloadSettings();
				Chat_UpdateOptions();
				break;

			case PSN_APPLY:
				int iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_GROUP));
				if (iLen > 0) {
					ptrW pszText((wchar_t*)mir_alloc(sizeof(wchar_t)*(iLen + 1)));
					GetDlgItemTextW(hwndDlg, IDC_CHAT_GROUP, pszText, iLen + 1);
					Chat_SetGroup(pszText);
				}
				else Chat_SetGroup(nullptr);

				iLen = SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN2, UDM_GETPOS, 0, 0);
				if (iLen > 0)
					db_set_b(0, CHAT_MODULE, "NicklistRowDist", (uint8_t)iLen);
				else
					db_unset(0, CHAT_MODULE, "NicklistRowDist");
				SaveBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), branch1, _countof(branch1));
			}
			return TRUE;
		}
		break;

	case WM_DESTROY:
		uint8_t b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading1, TVIS_EXPANDED)&TVIS_EXPANDED ? 1 : 0;
		db_set_b(0, CHAT_MODULE, "Branch1Exp", b);
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcOptions2(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HTREEITEM hListHeading2 = nullptr;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN2, UDM_SETRANGE, 0, MAKELONG(5000, 0));
		SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN2, UDM_SETPOS, 0, MAKELONG(db_get_w(0, CHAT_MODULE, "LogLimit", 100), 0));
		SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN3, UDM_SETRANGE, 0, MAKELONG(10000, 0));
		SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN3, UDM_SETPOS, 0, MAKELONG(db_get_w(0, CHAT_MODULE, "LoggingLimit", 100), 0));
		{
			wchar_t tszTemp[MAX_PATH];
			PathToRelativeW(g_Settings.pszLogDir, tszTemp);
			SetDlgItemText(hwndDlg, IDC_CHAT_LOGDIRECTORY, tszTemp);
		}

		wchar_t tszTooltipText[2048];
		RECT rect;

		mir_snwprintf(tszTooltipText,
			L"%s - %s\n%s - %s\n%s - %s\n%s - %s\n\n"
			L"%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n\n"
			L"%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s",
			// contact vars
			L"%nick%", TranslateT("nick of current contact (if defined)"),
			L"%proto%", TranslateT("protocol name of current contact (if defined). Account name is used when protocol supports multiple accounts"),
			L"%accountname%", TranslateT("user-defined account name of current contact (if defined)."),
			L"%userid%", TranslateT("user ID of current contact (if defined). It is like UIN for ICQ, JID for Jabber, etc."),
			// global vars
			L"%miranda_path%", TranslateT("path to Miranda root folder"),
			L"%miranda_profilesdir%", TranslateT("path to folder containing Miranda profiles"),
			L"%miranda_profilename%", TranslateT("name of current Miranda profile (filename, without extension)"),
			L"%miranda_userdata%", TranslateT("will return parsed string %miranda_profilesdir%\\%miranda_profilename%"),
			L"%miranda_logpath%", TranslateT("will return parsed string %miranda_userdata%\\Logs"),
			L"%appdata%", TranslateT("same as environment variable %APPDATA% for currently logged-on Windows user"),
			L"%username%", TranslateT("username for currently logged-on Windows user"),
			L"%mydocuments%", TranslateT("\"My Documents\" folder for currently logged-on Windows user"),
			L"%desktop%", TranslateT("\"Desktop\" folder for currently logged-on Windows user"),
			L"%xxxxxxx%", TranslateT("any environment variable defined in current Windows session (like %systemroot%, %allusersprofile%, etc.)"),
			// date/time vars
			L"%d%", TranslateT("day of month, 1-31"),
			L"%dd%", TranslateT("day of month, 01-31"),
			L"%m%", TranslateT("month number, 1-12"),
			L"%mm%", TranslateT("month number, 01-12"),
			L"%mon%", TranslateT("abbreviated month name"),
			L"%month%", TranslateT("full month name"),
			L"%yy%", TranslateT("year without century, 01-99"),
			L"%yyyy%", TranslateT("year with century, 1901-9999"),
			L"%wday%", TranslateT("abbreviated weekday name"),
			L"%weekday%", TranslateT("full weekday name"));
		GetClientRect(GetDlgItem(hwndDlg, IDC_CHAT_LOGDIRECTORY), &rect);
		rect.left = -85;
		hPathTip = CreateToolTip(GetDlgItem(hwndDlg, IDC_CHAT_LOGDIRECTORY), tszTooltipText, TranslateT("Variables"), &rect);
		SetTimer(hwndDlg, 0, 3000, nullptr);

		SetDlgItemText(hwndDlg, IDC_CHAT_HIGHLIGHTWORDS, g_Settings.pszHighlightWords);
		SetDlgItemText(hwndDlg, IDC_CHAT_LOGTIMESTAMP, g_Settings.pszTimeStampLog);
		SetDlgItemText(hwndDlg, IDC_CHAT_TIMESTAMP, g_Settings.pszTimeStamp);
		SetDlgItemText(hwndDlg, IDC_CHAT_OUTSTAMP, g_Settings.pszOutgoingNick);
		SetDlgItemText(hwndDlg, IDC_CHAT_INSTAMP, g_Settings.pszIncomingNick);
		CheckDlgButton(hwndDlg, IDC_CHAT_HIGHLIGHT, g_Settings.bHighlightEnabled ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_HIGHLIGHTWORDS), g_Settings.bHighlightEnabled ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHAT_LOGGING, g_Settings.bLoggingEnabled ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_LOGDIRECTORY), g_Settings.bLoggingEnabled ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_LOGDIRCHOOSE), g_Settings.bLoggingEnabled ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIMIT), g_Settings.bLoggingEnabled ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIMITTEXT2), g_Settings.bLoggingEnabled ? TRUE : FALSE);

		hListHeading2 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), TranslateT("Appearance"), db_get_b(0, CHAT_MODULE, "Branch2Exp", 0) ? TRUE : FALSE);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading2, branch2, _countof(branch2), 0x0);
		SendMessage(hwndDlg, OPT_FIXHEADINGS, 0, 0);
		break;

	case OPT_FIXHEADINGS:
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading2);
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_CHAT_INSTAMP
			|| LOWORD(wParam) == IDC_CHAT_OUTSTAMP
			|| LOWORD(wParam) == IDC_CHAT_TIMESTAMP
			|| LOWORD(wParam) == IDC_CHAT_LOGLIMIT
			|| LOWORD(wParam) == IDC_CHAT_HIGHLIGHTWORDS
			|| LOWORD(wParam) == IDC_CHAT_LOGDIRECTORY
			|| LOWORD(wParam) == IDC_CHAT_LOGTIMESTAMP
			|| LOWORD(wParam) == IDC_CHAT_LIMIT)
			&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))	return 0;

		switch (LOWORD(wParam)) {
		case IDC_CHAT_LOGGING:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_LOGDIRECTORY), IsDlgButtonChecked(hwndDlg, IDC_CHAT_LOGGING) == BST_CHECKED ? TRUE : FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_LOGDIRCHOOSE), IsDlgButtonChecked(hwndDlg, IDC_CHAT_LOGGING) == BST_CHECKED ? TRUE : FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIMIT), IsDlgButtonChecked(hwndDlg, IDC_CHAT_LOGGING) == BST_CHECKED ? TRUE : FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIMITTEXT2), IsDlgButtonChecked(hwndDlg, IDC_CHAT_LOGGING) == BST_CHECKED ? TRUE : FALSE);
			break;

		case IDC_CHAT_HIGHLIGHT:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_HIGHLIGHTWORDS), IsDlgButtonChecked(hwndDlg, IDC_CHAT_HIGHLIGHT) == BST_CHECKED ? TRUE : FALSE);
			break;

		case IDC_CHAT_LOGDIRCHOOSE:
			LPMALLOC psMalloc;
			if (SUCCEEDED(CoGetMalloc(1, &psMalloc))) {
				wchar_t tszDirectory[MAX_PATH], tszTemp[MAX_PATH];
				BROWSEINFO bi = {};
				bi.hwndOwner = hwndDlg;
				bi.pszDisplayName = tszDirectory;
				bi.lpszTitle = TranslateT("Select folder");
				bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_RETURNONLYFSDIRS;
				bi.lpfn = BrowseCallbackProc;
				bi.lParam = (LPARAM)tszDirectory;

				LPITEMIDLIST idList = SHBrowseForFolder(&bi);
				if (idList) {
					SHGetPathFromIDList(idList, tszDirectory);
					mir_wstrcat(tszDirectory, L"\\");
					PathToRelativeW(tszDirectory, tszTemp);
					SetDlgItemText(hwndDlg, IDC_CHAT_LOGDIRECTORY, mir_wstrlen(tszTemp) > 1 ? tszTemp : DEFLOGFILENAME);
				}
				psMalloc->Free(idList);
				psMalloc->Release();
			}
			break;
		}

		if (lParam != 0)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case UM_CHECKSTATECHANGE:
		{
			TVITEM tvi = { 0 };
			tvi.mask = TVIF_HANDLE | TVIF_STATE;
			tvi.hItem = (HTREEITEM)lParam;
			TreeView_GetItem((HWND)wParam, &tvi);
			if (tvi.hItem == hListHeading2)
				CheckBranches(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading2);
			else
				PostMessage(hwndDlg, OPT_FIXHEADINGS, 0, 0);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_CHAT_CHECKBOXES) {
			if (((LPNMHDR)lParam)->code == NM_CLICK) {
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti))
					if (hti.flags & TVHT_ONITEMSTATEICON)
						SendMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom, (LPARAM)hti.hItem);
			}
			else if (((LPNMHDR)lParam)->code == TVN_KEYDOWN) {
				if (((LPNMTVKEYDOWN)lParam)->wVKey == VK_SPACE)
					SendMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom, (LPARAM)TreeView_GetSelection(((LPNMHDR)lParam)->hwndFrom));
			}
		}
		else if (((LPNMHDR)lParam)->idFrom == 0) {
			switch (((LPNMHDR)lParam)->code) {
			case PSN_WIZFINISH:
				g_chatApi.ReloadSettings();
				Chat_UpdateOptions();
				break;

			case PSN_APPLY:
				char *pszText = nullptr;
				int iLen;

				iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_HIGHLIGHTWORDS));
				if (iLen > 0) {
					wchar_t *ptszText = (wchar_t*)mir_alloc((iLen + 2) * sizeof(wchar_t));
					wchar_t *p2 = nullptr;

					if (ptszText) {
						GetDlgItemText(hwndDlg, IDC_CHAT_HIGHLIGHTWORDS, ptszText, iLen + 1);
						p2 = wcschr(ptszText, (wchar_t)',');
						while (p2) {
							*p2 = ' ';
							p2 = wcschr(ptszText, (wchar_t)',');
						}
						db_set_ws(0, CHAT_MODULE, "HighlightWords", ptszText);
						mir_free(ptszText);
					}
				}
				else db_unset(0, CHAT_MODULE, "HighlightWords");

				iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_LOGDIRECTORY));
				if (iLen > 0) {
					wchar_t *pszText1 = (wchar_t*)malloc(iLen * sizeof(wchar_t) + 2);
					GetDlgItemText(hwndDlg, IDC_CHAT_LOGDIRECTORY, pszText1, iLen + 1);
					db_set_ws(0, CHAT_MODULE, "LogDirectory", pszText1);
					free(pszText1);
				}
				else {
					mir_wstrncpy(g_Settings.pszLogDir, DEFLOGFILENAME, MAX_PATH);
					db_unset(0, CHAT_MODULE, "LogDirectory");
				}
				g_chatApi.SM_InvalidateLogDirectories();

				iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_LOGTIMESTAMP));
				if (iLen > 0) {
					pszText = (char*)mir_realloc(pszText, iLen + 1);
					GetDlgItemTextA(hwndDlg, IDC_CHAT_LOGTIMESTAMP, pszText, iLen + 1);
					db_set_s(0, CHAT_MODULE, "LogTimestamp", pszText);
				}
				else db_unset(0, CHAT_MODULE, "LogTimestamp");

				iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_TIMESTAMP));
				if (iLen > 0) {
					pszText = (char*)mir_realloc(pszText, iLen + 1);
					GetDlgItemTextA(hwndDlg, IDC_CHAT_TIMESTAMP, pszText, iLen + 1);
					db_set_s(0, CHAT_MODULE, "HeaderTime", pszText);
				}
				else db_unset(0, CHAT_MODULE, "HeaderTime");

				iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_INSTAMP));
				if (iLen > 0) {
					pszText = (char*)mir_realloc(pszText, iLen + 1);
					GetDlgItemTextA(hwndDlg, IDC_CHAT_INSTAMP, pszText, iLen + 1);
					db_set_s(0, CHAT_MODULE, "HeaderIncoming", pszText);
				}
				else db_unset(0, CHAT_MODULE, "HeaderIncoming");

				iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_OUTSTAMP));
				if (iLen > 0) {
					pszText = (char*)mir_realloc(pszText, iLen + 1);
					GetDlgItemTextA(hwndDlg, IDC_CHAT_OUTSTAMP, pszText, iLen + 1);
					db_set_s(0, CHAT_MODULE, "HeaderOutgoing", pszText);
				}
				else db_unset(0, CHAT_MODULE, "HeaderOutgoing");

				g_Settings.bHighlightEnabled = IsDlgButtonChecked(hwndDlg, IDC_CHAT_HIGHLIGHT) == BST_CHECKED ? TRUE : FALSE;
				db_set_b(0, CHAT_MODULE, "HighlightEnabled", (uint8_t)g_Settings.bHighlightEnabled);

				g_Settings.bLoggingEnabled = IsDlgButtonChecked(hwndDlg, IDC_CHAT_LOGGING) == BST_CHECKED ? TRUE : FALSE;
				db_set_b(0, CHAT_MODULE, "LoggingEnabled", (uint8_t)g_Settings.bLoggingEnabled);

				iLen = SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN2, UDM_GETPOS, 0, 0);
				db_set_w(0, CHAT_MODULE, "LogLimit", (uint16_t)iLen);
				iLen = SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN3, UDM_GETPOS, 0, 0);
				db_set_w(0, CHAT_MODULE, "LoggingLimit", (uint16_t)iLen);

				SaveBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), branch2, _countof(branch2));

				mir_free(pszText);

				g_Settings.dwIconFlags = db_get_dw(0, CHAT_MODULE, "IconFlags", 0x0000);
				g_Settings.dwTrayIconFlags = db_get_dw(0, CHAT_MODULE, "TrayIconFlags", 0x1000);
				g_Settings.dwPopupFlags = db_get_dw(0, CHAT_MODULE, "PopupFlags", 0x0000);
				g_Settings.bStripFormat = db_get_b(0, CHAT_MODULE, "TrimFormatting", 0) != 0;
				g_Settings.bLogIndentEnabled = (db_get_b(0, CHAT_MODULE, "LogIndentEnabled", 1) != 0) ? TRUE : FALSE;

				g_chatApi.MM_FontsChanged();
				return TRUE;
			}
		}
		break;

	case WM_TIMER:
		if (IsWindow(hPathTip))
			KillTimer(hPathTip, 4); // It will prevent tooltip autoclosing
		break;

	case WM_DESTROY:
		if (hPathTip) {
			KillTimer(hwndDlg, 0);
			DestroyWindow(hPathTip);
			hPathTip = nullptr;
		}

		uint8_t b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading2, TVIS_EXPANDED) & TVIS_EXPANDED ? 1 : 0;
		db_set_b(0, CHAT_MODULE, "Branch2Exp", b);
		break;
	}
	return FALSE;
}

void ChatOptInitialize(WPARAM wParam)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 910000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = LPGEN("Group chats");

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS1);
	odp.pfnDlgProc = DlgProcOptions1;
	odp.szTab.a = LPGEN("General");
	g_plugin.addOptions(wParam, &odp);

	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS2);
	odp.pfnDlgProc = DlgProcOptions2;
	odp.szTab.a = LPGEN("Log formatting");
	g_plugin.addOptions(wParam, &odp);
}
