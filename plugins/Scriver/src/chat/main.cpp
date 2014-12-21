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

#include "../commonheaders.h"

// globals
CHAT_MANAGER *pci, saveCI;
HMENU g_hMenu = NULL;

GlobalLogSettings g_Settings;

void LoadModuleIcons(MODULEINFO *mi)
{
	HIMAGELIST hList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);

	int overlayIcon = ImageList_AddIcon(hList, GetCachedIcon("chat_overlay"));
	ImageList_SetOverlayImage(hList, overlayIcon, 1);

	int index = ImageList_AddIcon(hList, LoadSkinnedProtoIcon(mi->pszModule, ID_STATUS_ONLINE));
	mi->hOnlineIcon = ImageList_GetIcon(hList, index, ILD_TRANSPARENT);
	mi->hOnlineTalkIcon = ImageList_GetIcon(hList, index, ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));

	index = ImageList_AddIcon(hList, LoadSkinnedProtoIcon(mi->pszModule, ID_STATUS_OFFLINE));
	mi->hOfflineIcon = ImageList_GetIcon(hList, index, ILD_TRANSPARENT);
	mi->hOfflineTalkIcon = ImageList_GetIcon(hList, index, ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));

	ImageList_Destroy(hList);
}

static void OnAddLog(SESSION_INFO *si, int isOk)
{
	if (isOk && si->hWnd)
		SendMessage(si->hWnd, GC_ADDLOG, 0, 0);
	else if (si->hWnd)
		SendMessage(si->hWnd, GC_REDRAWLOG2, 0, 0);
}

static void OnCreateSession(SESSION_INFO *si, MODULEINFO*)
{
	si->flags = CWDF_RTF_INPUT;
}

static void OnDblClickSession(SESSION_INFO *si)
{
	PostMessage(si->hWnd, GC_CLOSEWINDOW, 0, 0);
}

static void OnRemoveSession(SESSION_INFO *si)
{
	if (si->hWnd)
		SendMessage(si->hWnd, GC_EVENT_CONTROL + WM_USER + 500, SESSION_TERMINATE, 0);
}

static void OnRenameSession(SESSION_INFO *si)
{
	if (si->hWnd)
		SendMessage(si->hWnd, DM_UPDATETITLEBAR, 0, 0);
}

static void OnReplaceSession(SESSION_INFO *si)
{
	if (si->hWnd)
		RedrawWindow(GetDlgItem(si->hWnd, IDC_CHAT_LIST), NULL, NULL, RDW_INVALIDATE);
}

static void OnEventBroadcast(SESSION_INFO *si, GCEVENT *gce)
{
	if (pci->SM_AddEvent(si->ptszID, si->pszModule, gce, FALSE) && si->hWnd && si->bInitDone)
		SendMessage(si->hWnd, GC_ADDLOG, 0, 0);
	else if (si->hWnd && si->bInitDone)
		SendMessage(si->hWnd, GC_REDRAWLOG2, 0, 0);
}

static void OnSetStatusBar(SESSION_INFO *si)
{
	if (si->hWnd)
		SendMessage(si->hWnd, DM_UPDATETITLEBAR, 0, 0);
}

static void OnNewUser(SESSION_INFO *si, USERINFO*)
{
	if (si->hWnd)
		SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);
}

static void OnSetStatus(SESSION_INFO *si, int)
{
	PostMessage(si->hWnd, GC_FIXTABICONS, 0, 0);
}

static void OnFlashHighlight(SESSION_INFO *si, int bInactive)
{
	if (!bInactive || !si->hWnd)
		return;

	if (g_Settings.bFlashWindowHighlight)
		SendMessage(GetParent(si->hWnd), CM_STARTFLASHING, 0, 0);
	SendMessage(si->hWnd, GC_SETMESSAGEHIGHLIGHT, 0, 0);
}

static void OnFlashWindow(SESSION_INFO *si, int bInactive)
{
	if (!bInactive || !si->hWnd)
		return;

	if (g_Settings.bFlashWindow)
		SendMessage(GetParent(si->hWnd), CM_STARTFLASHING, 0, 0);
	SendMessage(si->hWnd, GC_SETTABHIGHLIGHT, 0, 0);
}

static void OnCreateModule(MODULEINFO *mi)
{
	LoadModuleIcons(mi);
	mi->hOnlineIconBig = LoadSkinnedProtoIconBig(mi->pszModule, ID_STATUS_ONLINE);
	mi->hOfflineIconBig = LoadSkinnedProtoIconBig(mi->pszModule, ID_STATUS_OFFLINE);
}

static BOOL DoTrayIcon(SESSION_INFO *si, GCEVENT *gce)
{
	if (gce->pDest->iType & g_Settings.dwTrayIconFlags)
		return saveCI.DoTrayIcon(si, gce);
	return TRUE;
}

static BOOL DoPopup(SESSION_INFO *si, GCEVENT *gce)
{
	if (gce->pDest->iType & g_Settings.dwPopupFlags)
		return saveCI.DoPopup(si, gce);
	return TRUE;
}

static void OnLoadSettings()
{
	if (g_Settings.MessageBoxFont)
		DeleteObject(g_Settings.MessageBoxFont);

	LOGFONT lf;
	LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, &lf, NULL);
	g_Settings.MessageBoxFont = CreateFontIndirect(&lf);
}

int Chat_Load()
{
	CHAT_MANAGER_INITDATA data = { &g_Settings, sizeof(MODULEINFO), sizeof(SESSION_INFO), LPGENT("Messaging")_T("/")LPGENT("Group chats"), FONTMODE_SKIP };
	mir_getCI(&data);
	saveCI = *pci;

	pci->OnCreateModule = OnCreateModule;
	pci->OnNewUser = OnNewUser;
	pci->OnLoadSettings = OnLoadSettings;

	pci->OnSetStatus = OnSetStatus;

	pci->OnAddLog = OnAddLog;

	pci->OnCreateSession = OnCreateSession;
	pci->OnRemoveSession = OnRemoveSession;
	pci->OnRenameSession = OnRenameSession;
	pci->OnReplaceSession = OnReplaceSession;
	pci->OnDblClickSession = OnDblClickSession;

	pci->OnEventBroadcast = OnEventBroadcast;
	pci->OnSetStatusBar = OnSetStatusBar;
	pci->OnFlashWindow = OnFlashWindow;
	pci->OnFlashHighlight = OnFlashHighlight;
	pci->ShowRoom = ShowRoom;

	pci->DoPopup = DoPopup;
	pci->DoTrayIcon = DoTrayIcon;
	pci->ReloadSettings();

	g_hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU));
	TranslateMenu(g_hMenu);
	return 0;
}

int Chat_Unload(void)
{
	db_set_w(NULL, CHAT_MODULE, "SplitterX", (WORD)g_Settings.iSplitterX);

	DestroyMenu(g_hMenu);
	return 0;
}

int Chat_ModulesLoaded(WPARAM, LPARAM)
{
	OptionsInit();
	return 0;
}
