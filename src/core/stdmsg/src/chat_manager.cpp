/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-22 Miranda NG team,
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

pfnDoTrayIcon oldDoTrayIcon;
pfnDoPopup    oldDoPopup;

SESSION_INFO* SM_GetPrevWindow(SESSION_INFO *si)
{
	int i = g_chatApi.arSessions.indexOf(si);
	if (i == -1)
		return nullptr;

	for (i--; i >= 0; i--) {
		SESSION_INFO *p = g_chatApi.arSessions[i];
		if (p->pDlg)
			return p;
	}

	return nullptr;
}

SESSION_INFO* SM_GetNextWindow(SESSION_INFO *si)
{
	int i = g_chatApi.arSessions.indexOf(si);
	if (i == -1)
		return nullptr;

	for (i++; i < g_chatApi.arSessions.getCount(); i++) {
		SESSION_INFO *p = g_chatApi.arSessions[i];
		if (p->pDlg)
			return p;
	}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

HMENU g_hMenu = nullptr;

GlobalLogSettings g_Settings;

static MODULEINFO* MM_CreateModule()
{
	return new MODULEINFO();
}

static void OnReplaceSession(SESSION_INFO *si)
{
	if (si->pDlg)
		RedrawWindow(GetDlgItem(si->pDlg->GetHwnd(), IDC_SRMM_NICKLIST), nullptr, nullptr, RDW_INVALIDATE);
}

static void OnFlashHighlight(SESSION_INFO *si, int bInactive)
{
	if (!bInactive || si->pDlg == nullptr)
		return;

	if (g_Settings.bTabsEnable)
		g_pTabDialog->SetMessageHighlight(si->pDlg);
	else if (g_Settings.bFlashWindowHighlight)
		si->pDlg->StartFlash();
}

static void OnFlashWindow(SESSION_INFO *si, int bInactive)
{
	if (!bInactive || si->pDlg == nullptr)
		return;

	if (g_Settings.bTabsEnable)
		g_pTabDialog->SetTabHighlight(si->pDlg);
	else if (g_Settings.bFlashWindow)
		si->pDlg->StartFlash();
}

static BOOL DoTrayIcon(SESSION_INFO *si, GCEVENT *gce)
{
	if (gce->iType & g_Settings.dwTrayIconFlags)
		return oldDoTrayIcon(si, gce);
	return TRUE;
}

static BOOL DoPopup(SESSION_INFO *si, GCEVENT *gce)
{
	if (gce->iType & g_Settings.dwPopupFlags)
		return oldDoPopup(si, gce);
	return TRUE;
}

static void OnLoadSettings()
{
	g_Settings.iX = db_get_dw(0, CHAT_MODULE, "roomx", -1);
	g_Settings.iY = db_get_dw(0, CHAT_MODULE, "roomy", -1);

	g_Settings.bTabsEnable = db_get_b(0, CHAT_MODULE, "Tabs", 1) != 0;
	g_Settings.bTabsAtBottom = db_get_b(0, CHAT_MODULE, "TabBottom", 0) != 0;
	g_Settings.bTabCloseOnDblClick = db_get_b(0, CHAT_MODULE, "TabCloseOnDblClick", 0) != 0;

	g_Settings.iSplitterX = db_get_w(0, CHAT_MODULE, "SplitterX", 105);
	if (g_Settings.iSplitterX <= 50)
		g_Settings.iSplitterX = 105;
	g_Settings.iSplitterY = db_get_w(0, CHAT_MODULE, "SplitterY", 90);
	if (g_Settings.iSplitterY <= 65)
		g_Settings.iSplitterY = 90;
}

static void RegisterFonts()
{
	ColourIDW colourid = {};
	strncpy(colourid.dbSettingsGroup, CHAT_MODULE, sizeof(colourid.dbSettingsGroup));
	wcsncpy(colourid.group, LPGENW("Message sessions") L"/" LPGENW("Chat module"), _countof(colourid.group));

	strncpy(colourid.setting, "ColorLogBG", _countof(colourid.setting));
	wcsncpy(colourid.name, LPGENW("Group chat log background"), _countof(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	g_plugin.addColor(&colourid);

	strncpy(colourid.setting, "ColorMessageBG", _countof(colourid.setting));
	wcsncpy(colourid.name, LPGENW("Message background"), _countof(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	g_plugin.addColor(&colourid);

	strncpy(colourid.setting, "ColorNicklistBG", _countof(colourid.setting));
	wcsncpy(colourid.name, LPGENW("Nick list background"), _countof(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	g_plugin.addColor(&colourid);

	strncpy(colourid.setting, "ColorNicklistLines", _countof(colourid.setting));
	wcsncpy(colourid.name, LPGENW("Nick list lines"), _countof(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_INACTIVEBORDER);
	g_plugin.addColor(&colourid);

	strncpy(colourid.setting, "ColorNicklistSelectedBG", _countof(colourid.setting));
	wcsncpy(colourid.name, LPGENW("Nick list background (selected)"), _countof(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_HIGHLIGHT);
	g_plugin.addColor(&colourid);
}

static void ShowRoom(SESSION_INFO *si)
{
	if (!si)
		return;

	// Do we need to create a window?
	if (si->pDlg == nullptr) {
		CTabbedWindow *pContainer = GetContainer();
		if (g_Settings.bTabsEnable) {
			pContainer->AddPage(si);
			PostMessage(pContainer->GetHwnd(), WM_SIZE, 0, 0);
		}
		else {
			CMsgDialog *pDlg = pContainer->m_pEmbed = new CMsgDialog(pContainer, si);
			pContainer->Create();
			pDlg->SetParent(pContainer->GetHwnd());
			pDlg->Create();
			pContainer->Show();
			pContainer->FixTabIcons(pDlg);
			PostMessage(pContainer->GetHwnd(), WM_SIZE, 0, 0);
		}

		if (si->iType != GCW_SERVER)
			si->pDlg->UpdateNickList();
		else
			si->pDlg->UpdateTitle();
		si->pDlg->UpdateStatusBar();
	}
	else if (g_Settings.bTabsEnable && g_pTabDialog)
		g_pTabDialog->m_tab.ActivatePage(g_pTabDialog->m_tab.GetDlgIndex(si->pDlg));

	SetWindowLongPtr(si->pDlg->GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(si->pDlg->GetHwnd(), GWL_EXSTYLE) | WS_EX_APPWINDOW);

	if (IsIconic(si->pDlg->GetHwnd()))
		si->pDlg->Show(SW_NORMAL);
	si->pDlg->Show(SW_SHOW);
	SetForegroundWindow(si->pDlg->GetHwnd());
}

int OnCheckPlugins(WPARAM, LPARAM)
{
	g_plugin.bSmileyInstalled = ServiceExists(MS_SMILEYADD_REPLACESMILEYS);
	return 0;
}

void Load_ChatModule()
{
	AddIcons();
	RegisterFonts();

	CHAT_MANAGER_INITDATA data = { &g_Settings, sizeof(MODULEINFO), sizeof(SESSION_INFO), LPGENW("Message sessions") L"/" LPGENW("Chat module"), FONTMODE_USE, &g_plugin };
	Chat_CustomizeApi(&data);

	g_chatApi.MM_CreateModule = MM_CreateModule;
	g_chatApi.OnReplaceSession = OnReplaceSession;

	g_chatApi.OnLoadSettings = OnLoadSettings;
	g_chatApi.OnFlashWindow = OnFlashWindow;
	g_chatApi.OnFlashHighlight = OnFlashHighlight;
	g_chatApi.ShowRoom = ShowRoom;

	Srmm_CreateHotkey(LPGEN("Messaging"), LPGEN("Send message"));

	oldDoPopup = g_chatApi.DoPopup; g_chatApi.DoPopup = DoPopup;
	oldDoTrayIcon = g_chatApi.DoTrayIcon; g_chatApi.DoTrayIcon = DoTrayIcon;
	g_chatApi.ReloadSettings();

	g_hMenu = LoadMenu(g_plugin.getInst(), MAKEINTRESOURCE(IDR_MENU));

	HookEvent(ME_SYSTEM_MODULELOAD, OnCheckPlugins);
}

void Unload_ChatModule()
{
	db_set_w(0, CHAT_MODULE, "SplitterX", (uint16_t)g_Settings.iSplitterX);
	db_set_w(0, CHAT_MODULE, "SplitterY", (uint16_t)g_Settings.iSplitterY);
	db_set_dw(0, CHAT_MODULE, "roomx", g_Settings.iX);
	db_set_dw(0, CHAT_MODULE, "roomy", g_Settings.iY);
	db_set_dw(0, CHAT_MODULE, "roomwidth", g_Settings.iWidth);
	db_set_dw(0, CHAT_MODULE, "roomheight", g_Settings.iHeight);

	DestroyMenu(g_hMenu);
}
