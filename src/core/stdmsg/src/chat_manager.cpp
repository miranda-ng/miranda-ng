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

/////////////////////////////////////////////////////////////////////////////////////////

CHAT_MANAGER *pci, saveCI;

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

static void OnReplaceSession(SESSION_INFO *si)
{
	if (si->hWnd)
		RedrawWindow(GetDlgItem(si->hWnd, IDC_LIST), NULL, NULL, RDW_INVALIDATE);
}

static void OnNewUser(SESSION_INFO *si, USERINFO*)
{
	if (si->hWnd)
		SendMessage(si->hWnd, GC_UPDATENICKLIST, 0, 0);
}

static void OnFlashHighlight(SESSION_INFO *si, int bInactive)
{
	if (!bInactive)
		return;

	if (!g_Settings.bTabsEnable && si->hWnd && g_Settings.bFlashWindowHighlight)
		SetTimer(si->hWnd, TIMERID_FLASHWND, 900, NULL);
	if (g_Settings.bTabsEnable && si->pDlg)
		SendMessage(si->hWnd, GC_SETMESSAGEHIGHLIGHT, 0, (LPARAM)si->pDlg);
}

static void OnFlashWindow(SESSION_INFO *si, int bInactive)
{
	if (!bInactive)
		return;

	if (!g_Settings.bTabsEnable && si->hWnd && g_Settings.bFlashWindow)
		SetTimer(si->hWnd, TIMERID_FLASHWND, 900, NULL);
	if (g_Settings.bTabsEnable && si->pDlg)
		SendMessage(si->hWnd, GC_SETTABHIGHLIGHT, 0, (LPARAM)si->pDlg);
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
	g_Settings.bTabRestore = db_get_b(NULL, CHAT_MODULE, "TabRestore", 0) != 0;
	g_Settings.bTabsAtBottom = db_get_b(NULL, CHAT_MODULE, "TabBottom", 0) != 0;
	g_Settings.bTabCloseOnDblClick = db_get_b(NULL, CHAT_MODULE, "TabCloseOnDblClick", 0) != 0;

	g_Settings.iSplitterX = db_get_w(NULL, CHAT_MODULE, "SplitterX", 105);
	if (g_Settings.iSplitterX <= 50)
		g_Settings.iSplitterX = 105;
	g_Settings.iSplitterY = db_get_w(NULL, CHAT_MODULE, "SplitterY", 90);
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
	saveCI = *pci;

	pci->OnNewUser = OnNewUser;

	pci->OnCreateModule = OnCreateModule;
	pci->OnReplaceSession = OnReplaceSession;

	pci->OnLoadSettings = OnLoadSettings;
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

	HookEvent(ME_SYSTEM_MODULELOAD, OnCheckPlugins);
	InitTabs();
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
