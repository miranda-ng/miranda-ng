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

// add icons to the skinning module

static IconItem iconList1[] =
{
	{ LPGEN("Window icon"),           "window",    IDI_CHANMGR    },
	{ LPGEN("Text color"),            "fgcol",     IDI_COLOR      },
	{ LPGEN("Background color"),      "bkgcol",    IDI_BKGCOLOR   },
	{ LPGEN("Bold"),                  "bold",      IDI_BBOLD      },
	{ LPGEN("Italics"),               "italics",   IDI_BITALICS   },
	{ LPGEN("Underlined"),            "underline", IDI_BUNDERLINE },
	{ LPGEN("Strikethrough"),         "strikeout", IDI_STRIKEOUT  },
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

struct CChatOptionsBaseDlg : public CDlgBase
{
	CChatOptionsBaseDlg(int iDlgId) :
		CDlgBase(g_plugin, iDlgId)
	{
		m_OnFinishWizard = Callback(this, &CChatOptionsBaseDlg::onFinish);
	}

	void onFinish(void *)
	{
		g_chatApi.ReloadSettings();
		Chat_UpdateOptions();
		Chat_ReconfigureFilters();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// General options

class COptMainDlg : public CChatOptionsBaseDlg
{
	uint32_t m_dwFlags;

	CCtrlEdit edtGroup;
	CCtrlCheck chkUseGroup;
	CCtrlTreeOpts checkBoxes;

public:
	COptMainDlg() :
		CChatOptionsBaseDlg(IDD_OPTIONS1),
		edtGroup(this, IDC_GROUP),
		checkBoxes(this, IDC_CHECKBOXES),
		chkUseGroup(this, IDC_CHAT_USEGROUP)
	{
		CreateLink(chkUseGroup, Chat::bUseGroup);
		chkUseGroup.OnChange = Callback(this, &COptMainDlg::onChange_UseGroup);

		m_dwFlags = db_get_dw(0, CHAT_MODULE, "IconFlags");

		auto *pwszSection = TranslateT("Appearance and functionality of chat room windows");
		checkBoxes.AddOption(pwszSection, TranslateT("Flash window when someone speaks"), Chat::bFlashWindow);
		checkBoxes.AddOption(pwszSection, TranslateT("Flash window when a word is highlighted"), Chat::bFlashWindowHighlight);
		checkBoxes.AddOption(pwszSection, TranslateT("Show list of users in the chat room"), Chat::bShowNicklist);
		checkBoxes.AddOption(pwszSection, TranslateT("Show the topic of the room on your contact list (if supported)"), Chat::bTopicOnClist);
		checkBoxes.AddOption(pwszSection, TranslateT("Do not play sounds when the chat room is focused"), g_plugin.bSoundsFocus);
		checkBoxes.AddOption(pwszSection, TranslateT("Do not pop up the window when joining a chat room"), Chat::bPopupOnJoin);
		checkBoxes.AddOption(pwszSection, TranslateT("Show contact statuses if protocol supports them"), Chat::bShowContactStatus);
		checkBoxes.AddOption(pwszSection, TranslateT("Display contact status icon before user role icon"), Chat::bContactStatusFirst);

		pwszSection = TranslateT("Appearance of the message log");
		checkBoxes.AddOption(pwszSection, TranslateT("Prefix all events with a timestamp"), Chat::bShowTime);
		checkBoxes.AddOption(pwszSection, TranslateT("Only prefix with timestamp if it has changed"), Chat::bShowTimeIfChanged);
		checkBoxes.AddOption(pwszSection, TranslateT("Timestamp has same color as the event"), Chat::bTimeStampEventColour);
		checkBoxes.AddOption(pwszSection, TranslateT("Indent the second line of a message"), Chat::bLogIndentEnabled);
		checkBoxes.AddOption(pwszSection, TranslateT("Limit user names in the message log to 20 characters"), Chat::bLogLimitNames);
		checkBoxes.AddOption(pwszSection, TranslateT("Add ':' to auto-completed user names"), g_plugin.bAddColonToAutoComplete);
		checkBoxes.AddOption(pwszSection, TranslateT("Strip colors from messages in the log"), Chat::bStripFormat);
		checkBoxes.AddOption(pwszSection, TranslateT("Enable the 'event filter' for new rooms"), Chat::bFilterEnabled);

		pwszSection = TranslateT("Icons to display in the message log");
		checkBoxes.AddOption(pwszSection, TranslateT("Show icon for topic changes"), m_dwFlags, GC_EVENT_TOPIC);
		checkBoxes.AddOption(pwszSection, TranslateT("Show icon for users joining"), m_dwFlags, GC_EVENT_JOIN);
		checkBoxes.AddOption(pwszSection, TranslateT("Show icon for users disconnecting"), m_dwFlags, GC_EVENT_QUIT);
		checkBoxes.AddOption(pwszSection, TranslateT("Show icon for messages"), m_dwFlags, GC_EVENT_MESSAGE);
		checkBoxes.AddOption(pwszSection, TranslateT("Show icon for actions"), m_dwFlags, GC_EVENT_ACTION);
		checkBoxes.AddOption(pwszSection, TranslateT("Show icon for highlights"), m_dwFlags, GC_EVENT_HIGHLIGHT);
		checkBoxes.AddOption(pwszSection, TranslateT("Show icon for users leaving"), m_dwFlags, GC_EVENT_PART);
		checkBoxes.AddOption(pwszSection, TranslateT("Show icon for users kicking other user"), m_dwFlags, GC_EVENT_KICK);
		checkBoxes.AddOption(pwszSection, TranslateT("Show icon for notices"), m_dwFlags, GC_EVENT_NOTICE);
		checkBoxes.AddOption(pwszSection, TranslateT("Show icon for name changes"), m_dwFlags, GC_EVENT_NICK);
		checkBoxes.AddOption(pwszSection, TranslateT("Show icon for information messages"), m_dwFlags, GC_EVENT_INFORMATION);
		checkBoxes.AddOption(pwszSection, TranslateT("Show icon for status changes"), m_dwFlags, GC_EVENT_ADDSTATUS);
	}

	bool OnInitDialog() override
	{
		edtGroup.SetText(Chat_GetGroup());
		return true;
	}

	bool OnApply() override
	{
		Chat_SetGroup(ptrW(rtrimw(edtGroup.GetText())));
		db_set_dw(0, CHAT_MODULE, "IconFlags", m_dwFlags);
		return true;
	}

	void onChange_UseGroup(CCtrlCheck *)
	{
		edtGroup.Enable(chkUseGroup.IsChecked());
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Log & other options

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

class COptLogDlg : public CChatOptionsBaseDlg
{
	CCtrlEdit edtLogDir, edtLogTimestamp, edtTimestamp, edtHighlight, edtInStamp, edtOutStamp, edtLimit;
	CCtrlSpin spin2, spin3, spin4;
	CCtrlCheck chkLogging, chkHighlight;
	CCtrlButton btnFontChoose;

public:
	COptLogDlg() :
		CChatOptionsBaseDlg(IDD_OPTIONS2),
		spin2(this, IDC_SPIN2, 5000),
		spin3(this, IDC_SPIN3, 10000),
		spin4(this, IDC_SPIN4, 255, 10),
		
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

	if (!Srmm_IsCustomLogUsed(true)) {
		odp.position = 910000001;
		odp.szTab.a = LPGEN("Chat log");
		odp.pDialog = new COptLogDlg();
		g_plugin.addOptions(wParam, &odp);
	}
	return 0;
}
