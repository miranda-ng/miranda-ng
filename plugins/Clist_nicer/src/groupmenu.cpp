/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org),
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

static INT_PTR CommandHelper(WPARAM wParam, LPARAM)
{
	SendMessage(g_clistApi.hwndContactList, WM_COMMAND, MAKELONG(wParam, BN_CLICKED), 1);
	return 0;
}

static int OnBuildGroupMenu(WPARAM, LPARAM)
{
	for (auto &it : hAppearanceItems)
		Menu_SetChecked(it.hMenu, (cfg::dat.dwFlags & it.mask) != 0);
	return 0;
}

IconItem iconItem[] =
{
	{ LPGEN("Contact list"), "clist", IDI_CLIST }
};

void InitIconLibMenuIcons(void)
{
	g_plugin.registerIcon(LPGEN("Contact list"), iconItem);
}

void InitGroupMenus(void)
{
	HookEvent(ME_CLIST_PREBUILDGROUPMENU, OnBuildGroupMenu);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0xe386678a, 0x5aee, 0x4bfa, 0xa8, 0x23, 0xd, 0xa0, 0x11, 0x99, 0xb1, 0x98);
	mi.position = 500001;
	mi.pszService = MS_CLIST_SHOWHIDE;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_SHOWHIDE);
	mi.name.a = LPGEN("&Hide/show");
	Menu_AddGroupMenuItem(&mi);

	// Appearance menu
	SET_UID(mi, 0x70c3cd5d, 0x775, 0x4197, 0x9a, 0x7f, 0x76, 0xd6, 0xba, 0x57, 0xf5, 0x78);
	mi.position = 390100;
	mi.hIcolibItem = iconItem[0].hIcolib;
	mi.name.a = LPGEN("Appearance");
	mi.root = Menu_AddGroupMenuItem(&mi);

	UNSET_UID(mi);
	mi.hIcolibItem = nullptr;
	mi.pszService = "CLISTMENUSGroup/CommandHelper";
	CreateServiceFunction(mi.pszService, CommandHelper);
	for (auto &it : hAppearanceItems) {
		GroupMenuParam gmp;
		gmp.wParam = it.command;
		mi.name.a = it.name;
		mi.position = it.position;
		it.hMenu = Menu_AddGroupMenuItem(&mi, &gmp);
	}

	SET_UID(mi, 0xc2895531, 0x98e9, 0x40bc, 0x9b, 0x8e, 0xfb, 0x33, 0xe8, 0xab, 0xcc, 0xef);
	mi.root = nullptr;
	mi.position = 400000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_OPTIONS);
	mi.pszService = "Options/OptionsCommand";
	mi.name.a = LPGEN("&Options...");
	Menu_AddGroupMenuItem(&mi);

}
