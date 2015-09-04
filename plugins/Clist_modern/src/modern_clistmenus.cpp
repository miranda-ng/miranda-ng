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
#include "modern_clist.h"
#include "m_genmenu.h"
#include "m_clui.h"
#include "modern_commonprototypes.h"

int LoadFavoriteContactMenu();

#pragma hdrstop

INT_PTR CloseAction(WPARAM, LPARAM)
{
	int k;
	g_CluiData.bSTATE = STATE_PREPARETOEXIT;  // workaround for avatar service and other wich destroys service on OK_TOEXIT
	do
		k = CallService(MS_SYSTEM_OKTOEXIT, 0, 0);
	while (!k);

	if (k)
		DestroyWindow(pcli->hwndContactList);

	return 0;
}

int InitCustomMenus(void)
{
	CreateServiceFunction("CloseAction", CloseAction);
	LoadFavoriteContactMenu();
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//
//  FAVORITE CONTACT SYSTEM
//
//////////////////////////////////////////////////////////////////////////

#define CLUI_FAVSETRATE "CLUI/SetContactRate"  //LParam is rate, Wparam is contact handle
#define CLUI_FAVTOGGLESHOWOFFLINE "CLUI/ToggleContactShowOffline"

static HGENMENU hFavoriteContactMenu = NULL;
static HGENMENU *hFavoriteContactMenuItems = NULL;

static HGENMENU hShowIfOflineItem = NULL;
static HANDLE hOnContactMenuBuild_FAV = NULL;

static TCHAR *FAVMENUROOTNAME = LPGENT("&Contact rate");

static TCHAR *rates[] = {
	LPGENT("None"),
	LPGENT("Low"),
	LPGENT("Medium"),
	LPGENT("High")
};

static IconItem iconList[] =
{
	{ LPGEN("Contact rate: none"), "Rate1", IDI_FAVORITE_0 },
	{ LPGEN("Contact rate: low"), "Rate2", IDI_FAVORITE_1 },
	{ LPGEN("Contact rate: medium"), "Rate3", IDI_FAVORITE_2 },
	{ LPGEN("Contact rate: high"), "Rate4", IDI_FAVORITE_3 }
};

static int FAV_OnContactMenuBuild(WPARAM hContact, LPARAM)
{
	BYTE bContactRate = db_get_b(hContact, "CList", "Rate", 0);
	if (bContactRate > _countof(rates) - 1)
		bContactRate = _countof(rates) - 1;

	BOOL bModifyMenu = FALSE;

	CMenuItem mi;

	SET_UID(mi, 0xf99a2320, 0xc024, 0x48bd, 0x81, 0xf7, 0x9f, 0xa2, 0x5, 0xb0, 0x7f, 0xdc);
	mi.hIcolibItem = iconList[bContactRate].hIcolib;
	mi.flags = CMIF_TCHAR;
	if (!bContactRate)
		mi.name.t = FAVMENUROOTNAME;
	else {
		TCHAR *str1 = TranslateTS(FAVMENUROOTNAME), *str2 = TranslateTS(rates[bContactRate]);
		size_t bufsize = (mir_tstrlen(str1) + mir_tstrlen(str2) + 15) * sizeof(TCHAR);
		TCHAR *name = (TCHAR *)_alloca(bufsize);
		mir_sntprintf(name, (bufsize / sizeof(TCHAR)), _T("%s (%s)"), str1, str2);
		mi.name.t = name;
		mi.flags |= CMIF_KEEPUNTRANSLATED;
	}
	if (!hFavoriteContactMenu)
		hFavoriteContactMenu = Menu_AddContactMenuItem(&mi);
	else {
		Menu_ModifyItem(hFavoriteContactMenu, mi.name.t);
		bModifyMenu = TRUE;
	}

	Menu_ConfigureItem(hFavoriteContactMenu, MCI_OPT_UNIQUENAME, "ModernClistMenu_ContactRate");

	mi.root = hFavoriteContactMenu;
	if (!hFavoriteContactMenuItems) {
		hFavoriteContactMenuItems = (HGENMENU *)malloc(sizeof(HANDLE) * _countof(rates));
		memset(hFavoriteContactMenuItems, 0, sizeof(HANDLE) * _countof(rates));
	}

	int i;
	UNSET_UID(mi);
	for (i = 0; i < _countof(rates); i++) {
		mi.flags = CMIF_TCHAR | ((bContactRate == i) ? CMIF_CHECKED : 0);
		if (bModifyMenu && hFavoriteContactMenuItems[i])
			Menu_ModifyItem(hFavoriteContactMenuItems[i], NULL, iconList[i].hIcolib, mi.flags);
		else {
			mi.hIcolibItem = iconList[i].hIcolib;
			mi.name.t = rates[i];
			mi.pszService = CLUI_FAVSETRATE;
			hFavoriteContactMenuItems[i] = Menu_AddContactMenuItem(&mi);
			Menu_ConfigureItem(hFavoriteContactMenuItems[i], MCI_OPT_EXECPARAM, i);
		}
	}

	mi.hIcolibItem = NULL;
	mi.flags = CMIF_TCHAR | (db_get_b(hContact, "CList", "noOffline", 0) ? CMIF_CHECKED : 0);
	if (bModifyMenu && hShowIfOflineItem)
		Menu_ModifyItem(hShowIfOflineItem, NULL, INVALID_HANDLE_VALUE, mi.flags);
	else {
		SET_UID(mi, 0xbb78b0d3, 0xb56, 0x4d4c, 0xb5, 0x4d, 0x7c, 0x12, 0xc9, 0x6c, 0x5e, 0xe8);
		mi.pszService = CLUI_FAVTOGGLESHOWOFFLINE;
		mi.position = -100000000;
		mi.name.t = LPGENT("Show even if offline");
		hShowIfOflineItem = Menu_AddContactMenuItem(&mi);
		Menu_ConfigureItem(hShowIfOflineItem, MCI_OPT_EXECPARAM, i + 100000000);
	}

	return 0;
}

INT_PTR FAV_SetRate(WPARAM hContact, LPARAM nRate)
{
	if (hContact)
		db_set_b(hContact, "CList", "Rate", (BYTE)nRate);

	return 0;
}

INT_PTR FAV_ToggleShowOffline(WPARAM hContact, LPARAM)
{
	if (hContact)
		db_set_b(hContact, "CList", "noOffline", !db_get_b(hContact, "CList", "noOffline", 0));

	return 0;
}

int LoadFavoriteContactMenu()
{
	Icon_Register(g_hInst, LPGEN("Contact list"), iconList, _countof(iconList));

	CreateServiceFunction(CLUI_FAVSETRATE, FAV_SetRate);
	CreateServiceFunction(CLUI_FAVTOGGLESHOWOFFLINE, FAV_ToggleShowOffline);
	hOnContactMenuBuild_FAV = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, FAV_OnContactMenuBuild);
	return 0;
}

int UnloadFavoriteContactMenu()
{
	free(hFavoriteContactMenuItems);
	hFavoriteContactMenuItems = NULL;

	Menu_RemoveItem(hFavoriteContactMenu); hFavoriteContactMenu = NULL;
	return 0;
}
