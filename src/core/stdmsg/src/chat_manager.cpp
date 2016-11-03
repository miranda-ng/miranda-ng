/*
Chat module plugin for Miranda IM

Copyright 2000-12 Miranda IM, 2012-16 Miranda NG project,
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

TABLIST *g_TabList = 0;

BOOL SM_SetTabbedWindowHwnd(SESSION_INFO *si, HWND hwnd)
{
	for (SESSION_INFO *p = pci->wndList; p != NULL; p = p->next) {
		if (si && si == p)
			p->hWnd = hwnd;
		else
			p->hWnd = NULL;
	}
	return TRUE;
}

SESSION_INFO* SM_GetPrevWindow(SESSION_INFO *si)
{
	if (!si)
		return NULL;

	BOOL bFound = FALSE;
	SESSION_INFO *pTemp = pci->wndList;
	while (pTemp != NULL) {
		if (si == pTemp) {
			if (bFound)
				return NULL;
			else
				bFound = TRUE;
		}
		else if (bFound == TRUE && pTemp->hWnd)
			return pTemp;
		pTemp = pTemp->next;
		if (pTemp == NULL && bFound)
			pTemp = pci->wndList;
	}
	return NULL;
}

SESSION_INFO* SM_GetNextWindow(SESSION_INFO *si)
{
	if (!si)
		return NULL;

	SESSION_INFO *pTemp = pci->wndList, *pLast = NULL;
	while (pTemp != NULL) {
		if (si == pTemp) {
			if (pLast) {
				if (pLast != pTemp)
					return pLast;
				else
					return NULL;
			}
		}
		if (pTemp->hWnd)
			pLast = pTemp;
		pTemp = pTemp->next;
		if (pTemp == NULL)
			pTemp = pci->wndList;
	}
	return NULL;
}

//---------------------------------------------------
//		Tab list manager functions
//
//		Necessary to keep track of what tabs should
//		be restored
//---------------------------------------------------

BOOL TabM_AddTab(const wchar_t *pszID, const char* pszModule)
{
	TABLIST *node = NULL;
	if (!pszID || !pszModule)
		return FALSE;

	node = (TABLIST*)mir_alloc(sizeof(TABLIST));
	memset(node, 0, sizeof(TABLIST));
	node->pszID = mir_wstrdup(pszID);
	node->pszModule = mir_strdup(pszModule);

	if (g_TabList == NULL) { // list is empty
		g_TabList = node;
		node->next = NULL;
	}
	else {
		node->next = g_TabList;
		g_TabList = node;
	}
	return TRUE;
}

BOOL TabM_RemoveAll(void)
{
	while (g_TabList != NULL) {
		TABLIST *pLast = g_TabList->next;
		mir_free(g_TabList->pszModule);
		mir_free(g_TabList->pszID);
		mir_free(g_TabList);
		g_TabList = pLast;
	}
	g_TabList = NULL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

CHAT_MANAGER *pci, saveCI;

SESSION_INFO g_TabSession;
HMENU g_hMenu = NULL;

BOOL SmileyAddInstalled = FALSE, PopupInstalled = FALSE;
HIMAGELIST hIconsList;

GlobalLogSettings g_Settings;

static void OnCreateModule(MODULEINFO *mi)
{
	mi->OnlineIconIndex = ImageList_AddIcon(hIconsList, Skin_LoadProtoIcon(mi->pszModule, ID_STATUS_ONLINE));
	mi->hOnlineIcon = ImageList_GetIcon(hIconsList, mi->OnlineIconIndex, ILD_TRANSPARENT);
	mi->hOnlineTalkIcon = ImageList_GetIcon(hIconsList, mi->OnlineIconIndex, ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));
	ImageList_AddIcon(hIconsList, mi->hOnlineTalkIcon);

	mi->OfflineIconIndex = ImageList_AddIcon(hIconsList, Skin_LoadProtoIcon(mi->pszModule, ID_STATUS_OFFLINE));
	mi->hOfflineIcon = ImageList_GetIcon(hIconsList, mi->OfflineIconIndex, ILD_TRANSPARENT);
	mi->hOfflineTalkIcon = ImageList_GetIcon(hIconsList, mi->OfflineIconIndex, ILD_TRANSPARENT | INDEXTOOVERLAYMASK(1));
	ImageList_AddIcon(hIconsList, mi->hOfflineTalkIcon);
}

static void OnAddLog(SESSION_INFO *si, int isOk)
{
	if (isOk && si->hWnd) {
		g_TabSession.pLog = si->pLog;
		g_TabSession.pLogEnd = si->pLogEnd;
		SendMessage(si->hWnd, GC_ADDLOG, 0, 0);
	}
	else if (si->hWnd) {
		g_TabSession.pLog = si->pLog;
		g_TabSession.pLogEnd = si->pLogEnd;
		SendMessage(si->hWnd, GC_REDRAWLOG2, 0, 0);
	}
}

static void OnClearLog(SESSION_INFO *si)
{
	if (si->hWnd) {
		g_TabSession.pLog = si->pLog;
		g_TabSession.pLogEnd = si->pLogEnd;
	}
}

static void OnDblClickSession(SESSION_INFO *si)
{
	if (g_Settings.bTabsEnable)
		SendMessage(si->hWnd, GC_REMOVETAB, 1, (LPARAM)si);
	else
		PostMessage(si->hWnd, GC_CLOSEWINDOW, 0, 0);
}

static void OnRemoveSession(SESSION_INFO *si)
{
	if (!g_Settings.bTabsEnable) {
		if (si->hWnd)
			SendMessage(si->hWnd, GC_CONTROL_MSG, SESSION_TERMINATE, 0);
	}
	else if (g_TabSession.hWnd)
		SendMessage(g_TabSession.hWnd, GC_REMOVETAB, 1, (LPARAM)si);

	if (si->hWnd)
		g_TabSession.nUsersInNicklist = 0;
}

static void OnRenameSession(SESSION_INFO *si)
{
	if (g_TabSession.hWnd && g_Settings.bTabsEnable) {
		g_TabSession.ptszName = si->ptszName;
		SendMessage(g_TabSession.hWnd, GC_SESSIONNAMECHANGE, 0, (LPARAM)si);
	}
}

static void OnReplaceSession(SESSION_INFO *si)
{
	if (si->hWnd)
		g_TabSession.nUsersInNicklist = 0;

	if (!g_Settings.bTabsEnable) {
		if (si->hWnd)
			RedrawWindow(GetDlgItem(si->hWnd, IDC_LIST), NULL, NULL, RDW_INVALIDATE);
	}
	else if (g_TabSession.hWnd)
		RedrawWindow(GetDlgItem(g_TabSession.hWnd, IDC_LIST), NULL, NULL, RDW_INVALIDATE);
}

static void OnOfflineSession(SESSION_INFO *si)
{
	if (si->hWnd) {
		g_TabSession.nUsersInNicklist = 0;
		if (g_Settings.bTabsEnable)
			g_TabSession.pUsers = 0;
	}
}

static void OnEventBroadcast(SESSION_INFO *si, GCEVENT *gce)
{
	if (pci->SM_AddEvent(si->ptszID, si->pszModule, gce, FALSE) && si->hWnd && si->bInitDone) {
		g_TabSession.pLog = si->pLog;
		g_TabSession.pLogEnd = si->pLogEnd;
		SendMessage(si->hWnd, GC_ADDLOG, 0, 0);
	}
	else if (si->hWnd && si->bInitDone) {
		g_TabSession.pLog = si->pLog;
		g_TabSession.pLogEnd = si->pLogEnd;
		SendMessage(si->hWnd, GC_REDRAWLOG2, 0, 0);
	}
}

static void OnSetStatusBar(SESSION_INFO *si)
{
	if (si->hWnd) {
		g_TabSession.ptszStatusbarText = si->ptszStatusbarText;
		SendMessage(si->hWnd, GC_UPDATESTATUSBAR, 0, 0);
	}
}

static void OnAddUser(SESSION_INFO *si, USERINFO*)
{
	if (si->hWnd)
		g_TabSession.nUsersInNicklist++;
}

static void OnNewUser(SESSION_INFO *si, USERINFO*)
{
	if (si->hWnd) {
		g_TabSession.pUsers = si->pUsers;
		SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);
	}
}

static void OnRemoveUser(SESSION_INFO *si, USERINFO*)
{
	if (si->hWnd) {
		g_TabSession.pUsers = si->pUsers;
		g_TabSession.nUsersInNicklist--;
	}
}

static void OnAddStatus(SESSION_INFO *si, STATUSINFO*)
{
	if (g_Settings.bTabsEnable && si->hWnd)
		g_TabSession.pStatuses = si->pStatuses;
}

static void OnSetStatus(SESSION_INFO *si, int wStatus)
{
	if (g_Settings.bTabsEnable) {
		if (si->hWnd)
			g_TabSession.wStatus = wStatus;
		if (g_TabSession.hWnd)
			PostMessage(g_TabSession.hWnd, GC_FIXTABICONS, 0, (LPARAM)si);
	}
}

static void OnSetTopic(SESSION_INFO *si)
{
	if (si->hWnd)
		g_TabSession.ptszTopic = si->ptszTopic;
}

static void OnFlashHighlight(SESSION_INFO *si, int bInactive)
{
	if (!bInactive)
		return;

	if (!g_Settings.bTabsEnable && si->hWnd && g_Settings.bFlashWindowHighlight)
		SetTimer(si->hWnd, TIMERID_FLASHWND, 900, NULL);
	if (g_Settings.bTabsEnable && g_TabSession.hWnd)
		SendMessage(g_TabSession.hWnd, GC_SETMESSAGEHIGHLIGHT, 0, (LPARAM)si);
}

static void OnFlashWindow(SESSION_INFO *si, int bInactive)
{
	if (!bInactive)
		return;

	if (!g_Settings.bTabsEnable && si->hWnd && g_Settings.bFlashWindow)
		SetTimer(si->hWnd, TIMERID_FLASHWND, 900, NULL);
	if (g_Settings.bTabsEnable && g_TabSession.hWnd)
		SendMessage(g_TabSession.hWnd, GC_SETTABHIGHLIGHT, 0, (LPARAM)si);
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
	if (g_Settings.MessageAreaFont)
		DeleteObject(g_Settings.MessageAreaFont);

	LOGFONT lf;
	LoadMessageFont(&lf, &g_Settings.MessageAreaColor);
	g_Settings.MessageAreaFont = CreateFontIndirect(&lf);

	g_Settings.iX = db_get_dw(NULL, CHAT_MODULE, "roomx", -1);
	g_Settings.iY = db_get_dw(NULL, CHAT_MODULE, "roomy", -1);

	g_Settings.bTabsEnable = db_get_b(NULL, CHAT_MODULE, "Tabs", 1) != 0;
	g_Settings.TabRestore = db_get_b(NULL, CHAT_MODULE, "TabRestore", 0) != 0;
	g_Settings.TabsAtBottom = db_get_b(NULL, CHAT_MODULE, "TabBottom", 0) != 0;
	g_Settings.TabCloseOnDblClick = db_get_b(NULL, CHAT_MODULE, "TabCloseOnDblClick", 0) != 0;
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

int OnCheckPlugins(WPARAM, LPARAM)
{
	SmileyAddInstalled = ServiceExists(MS_SMILEYADD_REPLACESMILEYS);
	PopupInstalled = ServiceExists(MS_POPUP_ADDPOPUPT);
	return 0;
}

static void TabsInit()
{
	memset(&g_TabSession, 0, sizeof(SESSION_INFO));
	g_TabSession.iType = GCW_TABROOM;
	g_TabSession.iSplitterX = g_Settings.iSplitterX;
	g_TabSession.iSplitterY = g_Settings.iSplitterY;
	g_TabSession.iLogFilterFlags = (int)db_get_dw(NULL, CHAT_MODULE, "FilterFlags", 0x03E0);
	g_TabSession.bFilterEnabled = db_get_b(NULL, CHAT_MODULE, "FilterEnabled", 0);
	g_TabSession.bNicklistEnabled = db_get_b(NULL, CHAT_MODULE, "ShowNicklist", 1);
	g_TabSession.iFG = 4;
	g_TabSession.bFGSet = TRUE;
	g_TabSession.iBG = 2;
	g_TabSession.bBGSet = TRUE;
}

void Load_ChatModule()
{
	AddIcons();
	RegisterFonts();

	CHAT_MANAGER_INITDATA data = { &g_Settings, sizeof(MODULEINFO), sizeof(SESSION_INFO), LPGENW("Chat module"), FONTMODE_SKIP };
	pci = Chat_GetInterface(&data);
	saveCI = *pci;

	pci->OnAddUser = OnAddUser;
	pci->OnNewUser = OnNewUser;
	pci->OnRemoveUser = OnRemoveUser;

	pci->OnAddStatus = OnAddStatus;
	pci->OnSetStatus = OnSetStatus;
	pci->OnSetTopic = OnSetTopic;

	pci->OnAddLog = OnAddLog;
	pci->OnClearLog = OnClearLog;

	pci->OnCreateModule = OnCreateModule;
	pci->OnOfflineSession = OnOfflineSession;
	pci->OnRemoveSession = OnRemoveSession;
	pci->OnRenameSession = OnRenameSession;
	pci->OnReplaceSession = OnReplaceSession;
	pci->OnDblClickSession = OnDblClickSession;

	pci->OnEventBroadcast = OnEventBroadcast;
	pci->OnLoadSettings = OnLoadSettings;
	pci->OnSetStatusBar = OnSetStatusBar;
	pci->OnFlashWindow = OnFlashWindow;
	pci->OnFlashHighlight = OnFlashHighlight;
	pci->ShowRoom = ShowRoom;

	pci->DoPopup = DoPopup;
	pci->DoTrayIcon = DoTrayIcon;
	pci->ReloadSettings();

	g_hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_MENU));

	hIconsList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 0, 100);
	ImageList_AddIcon(hIconsList, Skin_LoadIcon(SKINICON_EVENT_MESSAGE));
	ImageList_AddIcon(hIconsList, LoadIconEx("overlay", FALSE));
	ImageList_SetOverlayImage(hIconsList, 1, 1);

	TabsInit();

	HookEvent(ME_SYSTEM_MODULELOAD, OnCheckPlugins);
}

void Unload_ChatModule()
{
	db_set_w(NULL, CHAT_MODULE, "SplitterX", (WORD)g_Settings.iSplitterX);
	db_set_w(NULL, CHAT_MODULE, "SplitterY", (WORD)g_Settings.iSplitterY);
	db_set_dw(NULL, CHAT_MODULE, "roomx", g_Settings.iX);
	db_set_dw(NULL, CHAT_MODULE, "roomy", g_Settings.iY);
	db_set_dw(NULL, CHAT_MODULE, "roomwidth", g_Settings.iWidth);
	db_set_dw(NULL, CHAT_MODULE, "roomheight", g_Settings.iHeight);

	if (g_Settings.MessageAreaFont)
		DeleteObject(g_Settings.MessageAreaFont);
	DestroyMenu(g_hMenu);
}
