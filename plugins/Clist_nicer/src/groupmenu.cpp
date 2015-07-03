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

static HGENMENU hGroupMainMenuItemProxy;
static HGENMENU hHideShowMainMenuItem;
static HGENMENU hGroupStatusMenuItemProxy;
static HGENMENU hEventAreaMenuItemProxy;

struct
{
	char *name;
	int command, mask, position;
	HGENMENU hMenu;
}
static hAppearanceItems[] = 
{
	{ LPGEN("Show status icons"), POPUP_SHOWSTATUSICONS, CLUI_FRAME_STATUSICONS, 10001 },
	{ LPGEN("Show metacontact protocol icons"), POPUP_SHOWMETAICONS, CLUI_USEMETAICONS, 10002 },
	{ LPGEN("Show additional buttons"), POPUP_BUTTONS, CLUI_FRAME_SHOWBOTTOMBUTTONS, 110001 },
	{ LPGEN("Draw sunken frame"), POPUP_FRAME, CLUI_FRAME_CLISTSUNKEN, 110002 }
};

static HMENU hMenuOldContext;

static INT_PTR GroupMenu_OnAddService(WPARAM wParam, LPARAM lParam)
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

static INT_PTR CommandHelper(WPARAM wParam, LPARAM)
{
	SendMessage(pcli->hwndContactList, WM_COMMAND, MAKELONG(wParam, BN_CLICKED), 1);
	return 0;
}

static int OnBuildGroupMenu(WPARAM, LPARAM)
{
	for (int i = 0; i < _countof(hAppearanceItems); i++)
		Menu_SetChecked(hAppearanceItems[i].hMenu, (cfg::dat.dwFlags & hAppearanceItems[i].mask) != 0);
	return 0;
}

IconItemT iconItem[] = {
	{ LPGENT("Contact list"), "clist", IDI_CLIST }
};

void InitIconLibMenuIcons(void)
{
	Icon_RegisterT(g_hInst, LPGENT("Contact list"), iconItem, _countof(iconItem));
}

void InitGroupMenus(void)
{
	CreateServiceFunction("CLISTMENUSGroup/GroupMenuOnAddService", GroupMenu_OnAddService);

	HookEvent(ME_CLIST_PREBUILDGROUPMENU, OnBuildGroupMenu);

	// add exit command to menu
	CMenuItem mi;
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

	// Appearance menu
	mi.position = 390100;
	mi.hIcolibItem = iconItem[0].hIcolib;
	mi.name.a = LPGEN("Appearance");
	mi.root = Menu_AddGroupMenuItem(&mi);

	mi.hIcolibItem = NULL;
	mi.pszService = "CLISTMENUSGroup/CommandHelper";
	CreateServiceFunction(mi.pszService, CommandHelper);
	for (int i = 0; i < _countof(hAppearanceItems); i++) {
		GroupMenuParam gmp;
		gmp.wParam = hAppearanceItems[i].command;
		mi.name.a = hAppearanceItems[i].name;
		mi.position = hAppearanceItems[i].position;
		hAppearanceItems[i].hMenu = Menu_AddGroupMenuItem(&mi, &gmp);
	}

	mi.root = NULL;
	mi.position = 400000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_OPTIONS);
	mi.pszService = "Options/OptionsCommand";
	mi.name.a = LPGEN("&Options...");
	Menu_AddGroupMenuItem(&mi);

	mi.position = 1000000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MIRANDA);
	mi.pszService = "CLN/About";
	mi.name.a = LPGEN("&About the contact list...");
	Menu_AddGroupMenuItem(&mi);
}
