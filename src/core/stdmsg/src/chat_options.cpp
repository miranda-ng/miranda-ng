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
#include "stdafx.h"

struct branch_t
{
	const wchar_t *szDescr;
	const char *szDBName;
	int iMode;
	bool bDefault;
	HTREEITEM hItem;
};

static branch_t branch1[] = {
	{ LPGENW("Send message by pressing the 'Enter' key"), "SendOnEnter", 0, true },
	{ LPGENW("Send message by pressing the 'Enter' key twice"), "SendOnDblEnter", 0, false },
	{ LPGENW("Flash window when someone speaks"), "FlashWindow", 0, false },
	{ LPGENW("Flash window when a word is highlighted"), "FlashWindowHighlight", 0, true },
	{ LPGENW("Show list of users in the chat room"), "ShowNicklist", 0, true },
	{ LPGENW("Show button for sending messages"), "ShowSend", 0, false },
	{ LPGENW("Show buttons for controlling the chat room"), "ShowTopButtons", 0, true },
	{ LPGENW("Show buttons for formatting the text you are typing"), "ShowFormatButtons", 0, true },
	{ LPGENW("Show button menus when right clicking the buttons"), "RightClickFilter", 0, false },
	{ LPGENW("Show the topic of the room on your contact list (if supported)"), "TopicOnClist", 0, false },
	{ LPGENW("Do not play sounds when the chat room is focused"), "SoundsFocus", 0, false },
	{ LPGENW("Do not pop up the window when joining a chat room"), "PopupOnJoin", 0, false },
	{ LPGENW("Toggle the visible state when double clicking in the contact list"), "ToggleVisibility", 0, false },
	{ LPGENW("Show contact statuses if protocol supports them"), "ShowContactStatus", 0, false },
	{ LPGENW("Display contact status icon before user role icon"), "ContactStatusFirst", 0, false },
};

static branch_t branch2[] = {
	{ LPGENW("Prefix all events with a timestamp"), "ShowTimeStamp", 0, true },
	{ LPGENW("Only prefix with timestamp if it has changed"), "ShowTimeStampIfChanged", 0, false },
	{ LPGENW("Timestamp has same color as the event"), "TimeStampEventColour", 0, false },
	{ LPGENW("Indent the second line of a message"), "LogIndentEnabled", 0, true },
	{ LPGENW("Limit user names in the message log to 20 characters"), "LogLimitNames", 0, true },
	{ LPGENW("Add ':' to auto-completed user names"), "AddColonToAutoComplete", 0, true },
	{ LPGENW("Strip colors from messages in the log"), "StripFormatting", 0, false },
	{ LPGENW("Enable the 'event filter' for new rooms"), "FilterEnabled", 0, 0 }
};

static branch_t branch3[] = {
	{ LPGENW("Show topic changes"), "FilterFlags", GC_EVENT_TOPIC, false },
	{ LPGENW("Show users joining"), "FilterFlags", GC_EVENT_JOIN, false },
	{ LPGENW("Show users disconnecting"), "FilterFlags", GC_EVENT_QUIT, false },
	{ LPGENW("Show messages"), "FilterFlags", GC_EVENT_MESSAGE, true },
	{ LPGENW("Show actions"), "FilterFlags", GC_EVENT_ACTION, true },
	{ LPGENW("Show users leaving"), "FilterFlags", GC_EVENT_PART, false },
	{ LPGENW("Show users being kicked"), "FilterFlags", GC_EVENT_KICK, true },
	{ LPGENW("Show notices"), "FilterFlags", GC_EVENT_NOTICE, true },
	{ LPGENW("Show users changing name"), "FilterFlags", GC_EVENT_NICK, false },
	{ LPGENW("Show information messages"), "FilterFlags", GC_EVENT_INFORMATION, true },
	{ LPGENW("Show status changes of users"), "FilterFlags", GC_EVENT_ADDSTATUS, false },
};

static branch_t branch4[] = {
	{ LPGENW("Show icon for topic changes"), "IconFlags", GC_EVENT_TOPIC, false },
	{ LPGENW("Show icon for users joining"), "IconFlags", GC_EVENT_JOIN, true },
	{ LPGENW("Show icon for users disconnecting"), "IconFlags", GC_EVENT_QUIT, false },
	{ LPGENW("Show icon for messages"), "IconFlags", GC_EVENT_MESSAGE, false },
	{ LPGENW("Show icon for actions"), "IconFlags", GC_EVENT_ACTION, false },
	{ LPGENW("Show icon for highlights"), "IconFlags", GC_EVENT_HIGHLIGHT, false },
	{ LPGENW("Show icon for users leaving"), "IconFlags", GC_EVENT_PART, false },
	{ LPGENW("Show icon for users kicking other user"), "IconFlags", GC_EVENT_KICK, false },
	{ LPGENW("Show icon for notices"), "IconFlags", GC_EVENT_NOTICE, false },
	{ LPGENW("Show icon for name changes"), "IconFlags", GC_EVENT_NICK, false },
	{ LPGENW("Show icon for information messages"), "IconFlags", GC_EVENT_INFORMATION, false },
	{ LPGENW("Show icon for status changes"), "IconFlags", GC_EVENT_ADDSTATUS, false },
};

static branch_t branch5[] = {
	{ LPGENW("Show icons in tray only when the chat room is not active"), "TrayIconInactiveOnly", 0, true },
	{ LPGENW("Show icon in tray for topic changes"), "TrayIconFlags", GC_EVENT_TOPIC, false },
	{ LPGENW("Show icon in tray for users joining"), "TrayIconFlags", GC_EVENT_JOIN, false },
	{ LPGENW("Show icon in tray for users disconnecting"), "TrayIconFlags", GC_EVENT_QUIT, false },
	{ LPGENW("Show icon in tray for messages"), "TrayIconFlags", GC_EVENT_MESSAGE, false },
	{ LPGENW("Show icon in tray for actions"), "TrayIconFlags", GC_EVENT_ACTION, false },
	{ LPGENW("Show icon in tray for highlights"), "TrayIconFlags", GC_EVENT_HIGHLIGHT, true },
	{ LPGENW("Show icon in tray for users leaving"), "TrayIconFlags", GC_EVENT_PART, false },
	{ LPGENW("Show icon in tray for users kicking other user"), "TrayIconFlags", GC_EVENT_KICK, false },
	{ LPGENW("Show icon in tray for notices"), "TrayIconFlags", GC_EVENT_NOTICE, false },
	{ LPGENW("Show icon in tray for name changes"), "TrayIconFlags", GC_EVENT_NICK, false },
	{ LPGENW("Show icon in tray for information messages"), "TrayIconFlags", GC_EVENT_INFORMATION, false },
	{ LPGENW("Show icon in tray for status changes"), "TrayIconFlags", GC_EVENT_ADDSTATUS, false },
};

static branch_t branch6[] = {
	{ LPGENW("Show popups only when the chat room is not active"), "PopupInactiveOnly", 0, true },
	{ LPGENW("Show popup for topic changes"), "PopupFlags", GC_EVENT_TOPIC, false },
	{ LPGENW("Show popup for users joining"), "PopupFlags", GC_EVENT_JOIN, false },
	{ LPGENW("Show popup for users disconnecting"), "PopupFlags", GC_EVENT_QUIT, false },
	{ LPGENW("Show popup for messages"), "PopupFlags", GC_EVENT_MESSAGE, false },
	{ LPGENW("Show popup for actions"), "PopupFlags", GC_EVENT_ACTION, false },
	{ LPGENW("Show popup for highlights"), "PopupFlags", GC_EVENT_HIGHLIGHT, false },
	{ LPGENW("Show popup for users leaving"), "PopupFlags", GC_EVENT_PART, false },
	{ LPGENW("Show popup for users kicking other user"), "PopupFlags", GC_EVENT_KICK, false },
	{ LPGENW("Show popup for notices"), "PopupFlags", GC_EVENT_NOTICE, false },
	{ LPGENW("Show popup for name changes"), "PopupFlags", GC_EVENT_NICK, false },
	{ LPGENW("Show popup for information messages"), "PopupFlags", GC_EVENT_INFORMATION, false },
	{ LPGENW("Show popup for status changes"), "PopupFlags", GC_EVENT_ADDSTATUS, false },
};

static HTREEITEM InsertBranch(HWND hwndTree, char* pszDescr, BOOL bExpanded)
{
	TVINSERTSTRUCT tvis = {};
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_STATE;
	tvis.item.pszText = Langpack_PcharToTchar(pszDescr);
	tvis.item.stateMask = bExpanded ? TVIS_STATEIMAGEMASK | TVIS_EXPANDED : TVIS_STATEIMAGEMASK;
	tvis.item.state = bExpanded ? INDEXTOSTATEIMAGEMASK(1) | TVIS_EXPANDED : INDEXTOSTATEIMAGEMASK(1);
	HTREEITEM res = TreeView_InsertItem(hwndTree, &tvis);
	mir_free(tvis.item.pszText);
	return res;
}

static void FillBranch(HWND hwndTree, HTREEITEM hParent, struct branch_t *branch, int nValues, DWORD defaultval)
{
	int iState;

	if (hParent == 0)
		return;

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
		BYTE bChecked = (((tvi.state & TVIS_STATEIMAGEMASK) >> 12) == 1) ? 0 : 1;
		if (branch[i].iMode) {
			if (bChecked)
				iState |= branch[i].iMode;
			if (iState & GC_EVENT_ADDSTATUS)
				iState |= GC_EVENT_REMOVESTATUS;
			db_set_dw(0, CHAT_MODULE, branch[i].szDBName, (DWORD)iState);
		}
		else db_set_b(0, CHAT_MODULE, branch[i].szDBName, bChecked);
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
		if (tvi.hItem != branch1[0].hItem && tvi.hItem != branch1[1].hItem)
			TreeView_SetItem(hwndTree, &tvi);
		tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
	}
}

static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	wchar_t szDir[MAX_PATH];
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
HICON LoadIconEx(const char *pszIcoLibName, bool big)
{
	char szTemp[256];
	mir_snprintf(szTemp, "chat_%s", pszIcoLibName);
	return IcoLib_GetIcon(szTemp, big);
}

HANDLE GetIconHandle(const char *pszIcoLibName)
{
	char szTemp[256];
	mir_snprintf(szTemp, "chat_%s", pszIcoLibName);
	return IcoLib_GetIconHandle(szTemp);
}

static void InitSetting(wchar_t** ppPointer, char* pszSetting, wchar_t* pszDefault)
{
	DBVARIANT dbv;
	if (!db_get_ws(0, CHAT_MODULE, pszSetting, &dbv)) {
		replaceStrW(*ppPointer, dbv.ptszVal);
		db_free(&dbv);
	}
	else replaceStrW(*ppPointer, pszDefault);
}

/////////////////////////////////////////////////////////////////////////////////////////
// General options

class COptMainDlg : public CDlgBase
{
	HTREEITEM hListHeading0, hListHeading1, hListHeading2, hListHeading3, hListHeading4, hListHeading5, hListHeading6;

	void FixHeadings()
	{
		CheckHeading(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading1);
		CheckHeading(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading2);
		CheckHeading(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading3);
		CheckHeading(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading4);
		CheckHeading(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading5);
		CheckHeading(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading6);
	}

public:
	COptMainDlg()
		: CDlgBase(g_hInst, IDD_OPTIONS1)
	{}

	virtual void OnInitDialog() override
	{
		SetWindowLongPtr(GetDlgItem(m_hwnd, IDC_CHECKBOXES), GWL_STYLE, GetWindowLongPtr(GetDlgItem(m_hwnd, IDC_CHECKBOXES), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		hListHeading1 = InsertBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), LPGEN("Appearance and functionality of chat room windows"), db_get_b(0, CHAT_MODULE, "Branch1Exp", 0) ? TRUE : FALSE);
		hListHeading2 = InsertBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), LPGEN("Appearance of the message log"), db_get_b(0, CHAT_MODULE, "Branch2Exp", 0) ? TRUE : FALSE);
		hListHeading3 = InsertBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), LPGEN("Default events to show in new chat rooms if the 'event filter' is enabled"), db_get_b(0, CHAT_MODULE, "Branch3Exp", 0) ? TRUE : FALSE);
		hListHeading4 = InsertBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), LPGEN("Icons to display in the message log"), db_get_b(0, CHAT_MODULE, "Branch4Exp", 0) ? TRUE : FALSE);
		hListHeading5 = InsertBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), LPGEN("Icons to display in the tray"), db_get_b(0, CHAT_MODULE, "Branch5Exp", 0) ? TRUE : FALSE);
		if (PopupInstalled)
			hListHeading6 = InsertBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), LPGEN("Popups to display"), db_get_b(0, CHAT_MODULE, "Branch6Exp", 0) ? TRUE : FALSE);

		FillBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading1, branch1, _countof(branch1), 0);
		FillBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading2, branch2, _countof(branch2), 0);
		FillBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading3, branch3, _countof(branch3), 0x03E0);
		FillBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading4, branch4, _countof(branch4), 0x0000);
		FillBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading5, branch5, _countof(branch5), 0x1000);
		FillBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading6, branch6, _countof(branch6), 0x0000);
		FixHeadings();
	}

	virtual void OnApply() override
	{
		SaveBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), branch1, _countof(branch1));
		SaveBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), branch2, _countof(branch2));
		SaveBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), branch3, _countof(branch3));
		SaveBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), branch4, _countof(branch4));
		SaveBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), branch5, _countof(branch5));
		if (PopupInstalled)
			SaveBranch(GetDlgItem(m_hwnd, IDC_CHECKBOXES), branch6, _countof(branch6));

		pci->ReloadSettings();
		Chat_UpdateOptions();
	}

	virtual void OnDestroy() override
	{
		BYTE b = TreeView_GetItemState(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading1, TVIS_EXPANDED) & TVIS_EXPANDED ? 1 : 0;
		db_set_b(0, CHAT_MODULE, "Branch1Exp", b);
		b = TreeView_GetItemState(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading2, TVIS_EXPANDED) & TVIS_EXPANDED ? 1 : 0;
		db_set_b(0, CHAT_MODULE, "Branch2Exp", b);
		b = TreeView_GetItemState(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading3, TVIS_EXPANDED) & TVIS_EXPANDED ? 1 : 0;
		db_set_b(0, CHAT_MODULE, "Branch3Exp", b);
		b = TreeView_GetItemState(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading4, TVIS_EXPANDED) & TVIS_EXPANDED ? 1 : 0;
		db_set_b(0, CHAT_MODULE, "Branch4Exp", b);
		b = TreeView_GetItemState(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading5, TVIS_EXPANDED) & TVIS_EXPANDED ? 1 : 0;
		db_set_b(0, CHAT_MODULE, "Branch5Exp", b);

		if (PopupInstalled) {
			b = TreeView_GetItemState(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading6, TVIS_EXPANDED) & TVIS_EXPANDED ? 1 : 0;
			db_set_b(0, CHAT_MODULE, "Branch6Exp", b);
		}
	}

	virtual INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		if (msg == WM_NOTIFY) {
			LPNMHDR pHdr = (LPNMHDR)lParam;
			if (pHdr->code == NM_CLICK && pHdr->idFrom == IDC_CHECKBOXES) {
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti)) {
					if (hti.flags & TVHT_ONITEMSTATEICON) {
						TVITEM tvi = {};
						tvi.mask = TVIF_HANDLE | TVIF_STATE;
						tvi.hItem = hti.hItem;
						TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom, &tvi);
						if (tvi.hItem == branch1[0].hItem && INDEXTOSTATEIMAGEMASK(1) == tvi.state)
							TreeView_SetItemState(((LPNMHDR)lParam)->hwndFrom, branch1[1].hItem, INDEXTOSTATEIMAGEMASK(1), TVIS_STATEIMAGEMASK);
						if (tvi.hItem == branch1[1].hItem && INDEXTOSTATEIMAGEMASK(1) == tvi.state)
							TreeView_SetItemState(((LPNMHDR)lParam)->hwndFrom, branch1[0].hItem, INDEXTOSTATEIMAGEMASK(1), TVIS_STATEIMAGEMASK);

						if (tvi.hItem == hListHeading0)
							CheckBranches(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading0);
						else if (tvi.hItem == hListHeading1)
							CheckBranches(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading1);
						else if (tvi.hItem == hListHeading2)
							CheckBranches(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading2);
						else if (tvi.hItem == hListHeading3)
							CheckBranches(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading3);
						else if (tvi.hItem == hListHeading4)
							CheckBranches(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading4);
						else if (tvi.hItem == hListHeading5)
							CheckBranches(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading5);
						else if (tvi.hItem == hListHeading6)
							CheckBranches(GetDlgItem(m_hwnd, IDC_CHECKBOXES), hListHeading6);
						else
							FixHeadings();
						NotifyChange();
					}
				}
			}
		}

		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Log & other options

class COptLogDlg : public CDlgBase
{
	CCtrlCheck chkLogging, chkHighlight;
	CCtrlButton btnFontChoose;

public:
	COptLogDlg()
		: CDlgBase(g_hInst, IDD_OPTIONS2),
		chkLogging(this, IDC_LOGGING),
		chkHighlight(this, IDC_HIGHLIGHT),
		btnFontChoose(this, IDC_FONTCHOOSE)
	{
		chkLogging.OnChange = Callback(this, &COptLogDlg::onChange_Logging);
		chkHighlight.OnChange = Callback(this, &COptLogDlg::onChange_Highlight);

		btnFontChoose.OnClick = Callback(this, &COptLogDlg::onClick_Font);
	}

	virtual void OnInitDialog() override
	{
		SendDlgItemMessage(m_hwnd, IDC_SPIN2, UDM_SETRANGE, 0, MAKELONG(5000, 0));
		SendDlgItemMessage(m_hwnd, IDC_SPIN2, UDM_SETPOS, 0, MAKELONG(db_get_w(0, CHAT_MODULE, "LogLimit", 100), 0));
		SendDlgItemMessage(m_hwnd, IDC_SPIN3, UDM_SETRANGE, 0, MAKELONG(10000, 0));
		SendDlgItemMessage(m_hwnd, IDC_SPIN3, UDM_SETPOS, 0, MAKELONG(db_get_w(0, CHAT_MODULE, "LoggingLimit", 100), 0));
		SendDlgItemMessage(m_hwnd, IDC_SPIN4, UDM_SETRANGE, 0, MAKELONG(255, 10));
		SendDlgItemMessage(m_hwnd, IDC_SPIN4, UDM_SETPOS, 0, MAKELONG(db_get_b(0, CHAT_MODULE, "NicklistRowDist", 12), 0));
		{
			wchar_t* pszGroup = nullptr;
			InitSetting(&pszGroup, "AddToGroup", L"Chat rooms");
			SetDlgItemText(m_hwnd, IDC_GROUP, pszGroup);
			mir_free(pszGroup);
		}
		{
			wchar_t szTemp[MAX_PATH];
			PathToRelativeW(g_Settings.pszLogDir, szTemp);
			SetDlgItemText(m_hwnd, IDC_LOGDIRECTORY, szTemp);
		}
		SetDlgItemText(m_hwnd, IDC_HIGHLIGHTWORDS, g_Settings.pszHighlightWords);
		SetDlgItemText(m_hwnd, IDC_LOGTIMESTAMP, g_Settings.pszTimeStampLog);
		SetDlgItemText(m_hwnd, IDC_TIMESTAMP, g_Settings.pszTimeStamp);
		SetDlgItemText(m_hwnd, IDC_OUTSTAMP, g_Settings.pszOutgoingNick);
		SetDlgItemText(m_hwnd, IDC_INSTAMP, g_Settings.pszIncomingNick);
		CheckDlgButton(m_hwnd, IDC_HIGHLIGHT, g_Settings.bHighlightEnabled ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(m_hwnd, IDC_HIGHLIGHTWORDS), g_Settings.bHighlightEnabled ? TRUE : FALSE);
		CheckDlgButton(m_hwnd, IDC_LOGGING, g_Settings.bLoggingEnabled ? BST_CHECKED : BST_UNCHECKED);
		EnableWindow(GetDlgItem(m_hwnd, IDC_LOGDIRECTORY), g_Settings.bLoggingEnabled ? TRUE : FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_FONTCHOOSE), g_Settings.bLoggingEnabled ? TRUE : FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_LIMIT), g_Settings.bLoggingEnabled ? TRUE : FALSE);
	}

	virtual void OnApply() override
	{
		wchar_t *pszText = nullptr;
		int iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_HIGHLIGHTWORDS));
		if (iLen > 0) {
			wchar_t *ptszText = (wchar_t *)mir_alloc((iLen + 2) * sizeof(wchar_t));
			wchar_t *p2 = nullptr;

			if (ptszText) {
				GetDlgItemText(m_hwnd, IDC_HIGHLIGHTWORDS, ptszText, iLen + 1);
				p2 = wcschr(ptszText, ',');
				while (p2) {
					*p2 = ' ';
					p2 = wcschr(ptszText, ',');
				}
				db_set_ws(0, CHAT_MODULE, "HighlightWords", ptszText);
				mir_free(ptszText);
			}
		}
		else db_unset(0, CHAT_MODULE, "HighlightWords");

		iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_LOGDIRECTORY));
		if (iLen > 0) {
			pszText = (wchar_t *)mir_realloc(pszText, (iLen + 1) * sizeof(wchar_t));
			GetDlgItemText(m_hwnd, IDC_LOGDIRECTORY, pszText, iLen + 1);
			db_set_ws(0, CHAT_MODULE, "LogDirectory", pszText);
		}
		else db_unset(0, CHAT_MODULE, "LogDirectory");
		pci->SM_InvalidateLogDirectories();

		iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_LOGTIMESTAMP));
		if (iLen > 0) {
			pszText = (wchar_t *)mir_realloc(pszText, (iLen + 1) * sizeof(wchar_t));
			GetDlgItemText(m_hwnd, IDC_LOGTIMESTAMP, pszText, iLen + 1);
			db_set_ws(0, CHAT_MODULE, "LogTimestamp", pszText);
		}
		else db_unset(0, CHAT_MODULE, "LogTimestamp");

		iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_TIMESTAMP));
		if (iLen > 0) {
			pszText = (wchar_t *)mir_realloc(pszText, (iLen + 1) * sizeof(wchar_t));
			GetDlgItemText(m_hwnd, IDC_TIMESTAMP, pszText, iLen + 1);
			db_set_ws(0, CHAT_MODULE, "HeaderTime", pszText);
		}
		else db_unset(0, CHAT_MODULE, "HeaderTime");

		iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_INSTAMP));
		if (iLen > 0) {
			pszText = (wchar_t *)mir_realloc(pszText, (iLen + 1) * sizeof(wchar_t));
			GetDlgItemText(m_hwnd, IDC_INSTAMP, pszText, iLen + 1);
			db_set_ws(0, CHAT_MODULE, "HeaderIncoming", pszText);
		}
		else db_unset(0, CHAT_MODULE, "HeaderIncoming");

		iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_OUTSTAMP));
		if (iLen > 0) {
			pszText = (wchar_t *)mir_realloc(pszText, (iLen + 1) * sizeof(wchar_t));
			GetDlgItemText(m_hwnd, IDC_OUTSTAMP, pszText, iLen + 1);
			db_set_ws(0, CHAT_MODULE, "HeaderOutgoing", pszText);
		}
		else db_unset(0, CHAT_MODULE, "HeaderOutgoing");

		g_Settings.bHighlightEnabled = IsDlgButtonChecked(m_hwnd, IDC_HIGHLIGHT) == BST_CHECKED;
		db_set_b(0, CHAT_MODULE, "HighlightEnabled", g_Settings.bHighlightEnabled);

		g_Settings.bLoggingEnabled = IsDlgButtonChecked(m_hwnd, IDC_LOGGING) == BST_CHECKED;
		db_set_b(0, CHAT_MODULE, "LoggingEnabled", g_Settings.bLoggingEnabled);

		iLen = SendDlgItemMessage(m_hwnd, IDC_SPIN2, UDM_GETPOS, 0, 0);
		db_set_w(0, CHAT_MODULE, "LogLimit", (WORD)iLen);
		iLen = SendDlgItemMessage(m_hwnd, IDC_SPIN3, UDM_GETPOS, 0, 0);
		db_set_w(0, CHAT_MODULE, "LoggingLimit", (WORD)iLen);

		iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_GROUP));
		if (iLen > 0) {
			pszText = (wchar_t *)mir_realloc(pszText, (iLen + 1) * sizeof(wchar_t));
			GetDlgItemText(m_hwnd, IDC_GROUP, pszText, iLen + 1);
			db_set_ws(0, CHAT_MODULE, "AddToGroup", pszText);
		}
		else db_set_s(0, CHAT_MODULE, "AddToGroup", "");
		mir_free(pszText);

		iLen = SendDlgItemMessage(m_hwnd, IDC_SPIN4, UDM_GETPOS, 0, 0);
		if (iLen > 0)
			db_set_b(0, CHAT_MODULE, "NicklistRowDist", (BYTE)iLen);
		else
			db_unset(0, CHAT_MODULE, "NicklistRowDist");

		pci->ReloadSettings();
		Chat_UpdateOptions();
	}

	void onChange_Logging(CCtrlCheck*)
	{
		BOOL bStatus = chkLogging.GetState() != 0;
		EnableWindow(GetDlgItem(m_hwnd, IDC_LOGDIRECTORY), bStatus);
		EnableWindow(GetDlgItem(m_hwnd, IDC_FONTCHOOSE), bStatus);
		EnableWindow(GetDlgItem(m_hwnd, IDC_LIMIT), bStatus);
	}

	void onClick_Font(CCtrlButton*)
	{
		wchar_t szDirectory[MAX_PATH];
		wchar_t szTemp[MAX_PATH];

		BROWSEINFO bi = {};
		bi.hwndOwner = m_hwnd;
		bi.pszDisplayName = szDirectory;
		bi.lpszTitle = TranslateT("Select folder");
		bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_RETURNONLYFSDIRS;
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = (LPARAM)szDirectory;
		LPITEMIDLIST idList = SHBrowseForFolder(&bi);
		if (idList) {
			SHGetPathFromIDList(idList, szDirectory);
			mir_wstrcat(szDirectory, L"\\");
			PathToRelativeW(szDirectory, szTemp);
			SetDlgItemText(m_hwnd, IDC_LOGDIRECTORY, mir_wstrlen(szTemp) > 1 ? szTemp : L"Logs\\");
			CoTaskMemFree(idList);
		}
	}

	void onChange_Highlight(CCtrlCheck*)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_HIGHLIGHTWORDS), chkHighlight.GetState() != 0);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Popup options

class COptPopupDlg : public CDlgBase
{
	CCtrlCheck chkRadio1, chkRadio2, chkRadio3;

public:
	COptPopupDlg()
		: CDlgBase(g_hInst, IDD_OPTIONSPOPUP),
		chkRadio1(this, IDC_RADIO1),
		chkRadio2(this, IDC_RADIO2),
		chkRadio3(this, IDC_RADIO3)
	{
		chkRadio1.OnChange = chkRadio2.OnChange = chkRadio3.OnChange = Callback(this, &COptPopupDlg::onChange_Radio);
	}

	virtual void OnInitDialog() override
	{
		SendDlgItemMessage(m_hwnd, IDC_BKG, CPM_SETCOLOUR, 0, g_Settings.crPUBkgColour);
		SendDlgItemMessage(m_hwnd, IDC_TEXT, CPM_SETCOLOUR, 0, g_Settings.crPUTextColour);

		if (g_Settings.iPopupStyle == 2)
			CheckDlgButton(m_hwnd, IDC_RADIO2, BST_CHECKED);
		else if (g_Settings.iPopupStyle == 3)
			CheckDlgButton(m_hwnd, IDC_RADIO3, BST_CHECKED);
		else
			CheckDlgButton(m_hwnd, IDC_RADIO1, BST_CHECKED);

		EnableWindow(GetDlgItem(m_hwnd, IDC_BKG), IsDlgButtonChecked(m_hwnd, IDC_RADIO3) == BST_CHECKED ? TRUE : FALSE);
		EnableWindow(GetDlgItem(m_hwnd, IDC_TEXT), IsDlgButtonChecked(m_hwnd, IDC_RADIO3) == BST_CHECKED ? TRUE : FALSE);

		SendDlgItemMessage(m_hwnd, IDC_SPIN1, UDM_SETRANGE, 0, MAKELONG(100, -1));
		SendDlgItemMessage(m_hwnd, IDC_SPIN1, UDM_SETPOS, 0, MAKELONG(g_Settings.iPopupTimeout, 0));
	}

	virtual void OnApply() override
	{
		int iLen;
		if (IsDlgButtonChecked(m_hwnd, IDC_RADIO2) == BST_CHECKED)
			iLen = 2;
		else if (IsDlgButtonChecked(m_hwnd, IDC_RADIO3) == BST_CHECKED)
			iLen = 3;
		else
			iLen = 1;
		g_Settings.iPopupStyle = iLen;
		db_set_b(0, CHAT_MODULE, "PopupStyle", (BYTE)iLen);

		iLen = SendDlgItemMessage(m_hwnd, IDC_SPIN1, UDM_GETPOS, 0, 0);
		g_Settings.iPopupTimeout = iLen;
		db_set_w(0, CHAT_MODULE, "PopupTimeout", (WORD)iLen);

		g_Settings.crPUBkgColour = SendDlgItemMessage(m_hwnd, IDC_BKG, CPM_GETCOLOUR, 0, 0);
		db_set_dw(0, CHAT_MODULE, "PopupColorBG", (DWORD)SendDlgItemMessage(m_hwnd, IDC_BKG, CPM_GETCOLOUR, 0, 0));
		g_Settings.crPUTextColour = SendDlgItemMessage(m_hwnd, IDC_TEXT, CPM_GETCOLOUR, 0, 0);
		db_set_dw(0, CHAT_MODULE, "PopupColorText", (DWORD)SendDlgItemMessage(m_hwnd, IDC_TEXT, CPM_GETCOLOUR, 0, 0));
	}
	
	void onChange_Radio(CCtrlCheck*)
	{
		BOOL bStatus = chkRadio3.GetState() != 0;
		EnableWindow(GetDlgItem(m_hwnd, IDC_BKG), bStatus);
		EnableWindow(GetDlgItem(m_hwnd, IDC_TEXT), bStatus);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Module init procedure

int ChatOptionsInitialize(WPARAM wParam)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.hInstance = g_hInst;
	odp.flags = ODPF_BOLDGROUPS;
	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = LPGEN("Group chats");

	odp.position = 910000000;
	odp.szTab.a = LPGEN("General");
	odp.pDialog = new COptMainDlg();
	Options_AddPage(wParam, &odp);

	odp.position = 910000001;
	odp.szTab.a = LPGEN("Chat log");
	odp.pDialog = new COptLogDlg();
	Options_AddPage(wParam, &odp);

	if (PopupInstalled) {
		odp.position = 910000002;
		odp.szTitle.a = LPGEN("Chat");
		odp.szGroup.a = LPGEN("Popups");
		odp.szTab.a = nullptr;
		odp.pDialog = new COptPopupDlg();
		Options_AddPage(wParam, &odp);
	}
	return 0;
}
