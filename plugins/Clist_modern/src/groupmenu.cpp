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
#include "m_clui.h"
#include "modern_clist.h"
#include "modern_clc.h"

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////         Group MENU          //////////////////////////////

HGENMENU hGroupMainMenuItemProxy;
HGENMENU hHideShowMainMenuItem;
HGENMENU hGroupStatusMenuItemProxy;

HGENMENU hHideOfflineUsersMenuItem;
HGENMENU hHideOfflineUsersOutHereMenuItem;
HGENMENU hHideEmptyGroupsMenuItem;

HGENMENU hDisableGroupsMenuItem;
HGENMENU hNewGroupMenuItem;
HGENMENU hNewSubGroupMenuItem;

void InitSubGroupMenus(void);

INT_PTR GroupMenuOnAddService(WPARAM wParam, LPARAM lParam)
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

	return TRUE;
}

INT_PTR HideGroupsHelper(WPARAM, LPARAM)
{
	int newVal = !(GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS);
	db_set_b(NULL, "CList", "HideEmptyGroups", (BYTE)newVal);
	SendMessage(pcli->hwndContactTree, CLM_SETHIDEEMPTYGROUPS, newVal, 0);
	return 0;
}

INT_PTR UseGroupsHelper(WPARAM, LPARAM)
{
	int newVal = !(GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_USEGROUPS);
	db_set_b(NULL, "CList", "UseGroups", (BYTE)newVal);
	SendMessage(pcli->hwndContactTree, CLM_SETUSEGROUPS, newVal,0);
	return 0;
}

INT_PTR HideOfflineRootHelper(WPARAM, LPARAM)
{
	SendMessage(pcli->hwndContactTree, CLM_SETHIDEOFFLINEROOT,
		!SendMessage(pcli->hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0),
		0);
	return 0;
}

INT_PTR CreateGroupHelper(WPARAM, LPARAM)
{
	SendMessage(pcli->hwndContactTree, CLM_SETHIDEEMPTYGROUPS, 0, 0);
	SendMessage(pcli->hwndContactTree, CLM_SETUSEGROUPS, 1, 0);
	Clist_CreateGroup(0, 0);
	return 0;
};

static int OnBuildGroupMenu(WPARAM, LPARAM)
{
	if (MirandaExiting()) return 0;

	int flags = db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT) ? CMIF_CHECKED : 0;
	Menu_ModifyItem(hHideOfflineUsersMenuItem, NULL, INVALID_HANDLE_VALUE, flags);

	flags = SendMessage(pcli->hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0) ? CMIF_CHECKED : 0;
	Menu_ModifyItem(hHideOfflineUsersOutHereMenuItem, NULL, INVALID_HANDLE_VALUE, flags);

	flags = GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS ? CMIF_CHECKED : 0;
	Menu_ModifyItem(hHideEmptyGroupsMenuItem, NULL, INVALID_HANDLE_VALUE, flags);

	flags = GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_USEGROUPS ? 0 : CMIF_CHECKED;
	Menu_ModifyItem(hDisableGroupsMenuItem, NULL, INVALID_HANDLE_VALUE, flags);
	return 0;
}

static IconItemT iconItem[] =
{ 
	{ LPGENT("New group"), "NewGroup", IDI_NEWGROUP2 }
};

void GroupMenus_Init(void)
{
	Icon_RegisterT(g_hInst, LPGENT("Contact list"), iconItem, _countof(iconItem));
}

void InitGroupMenus(void)
{
	CreateServiceFunction("CLISTMENUSGroup/GroupMenuOnAddService", GroupMenuOnAddService);
	CreateServiceFunction("CLISTMENUSGroup/HideGroupsHelper", HideGroupsHelper);
	CreateServiceFunction("CLISTMENUSGroup/UseGroupsHelper", UseGroupsHelper);
	CreateServiceFunction("CLISTMENUSGroup/HideOfflineRootHelper", HideOfflineRootHelper);
	CreateServiceFunction("CLISTMENUSGroup/CreateGroupHelper", CreateGroupHelper);

	HookEvent(ME_CLIST_PREBUILDGROUPMENU, OnBuildGroupMenu);

	InitSubGroupMenus();

	// add exit command to menu
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

	mi.position = 400000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_OPTIONS);
	mi.pszService = "Options/OptionsCommand";
	mi.name.a = LPGEN("&Options...");
	Menu_AddGroupMenuItem(&mi);

	mi.position = 500000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MIRANDA);
	mi.pszService = "Help/AboutCommand";
	mi.name.a = LPGEN("&About");
	Menu_AddGroupMenuItem(&mi);

	GroupMenuParam gmp = {};
	mi.flags = 0;
	mi.position = 100000;
	mi.hIcolibItem = iconItem[0].hIcolib;
	mi.pszService = "CLISTMENUSGroup/CreateGroupHelper";
	mi.name.a = LPGEN("&New group");
	hNewGroupMenuItem = Menu_AddGroupMenuItem(&mi, &gmp);
	DestroyIcon_protect((HICON)mi.hIcolibItem);

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

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////        SubGroup MENU        //////////////////////////////

HGENMENU hHideOfflineUsersHereMenuItem, hShowOfflineUsersHereMenuItem;

// wparam menu handle to pass to clc.c
// lparam WM_COMMAND HWND
static int OnBuildSubGroupMenu(WPARAM wParam, LPARAM)
{
	BOOL gray1 = FALSE;
	BOOL gray2 = FALSE;
	BOOL showOfflineinGroup = FALSE;

	ClcGroup *group = (ClcGroup *)wParam;
	if (wParam == 0) return 0;

	if (MirandaExiting()) return 0;
	//contact->group

	showOfflineinGroup = CLCItems_IsShowOfflineGroup(group);
	gray1 = (showOfflineinGroup != FALSE);
	gray2 = (group->hideOffline != FALSE);

	if (gray1 && gray2) gray1 = FALSE;  //should not be cause CLCItems_IsShowOfflineGroup return false if group->hideOffline

	Menu_EnableItem(hHideOfflineUsersHereMenuItem, !gray1);
	Menu_SetChecked(hHideOfflineUsersHereMenuItem, group->hideOffline && !gray1);

	Menu_EnableItem(hShowOfflineUsersHereMenuItem, !gray2);
	Menu_SetChecked(hShowOfflineUsersHereMenuItem, showOfflineinGroup && !gray2);
	return 0;
}

INT_PTR SubGroupMenuonAddService(WPARAM wParam, LPARAM)
{
	MENUITEMINFO *mii = (MENUITEMINFO*)wParam;
	if (mii == NULL)
		return 0;

	return TRUE;
}

INT_PTR GroupMenuExecProxy(WPARAM wParam, LPARAM lParam)
{
	SendMessage(lParam ? (HWND)lParam : (HWND)pcli->hwndContactTree, WM_COMMAND, wParam, 0);
	return 0;
}

void InitSubGroupMenus(void)
{
	CreateServiceFunction("CLISTMENUSSubGroup/SubGroupMenuOnAddService", SubGroupMenuonAddService);
	CreateServiceFunction("CLISTMENUSSubGroup/GroupMenuExecProxy", GroupMenuExecProxy);

	HookEvent(ME_CLIST_PREBUILDSUBGROUPMENU, OnBuildSubGroupMenu);

	// add exit command to menu
	GroupMenuParam gmp;

	CMenuItem mi;
	mi.position = 1000;
	mi.hIcolibItem = iconItem[0].hIcolib;
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.name.a = LPGEN("&New subgroup");
	gmp.lParam = 0;
	gmp.wParam = POPUP_NEWSUBGROUP;
	hNewSubGroupMenuItem = Menu_AddSubGroupMenuItem(&mi, &gmp);

	mi.position = 1001;
	mi.hIcolibItem = NULL;
	mi.hIcolibItem = NULL;
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.name.a = LPGEN("&Hide offline users in here");
	gmp.lParam = 0;
	gmp.wParam = POPUP_GROUPHIDEOFFLINE;
	hHideOfflineUsersHereMenuItem = Menu_AddSubGroupMenuItem(&mi, &gmp);

	mi.position = 1002;
	mi.hIcolibItem = NULL;
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.name.a = LPGEN("&Show offline users in here");
	gmp.lParam = 0;
	gmp.wParam = POPUP_GROUPSHOWOFFLINE;
	hShowOfflineUsersHereMenuItem = Menu_AddSubGroupMenuItem(&mi, &gmp);

	memset(&mi, 0, sizeof(mi));
	mi.position = 900001;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_RENAME);
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.name.a = LPGEN("&Rename group");
	gmp.lParam = 0;
	gmp.wParam = POPUP_RENAMEGROUP;
	Menu_AddSubGroupMenuItem(&mi, &gmp);

	mi.position = 900002;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_DELETE);
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.name.a = LPGEN("&Delete group");
	gmp.lParam = 0;
	gmp.wParam = POPUP_DELETEGROUP;
	Menu_AddSubGroupMenuItem(&mi, &gmp);
}
