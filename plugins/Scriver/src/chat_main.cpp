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
CHAT_MANAGER *pci;

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

	if (g_Settings.bFlashWindowHighlight)
		SendMessage(GetParent(si->pDlg->GetHwnd()), CM_STARTFLASHING, 0, 0);

	si->wState |= GC_EVENT_HIGHLIGHT;
	si->pDlg->FixTabIcons();
	si->pDlg->UpdateTitle();

	HWND hwndParent = GetParent(si->pDlg->GetHwnd());
	if (g_Settings.bFlashWindowHighlight && GetActiveWindow() != si->pDlg->GetHwnd() && GetForegroundWindow() != hwndParent)
		SendMessage(hwndParent, CM_STARTFLASHING, 0, 0);
}

static void OnFlashWindow(SESSION_INFO *si, int bInactive)
{
	if (!bInactive || !si->pDlg)
		return;

	if (g_Settings.bFlashWindow)
		SendMessage(GetParent(si->pDlg->GetHwnd()), CM_STARTFLASHING, 0, 0);

	si->pDlg->FixTabIcons();
	si->pDlg->UpdateTitle();

	HWND hwndParent = GetParent(si->pDlg->GetHwnd());
	if (g_Settings.bFlashWindow && GetActiveWindow() != hwndParent && GetForegroundWindow() != hwndParent)
		SendMessage(hwndParent, CM_STARTFLASHING, 0, 0);
}

static void OnDestroyModule(MODULEINFO *mi)
{
	if (mi->hOnlineIcon) DestroyIcon(mi->hOnlineIcon);
	if (mi->hOnlineTalkIcon) DestroyIcon(mi->hOnlineTalkIcon);
	if (mi->hOfflineIcon) DestroyIcon(mi->hOfflineIcon);
	if (mi->hOfflineTalkIcon) DestroyIcon(mi->hOfflineTalkIcon);
}

static void OnCreateModule(MODULEINFO *mi)
{
	HIMAGELIST hList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);

	int overlayIcon = ImageList_AddIcon(hList, GetCachedIcon("chat_overlay"));
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
	if (g_Settings.MessageBoxFont)
		DeleteObject(g_Settings.MessageBoxFont);

	LOGFONT lf;
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, &lf, nullptr);
	g_Settings.MessageBoxFont = CreateFontIndirect(&lf);
}

int Chat_Load()
{
	CHAT_MANAGER_INITDATA data = { &g_Settings, sizeof(MODULEINFO), sizeof(SESSION_INFO), LPGENW("Messaging") L"/" LPGENW("Group chats"), FONTMODE_SKIP };
	pci = Chat_GetInterface(&data);

	pci->OnCreateModule = OnCreateModule;
	pci->OnDestroyModule = OnDestroyModule;
	pci->OnLoadSettings = OnLoadSettings;

	pci->OnSetStatus = OnSetStatus;

	pci->OnReplaceSession = OnReplaceSession;

	pci->OnFlashWindow = OnFlashWindow;
	pci->OnFlashHighlight = OnFlashHighlight;
	pci->ShowRoom = ShowRoom;

	oldDoPopup = pci->DoPopup; pci->DoPopup = DoPopup;
	oldDoTrayIcon = pci->DoTrayIcon; pci->DoTrayIcon = DoTrayIcon;
	pci->ReloadSettings();

	return 0;
}
