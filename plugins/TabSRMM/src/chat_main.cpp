/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-24 Miranda NG team,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// chat module exports and functions to load/unload the plugin.

#include "stdafx.h"

HANDLE g_hWindowList;

pfnDoPopup oldDoPopup, oldLogToFile;
pfnDoTrayIcon oldDoTrayIcon;

TMUCSettings g_Settings;

static void OnCreateSession(SESSION_INFO *si, MODULEINFO *mi)
{
	if (si->hContact)
		Chat_SetFilters(si);
	if (si->pDlg)
		si->pDlg->UpdateStatusBar();

	if (mi)
		mi->idleTimeStamp = time(0);
}

static void OnReplaceSession(SESSION_INFO *si)
{
	if (si->hContact)
		Chat_SetFilters(si);
	if (si->pDlg)
		RedrawWindow(GetDlgItem(si->pDlg->GetHwnd(), IDC_SRMM_NICKLIST), nullptr, nullptr, RDW_INVALIDATE);
}

static void OnSetTopic(SESSION_INFO *si)
{
	if (si->pDlg)
		si->pDlg->m_pPanel.Invalidate(true);
}

static void OnNewUser(SESSION_INFO *si, USERINFO*)
{
	if (si->pDlg)
		si->pDlg->GetMyNick();
}

static void OnChangeNick(SESSION_INFO *si)
{
	if (si->pDlg) {
		si->pDlg->GetMyNick();
		si->pDlg->UpdateStatusBar();
	}
}

static void OnCreateModule(MODULEINFO *mi)
{
	mi->idleTimeStamp = time(0);
}

static void OnLoadSettings()
{
	g_Settings.dwIconFlags = db_get_dw(0, CHAT_MODULE, "IconFlags", 0x0000);
	g_Settings.UserListColors[CHAT_STATUS_NORMAL] = db_get_dw(0, CHATFONT_MODULE, "Font18Col", RGB(0, 0, 0));
	g_Settings.UserListColors[CHAT_STATUS_AWAY] = db_get_dw(0, CHATFONT_MODULE, "Font19Col", RGB(170, 170, 170));
	g_Settings.UserListColors[CHAT_STATUS_OFFLINE] = db_get_dw(0, CHATFONT_MODULE, "Font5Col", RGB(160, 90, 90));

	g_Settings.bLogSymbols = g_plugin.bLogSymbols;
	g_Settings.bScaleIcons = g_plugin.bScaleIcons;
	g_Settings.bColorizeNicks = g_plugin.bColorizeNicks;
	g_Settings.bClickableNicks = g_plugin.bClickableNicks;
	g_Settings.bNewLineAfterNames = g_plugin.bNewLineAfterNames;
	g_Settings.bClassicIndicators = g_plugin.bClassicIndicators;
	g_Settings.bAlternativeSorting = g_plugin.bAlternativeSorting;
	g_Settings.bColorizeNicksInLog = g_plugin.bColorizeNicksInLog;
	g_Settings.bLogClassicIndicators = g_plugin.bLogClassicIndicators;

	replaceStrW(g_Settings.pszLogDir, M.getChatLogPath());
	replaceStrW(g_Settings.pwszAutoText, db_get_wsa(0, CHAT_MODULE, "TextAutocomplete", L":"));

	g_Settings.LogIconSize = (g_Settings.bScaleIcons) ? 12 : 16;

	// nicklist
	if (g_Settings.UserListFonts[0]) {
		DeleteObject(g_Settings.UserListFonts[CHAT_STATUS_NORMAL]);
		DeleteObject(g_Settings.UserListFonts[CHAT_STATUS_AWAY]);
		DeleteObject(g_Settings.UserListFonts[CHAT_STATUS_OFFLINE]);
	}

	LOGFONT lf;
	g_chatApi.LoadMsgDlgFont(18, &lf, nullptr);
	g_Settings.UserListFonts[CHAT_STATUS_NORMAL] = CreateFontIndirect(&lf);

	g_chatApi.LoadMsgDlgFont(19, &lf, nullptr);
	g_Settings.UserListFonts[CHAT_STATUS_AWAY] = CreateFontIndirect(&lf);

	g_chatApi.LoadMsgDlgFont(5, &lf, nullptr);
	g_Settings.UserListFonts[CHAT_STATUS_OFFLINE] = CreateFontIndirect(&lf);

	int ih = Chat_GetTextPixelSize(L"AQGglo", g_Settings.UserListFonts[CHAT_STATUS_NORMAL], false);
	int ih2 = Chat_GetTextPixelSize(L"AQGglo", g_Settings.UserListFonts[CHAT_STATUS_AWAY], false);
	g_Settings.iNickListFontHeight = max(db_get_b(0, CHAT_MODULE, "NicklistRowDist", 12), (ih > ih2 ? ih : ih2));

	for (int i = 0; i < 5; i++) {
		char szBuf[40];
		mir_snprintf(szBuf, "NickColor%d", i);
		g_Settings.nickColors[i] = db_get_dw(0, CHAT_MODULE, szBuf, g_Settings.UserListColors[0]);
	}
	g_Settings.nickColors[5] = db_get_dw(0, CHAT_MODULE, "NickColor5", GetSysColor(COLOR_HIGHLIGHT));
	g_Settings.nickColors[6] = db_get_dw(0, CHAT_MODULE, "NickColor6", GetSysColor(COLOR_HIGHLIGHTTEXT));

	if (g_Settings.SelectionBGBrush)
		DeleteObject(g_Settings.SelectionBGBrush);
	g_Settings.SelectionBGBrush = CreateSolidBrush(g_Settings.nickColors[5]);
}

/////////////////////////////////////////////////////////////////////////////////////////

void Chat_ModulesLoaded()
{
	g_Settings.hIconOverlay = LoadIconEx("overlay");
}

/////////////////////////////////////////////////////////////////////////////////////////
// load the group chat module

static int CopyChatSetting(const char *szSetting, void *param)
{
	LIST<char> *szSettings = (LIST<char>*)param;
	szSettings->insert(mir_strdup(szSetting));
	return 0;
}

static void CheckUpdate()
{
	// already converted?
	int compat = db_get_b(0, "Compatibility", "TabChatFonts", 0);
	if (compat >= 3)
		return;

	if (compat == 0) {
		LIST<char> szSettings(120);
		db_enum_settings(0, CopyChatSetting, CHAT_OLDFONTMODULE, &szSettings);

		DBVARIANT dbv;
		for (auto &p : szSettings.rev_iter()) {
			db_get(0, CHAT_OLDFONTMODULE, p, &dbv);
			db_set(0, CHATFONT_MODULE, p, &dbv);
			db_free(&dbv);
			mir_free(p);
		}

		db_delete_module(0, CHAT_OLDFONTMODULE);
		compat++;
	}

	if (compat == 1) {
		uint32_t oldBackColor = db_get_dw(0, FONTMODULE, "BkgColourMUC", SRMSGDEFSET_BKGCOLOUR);
		db_set_dw(0, CHAT_MODULE, "ColorLogBG", oldBackColor);
		db_unset(0, FONTMODULE, "BkgColourMUC");
		compat++;
	}

	if (compat == 2) {
		COLORREF color0 = db_get_dw(0, CHAT_MODULE, "NickColor2", 0);
		COLORREF color2 = db_get_dw(0, CHAT_MODULE, "NickColor0", 0);
		db_set_dw(0, CHAT_MODULE, "NickColor0", color0);
		db_set_dw(0, CHAT_MODULE, "NickColor2", color2);
		compat++;
	}

	db_set_b(0, "Compatibility", "TabChatFonts", 3);
}

/////////////////////////////////////////////////////////////////////////////////////////
// chat session creator

void ShowRoom(TContainerData *pContainer, SESSION_INFO *si)
{
	if (si)
		AutoCreateWindow(pContainer, si->hContact, true);
}

static void stubShowRoom(SESSION_INFO *si)
{
	if (!si)
		return;

	// when a group chat window is popped up from dblclick, we always activate it
	if (si->pDlg)
		si->pDlg->ActivateTab();
	else
		AutoCreateWindow(nullptr, si->hContact, true);
}

static MODULEINFO* MM_CreateModule()
{
	return new MODULEINFO();
}

static SESSION_INFO* SM_CreateSession()
{
	return new SESSION_INFO();
}

// load the module
int Chat_Load()
{
	if (M.GetBool(CHAT_MODULE, "UseCommaAsColon", false)) {
		db_unset(0, CHAT_MODULE, "UseCommaAsColon");
		db_set_ws(0, CHAT_MODULE, "TextAutocomplete", L",");
	}
	if (M.GetBool(CHAT_MODULE, "AddColonToAutoComplete", false)) {
		db_unset(0, CHAT_MODULE, "AddColonToAutoComplete");
		db_set_ws(0, CHAT_MODULE, "TextAutocomplete", L":");
	}

	CheckUpdate();
	Utils::RTF_CTableInit();

	CHAT_MANAGER_INITDATA data = { &g_Settings, sizeof(MODULEINFO), LPGENW("Message sessions") L"/" LPGENW("Group chats"), FONTMODE_ALTER, &g_plugin };
	Chat_CustomizeApi(&data);

	g_chatApi.CreateNick = OnCreateNick;
	g_chatApi.MM_CreateModule = MM_CreateModule;
	g_chatApi.OnCreateModule = OnCreateModule;
	g_chatApi.OnNewUser = OnNewUser;

	g_chatApi.OnSetTopic = OnSetTopic;

	g_chatApi.SM_CreateSession = SM_CreateSession;
	g_chatApi.OnCreateSession = OnCreateSession;
	g_chatApi.OnReplaceSession = OnReplaceSession;

	g_chatApi.ShowRoom = stubShowRoom;
	g_chatApi.OnChangeNick = OnChangeNick;
	g_chatApi.OnLoadSettings = OnLoadSettings;
	g_chatApi.DbEventIsShown = DbEventIsShown;

	Srmm_CreateHotkey(TABSRMM_HK_SECTION_GENERIC, LPGEN("Send message"));

	// this operation is unsafe, that's why we restore the old pci state on exit
	g_chatApi.DoSoundsFlashPopupTrayStuff = DoSoundsFlashPopupTrayStuff;
	g_chatApi.IsHighlighted = IsHighlighted;
	oldDoPopup = g_chatApi.DoPopup; g_chatApi.DoPopup = DoPopup;
	oldDoTrayIcon = g_chatApi.DoTrayIcon; g_chatApi.ShowPopup = ShowPopup;
	g_chatApi.UM_CompareItem = UM_CompareItem;
	g_chatApi.ReloadSettings();

	g_Settings.Highlight = new CMUCHighlight();
	return 0;
}

// unload the module. final cleanup
int Chat_Unload(void)
{
	if (g_Settings.SelectionBGBrush)
		DeleteObject(g_Settings.SelectionBGBrush);

	for (auto &it : g_Settings.UserListFonts)
		if (it)
			DeleteObject(it);

	delete g_Settings.Highlight;
	return 0;
}
