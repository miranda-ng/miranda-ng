/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////         Group MENU          //////////////////////////////

HGENMENU hGroupMainMenuItemProxy;
HGENMENU hHideShowMainMenuItem;
HGENMENU hGroupStatusMenuItemProxy;
HGENMENU hAppearanceMenuItemProxy;
HGENMENU hEventAreaMenuItemProxy;

static HMENU hMenuOldContext;

HGENMENU hHideOfflineUsersMenuItem;
HGENMENU hHideOfflineUsersOutHereMenuItem;
HGENMENU hHideEmptyGroupsMenuItem;
HGENMENU hDisableGroupsMenuItem;
HGENMENU hNewGroupMenuItem;
HGENMENU hNewSubGroupMenuItem;

void InitSubGroupMenus(void);

INT_PTR GroupMenu_OnAddService(WPARAM wParam, LPARAM lParam)
{
	MENUITEMINFO *mii = (MENUITEMINFO*)wParam;
	if (mii == NULL)
		return 0;

	if (hHideShowMainMenuItem == (HANDLE)lParam) {
		mii->fMask |= MIIM_STATE;
		mii->fState |= MFS_DEFAULT;

	}
	if (hGroupMainMenuItemProxy == (HANDLE)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		mii->hSubMenu = Menu_GetMainMenu();
	}

	if (hGroupStatusMenuItemProxy == (HANDLE)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		mii->hSubMenu = Menu_GetStatusMenu();
	}
	if (hAppearanceMenuItemProxy == (HANDLE)lParam) {
		hMenuOldContext = GetSubMenu(LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT)), 3);
		TranslateMenu(hMenuOldContext);

		CheckMenuItem(hMenuOldContext, POPUP_FRAME, MF_BYCOMMAND | (cfg::dat.dwFlags & CLUI_FRAME_CLISTSUNKEN ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(hMenuOldContext, POPUP_BUTTONS, MF_BYCOMMAND | (cfg::dat.dwFlags & CLUI_FRAME_SHOWBOTTOMBUTTONS ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(hMenuOldContext, POPUP_SHOWMETAICONS, MF_BYCOMMAND | (cfg::dat.dwFlags & CLUI_USEMETAICONS ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(hMenuOldContext, POPUP_SHOWSTATUSICONS, MF_BYCOMMAND | (cfg::dat.dwFlags & CLUI_FRAME_STATUSICONS ? MF_CHECKED : MF_UNCHECKED));

		mii->fMask |= MIIM_SUBMENU;
		mii->hSubMenu = hMenuOldContext;
	}
	return TRUE;
}

INT_PTR HideGroupsHelper(WPARAM, LPARAM)
{
	int newVal = !(GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS);
	cfg::writeByte("CList", "HideEmptyGroups", (BYTE)newVal);
	SendMessage(pcli->hwndContactTree, CLM_SETHIDEEMPTYGROUPS, newVal, 0);
	return 0;
}

INT_PTR UseGroupsHelper(WPARAM, LPARAM)
{
	int newVal = !(GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_USEGROUPS);
	cfg::writeByte("CList", "UseGroups", (BYTE)newVal);
	SendMessage(pcli->hwndContactTree, CLM_SETUSEGROUPS, newVal,0);
	return 0;
}

INT_PTR HideOfflineRootHelper(WPARAM, LPARAM)
{
	SendMessage(pcli->hwndContactTree, CLM_SETHIDEOFFLINEROOT, !SendMessage(pcli->hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0), 0);
	return 0;
}

static int OnBuildGroupMenu(WPARAM, LPARAM)
{
	bool bEnabled = 0 != cfg::getByte("CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT);
	Menu_EnableItem(hHideOfflineUsersMenuItem, bEnabled);

	bEnabled = 0 != SendMessage(pcli->hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0);
	Menu_EnableItem(hHideOfflineUsersOutHereMenuItem, bEnabled);

	bEnabled = 0 != (GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS);
	Menu_EnableItem(hHideEmptyGroupsMenuItem, bEnabled);

	bEnabled = 0 != (GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_USEGROUPS);
	Menu_EnableItem(hDisableGroupsMenuItem, bEnabled);

	Menu_EnableItem(hGroupMainMenuItemProxy, bEnabled);
	Menu_EnableItem(hAppearanceMenuItemProxy, bEnabled);
	return 0;
}

IconItemT iconItem[] = {
	{ LPGENT("New group"),    "new_group", IDI_ADDGROUP },
	{ LPGENT("Contact list"), "clist",     IDI_CLIST }
};

void InitIconLibMenuIcons(void)
{
	Icon_RegisterT(g_hInst, LPGENT("Contact list"), iconItem, _countof(iconItem));
}

void InitGroupMenus(void)
{
	CreateServiceFunction("CLISTMENUSGroup/GroupMenuOnAddService", GroupMenu_OnAddService);
	CreateServiceFunction("CLISTMENUSGroup/HideGroupsHelper", HideGroupsHelper);
	CreateServiceFunction("CLISTMENUSGroup/UseGroupsHelper", UseGroupsHelper);
	CreateServiceFunction("CLISTMENUSGroup/HideOfflineRootHelper", HideOfflineRootHelper);

	HookEvent(ME_CLIST_PREBUILDGROUPMENU, OnBuildGroupMenu);

	InitSubGroupMenus();

	// add exit command to menu
	GroupMenuParam gmp;

	CMenuItem mi;
	mi.position = 1900000;
	mi.pszService = "CloseAction";
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_EXIT);
	mi.name.a = LPGEN("E&xit");
	Menu_AddGroupMenuItem(&mi);

	mi.position = 500;
	mi.pszService = MS_CLIST_SHOWHIDE;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_SHOWHIDE);
	mi.name.a = LPGEN("&Hide/show");
	hHideShowMainMenuItem = Menu_AddGroupMenuItem(&mi);

	mi.position = 200000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_FINDUSER);
	mi.pszService = "FindAdd/FindAddCommand";
	mi.name.a = LPGEN("&Find/add contacts...");
	Menu_AddGroupMenuItem(&mi);

	mi.position = 300000;
	mi.pszService = "";
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MAINMENU);
	mi.name.a = LPGEN("&Main menu");
	hGroupMainMenuItemProxy = Menu_AddGroupMenuItem(&mi);

	mi.position = 300100;
	mi.pszService = "";
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_STATUS);
	mi.name.a = LPGEN("&Status");
	hGroupStatusMenuItemProxy = Menu_AddGroupMenuItem(&mi);

	mi.position = 390100;
	mi.hIcolibItem = iconItem[1].hIcolib;
	mi.name.a = LPGEN("Appearance");
	hAppearanceMenuItemProxy = Menu_AddGroupMenuItem(&mi);

	mi.position = 400000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_OPTIONS);
	mi.pszService = "Options/OptionsCommand";
	mi.name.a = LPGEN("&Options...");
	Menu_AddGroupMenuItem(&mi);

	mi.position = 500000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MIRANDA);
	mi.pszService = "CLN/About";
	mi.name.a = LPGEN("&About the contact list...");
	Menu_AddGroupMenuItem(&mi);

	mi.flags = 0;
	mi.position = 100000;
	mi.hIcolibItem = iconItem[0].hIcolib;
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.name.a = LPGEN("&New group");
	hNewGroupMenuItem = Menu_AddGroupMenuItem(&mi, &gmp);

	mi.position = 100001;
	mi.hIcolibItem = NULL;
	mi.hIcolibItem = NULL;
	mi.pszService = MS_CLIST_SETHIDEOFFLINE;
	mi.name.a = LPGEN("&Hide offline users");
	gmp.wParam = -1;
	hHideOfflineUsersMenuItem = Menu_AddGroupMenuItem(&mi, &gmp);

	mi.position = 100002;
	mi.pszService = "CLISTMENUSGroup/HideOfflineRootHelper";
	mi.name.a = LPGEN("Hide &offline users out here");
	hHideOfflineUsersOutHereMenuItem = Menu_AddGroupMenuItem(&mi);

	mi.position = 100003;
	mi.pszService = "CLISTMENUSGroup/HideGroupsHelper";
	mi.name.a = LPGEN("Hide &empty groups");
	hHideEmptyGroupsMenuItem = Menu_AddGroupMenuItem(&mi);

	mi.position = 100004;
	mi.pszService = "CLISTMENUSGroup/UseGroupsHelper";
	mi.name.a = LPGEN("Disable &groups");
	hDisableGroupsMenuItem = Menu_AddGroupMenuItem(&mi);
}

//////////////////////////////SubGroup MENU/////////////////////////
HANDLE hSubGroupMainMenuItemProxy;
HANDLE hSubGroupStatusMenuItemProxy;
HANDLE hPreBuildSubGroupMenuEvent;
HGENMENU hHideOfflineUsersHereMenuItem;

static int OnBuildSubGroupMenu(WPARAM wParam, LPARAM)
{
	ClcGroup *group = (ClcGroup*)wParam;
	if (group != 0) // contact->group
		Menu_ModifyItem(hHideOfflineUsersHereMenuItem, NULL, INVALID_HANDLE_VALUE, group->hideOffline ? CMIF_CHECKED : 0);
	return 0;
}

// wparam menu handle to pass to clc.c
INT_PTR GroupMenuExecProxy(WPARAM wParam, LPARAM)
{
	SendMessage(pcli->hwndContactTree, WM_COMMAND, wParam, 0);
	return 0;
}

void InitSubGroupMenus(void)
{
	CreateServiceFunction("CLISTMENUSSubGroup/GroupMenuExecProxy", GroupMenuExecProxy);

	HookEvent(ME_CLIST_PREBUILDSUBGROUPMENU, OnBuildSubGroupMenu);

	// add exit command to menu
	GroupMenuParam gmp = {};

	CMenuItem mi;
	mi.position = 1000;
	mi.hIcolibItem = iconItem[0].hIcolib;
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.name.a = LPGEN("&New subgroup");
	gmp.wParam = POPUP_NEWSUBGROUP;
	hNewSubGroupMenuItem = Menu_AddSubGroupMenuItem(&mi, &gmp);

	mi.position = 1001;
	mi.hIcolibItem = NULL;
	mi.hIcolibItem = NULL;
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.name.a = LPGEN("&Hide offline users in here");
	gmp.wParam = POPUP_GROUPHIDEOFFLINE;
	hHideOfflineUsersHereMenuItem = (HGENMENU)Menu_AddSubGroupMenuItem(&mi, &gmp);

	memset(&mi, 0, sizeof(mi));
	mi.position = 900001;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_RENAME);
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.name.a = LPGEN("&Rename group");
	gmp.wParam = POPUP_RENAMEGROUP;
	Menu_AddSubGroupMenuItem(&mi, &gmp);

	mi.position = 900002;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_DELETE);
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.name.a = LPGEN("&Delete group");
	gmp.wParam = POPUP_DELETEGROUP;
	Menu_AddSubGroupMenuItem(&mi, &gmp);
}

//////////////////////////////END SubGroup MENU/////////////////////////
