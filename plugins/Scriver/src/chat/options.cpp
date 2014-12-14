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

#include "../commonheaders.h"

#define UM_CHECKSTATECHANGE (WM_USER+100)

static HWND hPathTip = 0;

struct branch_t
{
	TCHAR*    szDescr;
	char*     szDBName;
	int       iMode;
	BYTE      bDefault;
	HTREEITEM hItem;
};

static struct branch_t branch1[] = {
	{ LPGENT("Flash when someone speaks"), "FlashWindow", 0, 0, NULL },
	{ LPGENT("Flash when a word is highlighted"), "FlashWindowHighlight", 0, 1, NULL },
	{ LPGENT("Show chat nick list"), "ShowNicklist", 0, 1, NULL },
	{ LPGENT("Enable button context menus"), "RightClickFilter", 0, 0, NULL },
	{ LPGENT("Show topic on your contact list (if supported)"), "TopicOnClist", 0, 0, NULL },
	{ LPGENT("Do not play sounds when focused"), "SoundsFocus", 0, 0, NULL },
	{ LPGENT("Do not pop up when joining"), "PopupOnJoin", 0, 0, NULL },
	{ LPGENT("Show and hide by double clicking in the contact list"), "ToggleVisibility", 0, 0, NULL },
	{ LPGENT("Show contact statuses (if supported)"), "ShowContactStatus", 0, 0, NULL },
	{ LPGENT("Display contact status icon before role icon"), "ContactStatusFirst", 0, 0, NULL },
	{ LPGENT("Add ':' to auto-completed names"), "AddColonToAutoComplete", 0, 1, NULL }
};

static struct branch_t branch2[] = {
	{ LPGENT("Show icons"), "IconFlags", GC_EVENT_TOPIC | GC_EVENT_JOIN | GC_EVENT_QUIT |
		GC_EVENT_MESSAGE | GC_EVENT_ACTION | GC_EVENT_HIGHLIGHT | GC_EVENT_PART |
		GC_EVENT_KICK | GC_EVENT_NOTICE | GC_EVENT_NICK | GC_EVENT_INFORMATION | GC_EVENT_ADDSTATUS, 0, NULL },
	{ LPGENT("Prefix all events with a timestamp"), "ShowTimeStamp", 0, 1, NULL },
	{ LPGENT("Only prefix with timestamp if it has changed"), "ShowTimeStampIfChanged", 0, 0, NULL },
	{ LPGENT("Timestamp has same color as event"), "TimeStampEventColour", 0, 0, NULL },
	{ LPGENT("Indent the second line of a message"), "LogIndentEnabled", 0, 1, NULL },
	{ LPGENT("Limit user names to 20 characters"), "LogLimitNames", 0, 1, NULL },
	{ LPGENT("Strip colors from messages"), "StripFormatting", 0, 0, NULL },
	{ LPGENT("Enable 'event filter' for new rooms"), "FilterEnabled", 0, 0, NULL }
};

static struct branch_t branch3[] = {
	{ LPGENT("Show topic changes"), "FilterFlags", GC_EVENT_TOPIC, 0, NULL },
	{ LPGENT("Show users joining"), "FilterFlags", GC_EVENT_JOIN, 0, NULL },
	{ LPGENT("Show users disconnecting"), "FilterFlags", GC_EVENT_QUIT, 0, NULL },
	{ LPGENT("Show messages"), "FilterFlags", GC_EVENT_MESSAGE, 1, NULL },
	{ LPGENT("Show actions"), "FilterFlags", GC_EVENT_ACTION, 1, NULL },
	{ LPGENT("Show users leaving"), "FilterFlags", GC_EVENT_PART, 0, NULL },
	{ LPGENT("Show users being kicked"), "FilterFlags", GC_EVENT_KICK, 1, NULL },
	{ LPGENT("Show notices"), "FilterFlags", GC_EVENT_NOTICE, 1, NULL },
	{ LPGENT("Show users changing name"), "FilterFlags", GC_EVENT_NICK, 0, NULL },
	{ LPGENT("Show information messages"), "FilterFlags", GC_EVENT_INFORMATION, 1, NULL },
	{ LPGENT("Show status changes of users"), "FilterFlags", GC_EVENT_ADDSTATUS, 0, NULL },
};

static struct branch_t branch4[] = {
	{ LPGENT("Show icons in tray only when the chat room is not active"), "TrayIconInactiveOnly", 0, 1, NULL },
	{ LPGENT("Show icon in tray for topic changes"), "TrayIconFlags", GC_EVENT_TOPIC, 0, NULL },
	{ LPGENT("Show icon in tray for users joining"), "TrayIconFlags", GC_EVENT_JOIN, 0, NULL },
	{ LPGENT("Show icon in tray for users disconnecting"), "TrayIconFlags", GC_EVENT_QUIT, 0, NULL },
	{ LPGENT("Show icon in tray for messages"), "TrayIconFlags", GC_EVENT_MESSAGE, 0, NULL },
	{ LPGENT("Show icon in tray for actions"), "TrayIconFlags", GC_EVENT_ACTION, 0, NULL },
	{ LPGENT("Show icon in tray for highlights"), "TrayIconFlags", GC_EVENT_HIGHLIGHT, 1, NULL },
	{ LPGENT("Show icon in tray for users leaving"), "TrayIconFlags", GC_EVENT_PART, 0, NULL },
	{ LPGENT("Show icon in tray for users kicking other user"), "TrayIconFlags", GC_EVENT_KICK, 0, NULL },
	{ LPGENT("Show icon in tray for notices"), "TrayIconFlags", GC_EVENT_NOTICE, 0, NULL },
	{ LPGENT("Show icon in tray for name changes"), "TrayIconFlags", GC_EVENT_NICK, 0, NULL },
	{ LPGENT("Show icon in tray for information messages"), "TrayIconFlags", GC_EVENT_INFORMATION, 0, NULL },
	{ LPGENT("Show icon in tray for status changes"), "TrayIconFlags", GC_EVENT_ADDSTATUS, 0, NULL },
};

static struct branch_t branch6[] = {
	{ LPGENT("Show popups only when the chat room is not active"), "PopupInactiveOnly", 0, 1, NULL },
	{ LPGENT("Show popup for topic changes"), "PopupFlags", GC_EVENT_TOPIC, 0, NULL },
	{ LPGENT("Show popup for users joining"), "PopupFlags", GC_EVENT_JOIN, 0, NULL },
	{ LPGENT("Show popup for users disconnecting"), "PopupFlags", GC_EVENT_QUIT, 0, NULL },
	{ LPGENT("Show popup for messages"), "PopupFlags", GC_EVENT_MESSAGE, 0, NULL },
	{ LPGENT("Show popup for actions"), "PopupFlags", GC_EVENT_ACTION, 0, NULL },
	{ LPGENT("Show popup for highlights"), "PopupFlags", GC_EVENT_HIGHLIGHT, 0, NULL },
	{ LPGENT("Show popup for users leaving"), "PopupFlags", GC_EVENT_PART, 0, NULL },
	{ LPGENT("Show popup for users kicking other user"), "PopupFlags", GC_EVENT_KICK, 0, NULL },
	{ LPGENT("Show popup for notices"), "PopupFlags", GC_EVENT_NOTICE, 0, NULL },
	{ LPGENT("Show popup for name changes"), "PopupFlags", GC_EVENT_NICK, 0, NULL },
	{ LPGENT("Show popup for information messages"), "PopupFlags", GC_EVENT_INFORMATION, 0, NULL },
	{ LPGENT("Show popup for status changes"), "PopupFlags", GC_EVENT_ADDSTATUS, 0, NULL },
};

static HTREEITEM InsertBranch(HWND hwndTree, TCHAR* pszDescr, BOOL bExpanded)
{
	TVINSERTSTRUCT tvis;
	tvis.hParent = NULL;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_STATE;
	tvis.item.pszText = TranslateTS(pszDescr);
	tvis.item.stateMask = bExpanded ? TVIS_STATEIMAGEMASK | TVIS_EXPANDED : TVIS_STATEIMAGEMASK;
	tvis.item.state = bExpanded ? INDEXTOSTATEIMAGEMASK(1) | TVIS_EXPANDED : INDEXTOSTATEIMAGEMASK(1);
	return TreeView_InsertItem(hwndTree, &tvis);
}

static void FillBranch(HWND hwndTree, HTREEITEM hParent, struct branch_t *branch, int nValues, DWORD defaultval)
{
	int iState;

	TVINSERTSTRUCT tvis;
	tvis.hParent = hParent;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_STATE;
	for (int i = 0; i < nValues; i++) {
		tvis.item.pszText = TranslateTS(branch[i].szDescr);
		tvis.item.stateMask = TVIS_STATEIMAGEMASK;
		if (branch[i].iMode)
			iState = ((db_get_dw(NULL, CHAT_MODULE, branch[i].szDBName, defaultval)&branch[i].iMode)&branch[i].iMode) != 0 ? 2 : 1;
		else
			iState = db_get_b(NULL, CHAT_MODULE, branch[i].szDBName, branch[i].bDefault) != 0 ? 2 : 1;
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
		BYTE bChecked = ((tvi.state&TVIS_STATEIMAGEMASK) >> 12 == 1) ? 0 : 1;
		if (branch[i].iMode) {
			if (bChecked)
				iState |= branch[i].iMode;
			if (iState&GC_EVENT_ADDSTATUS)
				iState |= GC_EVENT_REMOVESTATUS;
			db_set_dw(NULL, CHAT_MODULE, branch[i].szDBName, (DWORD)iState);
		}
		else db_set_b(NULL, CHAT_MODULE, branch[i].szDBName, bChecked);
	}
}

static void CheckHeading(HWND hwndTree, HTREEITEM hHeading)
{
	BOOL bChecked = TRUE;

	if (hHeading == 0)
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

	if (hHeading == 0)
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

static void InitSetting(TCHAR **ppPointer, char *pszSetting, TCHAR *pszDefault)
{
	DBVARIANT dbv;
	if ( !db_get_ts(NULL, CHAT_MODULE, pszSetting, &dbv )) {
		replaceStrT(*ppPointer, dbv.ptszVal);
		db_free(&dbv);
	}
	else replaceStrT(*ppPointer, pszDefault);
}

#define OPT_FIXHEADINGS (WM_USER+1)

INT_PTR CALLBACK DlgProcOptions1(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static HTREEITEM hListHeading1 = 0;
	static HTREEITEM hListHeading4= 0;
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_CHAT_CHECKBOXES),GWL_STYLE,GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_CHAT_CHECKBOXES),GWL_STYLE)|TVS_NOHSCROLL|TVS_CHECKBOXES);
		SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN2,UDM_SETRANGE,0,MAKELONG(255,10));
		SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN2,UDM_SETPOS,0,MAKELONG(db_get_b(NULL,CHAT_MODULE,"NicklistRowDist",12),0));
		hListHeading1 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), TranslateT("Appearance and functionality of chat windows"), db_get_b(NULL, CHAT_MODULE, "Branch1Exp", 0)?TRUE:FALSE);
		hListHeading4 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), TranslateT("Icons to display in the tray"), db_get_b(NULL, CHAT_MODULE, "Branch5Exp", 0)?TRUE:FALSE);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading1, branch1, SIZEOF(branch1), 0);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading4, branch4, SIZEOF(branch4), 0x1000);
		SendMessage(hwndDlg, OPT_FIXHEADINGS, 0, 0);
		{
			TCHAR* pszGroup = NULL;
			InitSetting(&pszGroup, "AddToGroup", _T("Chat rooms"));
			SetDlgItemText(hwndDlg, IDC_CHAT_GROUP, pszGroup);
			mir_free(pszGroup);
		}
		break;

	case OPT_FIXHEADINGS:
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading1);
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading4);
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_CHAT_NICKROW || LOWORD(wParam) == IDC_CHAT_GROUP) && (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()))
			return 0;

		if (lParam != 0)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case UM_CHECKSTATECHANGE:
		{
			TVITEM tvi = {0};
			tvi.mask=TVIF_HANDLE|TVIF_STATE;
			tvi.hItem=(HTREEITEM) lParam;
			TreeView_GetItem((HWND)wParam,&tvi);
			if (tvi.hItem == hListHeading1)
				CheckBranches(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading1);
			else if (tvi.hItem == hListHeading4)
				CheckBranches(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading4);
			else
				PostMessage(hwndDlg, OPT_FIXHEADINGS, 0, 0);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case IDC_CHAT_CHECKBOXES:
			if (((LPNMHDR)lParam)->code==NM_CLICK) {
				TVHITTESTINFO hti;
				hti.pt.x=(short)LOWORD(GetMessagePos());
				hti.pt.y=(short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom,&hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom,&hti))
					if (hti.flags&TVHT_ONITEMSTATEICON) {
						SendMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom, (LPARAM)hti.hItem);
					}

			}
			else if (((LPNMHDR)lParam)->code == TVN_KEYDOWN) {
				if (((LPNMTVKEYDOWN) lParam)->wVKey == VK_SPACE)
					SendMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom,
						(LPARAM)TreeView_GetSelection(((LPNMHDR)lParam)->hwndFrom));
			}
			break;

		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				int iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_GROUP));
				if (iLen > 0) {
					ptrA pszText((char*)mir_alloc(iLen + 1));
					GetDlgItemTextA(hwndDlg, IDC_CHAT_GROUP, pszText, iLen + 1);
					db_set_s(NULL, CHAT_MODULE, "AddToGroup", pszText);
				}
				else db_set_s(NULL, CHAT_MODULE, "AddToGroup", "");

				iLen = SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN2, UDM_GETPOS, 0, 0);
				if (iLen > 0)
					db_set_b(NULL, CHAT_MODULE, "NicklistRowDist", (BYTE)iLen);
				else
					db_unset(NULL, CHAT_MODULE, "NicklistRowDist");
				SaveBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), branch1, SIZEOF(branch1));
				SaveBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), branch4, SIZEOF(branch4));

				pci->ReloadSettings();
				pci->SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);
			}
			return TRUE;
		}
		break;

	case WM_DESTROY:
		BYTE b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading1, TVIS_EXPANDED)&TVIS_EXPANDED?1:0;
		db_set_b(NULL, CHAT_MODULE, "Branch1Exp", b);
		b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading4, TVIS_EXPANDED)&TVIS_EXPANDED?1:0;
		db_set_b(NULL, CHAT_MODULE, "Branch5Exp", b);
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK DlgProcOptions2(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static HTREEITEM hListHeading2= 0;
	static HTREEITEM hListHeading3= 0;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_CHAT_CHECKBOXES),GWL_STYLE,GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_CHAT_CHECKBOXES),GWL_STYLE)|TVS_NOHSCROLL|TVS_CHECKBOXES);
		SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN2,UDM_SETRANGE,0,MAKELONG(5000,0));
		SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN2,UDM_SETPOS,0,MAKELONG(db_get_w(NULL,CHAT_MODULE,"LogLimit",100),0));
		SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN3,UDM_SETRANGE,0,MAKELONG(10000,0));
		SendDlgItemMessage(hwndDlg,IDC_CHAT_SPIN3,UDM_SETPOS,0,MAKELONG(db_get_w(NULL,CHAT_MODULE,"LoggingLimit",100),0));
		{
			TCHAR tszTemp[MAX_PATH];
			PathToRelativeT(g_Settings.pszLogDir, tszTemp);
			SetDlgItemText(hwndDlg, IDC_CHAT_LOGDIRECTORY, tszTemp);
		}
		if (ServiceExists(MS_UTILS_REPLACEVARS)) {
			TCHAR tszTooltipText[2048];
			RECT rect;

			mir_sntprintf(tszTooltipText, SIZEOF(tszTooltipText),
				_T("%s - %s\n%s - %s\n%s - %s\n%s - %s\n\n")
				_T("%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n\n")
				_T("%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s"),
				// contact vars
				_T("%nick%"),					TranslateT("nick of current contact (if defined)"),
				_T("%proto%"),					TranslateT("protocol name of current contact (if defined). Account name is used when protocol supports multiple accounts"),
				_T("%accountname%"),			TranslateT("user-defined account name of current contact (if defined)."),
				_T("%userid%"),					TranslateT("user ID of current contact (if defined). It is like UIN Number for ICQ, JID for Jabber, etc."),
				// global vars
				_T("%miranda_path%"),			TranslateT("path to Miranda root folder"),
				_T("%miranda_profilesdir%"),		TranslateT("path to folder containing Miranda profiles"),
				_T("%miranda_profilename%"),	TranslateT("name of current Miranda profile (filename, without extension)"),
				_T("%miranda_userdata%"),		TranslateT("will return parsed string %miranda_profilesdir%\\%miranda_profilename%"),
				_T("%miranda_logpath%"),		TranslateT("will return parsed string %miranda_userdata%\\Logs"),
				_T("%appdata%"),				TranslateT("same as environment variable %APPDATA% for currently logged-on Windows user"),
				_T("%username%"),				TranslateT("username for currently logged-on Windows user"),
				_T("%mydocuments%"),			TranslateT("\"My Documents\" folder for currently logged-on Windows user"),
				_T("%desktop%"),				TranslateT("\"Desktop\" folder for currently logged-on Windows user"),
				_T("%xxxxxxx%"),				TranslateT("any environment variable defined in current Windows session (like %systemroot%, %allusersprofile%, etc.)"),
				// date/time vars
				_T("%d%"),			TranslateT("day of month, 1-31"),
				_T("%dd%"),			TranslateT("day of month, 01-31"),
				_T("%m%"),			TranslateT("month number, 1-12"),
				_T("%mm%"),			TranslateT("month number, 01-12"),
				_T("%mon%"),		TranslateT("abbreviated month name"),
				_T("%month%"),		TranslateT("full month name"),
				_T("%yy%"),			TranslateT("year without century, 01-99"),
				_T("%yyyy%"),		TranslateT("year with century, 1901-9999"),
				_T("%wday%"),		TranslateT("abbreviated weekday name"),
				_T("%weekday%"),	TranslateT("full weekday name"));
			GetClientRect (GetDlgItem(hwndDlg, IDC_CHAT_LOGDIRECTORY), &rect);
			rect.left = -85;
			hPathTip = CreateToolTip(GetDlgItem(hwndDlg, IDC_CHAT_LOGDIRECTORY), tszTooltipText, TranslateT("Variables"), &rect);
			SetTimer(hwndDlg, 0, 3000, NULL);
		}

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

		hListHeading2 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), TranslateT("Appearance"), db_get_b(NULL, CHAT_MODULE, "Branch2Exp", 0) ? TRUE : FALSE);
		hListHeading3 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), TranslateT("Default events to show in new chat rooms if the 'event filter' is enabled"), db_get_b(NULL, CHAT_MODULE, "Branch3Exp", 0) ? TRUE : FALSE);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading2, branch2, SIZEOF(branch2), 0x0);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading3, branch3, SIZEOF(branch3), 0x03E0);
		SendMessage(hwndDlg, OPT_FIXHEADINGS, 0, 0);
		break;

	case OPT_FIXHEADINGS:
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading2);
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading3);
		break;

	case WM_COMMAND:
		if (	(LOWORD(wParam)		  == IDC_CHAT_INSTAMP
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
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_LOGDIRECTORY), IsDlgButtonChecked(hwndDlg, IDC_CHAT_LOGGING) == BST_CHECKED?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_LOGDIRCHOOSE), IsDlgButtonChecked(hwndDlg, IDC_CHAT_LOGGING) == BST_CHECKED?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIMIT), IsDlgButtonChecked(hwndDlg, IDC_CHAT_LOGGING) == BST_CHECKED?TRUE:FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_LIMITTEXT2), IsDlgButtonChecked(hwndDlg, IDC_CHAT_LOGGING) == BST_CHECKED?TRUE:FALSE);
			break;

		case IDC_CHAT_HIGHLIGHT:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_HIGHLIGHTWORDS), IsDlgButtonChecked(hwndDlg, IDC_CHAT_HIGHLIGHT) == BST_CHECKED ? TRUE : FALSE);
			break;

		case IDC_CHAT_LOGDIRCHOOSE:
			LPMALLOC psMalloc;
			if (SUCCEEDED(CoGetMalloc(1, &psMalloc))) {
				TCHAR tszDirectory[MAX_PATH], tszTemp[MAX_PATH];
				BROWSEINFO bi = { 0 };
				bi.hwndOwner = hwndDlg;
				bi.pszDisplayName = tszDirectory;
				bi.lpszTitle = TranslateT("Select Folder");
				bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_RETURNONLYFSDIRS;
				bi.lpfn = BrowseCallbackProc;
				bi.lParam = (LPARAM)tszDirectory;

				LPITEMIDLIST idList = SHBrowseForFolder(&bi);
				if (idList) {
					SHGetPathFromIDList(idList, tszDirectory);
					mir_tstrcat(tszDirectory, _T("\\"));
					PathToRelativeT(tszDirectory, tszTemp);
					SetDlgItemText(hwndDlg, IDC_CHAT_LOGDIRECTORY, mir_tstrlen(tszTemp) > 1 ? tszTemp : DEFLOGFILENAME);
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
			TVITEM tvi = {0};
			tvi.mask=TVIF_HANDLE|TVIF_STATE;
			tvi.hItem=(HTREEITEM) lParam;
			TreeView_GetItem((HWND)wParam,&tvi);
			if (tvi.hItem == hListHeading2)
				CheckBranches(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading2);
			else if (tvi.hItem == hListHeading3)
				CheckBranches(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading3);
			else
				PostMessage(hwndDlg, OPT_FIXHEADINGS, 0, 0);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_CHAT_CHECKBOXES) {
			if (((LPNMHDR)lParam)->code==NM_CLICK) {
				TVHITTESTINFO hti;
				hti.pt.x=(short)LOWORD(GetMessagePos());
				hti.pt.y=(short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom,&hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom,&hti))
					if (hti.flags&TVHT_ONITEMSTATEICON) {
						SendMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom, (LPARAM)hti.hItem);
					}
			}
			else if (((LPNMHDR) lParam)->code == TVN_KEYDOWN) {
				if (((LPNMTVKEYDOWN) lParam)->wVKey == VK_SPACE) {
					SendMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom,
						(LPARAM)TreeView_GetSelection(((LPNMHDR)lParam)->hwndFrom));
				}
			}
		}
		else if (((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_APPLY) {
			char *pszText = NULL;
			int iLen;

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_HIGHLIGHTWORDS));
			if (iLen > 0) {
				TCHAR *ptszText = (TCHAR*)mir_alloc((iLen + 2) * sizeof(TCHAR));
				TCHAR *p2 = NULL;

				if (ptszText) {
					GetDlgItemText(hwndDlg, IDC_CHAT_HIGHLIGHTWORDS, ptszText, iLen + 1);
					p2 = _tcschr(ptszText, (TCHAR)',');
					while (p2) {
						*p2 = ' ';
						p2 = _tcschr(ptszText, (TCHAR)',');
					}
					db_set_ts(NULL, CHAT_MODULE, "HighlightWords", ptszText);
					mir_free(ptszText);
				}
			}
			else db_unset(NULL, CHAT_MODULE, "HighlightWords");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_LOGDIRECTORY));
			if (iLen > 0) {
				TCHAR *pszText1 = (TCHAR*)malloc(iLen*sizeof(TCHAR)+2);
				GetDlgItemText(hwndDlg, IDC_CHAT_LOGDIRECTORY, pszText1, iLen + 1);
				db_set_ts(NULL, CHAT_MODULE, "LogDirectory", pszText1);
				free(pszText1);
			}
			else {
				mir_tstrncpy(g_Settings.pszLogDir, DEFLOGFILENAME, MAX_PATH);
				db_unset(NULL, CHAT_MODULE, "LogDirectory");
			}
			pci->SM_InvalidateLogDirectories();

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_LOGTIMESTAMP));
			if (iLen > 0) {
				pszText = (char*)mir_realloc(pszText, iLen + 1);
				GetDlgItemTextA(hwndDlg, IDC_CHAT_LOGTIMESTAMP, pszText, iLen + 1);
				db_set_s(NULL, CHAT_MODULE, "LogTimestamp", pszText);
			}
			else db_unset(NULL, CHAT_MODULE, "LogTimestamp");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_TIMESTAMP));
			if (iLen > 0) {
				pszText = (char*)mir_realloc(pszText, iLen + 1);
				GetDlgItemTextA(hwndDlg, IDC_CHAT_TIMESTAMP, pszText, iLen + 1);
				db_set_s(NULL, CHAT_MODULE, "HeaderTime", pszText);
			}
			else db_unset(NULL, CHAT_MODULE, "HeaderTime");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_INSTAMP));
			if (iLen > 0) {
				pszText = (char*)mir_realloc(pszText, iLen + 1);
				GetDlgItemTextA(hwndDlg, IDC_CHAT_INSTAMP, pszText, iLen + 1);
				db_set_s(NULL, CHAT_MODULE, "HeaderIncoming", pszText);
			}
			else db_unset(NULL, CHAT_MODULE, "HeaderIncoming");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_CHAT_OUTSTAMP));
			if (iLen > 0) {
				pszText = (char*)mir_realloc(pszText, iLen + 1);
				GetDlgItemTextA(hwndDlg, IDC_CHAT_OUTSTAMP, pszText, iLen + 1);
				db_set_s(NULL, CHAT_MODULE, "HeaderOutgoing", pszText);
			}
			else db_unset(NULL, CHAT_MODULE, "HeaderOutgoing");

			g_Settings.bHighlightEnabled = IsDlgButtonChecked(hwndDlg, IDC_CHAT_HIGHLIGHT) == BST_CHECKED ? TRUE : FALSE;
			db_set_b(NULL, CHAT_MODULE, "HighlightEnabled", (BYTE)g_Settings.bHighlightEnabled);

			g_Settings.bLoggingEnabled = IsDlgButtonChecked(hwndDlg, IDC_CHAT_LOGGING) == BST_CHECKED ? TRUE : FALSE;
			db_set_b(NULL, CHAT_MODULE, "LoggingEnabled", (BYTE)g_Settings.bLoggingEnabled);

			iLen = SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN2, UDM_GETPOS, 0, 0);
			db_set_w(NULL, CHAT_MODULE, "LogLimit", (WORD)iLen);
			iLen = SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN3, UDM_GETPOS, 0, 0);
			db_set_w(NULL, CHAT_MODULE, "LoggingLimit", (WORD)iLen);

			SaveBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), branch2, SIZEOF(branch2));
			SaveBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), branch3, SIZEOF(branch3));

			mir_free(pszText);

			g_Settings.dwIconFlags = db_get_dw(NULL, CHAT_MODULE, "IconFlags", 0x0000);
			g_Settings.dwTrayIconFlags = db_get_dw(NULL, CHAT_MODULE, "TrayIconFlags", 0x1000);
			g_Settings.dwPopupFlags = db_get_dw(NULL, CHAT_MODULE, "PopupFlags", 0x0000);
			g_Settings.bStripFormat = db_get_b(NULL, CHAT_MODULE, "TrimFormatting", 0) != 0;
			g_Settings.bTrayIconInactiveOnly = db_get_b(NULL, CHAT_MODULE, "TrayIconInactiveOnly", 1) != 0;
			g_Settings.bPopupInactiveOnly = db_get_b(NULL, CHAT_MODULE, "PopUpInactiveOnly", 1) != 0;
			g_Settings.bLogIndentEnabled = (db_get_b(NULL, CHAT_MODULE, "LogIndentEnabled", 1) != 0) ? TRUE : FALSE;

			pci->MM_FontsChanged();
			pci->SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);
			return TRUE;
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
			hPathTip = 0;
		}

		BYTE b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading2, TVIS_EXPANDED)&TVIS_EXPANDED?1:0;
		db_set_b(NULL, CHAT_MODULE, "Branch2Exp", b);
		b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), hListHeading3, TVIS_EXPANDED)&TVIS_EXPANDED?1:0;
		db_set_b(NULL, CHAT_MODULE, "Branch3Exp", b);
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcOptionsPopup(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		SendDlgItemMessage(hwndDlg, IDC_CHAT_BKG, CPM_SETCOLOUR, 0, g_Settings.crPUBkgColour);
		SendDlgItemMessage(hwndDlg, IDC_CHAT_TEXT, CPM_SETCOLOUR, 0, g_Settings.crPUTextColour);

		if (g_Settings.iPopupStyle == 2)
			CheckDlgButton(hwndDlg, IDC_CHAT_RADIO2, BST_CHECKED);
		else if (g_Settings.iPopupStyle == 3)
			CheckDlgButton(hwndDlg, IDC_CHAT_RADIO3, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_CHAT_RADIO1, BST_CHECKED);

		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_BKG), IsDlgButtonChecked(hwndDlg, IDC_CHAT_RADIO3) == BST_CHECKED ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_TEXT), IsDlgButtonChecked(hwndDlg, IDC_CHAT_RADIO3) == BST_CHECKED ? TRUE : FALSE);

		SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN1, UDM_SETRANGE, 0, MAKELONG(100, -1));
		SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN1, UDM_SETPOS, 0, MAKELONG(g_Settings.iPopupTimeout, 0));
		FillBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), NULL, branch6, SIZEOF(branch6), 0x0000);
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_CHAT_TIMEOUT) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;

		if (lParam)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		switch (LOWORD(wParam)) {
		case IDC_CHAT_RADIO1:
		case IDC_CHAT_RADIO2:
		case IDC_CHAT_RADIO3:
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_BKG), IsDlgButtonChecked(hwndDlg, IDC_CHAT_RADIO3) == BST_CHECKED ? TRUE : FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHAT_TEXT), IsDlgButtonChecked(hwndDlg, IDC_CHAT_RADIO3) == BST_CHECKED ? TRUE : FALSE);
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
					if (hti.flags & TVHT_ONITEMSTATEICON)
						SendMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom, (LPARAM)hti.hItem);
			}
			else if (((LPNMHDR)lParam)->code == TVN_KEYDOWN) {
				if (((LPNMTVKEYDOWN)lParam)->wVKey == VK_SPACE)
					SendMessage(hwndDlg, UM_CHECKSTATECHANGE, (WPARAM)((LPNMHDR)lParam)->hwndFrom,
						(LPARAM)TreeView_GetSelection(((LPNMHDR)lParam)->hwndFrom));
			}
			break;

		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				int iLen;
				if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_RADIO2) == BST_CHECKED)
					iLen = 2;
				else if (IsDlgButtonChecked(hwndDlg, IDC_CHAT_RADIO3) == BST_CHECKED)
					iLen = 3;
				else
					iLen = 1;

				g_Settings.iPopupStyle = iLen;
				db_set_b(NULL, CHAT_MODULE, "PopupStyle", (BYTE)iLen);

				iLen = SendDlgItemMessage(hwndDlg, IDC_CHAT_SPIN1, UDM_GETPOS, 0, 0);
				g_Settings.iPopupTimeout = iLen;
				db_set_w(NULL, CHAT_MODULE, "PopupTimeout", (WORD)iLen);

				g_Settings.crPUBkgColour = SendDlgItemMessage(hwndDlg, IDC_CHAT_BKG, CPM_GETCOLOUR, 0, 0);
				db_set_dw(NULL, CHAT_MODULE, "PopupColorBG", (DWORD)SendDlgItemMessage(hwndDlg, IDC_CHAT_BKG, CPM_GETCOLOUR, 0, 0));
				g_Settings.crPUTextColour = SendDlgItemMessage(hwndDlg, IDC_CHAT_TEXT, CPM_GETCOLOUR, 0, 0);
				db_set_dw(NULL, CHAT_MODULE, "PopupColorText", (DWORD)SendDlgItemMessage(hwndDlg, IDC_CHAT_TEXT, CPM_GETCOLOUR, 0, 0));
				SaveBranch(GetDlgItem(hwndDlg, IDC_CHAT_CHECKBOXES), branch6, SIZEOF(branch6));
			}
			return TRUE;
		}
		break;

	case UM_CHECKSTATECHANGE:
		PostMessage(hwndDlg, OPT_FIXHEADINGS, 0, 0);
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	}
	return FALSE;
}

static int OptionsInitialize(WPARAM wParam, LPARAM)
{
	if (g_dat.popupInstalled) {
		OPTIONSDIALOGPAGE odp = { sizeof(odp) };
		odp.position = 910000002;
		odp.hInstance = g_hInst;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONSPOPUP);
		odp.pszTitle = LPGEN("Messaging");
		odp.pszGroup = LPGEN("Popups");
		odp.pfnDlgProc = DlgProcOptionsPopup;
		odp.flags = ODPF_BOLDGROUPS;
		Options_AddPage(wParam, &odp);
	}

	return 0;
}

int OptionsInit(void)
{
	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);
	return 0;
}
