/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org),
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

static HGENMENU hShowOfflineUsersHereMenuItem;

// wparam menu handle to pass to clc.c
// lparam WM_COMMAND HWND
static int OnBuildSubGroupMenu(WPARAM wParam, LPARAM)
{
	bool showOfflineinGroup, gray;

	ClcGroup *group = (ClcGroup *)wParam;
	if (wParam == 0 || MirandaExiting())
		showOfflineinGroup = gray = false;
	else {
		showOfflineinGroup = group->bShowOffline;
		gray = !group->bHideOffline;
	}

	Menu_ShowItem(hShowOfflineUsersHereMenuItem, gray);
	Menu_SetChecked(hShowOfflineUsersHereMenuItem, showOfflineinGroup && gray);
	return 0;
}

static INT_PTR GroupMenuExecProxy(WPARAM, LPARAM lParam)
{
	SendMessage(g_clistApi.hwndContactTree, WM_COMMAND, lParam, 0);
	return 0;
}

void InitGroupMenus(void)
{
	CMenuItem mi(&g_plugin);

	SET_UID(mi, 0xe386678a, 0x5aee, 0x4bfa, 0xa8, 0x23, 0xd, 0xa0, 0x11, 0x99, 0xb1, 0x98);
	mi.position = 500001;
	mi.pszService = MS_CLIST_SHOWHIDE;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_SHOWHIDE);
	mi.name.a = LPGEN("&Hide/show");
	Menu_AddGroupMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDGROUPMENU, OnBuildSubGroupMenu);

	// add exit command to menu
	GroupMenuParam gmp = { 0, POPUP_GROUPSHOWOFFLINE };

	SET_UID(mi, 0x7E081A28, 0x19B3, 0x407F, 0x80, 0x6B, 0x70, 0xC3, 0xC3, 0xA9, 0xD2, 0xA4);
	mi.position = 900001;
	mi.name.a = LPGEN("Show offline users in here");
	mi.hIcolibItem = nullptr;
	mi.pszService = "Modern/GroupMenuExecProxy";
	hShowOfflineUsersHereMenuItem = Menu_AddGroupMenuItem(&mi, &gmp);
	CreateServiceFunction(mi.pszService, GroupMenuExecProxy);
}
