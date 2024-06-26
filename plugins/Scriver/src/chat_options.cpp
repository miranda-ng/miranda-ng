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

/////////////////////////////////////////////////////////////////////////////////////////

struct CChatOptionsBaseDlg : public CDlgBase
{
	CChatOptionsBaseDlg(int iDlgId) :
		CDlgBase(g_plugin, iDlgId)
	{
		m_OnFinishWizard = Callback(this, &CChatOptionsBaseDlg::onFinish);
	}

	void onFinish(void*)
	{
		g_chatApi.ReloadSettings();
		Chat_UpdateOptions();
		Chat_ReconfigureFilters();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class CGeneralOptsDlg : public CChatOptionsBaseDlg
{
	uint32_t m_dwFlags;

	CCtrlSpin spin2;
	CCtrlEdit m_group;
	CCtrlTreeOpts m_opts;

public:
	CGeneralOptsDlg() :
		CChatOptionsBaseDlg(IDD_OPTIONS1),
		m_opts(this, IDC_CHAT_CHECKBOXES),
		m_group(this, IDC_CHAT_GROUP),
		spin2(this, IDC_CHAT_SPIN2, 255, 10)
	{
		auto *pwszSection = TranslateT("Appearance and functionality of chat windows");
		m_opts.AddOption(pwszSection, TranslateT("Flash when someone speaks"), Chat::bFlashWindow);
		m_opts.AddOption(pwszSection, TranslateT("Flash when a word is highlighted"), Chat::bFlashWindowHighlight);
		m_opts.AddOption(pwszSection, TranslateT("Show chat nick list"), Chat::bShowNicklist);
		m_opts.AddOption(pwszSection, TranslateT("Show topic on your contact list (if supported)"), Chat::bTopicOnClist);
		m_opts.AddOption(pwszSection, TranslateT("Do not play sounds when focused"), g_plugin.bSoundsFocus);
		m_opts.AddOption(pwszSection, TranslateT("Do not pop up the window when joining a chat room"), Chat::bPopupOnJoin);
		m_opts.AddOption(pwszSection, TranslateT("Show contact statuses (if supported)"), Chat::bShowContactStatus);
		m_opts.AddOption(pwszSection, TranslateT("Display contact status icon before role icon"), Chat::bContactStatusFirst);
		m_opts.AddOption(pwszSection, TranslateT("Add ':' to auto-completed names"), g_plugin.bAddColonToAutoComplete);

		m_dwFlags = db_get_dw(0, CHAT_MODULE, "IconFlags");
		pwszSection = TranslateT("Icons to display in the message log");
		m_opts.AddOption(pwszSection, TranslateT("Show icon for topic changes"), m_dwFlags, GC_EVENT_TOPIC);
		m_opts.AddOption(pwszSection, TranslateT("Show icon for users joining"), m_dwFlags, GC_EVENT_JOIN);
		m_opts.AddOption(pwszSection, TranslateT("Show icon for users disconnecting"), m_dwFlags, GC_EVENT_QUIT);
		m_opts.AddOption(pwszSection, TranslateT("Show icon for messages"), m_dwFlags, GC_EVENT_MESSAGE);
		m_opts.AddOption(pwszSection, TranslateT("Show icon for actions"), m_dwFlags, GC_EVENT_ACTION);
		m_opts.AddOption(pwszSection, TranslateT("Show icon for highlights"), m_dwFlags, GC_EVENT_HIGHLIGHT);
		m_opts.AddOption(pwszSection, TranslateT("Show icon for users leaving"), m_dwFlags, GC_EVENT_PART);
		m_opts.AddOption(pwszSection, TranslateT("Show icon for users kicking other user"), m_dwFlags, GC_EVENT_KICK);
		m_opts.AddOption(pwszSection, TranslateT("Show icon for notices"), m_dwFlags, GC_EVENT_NOTICE);
		m_opts.AddOption(pwszSection, TranslateT("Show icon for name changes"), m_dwFlags, GC_EVENT_NICK);
		m_opts.AddOption(pwszSection, TranslateT("Show icon for information messages"), m_dwFlags, GC_EVENT_INFORMATION);
		m_opts.AddOption(pwszSection, TranslateT("Show icon for status changes"), m_dwFlags, GC_EVENT_ADDSTATUS);
	}

	bool OnInitDialog() override
	{
		spin2.SetPosition(db_get_b(0, CHAT_MODULE, "NicklistRowDist", 12));
		m_group.SetText(ptrW(Chat_GetGroup()));
		return true;
	}

	bool OnApply() override
	{
		ptrW wszGroup(m_group.GetText());
		if (mir_wstrlen(wszGroup) > 0)
			Chat_SetGroup(wszGroup);
		else 
			Chat_SetGroup(nullptr);

		int iPos = spin2.GetPosition();
		if (iPos > 0)
			db_set_b(0, CHAT_MODULE, "NicklistRowDist", (uint8_t)iPos);
		else
			db_unset(0, CHAT_MODULE, "NicklistRowDist");

		db_set_dw(0, CHAT_MODULE, "IconFlags", g_Settings.dwIconFlags = m_dwFlags);
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

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

class CLogOptsDlg : public CChatOptionsBaseDlg
{
	HWND hPathTip = nullptr;

	CCtrlSpin spin2, spin3;
	CCtrlCheck chkLogging, chkHighlight;
	CCtrlButton btnLogDir;
	CCtrlTreeOpts m_opts;

public:
	CLogOptsDlg() :
		CChatOptionsBaseDlg(IDD_OPTIONS2),
		m_opts(this, IDC_CHAT_CHECKBOXES),
		spin2(this, IDC_CHAT_SPIN2, 5000),
		spin3(this, IDC_CHAT_SPIN3, 10000),
		btnLogDir(this, IDC_CHAT_LOGDIRCHOOSE),
		chkLogging(this, IDC_CHAT_LOGGING),
		chkHighlight(this, IDC_CHAT_HIGHLIGHT)
	{
		btnLogDir.OnClick = Callback(this, &CLogOptsDlg::onClick_DirChange);

		chkLogging.OnChange = Callback(this, &CLogOptsDlg::onChange_Logging);
		chkHighlight.OnChange = Callback(this, &CLogOptsDlg::onChange_Highlight);

		auto *pwszSection = TranslateT("Appearance");
		m_opts.AddOption(pwszSection, TranslateT("Prefix all events with a timestamp"), Chat::bShowTime);
		m_opts.AddOption(pwszSection, TranslateT("Timestamp has same color as the event"), Chat::bTimeStampEventColour);
		m_opts.AddOption(pwszSection, TranslateT("Indent the second line of a message"), Chat::bLogIndentEnabled);
		m_opts.AddOption(pwszSection, TranslateT("Limit user names to 20 characters"), Chat::bLogLimitNames);
		m_opts.AddOption(pwszSection, TranslateT("Strip colors from messages"), Chat::bStripFormat);
		m_opts.AddOption(pwszSection, TranslateT("Enable 'event filter' for new rooms"), Chat::bFilterEnabled);
	}

	bool OnInitDialog() override
	{
		spin2.SetPosition(db_get_w(0, CHAT_MODULE, "LogLimit", 100));
		spin3.SetPosition(db_get_w(0, CHAT_MODULE, "LoggingLimit", 100));

		wchar_t tszTemp[MAX_PATH];
		PathToRelativeW(g_Settings.pszLogDir, tszTemp);
		SetDlgItemText(m_hwnd, IDC_CHAT_LOGDIRECTORY, tszTemp);

		wchar_t tszTooltipText[2048];
		mir_snwprintf(tszTooltipText,
			L"%s - %s\n%s - %s\n%s - %s\n%s - %s\n\n"
			L"%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n\n"
			L"%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s",
			// contact vars
			L"%nick%", TranslateT("nick of current contact (if defined)"),
			L"%proto%", TranslateT("protocol name of current contact (if defined). Account name is used when protocol supports multiple accounts"),
			L"%accountname%", TranslateT("user-defined account name of current contact (if defined)."),
			L"%userid%", TranslateT("user ID of current contact (if defined). It is like JID for Jabber, etc."),
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

		RECT rect;
		GetClientRect(GetDlgItem(m_hwnd, IDC_CHAT_LOGDIRECTORY), &rect);
		rect.left = -85;
		hPathTip = CreateToolTip(GetDlgItem(m_hwnd, IDC_CHAT_LOGDIRECTORY), tszTooltipText, TranslateT("Variables"), &rect);

		SetDlgItemText(m_hwnd, IDC_CHAT_HIGHLIGHTWORDS, g_Settings.pszHighlightWords);
		SetDlgItemText(m_hwnd, IDC_CHAT_LOGTIMESTAMP, g_Settings.pszTimeStampLog);
		SetDlgItemText(m_hwnd, IDC_CHAT_TIMESTAMP, g_Settings.pszTimeStamp);
		SetDlgItemText(m_hwnd, IDC_CHAT_OUTSTAMP, g_Settings.pszOutgoingNick);
		SetDlgItemText(m_hwnd, IDC_CHAT_INSTAMP, g_Settings.pszIncomingNick);
		CheckDlgButton(m_hwnd, IDC_CHAT_HIGHLIGHT, g_Settings.bHighlightEnabled ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_CHAT_LOGGING, g_Settings.bLoggingEnabled ? BST_CHECKED : BST_UNCHECKED);
		return true;
	}

	bool OnApply() override
	{
		char *pszText = nullptr;

		int iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_CHAT_HIGHLIGHTWORDS));
		if (iLen > 0) {
			wchar_t *ptszText = (wchar_t *)mir_alloc((iLen + 2) * sizeof(wchar_t));
			wchar_t *p2 = nullptr;

			if (ptszText) {
				GetDlgItemText(m_hwnd, IDC_CHAT_HIGHLIGHTWORDS, ptszText, iLen + 1);
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

		iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_CHAT_LOGDIRECTORY));
		if (iLen > 0) {
			wchar_t *pszText1 = (wchar_t *)malloc(iLen * sizeof(wchar_t) + 2);
			GetDlgItemText(m_hwnd, IDC_CHAT_LOGDIRECTORY, pszText1, iLen + 1);
			db_set_ws(0, CHAT_MODULE, "LogDirectory", pszText1);
			free(pszText1);
		}
		else {
			mir_wstrncpy(g_Settings.pszLogDir, DEFLOGFILENAME, MAX_PATH);
			db_unset(0, CHAT_MODULE, "LogDirectory");
		}
		g_chatApi.SM_InvalidateLogDirectories();

		iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_CHAT_LOGTIMESTAMP));
		if (iLen > 0) {
			pszText = (char *)mir_realloc(pszText, iLen + 1);
			GetDlgItemTextA(m_hwnd, IDC_CHAT_LOGTIMESTAMP, pszText, iLen + 1);
			db_set_s(0, CHAT_MODULE, "LogTimestamp", pszText);
		}
		else db_unset(0, CHAT_MODULE, "LogTimestamp");

		iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_CHAT_TIMESTAMP));
		if (iLen > 0) {
			pszText = (char *)mir_realloc(pszText, iLen + 1);
			GetDlgItemTextA(m_hwnd, IDC_CHAT_TIMESTAMP, pszText, iLen + 1);
			db_set_s(0, CHAT_MODULE, "HeaderTime", pszText);
		}
		else db_unset(0, CHAT_MODULE, "HeaderTime");

		iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_CHAT_INSTAMP));
		if (iLen > 0) {
			pszText = (char *)mir_realloc(pszText, iLen + 1);
			GetDlgItemTextA(m_hwnd, IDC_CHAT_INSTAMP, pszText, iLen + 1);
			db_set_s(0, CHAT_MODULE, "HeaderIncoming", pszText);
		}
		else db_unset(0, CHAT_MODULE, "HeaderIncoming");

		iLen = GetWindowTextLength(GetDlgItem(m_hwnd, IDC_CHAT_OUTSTAMP));
		if (iLen > 0) {
			pszText = (char *)mir_realloc(pszText, iLen + 1);
			GetDlgItemTextA(m_hwnd, IDC_CHAT_OUTSTAMP, pszText, iLen + 1);
			db_set_s(0, CHAT_MODULE, "HeaderOutgoing", pszText);
		}
		else db_unset(0, CHAT_MODULE, "HeaderOutgoing");

		g_Settings.bHighlightEnabled = IsDlgButtonChecked(m_hwnd, IDC_CHAT_HIGHLIGHT) == BST_CHECKED ? TRUE : FALSE;
		db_set_b(0, CHAT_MODULE, "HighlightEnabled", (uint8_t)g_Settings.bHighlightEnabled);

		g_Settings.bLoggingEnabled = IsDlgButtonChecked(m_hwnd, IDC_CHAT_LOGGING) == BST_CHECKED ? TRUE : FALSE;
		db_set_b(0, CHAT_MODULE, "LoggingEnabled", (uint8_t)g_Settings.bLoggingEnabled);

		iLen = SendDlgItemMessage(m_hwnd, IDC_CHAT_SPIN2, UDM_GETPOS, 0, 0);
		db_set_w(0, CHAT_MODULE, "LogLimit", (uint16_t)iLen);
		iLen = SendDlgItemMessage(m_hwnd, IDC_CHAT_SPIN3, UDM_GETPOS, 0, 0);
		db_set_w(0, CHAT_MODULE, "LoggingLimit", (uint16_t)iLen);

		mir_free(pszText);
		
		g_Settings.bStripFormat = db_get_b(0, CHAT_MODULE, "TrimFormatting", 0) != 0;
		g_Settings.bLogIndentEnabled = Chat::bLogIndentEnabled;
		return true;
	}

	void OnDestroy() override
	{
		if (hPathTip) {
			DestroyWindow(hPathTip);
			hPathTip = nullptr;
		}
	}

	void onChange_Logging(CCtrlCheck *pCheck)
	{
		bool bChecked = pCheck->IsChecked();
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHAT_LIMIT), bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHAT_LOGDIRECTORY), bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHAT_LOGDIRCHOOSE), bChecked);
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHAT_LIMITTEXT2), bChecked);
	}

	void onChange_Highlight(CCtrlCheck *pCheck)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_CHAT_HIGHLIGHTWORDS), pCheck->IsChecked());
	}

	void onClick_DirChange(CCtrlButton *)
	{
		LPMALLOC psMalloc;
		if (SUCCEEDED(CoGetMalloc(1, &psMalloc))) {
			wchar_t tszDirectory[MAX_PATH], tszTemp[MAX_PATH];
			BROWSEINFO bi = {};
			bi.hwndOwner = m_hwnd;
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
				SetDlgItemText(m_hwnd, IDC_CHAT_LOGDIRECTORY, mir_wstrlen(tszTemp) > 1 ? tszTemp : DEFLOGFILENAME);
			}
			psMalloc->Free(idList);
			psMalloc->Release();
		}
	}
};

void ChatOptInitialize(WPARAM wParam)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 910000000;
	odp.flags = ODPF_BOLDGROUPS;
	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = LPGEN("Group chats");

	odp.pDialog = new CGeneralOptsDlg();
	odp.szTab.a = LPGEN("General");
	g_plugin.addOptions(wParam, &odp);

	if (!Srmm_IsCustomLogUsed(true)) {
		odp.pDialog = new CLogOptsDlg();
		odp.szTab.a = LPGEN("Log formatting");
		g_plugin.addOptions(wParam, &odp);
	}
}
