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
	{ LPGENW("Flash window when someone speaks"), "FlashWindow", 0, false },
	{ LPGENW("Flash window when a word is highlighted"), "FlashWindowHighlight", 0, true },
	{ LPGENW("Show list of users in the chat room"), "ShowNicklist", 0, true },
	{ LPGENW("Show the topic of the room on your contact list (if supported)"), "TopicOnClist", 0, false },
	{ LPGENW("Do not play sounds when the chat room is focused"), "SoundsFocus", 0, false },
	{ LPGENW("Do not pop up the window when joining a chat room"), "PopupOnJoin", 0, false },
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

static IconItem iconList1[] =
{
	{ LPGEN("Window icon"),           "window",    IDI_CHANMGR    },
	{ LPGEN("Text color"),            "fgcol",     IDI_COLOR      },
	{ LPGEN("Background color"),      "bkgcol",    IDI_BKGCOLOR   },
	{ LPGEN("Bold"),                  "bold",      IDI_BBOLD      },
	{ LPGEN("Italics"),               "italics",   IDI_BITALICS   },
	{ LPGEN("Underlined"),            "underline", IDI_BUNDERLINE },
	{ LPGEN("Smiley button"),         "smiley",    IDI_BSMILEY    },
	{ LPGEN("Room history"),          "history",   IDI_HISTORY    },
	{ LPGEN("Room settings"),         "settings",  IDI_TOPICBUT   },
	{ LPGEN("Event filter disabled"), "filter",    IDI_FILTER     },
	{ LPGEN("Event filter enabled"),  "filter2",   IDI_FILTER2    },
	{ LPGEN("Hide nick list"),        "nicklist",  IDI_NICKLIST   },
	{ LPGEN("Show nick list"),        "nicklist2", IDI_NICKLIST2  },
	{ LPGEN("Icon overlay"),          "overlay",   IDI_OVERLAY    },
	{ LPGEN("Close"),                 "close",     IDI_CLOSE      },
};

static IconItem iconList2[] =
{
	{ LPGEN("Message in (10x10)"),    "log_message_in",   IDI_MESSAGE,    10 },
	{ LPGEN("Message out (10x10)"),   "log_message_out",  IDI_MESSAGEOUT, 10 },
	{ LPGEN("Action (10x10)"),        "log_action",       IDI_ACTION,     10 },
	{ LPGEN("Add status (10x10)"),    "log_addstatus",    IDI_ADDSTATUS,  10 },
	{ LPGEN("Remove status (10x10)"), "log_removestatus", IDI_REMSTATUS,  10 },
	{ LPGEN("Join (10x10)"),          "log_join",         IDI_JOIN,       10 },
	{ LPGEN("Leave (10x10)"),         "log_part",         IDI_PART,       10 },
	{ LPGEN("Quit (10x10)"),          "log_quit",         IDI_QUIT,       10 },
	{ LPGEN("Kick (10x10)"),          "log_kick",         IDI_KICK,       10 },
	{ LPGEN("Nick change (10x10)"),   "log_nick",         IDI_NICK,       10 },
	{ LPGEN("Notice (10x10)"),        "log_notice",       IDI_NOTICE,     10 },
	{ LPGEN("Topic (10x10)"),         "log_topic",        IDI_TOPIC,      10 },
	{ LPGEN("Highlight (10x10)"),     "log_highlight",    IDI_HIGHLIGHT,  10 },
	{ LPGEN("Information (10x10)"),   "log_info",         IDI_INFO,       10 }
};

void AddIcons(void)
{
	g_plugin.registerIcon(LPGEN("Messaging") "/" LPGEN("Group chats"), iconList1, "chat");
	g_plugin.registerIcon(LPGEN("Messaging") "/" LPGEN("Group chats log"), iconList2, "chat");
}

/////////////////////////////////////////////////////////////////////////////////////////
// General options

class COptMainDlg : public CDlgBase
{
	HTREEITEM hListHeading1, hListHeading2, hListHeading4;

	CCtrlTreeView checkBoxes;

	void CheckHeading(HTREEITEM hHeading)
	{
		BOOL bChecked = TRUE;

		if (hHeading == nullptr)
			return;

		TVITEMEX tvi;
		tvi.mask = TVIF_HANDLE | TVIF_STATE;
		tvi.hItem = checkBoxes.GetNextItem(hHeading, TVGN_CHILD);
		while (tvi.hItem && bChecked) {
			if (tvi.hItem != branch1[0].hItem && tvi.hItem != branch1[1].hItem) {
				checkBoxes.GetItem(&tvi);
				if (((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 1))
					bChecked = FALSE;
			}
			tvi.hItem = checkBoxes.GetNextSibling(tvi.hItem);
		}
		tvi.stateMask = TVIS_STATEIMAGEMASK;
		tvi.state = INDEXTOSTATEIMAGEMASK(bChecked ? 2 : 1);
		tvi.hItem = hHeading;
		checkBoxes.SetItem(&tvi);
	}

	void CheckBranches(HTREEITEM hHeading)
	{
		BOOL bChecked = TRUE;

		if (hHeading == nullptr)
			return;

		TVITEMEX tvi;
		tvi.mask = TVIF_HANDLE | TVIF_STATE;
		tvi.hItem = hHeading;
		checkBoxes.GetItem(&tvi);
		if (((tvi.state & TVIS_STATEIMAGEMASK) >> 12 == 2))
			bChecked = FALSE;
		tvi.hItem = checkBoxes.GetNextItem(hHeading, TVGN_CHILD);
		tvi.stateMask = TVIS_STATEIMAGEMASK;
		while (tvi.hItem) {
			tvi.state = INDEXTOSTATEIMAGEMASK(bChecked ? 2 : 1);
			if (tvi.hItem != branch1[0].hItem && tvi.hItem != branch1[1].hItem)
				checkBoxes.SetItem(&tvi);
			tvi.hItem = checkBoxes.GetNextSibling(tvi.hItem);
		}
	}

	HTREEITEM InsertBranch(char *pszDescr, BOOL bExpanded)
	{
		TVINSERTSTRUCT tvis = {};
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_TEXT | TVIF_STATE;
		tvis.item.pszText = Langpack_PcharToTchar(pszDescr);
		tvis.item.stateMask = bExpanded ? TVIS_STATEIMAGEMASK | TVIS_EXPANDED : TVIS_STATEIMAGEMASK;
		tvis.item.state = bExpanded ? INDEXTOSTATEIMAGEMASK(1) | TVIS_EXPANDED : INDEXTOSTATEIMAGEMASK(1);
		HTREEITEM res = checkBoxes.InsertItem(&tvis);
		mir_free(tvis.item.pszText);
		return res;
	}

	void FillBranch(HTREEITEM hParent, branch_t *branch, int nValues, uint32_t defaultval)
	{
		int iState;

		if (hParent == nullptr)
			return;

		TVINSERTSTRUCT tvis;
		tvis.hParent = hParent;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item.mask = TVIF_TEXT | TVIF_STATE;
		for (int i = 0; i < nValues; i++, branch++) {
			tvis.item.pszText = TranslateW(branch->szDescr);
			tvis.item.stateMask = TVIS_STATEIMAGEMASK;
			if (branch->iMode)
				iState = ((db_get_dw(0, CHAT_MODULE, branch->szDBName, defaultval) & branch->iMode) & branch->iMode) != 0 ? 2 : 1;
			else
				iState = db_get_b(0, CHAT_MODULE, branch->szDBName, branch->bDefault) != 0 ? 2 : 1;
			tvis.item.state = INDEXTOSTATEIMAGEMASK(iState);
			branch->hItem = checkBoxes.InsertItem(&tvis);
		}
	}

	void SaveBranch(branch_t *branch, int nValues)
	{
		int iState = 0;

		TVITEMEX tvi;
		tvi.mask = TVIF_HANDLE | TVIF_STATE;
		for (int i = 0; i < nValues; i++, branch++) {
			tvi.hItem = branch->hItem;
			checkBoxes.GetItem(&tvi);
			uint8_t bChecked = (((tvi.state & TVIS_STATEIMAGEMASK) >> 12) == 1) ? 0 : 1;
			if (branch->iMode) {
				if (bChecked)
					iState |= branch->iMode;
				if (iState & GC_EVENT_ADDSTATUS)
					iState |= GC_EVENT_REMOVESTATUS;
				db_set_dw(0, CHAT_MODULE, branch->szDBName, (uint32_t)iState);
			}
			else db_set_b(0, CHAT_MODULE, branch->szDBName, bChecked);
		}
	}

	void FixHeadings()
	{
		CheckHeading(hListHeading1);
		CheckHeading(hListHeading2);
		CheckHeading(hListHeading4);
	}

public:
	COptMainDlg()
		: CDlgBase(g_plugin, IDD_OPTIONS1),
		checkBoxes(this, IDC_CHECKBOXES)
	{
		checkBoxes.OnItemChanged = Callback(this, &COptMainDlg::onChange_Tree);
	}

	bool OnInitDialog() override
	{
		SetWindowLongPtr(checkBoxes.GetHwnd(), GWL_STYLE, GetWindowLongPtr(checkBoxes.GetHwnd(), GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);

		hListHeading1 = InsertBranch(LPGEN("Appearance and functionality of chat room windows"), db_get_b(0, CHAT_MODULE, "Branch1Exp", 0) ? TRUE : FALSE);
		hListHeading2 = InsertBranch(LPGEN("Appearance of the message log"), db_get_b(0, CHAT_MODULE, "Branch2Exp", 0) ? TRUE : FALSE);
		hListHeading4 = InsertBranch(LPGEN("Icons to display in the message log"), db_get_b(0, CHAT_MODULE, "Branch4Exp", 0) ? TRUE : FALSE);

		FillBranch(hListHeading1, branch1, _countof(branch1), 0);
		FillBranch(hListHeading2, branch2, _countof(branch2), 0);
		FillBranch(hListHeading4, branch4, _countof(branch4), 0x0000);

		FixHeadings();
		return true;
	}

	bool OnApply() override
	{
		SaveBranch(branch1, _countof(branch1));
		SaveBranch(branch2, _countof(branch2));
		SaveBranch(branch4, _countof(branch4));

		g_chatApi.ReloadSettings();
		Chat_UpdateOptions();
		return true;
	}

	void OnDestroy() override
	{
		uint8_t b = checkBoxes.GetItemState(hListHeading1, TVIS_EXPANDED) & TVIS_EXPANDED ? 1 : 0;
		db_set_b(0, CHAT_MODULE, "Branch1Exp", b);
		b = checkBoxes.GetItemState(hListHeading2, TVIS_EXPANDED) & TVIS_EXPANDED ? 1 : 0;
		db_set_b(0, CHAT_MODULE, "Branch2Exp", b);
		b = checkBoxes.GetItemState(hListHeading4, TVIS_EXPANDED) & TVIS_EXPANDED ? 1 : 0;
		db_set_b(0, CHAT_MODULE, "Branch4Exp", b);
	}

	void onChange_Tree(CCtrlTreeView::TEventInfo *evt)
	{
		TVITEMEX tvi;
		tvi.mask = TVIF_HANDLE | TVIF_STATE;
		tvi.hItem = evt->hItem;
		checkBoxes.GetItem(&tvi);
		
		if (tvi.hItem == branch1[0].hItem && INDEXTOSTATEIMAGEMASK(1) == tvi.state)
			checkBoxes.SetItemState(branch1[1].hItem, INDEXTOSTATEIMAGEMASK(1), TVIS_STATEIMAGEMASK);
		if (tvi.hItem == branch1[1].hItem && INDEXTOSTATEIMAGEMASK(1) == tvi.state)
			checkBoxes.SetItemState(branch1[0].hItem, INDEXTOSTATEIMAGEMASK(1), TVIS_STATEIMAGEMASK);

		if (tvi.hItem == hListHeading1)
			CheckBranches(hListHeading1);
		else if (tvi.hItem == hListHeading2)
			CheckBranches(hListHeading2);
		else if (tvi.hItem == hListHeading4)
			CheckBranches(hListHeading4);
		else
			FixHeadings();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Log & other options

class COptLogDlg : public CDlgBase
{
	CCtrlEdit edtGroup, edtLogDir, edtLogTimestamp, edtTimestamp, edtHighlight, edtInStamp, edtOutStamp, edtLimit;
	CCtrlSpin spin2, spin3, spin4;
	CCtrlCheck chkLogging, chkHighlight;
	CCtrlButton btnFontChoose;

public:
	COptLogDlg() :
		CDlgBase(g_plugin, IDD_OPTIONS2),
		spin2(this, IDC_SPIN2, 5000),
		spin3(this, IDC_SPIN3, 10000),
		spin4(this, IDC_SPIN4, 255, 10),
		
		edtGroup(this, IDC_GROUP),
		edtLimit(this, IDC_LIMIT),
		edtLogDir(this, IDC_LOGDIRECTORY),
		edtInStamp(this, IDC_INSTAMP),
		edtOutStamp(this, IDC_OUTSTAMP),
		edtTimestamp(this, IDC_TIMESTAMP),
		edtHighlight(this, IDC_HIGHLIGHTWORDS),
		edtLogTimestamp(this, IDC_LOGTIMESTAMP),

		chkLogging(this, IDC_LOGGING),
		chkHighlight(this, IDC_HIGHLIGHT),
		
		btnFontChoose(this, IDC_FONTCHOOSE)
	{
		chkLogging.OnChange = Callback(this, &COptLogDlg::onChange_Logging);
		chkHighlight.OnChange = Callback(this, &COptLogDlg::onChange_Highlight);

		btnFontChoose.OnClick = Callback(this, &COptLogDlg::onClick_Font);
	}

	bool OnInitDialog() override
	{
		spin2.SetPosition(db_get_w(0, CHAT_MODULE, "LogLimit", 100));
		spin3.SetPosition(db_get_w(0, CHAT_MODULE, "LoggingLimit", 100));
		spin4.SetPosition(db_get_b(0, CHAT_MODULE, "NicklistRowDist", 12));

		edtGroup.SetText(ptrW(Chat_GetGroup()));

		wchar_t szTemp[MAX_PATH];
		PathToRelativeW(g_Settings.pszLogDir, szTemp);
		edtLogDir.SetText(szTemp);

		edtInStamp.SetText(g_Settings.pszIncomingNick);
		edtOutStamp.SetText(g_Settings.pszOutgoingNick);
		edtHighlight.SetText(g_Settings.pszHighlightWords);
		edtTimestamp.SetText(g_Settings.pszTimeStamp);
		edtLogTimestamp.SetText(g_Settings.pszTimeStampLog);

		chkHighlight.SetState(g_Settings.bHighlightEnabled);
		edtHighlight.Enable(g_Settings.bHighlightEnabled);

		chkLogging.SetState(g_Settings.bLoggingEnabled);
		return true;
	}

	bool OnApply() override
	{
		ptrW pszText(rtrimw(edtHighlight.GetText()));
		if (*pszText) {
			for (wchar_t *p2 = wcschr(pszText, ','); p2; p2 = wcschr(pszText, ','))
				*p2 = ' ';
			db_set_ws(0, CHAT_MODULE, "HighlightWords", pszText);
		}
		else db_unset(0, CHAT_MODULE, "HighlightWords");

		pszText = rtrimw(edtLogDir.GetText());
		if (*pszText)
			db_set_ws(0, CHAT_MODULE, "LogDirectory", pszText);
		else
			db_unset(0, CHAT_MODULE, "LogDirectory");
		g_chatApi.SM_InvalidateLogDirectories();

		pszText = rtrimw(edtLogTimestamp.GetText());
		if (*pszText)
			db_set_ws(0, CHAT_MODULE, "LogTimestamp", pszText);
		else
			db_unset(0, CHAT_MODULE, "LogTimestamp");

		pszText = rtrimw(edtTimestamp.GetText());
		if (*pszText)
			db_set_ws(0, CHAT_MODULE, "HeaderTime", pszText);
		else
			db_unset(0, CHAT_MODULE, "HeaderTime");

		pszText = rtrimw(edtInStamp.GetText());
		if (*pszText)
			db_set_ws(0, CHAT_MODULE, "HeaderIncoming", pszText);
		else
			db_unset(0, CHAT_MODULE, "HeaderIncoming");

		pszText = rtrimw(edtOutStamp.GetText());
		if (*pszText)
			db_set_ws(0, CHAT_MODULE, "HeaderOutgoing", pszText);
		else
			db_unset(0, CHAT_MODULE, "HeaderOutgoing");

		Chat_SetGroup(ptrW(rtrimw(edtGroup.GetText())));

		g_Settings.bHighlightEnabled = chkHighlight.GetState();
		db_set_b(0, CHAT_MODULE, "HighlightEnabled", g_Settings.bHighlightEnabled);

		g_Settings.bLoggingEnabled = chkLogging.GetState();
		db_set_b(0, CHAT_MODULE, "LoggingEnabled", g_Settings.bLoggingEnabled);

		db_set_w(0, CHAT_MODULE, "LogLimit", spin2.GetPosition());
		db_set_w(0, CHAT_MODULE, "LoggingLimit", spin3.GetPosition());

		int iLen = spin4.GetPosition();
		if (iLen > 0)
			db_set_b(0, CHAT_MODULE, "NicklistRowDist", (uint8_t)iLen);
		else
			db_unset(0, CHAT_MODULE, "NicklistRowDist");

		g_chatApi.ReloadSettings();
		Chat_UpdateOptions();
		return true;
	}

	void onChange_Logging(CCtrlCheck*)
	{
		BOOL bStatus = chkLogging.GetState() != 0;
		edtLogDir.Enable(bStatus);
		btnFontChoose.Enable(bStatus);
		edtLimit.Enable(bStatus);
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
			edtLogDir.SetText(mir_wstrlen(szTemp) > 1 ? szTemp : L"Logs\\");
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
	CCtrlColor clrBack, clrText;

public:
	COptPopupDlg() :
		CDlgBase(g_plugin, IDD_OPTIONSPOPUP),
		clrBack(this, IDC_BKG),
		clrText(this, IDC_TEXT),
		chkRadio1(this, IDC_RADIO1),
		chkRadio2(this, IDC_RADIO2),
		chkRadio3(this, IDC_RADIO3)
	{
		chkRadio1.OnChange = chkRadio2.OnChange = chkRadio3.OnChange = Callback(this, &COptPopupDlg::onChange_Radio);
	}

	bool OnInitDialog() override
	{
		clrBack.SetColor(g_Settings.crPUBkgColour);
		clrText.SetColor(g_Settings.crPUTextColour);

		if (g_Settings.iPopupStyle == 2)
			CheckDlgButton(m_hwnd, IDC_RADIO2, BST_CHECKED);
		else if (g_Settings.iPopupStyle == 3)
			CheckDlgButton(m_hwnd, IDC_RADIO3, BST_CHECKED);
		else
			CheckDlgButton(m_hwnd, IDC_RADIO1, BST_CHECKED);
		onChange_Radio(0);

		SendDlgItemMessage(m_hwnd, IDC_SPIN1, UDM_SETRANGE, 0, MAKELONG(100, -1));
		SendDlgItemMessage(m_hwnd, IDC_SPIN1, UDM_SETPOS, 0, MAKELONG(g_Settings.iPopupTimeout, 0));
		return true;
	}

	bool OnApply() override
	{
		int iLen;
		if (IsDlgButtonChecked(m_hwnd, IDC_RADIO2) == BST_CHECKED)
			iLen = 2;
		else if (IsDlgButtonChecked(m_hwnd, IDC_RADIO3) == BST_CHECKED)
			iLen = 3;
		else
			iLen = 1;
		g_Settings.iPopupStyle = iLen;
		db_set_b(0, CHAT_MODULE, "PopupStyle", (uint8_t)iLen);

		iLen = SendDlgItemMessage(m_hwnd, IDC_SPIN1, UDM_GETPOS, 0, 0);
		g_Settings.iPopupTimeout = iLen;
		db_set_w(0, CHAT_MODULE, "PopupTimeout", (uint16_t)iLen);

		db_set_dw(0, CHAT_MODULE, "PopupColorBG", g_Settings.crPUBkgColour = clrBack.GetColor());
		db_set_dw(0, CHAT_MODULE, "PopupColorText", g_Settings.crPUTextColour = clrText.GetColor());
		return true;
	}
	
	void onChange_Radio(CCtrlCheck*)
	{
		bool bStatus = chkRadio3.GetState();
		clrBack.Enable(bStatus);
		clrText.Enable(bStatus);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Module init procedure

int ChatOptionsInitialize(WPARAM wParam)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = LPGEN("Group chats");

	odp.position = 910000000;
	odp.szTab.a = LPGEN("General");
	odp.pDialog = new COptMainDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.position = 910000001;
	odp.szTab.a = LPGEN("Chat log");
	odp.pDialog = new COptLogDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.position = 910000002;
	odp.szTitle.a = LPGEN("Chat");
	odp.szGroup.a = LPGEN("Popups");
	odp.szTab.a = nullptr;
	odp.pDialog = new COptPopupDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
