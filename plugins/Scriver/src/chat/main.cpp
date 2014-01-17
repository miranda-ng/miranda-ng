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
CHAT_MANAGER *pci;
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

static void OnSessionDblClick(SESSION_INFO *si)
{
	PostMessage(si->hWnd, GC_CLOSEWINDOW, 0, 0);
}

static void OnSessionRemove(SESSION_INFO *si)
{
	if (si->hWnd)
		SendMessage(si->hWnd, GC_EVENT_CONTROL + WM_USER + 500, SESSION_TERMINATE, 0);
}

static void OnSessionRename(SESSION_INFO *si)
{
	if (si->hWnd)
		SendMessage(si->hWnd, DM_UPDATETITLEBAR, 0, 0);
}

static void OnSessionReplace(SESSION_INFO *si)
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

static void OnSetStatus(SESSION_INFO *si, int wStatus)
{
	PostMessage(si->hWnd, GC_FIXTABICONS, 0, 0);
}

static void OnFlashWindow(SESSION_INFO *si, int bInactive)
{
	if (bInactive && si->hWnd && db_get_b(NULL, "Chat", "FlashWindowHighlight", 0) != 0)
		SendMessage(GetParent(si->hWnd), CM_STARTFLASHING, 0, 0);
	if (bInactive && si->hWnd)
		SendMessage(si->hWnd, GC_SETMESSAGEHIGHLIGHT, 0, 0);
}

static void OnCreateModule(MODULEINFO *mi)
{
	LoadModuleIcons(mi);
	mi->hOnlineIconBig = LoadSkinnedProtoIconBig(mi->pszModule, ID_STATUS_ONLINE);
	mi->hOfflineIconBig = LoadSkinnedProtoIconBig(mi->pszModule, ID_STATUS_OFFLINE);
}

void LoadChatIcons(void)
{
	pci->hIcons[ICON_ACTION]     = GetCachedIcon("chat_log_action");
	pci->hIcons[ICON_ADDSTATUS]  = GetCachedIcon("chat_log_addstatus");
	pci->hIcons[ICON_HIGHLIGHT]  = GetCachedIcon("chat_log_highlight");
	pci->hIcons[ICON_INFO]       = GetCachedIcon("chat_log_info");
	pci->hIcons[ICON_JOIN]       = GetCachedIcon("chat_log_join");
	pci->hIcons[ICON_KICK]       = GetCachedIcon("chat_log_kick");
	pci->hIcons[ICON_MESSAGE]    = GetCachedIcon("chat_log_message_in");
	pci->hIcons[ICON_MESSAGEOUT] = GetCachedIcon("chat_log_message_out");
	pci->hIcons[ICON_NICK]       = GetCachedIcon("chat_log_nick");
	pci->hIcons[ICON_NOTICE]     = GetCachedIcon("chat_log_notice");
	pci->hIcons[ICON_PART]       = GetCachedIcon("chat_log_part");
	pci->hIcons[ICON_QUIT]       = GetCachedIcon("chat_log_quit");
	pci->hIcons[ICON_REMSTATUS]  = GetCachedIcon("chat_log_removestatus");
	pci->hIcons[ICON_TOPIC]      = GetCachedIcon("chat_log_topic");
	pci->hIcons[ICON_STATUS1]    = GetCachedIcon("chat_status1");
	pci->hIcons[ICON_STATUS2]    = GetCachedIcon("chat_status2");
	pci->hIcons[ICON_STATUS3]    = GetCachedIcon("chat_status3");
	pci->hIcons[ICON_STATUS4]    = GetCachedIcon("chat_status4");
	pci->hIcons[ICON_STATUS0]    = GetCachedIcon("chat_status0");
	pci->hIcons[ICON_STATUS5]    = GetCachedIcon("chat_status5");
}

int Chat_Load()
{
	CHAT_MANAGER_INITDATA data = { &g_Settings, sizeof(MODULEINFO), sizeof(SESSION_INFO), LPGENT("Messaging")_T("/")LPGENT("Group chats") };
	mir_getCI(&data);
	pci->OnCreateModule = OnCreateModule;
	pci->OnNewUser = OnNewUser;

	pci->OnSetStatus = OnSetStatus;

	pci->OnAddLog = OnAddLog;

	pci->OnSessionRemove = OnSessionRemove;
	pci->OnSessionRename = OnSessionRename;
	pci->OnSessionReplace = OnSessionReplace;
	pci->OnSessionDblClick = OnSessionDblClick;

	pci->OnEventBroadcast = OnEventBroadcast;
	pci->OnSetStatusBar = OnSetStatusBar;
	pci->OnFlashWindow = OnFlashWindow;
	pci->ShowRoom = ShowRoom;

	LoadChatIcons();

	g_hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU));
	TranslateMenu(g_hMenu);
	return 0;
}

int Chat_Unload(void)
{
	db_set_w(NULL, "Chat", "SplitterX", (WORD)g_Settings.iSplitterX);

	DestroyMenu(g_hMenu);
	return 0;
}

int Chat_ModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	OptionsInit();
 	return 0;
}
