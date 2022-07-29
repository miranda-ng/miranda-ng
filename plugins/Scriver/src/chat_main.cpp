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

// globals
pfnDoTrayIcon oldDoTrayIcon;
pfnDoPopup    oldDoPopup;

GlobalLogSettings g_Settings;

static void OnReplaceSession(SESSION_INFO *si)
{
	if (si->pDlg)
		RedrawWindow(GetDlgItem(si->pDlg->GetHwnd(), IDC_SRMM_NICKLIST), nullptr, nullptr, RDW_INVALIDATE);
}

static void OnSetStatus(SESSION_INFO *si, int)
{
	if (si->pDlg)
		si->pDlg->FixTabIcons();
}

static void OnFlashHighlight(SESSION_INFO *si, int bInactive)
{
	if (!bInactive || !si->pDlg)
		return;

	si->wState |= GC_EVENT_HIGHLIGHT;
	si->pDlg->FixTabIcons();
	si->pDlg->UpdateTitle();

	if (g_Settings.bFlashWindowHighlight)
		si->pDlg->StartFlashing();
}

static void OnFlashWindow(SESSION_INFO *si, int bInactive)
{
	if (!bInactive || !si->pDlg)
		return;

	si->pDlg->FixTabIcons();
	si->pDlg->UpdateTitle();

	if (g_Settings.bFlashWindow)
		si->pDlg->StartFlashing();
}

static void OnDestroyModule(MODULEINFO *mi)
{
	if (mi->hOnlineIcon) DestroyIcon(mi->hOnlineIcon);
	if (mi->hOnlineTalkIcon) DestroyIcon(mi->hOnlineTalkIcon);
	if (mi->hOfflineIcon) DestroyIcon(mi->hOfflineIcon);
	if (mi->hOfflineTalkIcon) DestroyIcon(mi->hOfflineTalkIcon);
}

static MODULEINFO* MM_CreateModule()
{
	return new MODULEINFO();
}

static void OnCreateModule(MODULEINFO *mi)
{
	HIMAGELIST hList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
	int overlayIcon = g_plugin.addImgListIcon(hList, IDI_OVERLAY);
	ImageList_SetOverlayImage(hList, overlayIcon, 1);

	int index = ImageList_AddIcon(hList, Skin_LoadProtoIcon(mi->pszModule, ID_STATUS_ONLINE));
	mi->hOnlineIcon = ImageList_GetIcon(hList, index, ILD_TRANSPARENT);
	mi->hOnlineTalkIcon = ImageList_GetIcon(hList, index, ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));

	index = ImageList_AddIcon(hList, Skin_LoadProtoIcon(mi->pszModule, ID_STATUS_OFFLINE));
	mi->hOfflineIcon = ImageList_GetIcon(hList, index, ILD_TRANSPARENT);
	mi->hOfflineTalkIcon = ImageList_GetIcon(hList, index, ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));

	ImageList_Destroy(hList);

	mi->hOnlineIconBig = Skin_LoadProtoIcon(mi->pszModule, ID_STATUS_ONLINE, true);
	mi->hOfflineIconBig = Skin_LoadProtoIcon(mi->pszModule, ID_STATUS_OFFLINE, true);
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
	g_Settings.bAddColonToAutoComplete = db_get_b(0, CHAT_MODULE, "AddColonToAutoComplete", 1) != 0;

	if (g_Settings.MessageBoxFont)
		DeleteObject(g_Settings.MessageBoxFont);

	LOGFONT lf;
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, &lf, nullptr);
	g_Settings.MessageBoxFont = CreateFontIndirect(&lf);
}

static void ShowRoom(SESSION_INFO *si)
{
	if (si == nullptr)
		return;

	// Do we need to create a window?
	CMsgDialog *pDlg;
	if (si->pDlg == nullptr) {
		pDlg = new CMsgDialog(si);
		pDlg->Show();

		si->pDlg = pDlg;
	}
	else pDlg = si->pDlg;

	pDlg->UpdateTabControl();
	SendMessage(pDlg->GetHwnd(), WM_MOUSEACTIVATE, 0, 0);
	SetFocus(GetDlgItem(pDlg->GetHwnd(), IDC_SRMM_MESSAGE));
}

int Chat_Load()
{
	CHAT_MANAGER_INITDATA data = { &g_Settings, sizeof(MODULEINFO), sizeof(SESSION_INFO), LPGENW("Messaging") L"/" LPGENW("Group chats"), FONTMODE_SKIP, &g_plugin };
	Chat_CustomizeApi(&data);

	g_chatApi.MM_CreateModule = MM_CreateModule;
	g_chatApi.OnCreateModule = OnCreateModule;
	g_chatApi.OnDestroyModule = OnDestroyModule;
	g_chatApi.OnLoadSettings = OnLoadSettings;

	g_chatApi.OnSetStatus = OnSetStatus;

	g_chatApi.OnReplaceSession = OnReplaceSession;

	g_chatApi.OnFlashWindow = OnFlashWindow;
	g_chatApi.OnFlashHighlight = OnFlashHighlight;
	g_chatApi.ShowRoom = ShowRoom;

	Srmm_CreateHotkey(LPGEN("Messaging"), LPGEN("Action: Send message"));

	oldDoPopup = g_chatApi.DoPopup; g_chatApi.DoPopup = DoPopup;
	oldDoTrayIcon = g_chatApi.DoTrayIcon; g_chatApi.DoTrayIcon = DoTrayIcon;
	g_chatApi.ReloadSettings();

	return 0;
}
