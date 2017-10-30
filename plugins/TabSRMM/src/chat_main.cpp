/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (ñ) 2012-17 Miranda NG project,
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

CHAT_MANAGER *pci;
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
	g_Settings.dwIconFlags = M.GetDword(CHAT_MODULE, "IconFlags", 0x0000);
	g_Settings.bOpenInDefault = M.GetBool(CHAT_MODULE, "DefaultContainer", true);
	g_Settings.UserListColors[CHAT_STATUS_NORMAL] = M.GetDword(CHATFONT_MODULE, "Font18Col", RGB(0, 0, 0));
	g_Settings.UserListColors[CHAT_STATUS_AWAY] = M.GetDword(CHATFONT_MODULE, "Font19Col", RGB(170, 170, 170));
	g_Settings.UserListColors[CHAT_STATUS_OFFLINE] = M.GetDword(CHATFONT_MODULE, "Font5Col", RGB(160, 90, 90));
	g_Settings.bBBCodeInPopups = M.GetBool(CHAT_MODULE, "BBCodeInPopups", false);
	g_Settings.bClassicIndicators = M.GetBool(CHAT_MODULE, "ClassicIndicators", false);
	g_Settings.bLogClassicIndicators = M.GetBool(CHAT_MODULE, "LogClassicIndicators", false);
	g_Settings.bAlternativeSorting = M.GetBool(CHAT_MODULE, "AlternativeSorting", true);
	g_Settings.bAnnoyingHighlight = M.GetBool(CHAT_MODULE, "AnnoyingHighlight", false);
	g_Settings.bCreateWindowOnHighlight = M.GetBool(CHAT_MODULE, "CreateWindowOnHighlight", true);

	g_Settings.bLogSymbols = M.GetBool(CHAT_MODULE, "LogSymbols", true);
	g_Settings.bClickableNicks = M.GetBool(CHAT_MODULE, "ClickableNicks", true);
	g_Settings.bColorizeNicks = M.GetBool(CHAT_MODULE, "ColorizeNicks", true);
	g_Settings.bColorizeNicksInLog = M.GetBool(CHAT_MODULE, "ColorizeNicksInLog", true);
	g_Settings.bScaleIcons = M.GetBool(CHAT_MODULE, "ScaleIcons", true);

	g_Settings.bDoubleClick4Privat = M.GetBool(CHAT_MODULE, "DoubleClick4Privat", false);
	g_Settings.bShowContactStatus = M.GetBool(CHAT_MODULE, "ShowContactStatus", true);
	g_Settings.bContactStatusFirst = M.GetBool(CHAT_MODULE, "ContactStatusFirst", false);

	g_Settings.bNewLineAfterNames = M.GetBool(CHAT_MODULE, "NewlineAfterNames", false);
	g_Settings.bUseCommaAsColon = M.GetBool(CHAT_MODULE, "UseCommaAsColon", false);

	replaceStrW(g_Settings.pszLogDir, M.getChatLogPath());

	g_Settings.LogIconSize = (g_Settings.bScaleIcons) ? 12 : 16;

	// nicklist
	if (g_Settings.UserListFonts[0]) {
		DeleteObject(g_Settings.UserListFonts[CHAT_STATUS_NORMAL]);
		DeleteObject(g_Settings.UserListFonts[CHAT_STATUS_AWAY]);
		DeleteObject(g_Settings.UserListFonts[CHAT_STATUS_OFFLINE]);
	}

	LOGFONT lf;
	pci->LoadMsgDlgFont(18, &lf, nullptr);
	g_Settings.UserListFonts[CHAT_STATUS_NORMAL] = CreateFontIndirect(&lf);

	pci->LoadMsgDlgFont(19, &lf, nullptr);
	g_Settings.UserListFonts[CHAT_STATUS_AWAY] = CreateFontIndirect(&lf);

	pci->LoadMsgDlgFont(5, &lf, nullptr);
	g_Settings.UserListFonts[CHAT_STATUS_OFFLINE] = CreateFontIndirect(&lf);

	int ih = Chat_GetTextPixelSize(L"AQGglo", g_Settings.UserListFonts[CHAT_STATUS_NORMAL], false);
	int ih2 = Chat_GetTextPixelSize(L"AQGglo", g_Settings.UserListFonts[CHAT_STATUS_AWAY], false);
	g_Settings.iNickListFontHeight = max(M.GetByte(CHAT_MODULE, "NicklistRowDist", 12), (ih > ih2 ? ih : ih2));

	for (int i = 0; i < 5; i++) {
		char szBuf[40];
		mir_snprintf(szBuf, "NickColor%d", i);
		g_Settings.nickColors[i] = M.GetDword(CHAT_MODULE, szBuf, g_Settings.UserListColors[0]);
	}
	g_Settings.nickColors[5] = M.GetDword(CHAT_MODULE, "NickColor5", GetSysColor(COLOR_HIGHLIGHT));
	g_Settings.nickColors[6] = M.GetDword(CHAT_MODULE, "NickColor6", GetSysColor(COLOR_HIGHLIGHTTEXT));

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

static int CopyChatSetting(const char *szSetting, LPARAM param)
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
		for (int i = szSettings.getCount() - 1; i >= 0; i--) {
			char *p = szSettings[i];
			db_get(0, CHAT_OLDFONTMODULE, p, &dbv);
			db_set(0, CHATFONT_MODULE, p, &dbv);
			db_free(&dbv);
			mir_free(p);
		}

		db_delete_module(0, CHAT_OLDFONTMODULE);
		compat++;
	}

	if (compat == 1) {
		DWORD oldBackColor = db_get_dw(0, FONTMODULE, "BkgColourMUC", SRMSGDEFSET_BKGCOLOUR);
		db_set_dw(0, CHAT_MODULE, "ColorLogBG", oldBackColor);
		db_unset(0, FONTMODULE, "BkgColourMUC");
		compat++;
	}

	if (compat == 2) {
		COLORREF color0 = M.GetDword(CHAT_MODULE, "NickColor2", 0);
		COLORREF color2 = M.GetDword(CHAT_MODULE, "NickColor0", 0);
		db_set_dw(0, CHAT_MODULE, "NickColor0", color0);
		db_set_dw(0, CHAT_MODULE, "NickColor2", color2);
		compat++;
	}

	db_set_b(0, "Compatibility", "TabChatFonts", 3);
}

/////////////////////////////////////////////////////////////////////////////////////////

static void stubShowRoom(SESSION_INFO *si)
{
	ShowRoom(nullptr, si);
}

// load the module
int Chat_Load()
{
	CheckUpdate();
	Utils::RTF_CTableInit();

	CHAT_MANAGER_INITDATA data = { &g_Settings, sizeof(MODULEINFO), sizeof(SESSION_INFO), LPGENW("Message Sessions") L"/" LPGENW("Group chats"), FONTMODE_ALTER };
	pci = Chat_GetInterface(&data);
	
	pci->OnCreateModule = OnCreateModule;
	pci->OnNewUser = OnNewUser;

	pci->OnSetTopic = OnSetTopic;

	pci->OnCreateSession = OnCreateSession;
	pci->OnReplaceSession = OnReplaceSession;

	pci->OnChangeNick = OnChangeNick;
	pci->ShowRoom = stubShowRoom;
	pci->OnLoadSettings = OnLoadSettings;

	// this operation is unsafe, that's why we restore the old pci state on exit
	pci->DoSoundsFlashPopupTrayStuff = DoSoundsFlashPopupTrayStuff;
	pci->IsHighlighted = IsHighlighted;
	oldLogToFile = pci->LogToFile; pci->LogToFile = LogToFile;
	oldDoPopup = pci->DoPopup; pci->DoPopup = DoPopup;
	oldDoTrayIcon = pci->DoTrayIcon; pci->ShowPopup = ShowPopup;
	pci->Log_CreateRTF = Log_CreateRTF;
	pci->Log_CreateRtfHeader = Log_CreateRtfHeader;
	pci->UM_CompareItem = UM_CompareItem;
	pci->ReloadSettings();

	g_Settings.Highlight = new CMUCHighlight();
	return 0;
}

// unload the module. final cleanup
int Chat_Unload(void)
{
	if (g_Settings.SelectionBGBrush)
		DeleteObject(g_Settings.SelectionBGBrush);

	for (int i = 0; i < _countof(g_Settings.UserListFonts); i++)
		if (g_Settings.UserListFonts[i])
			DeleteObject(g_Settings.UserListFonts[i]);

	delete g_Settings.Highlight;
	return 0;
}
