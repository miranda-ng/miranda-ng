/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson

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

#include "resource.h"
#include "chat.h"

struct branch_t
{
	const TCHAR* szDescr;
	const char*  szDBName;
	int          iMode;
	BYTE         bDefault;
};

static const struct branch_t branch0[] = {
	{ LPGENT("Use a tabbed interface"), "Tabs", 0, 1 },
	{ LPGENT("Close tab on double click"), "TabCloseOnDblClick", 0, 0 },
	{ LPGENT("Restore previously open tabs when showing the window"), "TabRestore", 0, 0 },
	{ LPGENT("Show tabs at the bottom"), "TabBottom", 0, 0 },
};

static const struct branch_t branch1[] = {
	{ LPGENT("Send message by pressing the 'Enter' key"), "SendOnEnter", 0, 1 },
	{ LPGENT("Send message by pressing the 'Enter' key twice"), "SendOnDblEnter", 0, 0 },
	{ LPGENT("Flash window when someone speaks"), "FlashWindow", 0, 0 },
	{ LPGENT("Flash window when a word is highlighted"), "FlashWindowHighlight", 0, 1 },
	{ LPGENT("Show list of users in the chat room"), "ShowNicklist", 0, 1 },
	{ LPGENT("Show button for sending messages"), "ShowSend", 0, 0 },
	{ LPGENT("Show buttons for controlling the chat room"), "ShowTopButtons", 0, 1 },
	{ LPGENT("Show buttons for formatting the text you are typing"), "ShowFormatButtons", 0, 1 },
	{ LPGENT("Show button menus when right clicking the buttons"), "RightClickFilter", 0, 0 },
	{ LPGENT("Show new windows cascaded"), "CascadeWindows", 0, 1 },
	{ LPGENT("Save the size and position of chat rooms"), "SavePosition", 0, 0 },
	{ LPGENT("Show the topic of the room on your contact list (if supported)"), "TopicOnClist", 0, 0 },
	{ LPGENT("Do not play sounds when the chat room is focused"), "SoundsFocus", 0, 0 },
	{ LPGENT("Do not pop up the window when joining a chat room"), "PopupOnJoin", 0, 0 },
	{ LPGENT("Toggle the visible state when double clicking in the contact list"), "ToggleVisibility", 0, 0 },
	{ LPGENT("Show contact statuses if protocol supports them"), "ShowContactStatus", 0, 0 },
	{ LPGENT("Display contact status icon before user role icon"), "ContactStatusFirst", 0, 0 },
};

static const struct branch_t branch2[] = {
	{ LPGENT("Prefix all events with a timestamp"), "ShowTimeStamp", 0, 1 },
	{ LPGENT("Only prefix with timestamp if it has changed"), "ShowTimeStampIfChanged", 0, 0 },
	{ LPGENT("Timestamp has same color as the event"), "TimeStampEventColour", 0, 0 },
	{ LPGENT("Indent the second line of a message"), "LogIndentEnabled", 0, 1 },
	{ LPGENT("Limit user names in the message log to 20 characters"), "LogLimitNames", 0, 1 },
	{ LPGENT("Add ':' to auto-completed user names"), "AddColonToAutoComplete", 0, 1 },
	{ LPGENT("Strip colors from messages in the log"), "StripFormatting", 0, 0 },
	{ LPGENT("Enable the 'event filter' for new rooms"), "FilterEnabled", 0, 0 }
};

static const struct branch_t branch3[] = {
	{ LPGENT("Show topic changes"), "FilterFlags", GC_EVENT_TOPIC, 0 },
	{ LPGENT("Show users joining"), "FilterFlags", GC_EVENT_JOIN, 0 },
	{ LPGENT("Show users disconnecting"), "FilterFlags", GC_EVENT_QUIT, 0 },
	{ LPGENT("Show messages"), "FilterFlags", GC_EVENT_MESSAGE, 1 },
	{ LPGENT("Show actions"), "FilterFlags", GC_EVENT_ACTION, 1 },
	{ LPGENT("Show users leaving"), "FilterFlags", GC_EVENT_PART, 0 },
	{ LPGENT("Show users being kicked"), "FilterFlags", GC_EVENT_KICK, 1 },
	{ LPGENT("Show notices"), "FilterFlags", GC_EVENT_NOTICE, 1 },
	{ LPGENT("Show users changing name"), "FilterFlags", GC_EVENT_NICK, 0 },
	{ LPGENT("Show information messages"), "FilterFlags", GC_EVENT_INFORMATION, 1 },
	{ LPGENT("Show status changes of users"), "FilterFlags", GC_EVENT_ADDSTATUS, 0 },
};

static const struct branch_t branch4[] = {
	{ LPGENT("Show icon for topic changes"), "IconFlags", GC_EVENT_TOPIC, 0 },
	{ LPGENT("Show icon for users joining"), "IconFlags", GC_EVENT_JOIN, 1 },
	{ LPGENT("Show icon for users disconnecting"), "IconFlags", GC_EVENT_QUIT, 0 },
	{ LPGENT("Show icon for messages"), "IconFlags", GC_EVENT_MESSAGE, 0 },
	{ LPGENT("Show icon for actions"), "IconFlags", GC_EVENT_ACTION, 0 },
	{ LPGENT("Show icon for highlights"), "IconFlags", GC_EVENT_HIGHLIGHT, 0 },
	{ LPGENT("Show icon for users leaving"), "IconFlags", GC_EVENT_PART, 0 },
	{ LPGENT("Show icon for users kicking other user"), "IconFlags", GC_EVENT_KICK, 0 },
	{ LPGENT("Show icon for notices"), "IconFlags", GC_EVENT_NOTICE, 0 },
	{ LPGENT("Show icon for name changes"), "IconFlags", GC_EVENT_NICK, 0 },
	{ LPGENT("Show icon for information messages"), "IconFlags", GC_EVENT_INFORMATION, 0 },
	{ LPGENT("Show icon for status changes"), "IconFlags", GC_EVENT_ADDSTATUS, 0 },
};

static const struct branch_t branch5[] = {
	{ LPGENT("Show icons in tray only when the chat room is not active"), "TrayIconInactiveOnly", 0, 1 },
	{ LPGENT("Show icon in tray for topic changes"), "TrayIconFlags", GC_EVENT_TOPIC, 0 },
	{ LPGENT("Show icon in tray for users joining"), "TrayIconFlags", GC_EVENT_JOIN, 0 },
	{ LPGENT("Show icon in tray for users disconnecting"), "TrayIconFlags", GC_EVENT_QUIT, 0 },
	{ LPGENT("Show icon in tray for messages"), "TrayIconFlags", GC_EVENT_MESSAGE, 0 },
	{ LPGENT("Show icon in tray for actions"), "TrayIconFlags", GC_EVENT_ACTION, 0 },
	{ LPGENT("Show icon in tray for highlights"), "TrayIconFlags", GC_EVENT_HIGHLIGHT, 1 },
	{ LPGENT("Show icon in tray for users leaving"), "TrayIconFlags", GC_EVENT_PART, 0 },
	{ LPGENT("Show icon in tray for users kicking other user"), "TrayIconFlags", GC_EVENT_KICK, 0 },
	{ LPGENT("Show icon in tray for notices"), "TrayIconFlags", GC_EVENT_NOTICE, 0 },
	{ LPGENT("Show icon in tray for name changes"), "TrayIconFlags", GC_EVENT_NICK, 0 },
	{ LPGENT("Show icon in tray for information messages"), "TrayIconFlags", GC_EVENT_INFORMATION, 0 },
	{ LPGENT("Show icon in tray for status changes"), "TrayIconFlags", GC_EVENT_ADDSTATUS, 0 },
};

static const struct branch_t branch6[] = {
	{ LPGENT("Show popups only when the chat room is not active"), "PopupInactiveOnly", 0, 1 },
	{ LPGENT("Show popup for topic changes"), "PopupFlags", GC_EVENT_TOPIC, 0 },
	{ LPGENT("Show popup for users joining"), "PopupFlags", GC_EVENT_JOIN, 0 },
	{ LPGENT("Show popup for users disconnecting"), "PopupFlags", GC_EVENT_QUIT, 0 },
	{ LPGENT("Show popup for messages"), "PopupFlags", GC_EVENT_MESSAGE, 0 },
	{ LPGENT("Show popup for actions"), "PopupFlags", GC_EVENT_ACTION, 0 },
	{ LPGENT("Show popup for highlights"), "PopupFlags", GC_EVENT_HIGHLIGHT, 0 },
	{ LPGENT("Show popup for users leaving"), "PopupFlags", GC_EVENT_PART, 0 },
	{ LPGENT("Show popup for users kicking other user"), "PopupFlags", GC_EVENT_KICK, 0 },
	{ LPGENT("Show popup for notices"), "PopupFlags", GC_EVENT_NOTICE, 0 },
	{ LPGENT("Show popup for name changes"), "PopupFlags", GC_EVENT_NICK, 0 },
	{ LPGENT("Show popup for information messages"), "PopupFlags", GC_EVENT_INFORMATION, 0 },
	{ LPGENT("Show popup for status changes"), "PopupFlags", GC_EVENT_ADDSTATUS, 0 },
};

HTREEITEM hItemB0[SIZEOF(branch0)];
HTREEITEM hItemB1[SIZEOF(branch1)];
HTREEITEM hItemB2[SIZEOF(branch2)];
HTREEITEM hItemB3[SIZEOF(branch3)];
HTREEITEM hItemB4[SIZEOF(branch4)];
HTREEITEM hItemB5[SIZEOF(branch5)];
HTREEITEM hItemB6[SIZEOF(branch6)];

static HTREEITEM InsertBranch(HWND hwndTree, char* pszDescr, BOOL bExpanded)
{
	TVINSERTSTRUCT tvis = { 0 };
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_STATE;
	tvis.item.pszText = (TCHAR*)CallService(MS_LANGPACK_PCHARTOTCHAR, 0, (LPARAM)pszDescr);
	tvis.item.stateMask = bExpanded ? TVIS_STATEIMAGEMASK | TVIS_EXPANDED : TVIS_STATEIMAGEMASK;
	tvis.item.state = bExpanded ? INDEXTOSTATEIMAGEMASK(1) | TVIS_EXPANDED : INDEXTOSTATEIMAGEMASK(1);
	HTREEITEM res = TreeView_InsertItem(hwndTree, &tvis);
	mir_free(tvis.item.pszText);
	return res;
}

static void FillBranch(HWND hwndTree, HTREEITEM hParent, const struct branch_t *branch, HTREEITEM *hItemB, int nValues, DWORD defaultval)
{
	int iState;

	if (hParent == 0)
		return;

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
		hItemB[i] = TreeView_InsertItem(hwndTree, &tvis);
	}
}

static void SaveBranch(HWND hwndTree, const struct branch_t *branch, HTREEITEM *hItemB, int nValues)
{
	int iState = 0;

	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_STATE;
	for (int i = 0; i < nValues; i++) {
		tvi.hItem = hItemB[i];
		TreeView_GetItem(hwndTree, &tvi);
		BYTE bChecked = (((tvi.state & TVIS_STATEIMAGEMASK) >> 12) == 1) ? 0 : 1;
		if (branch[i].iMode) {
			if (bChecked)
				iState |= branch[i].iMode;
			if (iState & GC_EVENT_ADDSTATUS)
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
		if (tvi.hItem != hItemB1[0] && tvi.hItem != hItemB1[1]) {
			TreeView_GetItem(hwndTree, &tvi);
			if (((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 1))
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

	if (hHeading == 0)
		return;

	TVITEM tvi;
	tvi.mask = TVIF_HANDLE | TVIF_STATE;
	tvi.hItem = hHeading;
	TreeView_GetItem(hwndTree, &tvi);
	if (((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2))
		bChecked = FALSE;
	tvi.hItem = TreeView_GetNextItem(hwndTree, hHeading, TVGN_CHILD);
	tvi.stateMask = TVIS_STATEIMAGEMASK;
	while (tvi.hItem) {
		tvi.state = INDEXTOSTATEIMAGEMASK(bChecked ? 2 : 1);
		if (tvi.hItem != hItemB1[0] && tvi.hItem != hItemB1[1])
			TreeView_SetItem(hwndTree, &tvi);
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
}

static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	TCHAR szDir[MAX_PATH];
	switch (uMsg) {
	case BFFM_INITIALIZED:
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;

	case BFFM_SELCHANGED:
		if (SHGetPathFromIDList((LPITEMIDLIST)lp, szDir))
			SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szDir);
		break;
	}
	return 0;
}

// add icons to the skinning module

static IconItem iconList[] =
{
	{ LPGEN("Window icon"), "chat_window", IDI_CHANMGR, 0 },
	{ LPGEN("Text color"), "chat_fgcol", IDI_COLOR, 0 },
	{ LPGEN("Background color"), "chat_bkgcol", IDI_BKGCOLOR, 0 },
	{ LPGEN("Bold"), "chat_bold", IDI_BBOLD, 0 },
	{ LPGEN("Italics"), "chat_italics", IDI_BITALICS, 0 },
	{ LPGEN("Underlined"), "chat_underline", IDI_BUNDERLINE, 0 },
	{ LPGEN("Smiley button"), "chat_smiley", IDI_BSMILEY, 0 },
	{ LPGEN("Room history"), "chat_history", IDI_HISTORY, 0 },
	{ LPGEN("Room settings"), "chat_settings", IDI_TOPICBUT, 0 },
	{ LPGEN("Event filter disabled"), "chat_filter", IDI_FILTER, 0 },
	{ LPGEN("Event filter enabled"), "chat_filter2", IDI_FILTER2, 0 },
	{ LPGEN("Hide nick list"), "chat_nicklist", IDI_NICKLIST, 0 },
	{ LPGEN("Show nick list"), "chat_nicklist2", IDI_NICKLIST2, 0 },
	{ LPGEN("Icon overlay"), "chat_overlay", IDI_OVERLAY, 0 },
	{ LPGEN("Close"), "chat_close", IDI_CLOSE, 0 },

	{ LPGEN("Status 1 (10x10)"), "chat_status0", IDI_STATUS0, 10 },
	{ LPGEN("Status 2 (10x10)"), "chat_status1", IDI_STATUS1, 10 },
	{ LPGEN("Status 3 (10x10)"), "chat_status2", IDI_STATUS2, 10 },
	{ LPGEN("Status 4 (10x10)"), "chat_status3", IDI_STATUS3, 10 },
	{ LPGEN("Status 5 (10x10)"), "chat_status4", IDI_STATUS4, 10 },
	{ LPGEN("Status 6 (10x10)"), "chat_status5", IDI_STATUS5, 10 },

	{ LPGEN("Message in (10x10)"), "chat_log_message_in", IDI_MESSAGE, 10 },
	{ LPGEN("Message out (10x10)"), "chat_log_message_out", IDI_MESSAGEOUT, 10 },
	{ LPGEN("Action (10x10)"), "chat_log_action", IDI_ACTION, 10 },
	{ LPGEN("Add status (10x10)"), "chat_log_addstatus", IDI_ADDSTATUS, 10 },
	{ LPGEN("Remove status (10x10)"), "chat_log_removestatus", IDI_REMSTATUS, 10 },
	{ LPGEN("Join (10x10)"), "chat_log_join", IDI_JOIN, 10 },
	{ LPGEN("Leave (10x10)"), "chat_log_part", IDI_PART, 10 },
	{ LPGEN("Quit (10x10)"), "chat_log_quit", IDI_QUIT, 10 },
	{ LPGEN("Kick (10x10)"), "chat_log_kick", IDI_KICK, 10 },
	{ LPGEN("Nick change (10x10)"), "chat_log_nick", IDI_NICK, 10 },
	{ LPGEN("Notice (10x10)"), "chat_log_notice", IDI_NOTICE, 10 },
	{ LPGEN("Topic (10x10)"), "chat_log_topic", IDI_TOPIC, 10 },
	{ LPGEN("Highlight (10x10)"), "chat_log_highlight", IDI_HIGHLIGHT, 10 },
	{ LPGEN("Information (10x10)"), "chat_log_info", IDI_INFO, 10 }
};

void AddIcons(void)
{
	Icon_Register(g_hInst, LPGEN("Messaging") "/" LPGEN("Group chats"), iconList, 21);
	Icon_Register(g_hInst, LPGEN("Messaging") "/" LPGEN("Group chats log"), iconList + 21, 14);
}

// load icons from the skinning module if available
HICON LoadIconEx(char* pszIcoLibName, BOOL big)
{
	char szTemp[256];
	mir_snprintf(szTemp, SIZEOF(szTemp), "chat_%s", pszIcoLibName);
	return Skin_GetIcon(szTemp, big);
}

static void InitSetting(TCHAR** ppPointer, char* pszSetting, TCHAR* pszDefault)
{
	DBVARIANT dbv;
	if (!db_get_ts(NULL, CHAT_MODULE, pszSetting, &dbv)) {
		replaceStrT(*ppPointer, dbv.ptszVal);
		db_free(&dbv);
	}
	else replaceStrT(*ppPointer, pszDefault);
}

/////////////////////////////////////////////////////////////////////////////////////////
// General options

#define OPT_FIXHEADINGS (WM_USER+1)

static HTREEITEM hListHeading0, hListHeading1, hListHeading2, hListHeading3, hListHeading4, hListHeading5, hListHeading6;

static INT_PTR CALLBACK DlgProcOptions1(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHECKBOXES), GWL_STYLE, GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_CHECKBOXES), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		hListHeading0 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), LPGEN("Options for using a tabbed interface"), db_get_b(NULL, CHAT_MODULE, "Branch0Exp", 0) ? TRUE : FALSE);
		hListHeading1 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), LPGEN("Appearance and functionality of chat room windows"), db_get_b(NULL, CHAT_MODULE, "Branch1Exp", 0) ? TRUE : FALSE);
		hListHeading2 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), LPGEN("Appearance of the message log"), db_get_b(NULL, CHAT_MODULE, "Branch2Exp", 0) ? TRUE : FALSE);
		hListHeading3 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), LPGEN("Default events to show in new chat rooms if the 'event filter' is enabled"), db_get_b(NULL, CHAT_MODULE, "Branch3Exp", 0) ? TRUE : FALSE);
		hListHeading4 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), LPGEN("Icons to display in the message log"), db_get_b(NULL, CHAT_MODULE, "Branch4Exp", 0) ? TRUE : FALSE);
		hListHeading5 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), LPGEN("Icons to display in the tray"), db_get_b(NULL, CHAT_MODULE, "Branch5Exp", 0) ? TRUE : FALSE);
		if (PopupInstalled)
			hListHeading6 = InsertBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), LPGEN("Popups to display"), db_get_b(NULL, CHAT_MODULE, "Branch6Exp", 0) ? TRUE : FALSE);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading0, branch0, hItemB0, SIZEOF(branch0), 0);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading1, branch1, hItemB1, SIZEOF(branch1), 0);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading2, branch2, hItemB2, SIZEOF(branch2), 0);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading3, branch3, hItemB3, SIZEOF(branch3), 0x03E0);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading4, branch4, hItemB4, SIZEOF(branch4), 0x0000);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading5, branch5, hItemB5, SIZEOF(branch5), 0x1000);
		FillBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading6, branch6, hItemB6, SIZEOF(branch6), 0x0000);
		SendMessage(hwndDlg, OPT_FIXHEADINGS, 0, 0);
		break;

	case OPT_FIXHEADINGS:
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading1);
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading2);
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading3);
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading4);
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading5);
		CheckHeading(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading6);
		break;

	case WM_COMMAND:
		if (lParam != 0)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_CHECKBOXES:
			if (((LPNMHDR)lParam)->code == NM_CLICK) {
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti)) {
					if (hti.flags & TVHT_ONITEMSTATEICON) {
						TVITEM tvi = { 0 };
						tvi.mask = TVIF_HANDLE | TVIF_STATE;
						tvi.hItem = hti.hItem;
						TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
						if (tvi.hItem == hItemB1[0] && INDEXTOSTATEIMAGEMASK(1) == tvi.state)
							TreeView_SetItemState(((LPNMHDR)lParam)->hwndFrom, hItemB1[1], INDEXTOSTATEIMAGEMASK(1), TVIS_STATEIMAGEMASK);
						if (tvi.hItem == hItemB1[1] && INDEXTOSTATEIMAGEMASK(1) == tvi.state)
							TreeView_SetItemState(((LPNMHDR)lParam)->hwndFrom, hItemB1[0], INDEXTOSTATEIMAGEMASK(1), TVIS_STATEIMAGEMASK);

						if (tvi.hItem == hListHeading0)
							CheckBranches(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading0);
						else if (tvi.hItem == hListHeading1)
							CheckBranches(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading1);
						else if (tvi.hItem == hListHeading2)
							CheckBranches(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading2);
						else if (tvi.hItem == hListHeading3)
							CheckBranches(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading3);
						else if (tvi.hItem == hListHeading4)
							CheckBranches(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading4);
						else if (tvi.hItem == hListHeading5)
							CheckBranches(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading5);
						else if (tvi.hItem == hListHeading6)
							CheckBranches(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading6);
						else
							PostMessage(hwndDlg, OPT_FIXHEADINGS, 0, 0);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
				}
			}
			break;

		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				BYTE b = db_get_b(NULL, CHAT_MODULE, "Tabs", 1);
				SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch0, hItemB0, SIZEOF(branch0));
				SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch1, hItemB1, SIZEOF(branch1));
				SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch2, hItemB2, SIZEOF(branch2));
				SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch3, hItemB3, SIZEOF(branch3));
				SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch4, hItemB4, SIZEOF(branch4));
				SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch5, hItemB5, SIZEOF(branch5));
				if (PopupInstalled)
					SaveBranch(GetDlgItem(hwndDlg, IDC_CHECKBOXES), branch6, hItemB6, SIZEOF(branch6));
				g_Settings.dwIconFlags = db_get_dw(NULL, CHAT_MODULE, "IconFlags", 0x0000);
				g_Settings.dwTrayIconFlags = db_get_dw(NULL, CHAT_MODULE, "TrayIconFlags", 0x1000);
				g_Settings.dwPopupFlags = db_get_dw(NULL, CHAT_MODULE, "PopupFlags", 0x0000);
				g_Settings.bStripFormat = db_get_b(NULL, CHAT_MODULE, "TrimFormatting", 0) != 0;
				g_Settings.bTrayIconInactiveOnly = db_get_b(NULL, CHAT_MODULE, "TrayIconInactiveOnly", 1) != 0;
				g_Settings.bPopupInactiveOnly = db_get_b(NULL, CHAT_MODULE, "PopupInactiveOnly", 1) != 0;
				g_Settings.bLogIndentEnabled = db_get_b(NULL, CHAT_MODULE, "LogIndentEnabled", 1) != 0;

				if (b != db_get_b(NULL, CHAT_MODULE, "Tabs", 1)) {
					pci->SM_BroadcastMessage(NULL, GC_CLOSEWINDOW, 0, 1, FALSE);
					g_Settings.bTabsEnable = db_get_b(NULL, CHAT_MODULE, "Tabs", 1) != 0;
				}
				else pci->SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);

				return TRUE;
			}
		}
		break;

	case WM_DESTROY:
		BYTE b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading1, TVIS_EXPANDED)&TVIS_EXPANDED ? 1 : 0;
		db_set_b(NULL, CHAT_MODULE, "Branch1Exp", b);
		b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading2, TVIS_EXPANDED)&TVIS_EXPANDED ? 1 : 0;
		db_set_b(NULL, CHAT_MODULE, "Branch2Exp", b);
		b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading3, TVIS_EXPANDED)&TVIS_EXPANDED ? 1 : 0;
		db_set_b(NULL, CHAT_MODULE, "Branch3Exp", b);
		b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading4, TVIS_EXPANDED)&TVIS_EXPANDED ? 1 : 0;
		db_set_b(NULL, CHAT_MODULE, "Branch4Exp", b);
		b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading5, TVIS_EXPANDED)&TVIS_EXPANDED ? 1 : 0;
		db_set_b(NULL, CHAT_MODULE, "Branch5Exp", b);
		b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading0, TVIS_EXPANDED)&TVIS_EXPANDED ? 1 : 0;
		db_set_b(NULL, CHAT_MODULE, "Branch0Exp", b);
		if (PopupInstalled) {
			b = TreeView_GetItemState(GetDlgItem(hwndDlg, IDC_CHECKBOXES), hListHeading6, TVIS_EXPANDED)&TVIS_EXPANDED ? 1 : 0;
			db_set_b(NULL, CHAT_MODULE, "Branch6Exp", b);
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Log & other options

static INT_PTR CALLBACK DlgProcOptions2(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendDlgItemMessage(hwndDlg, IDC_SPIN2, UDM_SETRANGE, 0, MAKELONG(5000, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN2, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, CHAT_MODULE, "LogLimit", 100), 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN3, UDM_SETRANGE, 0, MAKELONG(10000, 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN3, UDM_SETPOS, 0, MAKELONG(db_get_w(NULL, CHAT_MODULE, "LoggingLimit", 100), 0));
		SendDlgItemMessage(hwndDlg, IDC_SPIN4, UDM_SETRANGE, 0, MAKELONG(255, 10));
		SendDlgItemMessage(hwndDlg, IDC_SPIN4, UDM_SETPOS, 0, MAKELONG(db_get_b(NULL, CHAT_MODULE, "NicklistRowDist", 12), 0));
		{
			TCHAR* pszGroup = NULL;
			InitSetting(&pszGroup, "AddToGroup", _T("Chat rooms"));
			SetDlgItemText(hwndDlg, IDC_GROUP, pszGroup);
			mir_free(pszGroup);
		}
		{
			TCHAR szTemp[MAX_PATH];
			PathToRelativeT(g_Settings.pszLogDir, szTemp);
			SetDlgItemText(hwndDlg, IDC_LOGDIRECTORY, szTemp);
		}
		SetDlgItemText(hwndDlg, IDC_HIGHLIGHTWORDS, g_Settings.pszHighlightWords);
		SetDlgItemText(hwndDlg, IDC_LOGTIMESTAMP, g_Settings.pszTimeStampLog);
		SetDlgItemText(hwndDlg, IDC_TIMESTAMP, g_Settings.pszTimeStamp);
		SetDlgItemText(hwndDlg, IDC_OUTSTAMP, g_Settings.pszOutgoingNick);
		SetDlgItemText(hwndDlg, IDC_INSTAMP, g_Settings.pszIncomingNick);
		CheckDlgButton(hwndDlg, IDC_HIGHLIGHT, g_Settings.bHighlightEnabled ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_HIGHLIGHTWORDS), g_Settings.bHighlightEnabled ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_LOGGING, g_Settings.bLoggingEnabled ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(hwndDlg, IDC_LOGDIRECTORY), g_Settings.bLoggingEnabled ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_FONTCHOOSE), g_Settings.bLoggingEnabled ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_LIMIT), g_Settings.bLoggingEnabled ? TRUE : FALSE);
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_INSTAMP
			|| LOWORD(wParam) == IDC_OUTSTAMP
			|| LOWORD(wParam) == IDC_TIMESTAMP
			|| LOWORD(wParam) == IDC_LOGLIMIT
			|| LOWORD(wParam) == IDC_HIGHLIGHTWORDS
			|| LOWORD(wParam) == IDC_LOGDIRECTORY
			|| LOWORD(wParam) == IDC_LOGTIMESTAMP
			|| LOWORD(wParam) == IDC_NICKROW2
			|| LOWORD(wParam) == IDC_GROUP
			|| LOWORD(wParam) == IDC_LIMIT)
			&& (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))	return 0;

		switch (LOWORD(wParam)) {
		case IDC_LOGGING:
			EnableWindow(GetDlgItem(hwndDlg, IDC_LOGDIRECTORY), IsDlgButtonChecked(hwndDlg, IDC_LOGGING) == BST_CHECKED ? TRUE : FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FONTCHOOSE), IsDlgButtonChecked(hwndDlg, IDC_LOGGING) == BST_CHECKED ? TRUE : FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_LIMIT), IsDlgButtonChecked(hwndDlg, IDC_LOGGING) == BST_CHECKED ? TRUE : FALSE);
			break;

		case IDC_FONTCHOOSE:
			{
				TCHAR szDirectory[MAX_PATH];
				TCHAR szTemp[MAX_PATH];

				BROWSEINFO bi = { 0 };
				bi.hwndOwner = hwndDlg;
				bi.pszDisplayName = szDirectory;
				bi.lpszTitle = TranslateT("Select folder");
				bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_RETURNONLYFSDIRS;
				bi.lpfn = BrowseCallbackProc;
				bi.lParam = (LPARAM)szDirectory;
				LPITEMIDLIST idList = SHBrowseForFolder(&bi);
				if (idList) {
					SHGetPathFromIDList(idList, szDirectory);
					mir_tstrcat(szDirectory, _T("\\"));
					PathToRelativeT(szDirectory, szTemp);
					SetDlgItemText(hwndDlg, IDC_LOGDIRECTORY, mir_tstrlen(szTemp) > 1 ? szTemp : _T("Logs\\"));
					CoTaskMemFree(idList);
				}
			}
			break;

		case IDC_HIGHLIGHT:
			EnableWindow(GetDlgItem(hwndDlg, IDC_HIGHLIGHTWORDS), IsDlgButtonChecked(hwndDlg, IDC_HIGHLIGHT) == BST_CHECKED ? TRUE : FALSE);
			break;
		}

		if (lParam != 0)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_APPLY) {
			TCHAR * pszText = NULL;
			int iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_HIGHLIGHTWORDS));
			if (iLen > 0) {
				TCHAR *ptszText = (TCHAR *)mir_alloc((iLen + 2) * sizeof(TCHAR));
				TCHAR *p2 = NULL;

				if (ptszText) {
					GetDlgItemText(hwndDlg, IDC_HIGHLIGHTWORDS, ptszText, iLen + 1);
					p2 = _tcschr(ptszText, ',');
					while (p2) {
						*p2 = ' ';
						p2 = _tcschr(ptszText, ',');
					}
					db_set_ts(NULL, CHAT_MODULE, "HighlightWords", ptszText);
					mir_free(ptszText);
				}
			}
			else db_unset(NULL, CHAT_MODULE, "HighlightWords");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_LOGDIRECTORY));
			if (iLen > 0) {
				pszText = (TCHAR *)mir_realloc(pszText, (iLen + 1) * sizeof(TCHAR));
				GetDlgItemText(hwndDlg, IDC_LOGDIRECTORY, pszText, iLen + 1);
				db_set_ts(NULL, CHAT_MODULE, "LogDirectory", pszText);
			}
			else db_unset(NULL, CHAT_MODULE, "LogDirectory");
			pci->SM_InvalidateLogDirectories();

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_LOGTIMESTAMP));
			if (iLen > 0) {
				pszText = (TCHAR *)mir_realloc(pszText, (iLen + 1) * sizeof(TCHAR));
				GetDlgItemText(hwndDlg, IDC_LOGTIMESTAMP, pszText, iLen + 1);
				db_set_ts(NULL, CHAT_MODULE, "LogTimestamp", pszText);
			}
			else db_unset(NULL, CHAT_MODULE, "LogTimestamp");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_TIMESTAMP));
			if (iLen > 0) {
				pszText = (TCHAR *)mir_realloc(pszText, (iLen + 1) * sizeof(TCHAR));
				GetDlgItemText(hwndDlg, IDC_TIMESTAMP, pszText, iLen + 1);
				db_set_ts(NULL, CHAT_MODULE, "HeaderTime", pszText);
			}
			else db_unset(NULL, CHAT_MODULE, "HeaderTime");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_INSTAMP));
			if (iLen > 0) {
				pszText = (TCHAR *)mir_realloc(pszText, (iLen + 1) * sizeof(TCHAR));
				GetDlgItemText(hwndDlg, IDC_INSTAMP, pszText, iLen + 1);
				db_set_ts(NULL, CHAT_MODULE, "HeaderIncoming", pszText);
			}
			else db_unset(NULL, CHAT_MODULE, "HeaderIncoming");

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_OUTSTAMP));
			if (iLen > 0) {
				pszText = (TCHAR *)mir_realloc(pszText, (iLen + 1) * sizeof(TCHAR));
				GetDlgItemText(hwndDlg, IDC_OUTSTAMP, pszText, iLen + 1);
				db_set_ts(NULL, CHAT_MODULE, "HeaderOutgoing", pszText);
			}
			else db_unset(NULL, CHAT_MODULE, "HeaderOutgoing");

			g_Settings.bHighlightEnabled = IsDlgButtonChecked(hwndDlg, IDC_HIGHLIGHT) == BST_CHECKED;
			db_set_b(NULL, CHAT_MODULE, "HighlightEnabled", g_Settings.bHighlightEnabled);

			g_Settings.bLoggingEnabled = IsDlgButtonChecked(hwndDlg, IDC_LOGGING) == BST_CHECKED;
			db_set_b(NULL, CHAT_MODULE, "LoggingEnabled", g_Settings.bLoggingEnabled);

			iLen = SendDlgItemMessage(hwndDlg, IDC_SPIN2, UDM_GETPOS, 0, 0);
			db_set_w(NULL, CHAT_MODULE, "LogLimit", (WORD)iLen);
			iLen = SendDlgItemMessage(hwndDlg, IDC_SPIN3, UDM_GETPOS, 0, 0);
			db_set_w(NULL, CHAT_MODULE, "LoggingLimit", (WORD)iLen);

			iLen = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_GROUP));
			if (iLen > 0) {
				pszText = (TCHAR *)mir_realloc(pszText, (iLen + 1) * sizeof(TCHAR));
				GetDlgItemText(hwndDlg, IDC_GROUP, pszText, iLen + 1);
				db_set_ts(NULL, CHAT_MODULE, "AddToGroup", pszText);
			}
			else db_set_s(NULL, CHAT_MODULE, "AddToGroup", "");
			mir_free(pszText);

			iLen = SendDlgItemMessage(hwndDlg, IDC_SPIN4, UDM_GETPOS, 0, 0);
			if (iLen > 0)
				db_set_b(NULL, CHAT_MODULE, "NicklistRowDist", (BYTE)iLen);
			else
				db_unset(NULL, CHAT_MODULE, "NicklistRowDist");

			pci->ReloadSettings();
			pci->SM_BroadcastMessage(NULL, GC_SETWNDPROPS, 0, 0, TRUE);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Popup options

static INT_PTR CALLBACK DlgProcOptionsPopup(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SendDlgItemMessage(hwndDlg, IDC_BKG, CPM_SETCOLOUR, 0, g_Settings.crPUBkgColour);
		SendDlgItemMessage(hwndDlg, IDC_TEXT, CPM_SETCOLOUR, 0, g_Settings.crPUTextColour);

		if (g_Settings.iPopupStyle == 2)
			CheckDlgButton(hwndDlg, IDC_RADIO2, BST_CHECKED);
		else if (g_Settings.iPopupStyle == 3)
			CheckDlgButton(hwndDlg, IDC_RADIO3, BST_CHECKED);
		else
			CheckDlgButton(hwndDlg, IDC_RADIO1, BST_CHECKED);

		EnableWindow(GetDlgItem(hwndDlg, IDC_BKG), IsDlgButtonChecked(hwndDlg, IDC_RADIO3) == BST_CHECKED ? TRUE : FALSE);
		EnableWindow(GetDlgItem(hwndDlg, IDC_TEXT), IsDlgButtonChecked(hwndDlg, IDC_RADIO3) == BST_CHECKED ? TRUE : FALSE);

		SendDlgItemMessage(hwndDlg, IDC_SPIN1, UDM_SETRANGE, 0, MAKELONG(100, -1));
		SendDlgItemMessage(hwndDlg, IDC_SPIN1, UDM_SETPOS, 0, MAKELONG(g_Settings.iPopupTimeout, 0));
		break;

	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_TIMEOUT) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;

		if (lParam != 0)
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		switch (LOWORD(wParam)) {

		case IDC_RADIO1:
		case IDC_RADIO2:
		case IDC_RADIO3:
			EnableWindow(GetDlgItem(hwndDlg, IDC_BKG), IsDlgButtonChecked(hwndDlg, IDC_RADIO3) == BST_CHECKED ? TRUE : FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_TEXT), IsDlgButtonChecked(hwndDlg, IDC_RADIO3) == BST_CHECKED ? TRUE : FALSE);
			break;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == 0 && ((LPNMHDR)lParam)->code == PSN_APPLY) {
			int iLen;

			if (IsDlgButtonChecked(hwndDlg, IDC_RADIO2) == BST_CHECKED)
				iLen = 2;
			else if (IsDlgButtonChecked(hwndDlg, IDC_RADIO3) == BST_CHECKED)
				iLen = 3;
			else
				iLen = 1;

			g_Settings.iPopupStyle = iLen;
			db_set_b(NULL, CHAT_MODULE, "PopupStyle", (BYTE)iLen);

			iLen = SendDlgItemMessage(hwndDlg, IDC_SPIN1, UDM_GETPOS, 0, 0);
			g_Settings.iPopupTimeout = iLen;
			db_set_w(NULL, CHAT_MODULE, "PopupTimeout", (WORD)iLen);

			g_Settings.crPUBkgColour = SendDlgItemMessage(hwndDlg, IDC_BKG, CPM_GETCOLOUR, 0, 0);
			db_set_dw(NULL, CHAT_MODULE, "PopupColorBG", (DWORD)SendDlgItemMessage(hwndDlg, IDC_BKG, CPM_GETCOLOUR, 0, 0));
			g_Settings.crPUTextColour = SendDlgItemMessage(hwndDlg, IDC_TEXT, CPM_GETCOLOUR, 0, 0);
			db_set_dw(NULL, CHAT_MODULE, "PopupColorText", (DWORD)SendDlgItemMessage(hwndDlg, IDC_TEXT, CPM_GETCOLOUR, 0, 0));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

int OptionsInitialize(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 910000000;
	odp.hInstance = g_hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS1);
	odp.pszGroup = LPGEN("Message sessions");
	odp.pszTitle = LPGEN("Group chats");
	odp.pszTab = LPGEN("General");
	odp.pfnDlgProc = DlgProcOptions1;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);

	odp.position = 910000001;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS2);
	odp.pszTab = LPGEN("Chat log");
	odp.pfnDlgProc = DlgProcOptions2;
	Options_AddPage(wParam, &odp);

	if (PopupInstalled) {
		odp.position = 910000002;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONSPOPUP);
		odp.pszTitle = LPGEN(CHAT_MODULE);
		odp.pszGroup = LPGEN("Popups");
		odp.pszTab = NULL;
		odp.pfnDlgProc = DlgProcOptionsPopup;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}
