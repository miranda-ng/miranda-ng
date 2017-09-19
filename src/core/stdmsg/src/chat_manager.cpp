/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-17 Miranda NG project,
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
	int i = pci->arSessions.indexOf(si);
	if (i == -1)
		return nullptr;

	for (i--; i >= 0; i--) {
		SESSION_INFO *p = pci->arSessions[i];
		if (p->pDlg)
			return p;
	}

	return nullptr;
}

SESSION_INFO* SM_GetNextWindow(SESSION_INFO *si)
{
	int i = pci->arSessions.indexOf(si);
	if (i == -1)
		return nullptr;

	for (i++; i < pci->arSessions.getCount(); i++) {
		SESSION_INFO *p = pci->arSessions[i];
		if (p->pDlg)
			return p;
	}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////

CHAT_MANAGER *pci;

HMENU g_hMenu = nullptr;

BOOL SmileyAddInstalled = FALSE, PopupInstalled = FALSE;
HIMAGELIST hIconsList;

GlobalLogSettings g_Settings;

static void OnDestroyModule(MODULEINFO *mi)
{
	if (mi->hOnlineIcon) DestroyIcon(mi->hOnlineIcon);
	if (mi->hOnlineTalkIcon) DestroyIcon(mi->hOnlineTalkIcon);
	if (mi->hOfflineIcon) DestroyIcon(mi->hOfflineIcon);
	if (mi->hOfflineTalkIcon) DestroyIcon(mi->hOfflineTalkIcon);
}

static void OnCreateModule(MODULEINFO *mi)
{
	OnDestroyModule(mi);

	mi->OnlineIconIndex = ImageList_AddIcon(hIconsList, Skin_LoadProtoIcon(mi->pszModule, ID_STATUS_ONLINE));
	mi->hOnlineIcon = ImageList_GetIcon(hIconsList, mi->OnlineIconIndex, ILD_TRANSPARENT);
	mi->hOnlineTalkIcon = ImageList_GetIcon(hIconsList, mi->OnlineIconIndex, ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));
	ImageList_AddIcon(hIconsList, mi->hOnlineTalkIcon);

	mi->OfflineIconIndex = ImageList_AddIcon(hIconsList, Skin_LoadProtoIcon(mi->pszModule, ID_STATUS_OFFLINE));
	mi->hOfflineIcon = ImageList_GetIcon(hIconsList, mi->OfflineIconIndex, ILD_TRANSPARENT);
	mi->hOfflineTalkIcon = ImageList_GetIcon(hIconsList, mi->OfflineIconIndex, ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));
	ImageList_AddIcon(hIconsList, mi->hOfflineTalkIcon);

	g_iMessageIconIndex = ImageList_AddIcon(hIconsList, Skin_LoadIcon(SKINICON_EVENT_MESSAGE));
}

static void OnReplaceSession(SESSION_INFO *si)
{
	if (si->pDlg)
		RedrawWindow(GetDlgItem(si->pDlg->GetHwnd(), IDC_SRMM_NICKLIST), nullptr, nullptr, RDW_INVALIDATE);
}

static void OnFlashHighlight(SESSION_INFO *si, int bInactive)
{
	if (!bInactive)
		return;

	if (!g_Settings.bTabsEnable && si->pDlg && g_Settings.bFlashWindowHighlight)
		SetTimer(si->pDlg->GetHwnd(), TIMERID_FLASHWND, 900, nullptr);
	if (g_Settings.bTabsEnable && si->pDlg)
		pDialog->SetMessageHighlight(si->pDlg);
}

static void OnFlashWindow(SESSION_INFO *si, int bInactive)
{
	if (!bInactive)
		return;

	if (!g_Settings.bTabsEnable && si->pDlg && g_Settings.bFlashWindow)
		SetTimer(si->pDlg->GetHwnd(), TIMERID_FLASHWND, 900, nullptr);
	if (g_Settings.bTabsEnable && si->pDlg)
		pDialog->SetTabHighlight(si->pDlg);
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
	if (g_Settings.MessageAreaFont)
		DeleteObject(g_Settings.MessageAreaFont);

	LOGFONT lf;
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, &lf, &g_Settings.MessageAreaColor);
	g_Settings.MessageAreaFont = CreateFontIndirect(&lf);

	g_Settings.iX = db_get_dw(0, CHAT_MODULE, "roomx", -1);
	g_Settings.iY = db_get_dw(0, CHAT_MODULE, "roomy", -1);

	g_Settings.bTabsEnable = db_get_b(0, CHAT_MODULE, "Tabs", 1) != 0;
	g_Settings.bTabRestore = db_get_b(0, CHAT_MODULE, "TabRestore", 0) != 0;
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
	ColourIDW colourid = { sizeof(colourid) };
	strncpy(colourid.dbSettingsGroup, CHAT_MODULE, sizeof(colourid.dbSettingsGroup));
	wcsncpy(colourid.group, LPGENW("Chat module"), _countof(colourid.group));

	strncpy(colourid.setting, "ColorLogBG", _countof(colourid.setting));
	wcsncpy(colourid.name, LPGENW("Group chat log background"), _countof(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	Colour_RegisterW(&colourid);

	strncpy(colourid.setting, "ColorMessageBG", _countof(colourid.setting));
	wcsncpy(colourid.name, LPGENW("Message background"), _countof(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	Colour_RegisterW(&colourid);

	strncpy(colourid.setting, "ColorNicklistBG", _countof(colourid.setting));
	wcsncpy(colourid.name, LPGENW("Nick list background"), _countof(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_WINDOW);
	Colour_RegisterW(&colourid);

	strncpy(colourid.setting, "ColorNicklistLines", _countof(colourid.setting));
	wcsncpy(colourid.name, LPGENW("Nick list lines"), _countof(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_INACTIVEBORDER);
	Colour_RegisterW(&colourid);

	strncpy(colourid.setting, "ColorNicklistSelectedBG", _countof(colourid.setting));
	wcsncpy(colourid.name, LPGENW("Nick list background (selected)"), _countof(colourid.name));
	colourid.defcolour = GetSysColor(COLOR_HIGHLIGHT);
	Colour_RegisterW(&colourid);
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
			CMsgDialog *pDlg = pContainer->m_pEmbed = new CChatRoomDlg(pContainer, si);
			pContainer->Create();
			pDlg->SetParent(pContainer->GetHwnd());
			pDlg->Create();
			pContainer->Show();
			PostMessage(pContainer->GetHwnd(), WM_SIZE, 0, 0);
		}

		if (si->iType != GCW_SERVER)
			si->pDlg->UpdateNickList();
		else
			si->pDlg->UpdateTitle();
		si->pDlg->RedrawLog();
		si->pDlg->UpdateStatusBar();
	}

	SetWindowLongPtr(si->pDlg->GetHwnd(), GWL_EXSTYLE, GetWindowLongPtr(si->pDlg->GetHwnd(), GWL_EXSTYLE) | WS_EX_APPWINDOW);

	if (IsIconic(si->pDlg->GetHwnd()))
		si->pDlg->Show(SW_NORMAL);
	si->pDlg->Show(SW_SHOW);
	SetForegroundWindow(si->pDlg->GetHwnd());
}

int OnCheckPlugins(WPARAM, LPARAM)
{
	SmileyAddInstalled = ServiceExists(MS_SMILEYADD_REPLACESMILEYS);
	PopupInstalled = ServiceExists(MS_POPUP_ADDPOPUPT);
	return 0;
}

void Load_ChatModule()
{
	AddIcons();
	RegisterFonts();

	CHAT_MANAGER_INITDATA data = { &g_Settings, sizeof(MODULEINFO), sizeof(SESSION_INFO), LPGENW("Chat module"), FONTMODE_SKIP };
	pci = Chat_GetInterface(&data);

	pci->OnCreateModule = OnCreateModule;
	pci->OnDestroyModule = OnDestroyModule;
	pci->OnReplaceSession = OnReplaceSession;

	pci->OnLoadSettings = OnLoadSettings;
	pci->OnFlashWindow = OnFlashWindow;
	pci->OnFlashHighlight = OnFlashHighlight;
	pci->ShowRoom = ShowRoom;

	oldDoPopup = pci->DoPopup; pci->DoPopup = DoPopup;
	oldDoTrayIcon = pci->DoTrayIcon; pci->DoTrayIcon = DoTrayIcon;
	pci->ReloadSettings();

	g_hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU));

	hIconsList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 0, 100);
	ImageList_AddIcon(hIconsList, Skin_LoadIcon(SKINICON_EVENT_MESSAGE));
	ImageList_AddIcon(hIconsList, LoadIconEx("overlay", FALSE));
	ImageList_SetOverlayImage(hIconsList, 1, 1);

	HookEvent(ME_SYSTEM_MODULELOAD, OnCheckPlugins);
}

void Unload_ChatModule()
{
	db_set_w(0, CHAT_MODULE, "SplitterX", (WORD)g_Settings.iSplitterX);
	db_set_w(0, CHAT_MODULE, "SplitterY", (WORD)g_Settings.iSplitterY);
	db_set_dw(0, CHAT_MODULE, "roomx", g_Settings.iX);
	db_set_dw(0, CHAT_MODULE, "roomy", g_Settings.iY);
	db_set_dw(0, CHAT_MODULE, "roomwidth", g_Settings.iWidth);
	db_set_dw(0, CHAT_MODULE, "roomheight", g_Settings.iHeight);

	if (g_Settings.MessageAreaFont)
		DeleteObject(g_Settings.MessageAreaFont);
	DestroyMenu(g_hMenu);
}
