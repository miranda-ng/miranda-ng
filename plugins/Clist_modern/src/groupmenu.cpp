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

HGENMENU hGroupMainMenuItemProxy, hGroupStatusMenuItemProxy;

void InitSubGroupMenus(void);

INT_PTR GroupMenuOnAddService(WPARAM wParam, LPARAM lParam)
{
	MENUITEMINFO *mii = (MENUITEMINFO*)wParam;
	if (mii == NULL)
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

	// add exit command to menu
	CMenuItem mi;
	mi.position = 500;
	mi.pszService = MS_CLIST_SHOWHIDE;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_SHOWHIDE);
	mi.name.a = LPGEN("&Hide/show");
	Menu_AddGroupMenuItem(&mi);

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
	GroupMenuParam gmp;

	CMenuItem mi;
	mi.position = 1002;
	mi.hIcolibItem = NULL;
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.name.a = LPGEN("&Show offline users in here");
	gmp.lParam = 0;
	gmp.wParam = POPUP_GROUPSHOWOFFLINE;
	hShowOfflineUsersHereMenuItem = Menu_AddSubGroupMenuItem(&mi, &gmp);
}
