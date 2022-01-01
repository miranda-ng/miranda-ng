/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

HGENMENU hGroupMainMenuItemProxy, hGroupStatusMenuItemProxy;

void InitSubGroupMenus(void);

INT_PTR GroupMenuOnAddService(WPARAM wParam, LPARAM lParam)
{
	MENUITEMINFO *mii = (MENUITEMINFO*)wParam;
	if (mii == nullptr)
		return 0;

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

void InitGroupMenus(void)
{
	CreateServiceFunction("CLISTMENUSGroup/GroupMenuOnAddService", GroupMenuOnAddService);

	InitSubGroupMenus();

	CMenuItem mi(&g_plugin);

	SET_UID(mi, 0xe386678a, 0x5aee, 0x4bfa, 0xa8, 0x23, 0xd, 0xa0, 0x11, 0x99, 0xb1, 0x98);
	mi.position = 500;
	mi.pszService = MS_CLIST_SHOWHIDE;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_SHOWHIDE);
	mi.name.a = LPGEN("&Hide/show");
	Menu_AddGroupMenuItem(&mi);

	SET_UID(mi, 0xb0f29663, 0x68b6, 0x494c, 0xaf, 0xab, 0xf6, 0x86, 0x45, 0xb8, 0xdb, 0xde);
	mi.position = 200000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_FINDUSER);
	mi.pszService = "FindAdd/FindAddCommand";
	mi.name.a = LPGEN("&Find/add contacts...");
	Menu_AddGroupMenuItem(&mi);

	SET_UID(mi, 0xff6855b4, 0x8c50, 0x43b7, 0x97, 0x51, 0xc1, 0x28, 0xa3, 0x10, 0x2b, 0x86);
	mi.position = 300000;
	mi.pszService = "";
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MAINMENU);
	mi.name.a = LPGEN("&Main menu");
	hGroupMainMenuItemProxy = Menu_AddGroupMenuItem(&mi);

	SET_UID(mi, 0xba91af46, 0x34e5, 0x4f3a, 0x88, 0x1c, 0xe7, 0xa3, 0x53, 0x58, 0x19, 0xde);
	mi.position = 300100;
	mi.pszService = "";
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_STATUS);
	mi.name.a = LPGEN("&Status");
	hGroupStatusMenuItemProxy = Menu_AddGroupMenuItem(&mi);

	SET_UID(mi, 0xc2895531, 0x98e9, 0x40bc, 0x9b, 0x8e, 0xfb, 0x33, 0xe8, 0xab, 0xcc, 0xef);
	mi.position = 400000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_OPTIONS);
	mi.pszService = "Options/OptionsCommand";
	mi.name.a = LPGEN("&Options...");
	Menu_AddGroupMenuItem(&mi);

	SET_UID(mi, 0x2221068a, 0x285d, 0x490c, 0xb1, 0x95, 0xff, 0x49, 0x75, 0xbc, 0xe4, 0x59);
	mi.position = 1000000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MIRANDA);
	mi.pszService = "Help/AboutCommand";
	mi.name.a = LPGEN("&About");
	Menu_AddGroupMenuItem(&mi);
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////        SubGroup MENU        //////////////////////////////

static HGENMENU hShowOfflineUsersHereMenuItem;

// wparam menu handle to pass to clc.c
// lparam WM_COMMAND HWND
static int OnBuildSubGroupMenu(WPARAM wParam, LPARAM)
{
	ClcGroup *group = (ClcGroup *)wParam;
	if (wParam == 0)
		return 0;

	if (MirandaExiting())
		return 0;

	bool showOfflineinGroup = CLCItems_IsShowOfflineGroup(group);
	bool gray2 = group->hideOffline == 0;

	Menu_EnableItem(hShowOfflineUsersHereMenuItem, gray2);
	Menu_SetChecked(hShowOfflineUsersHereMenuItem, showOfflineinGroup && gray2);
	return 0;
}

void InitSubGroupMenus(void)
{
	HookEvent(ME_CLIST_PREBUILDSUBGROUPMENU, OnBuildSubGroupMenu);

	// add exit command to menu
	GroupMenuParam gmp = { POPUP_GROUPSHOWOFFLINE, 0 };

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x7E081A28, 0x19B3, 0x407F, 0x80, 0x6B, 0x70, 0xC3, 0xC3, 0xA9, 0xD2, 0xA4);
	mi.position = 1002;
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.name.a = LPGEN("&Show offline users in here");
	hShowOfflineUsersHereMenuItem = Menu_AddSubGroupMenuItem(&mi, &gmp);
}
