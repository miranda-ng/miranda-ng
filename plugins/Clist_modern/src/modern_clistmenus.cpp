/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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

int LoadFavoriteContactMenu();

#pragma hdrstop

INT_PTR CloseAction(WPARAM, LPARAM)
{
	g_CluiData.bSTATE = STATE_PREPARETOEXIT;  // workaround for avatar service and other wich destroys service on OK_TOEXIT

	bool k;
	do {
		k = Miranda_OkToExit();
	}
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

static wchar_t *FAVMENUROOTNAME = LPGENW("&Contact rate");

static wchar_t *rates[] = {
	LPGENW("None"),
	LPGENW("Low"),
	LPGENW("Medium"),
	LPGENW("High")
};

static HGENMENU hFavoriteContactMenu = nullptr, hShowIfOflineItem = nullptr;
static HGENMENU hFavoriteContactMenuItems[_countof(rates)];

static IconItem iconList[] =
{
	{ LPGEN("Contact rate: none"),   "Rate1", IDI_FAVORITE_0 },
	{ LPGEN("Contact rate: low"),    "Rate2", IDI_FAVORITE_1 },
	{ LPGEN("Contact rate: medium"), "Rate3", IDI_FAVORITE_2 },
	{ LPGEN("Contact rate: high"),   "Rate4", IDI_FAVORITE_3 }
};

static int FAV_OnContactMenuBuild(WPARAM hContact, LPARAM)
{
	BYTE bContactRate = db_get_b(hContact, "CList", "Rate", 0);
	if (bContactRate > _countof(rates) - 1)
		bContactRate = _countof(rates) - 1;

	CMStringW tszName;
	if (!bContactRate)
		tszName = FAVMENUROOTNAME;
	else
		tszName.Format(L"%s (%s)", TranslateW(FAVMENUROOTNAME), TranslateW(rates[bContactRate]));
	Menu_ModifyItem(hFavoriteContactMenu, tszName);

	for (int i = 0; i < _countof(rates); i++)
		Menu_SetChecked(hFavoriteContactMenuItems[i], bContactRate == i);

	Menu_SetChecked(hShowIfOflineItem, db_get_b(hContact, "CList", "noOffline", 0) != 0);
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
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, FAV_OnContactMenuBuild);

	CMenuItem mi;
	SET_UID(mi, 0xf99a2320, 0xc024, 0x48bd, 0x81, 0xf7, 0x9f, 0xa2, 0x5, 0xb0, 0x7f, 0xdc);
	mi.name.w = FAVMENUROOTNAME;
	mi.flags = CMIF_UNICODE;
	hFavoriteContactMenu = Menu_AddContactMenuItem(&mi);
	Menu_ConfigureItem(hFavoriteContactMenu, MCI_OPT_UNIQUENAME, "ModernClistMenu_ContactRate");

	UNSET_UID(mi);
	mi.root = hFavoriteContactMenu;
	mi.flags = CMIF_UNICODE | CMIF_SYSTEM; // not included into new menu settings
	mi.pszService = CLUI_FAVSETRATE;
	for (int i = 0; i < _countof(rates); i++) {
		mi.hIcolibItem = iconList[i].hIcolib;
		mi.name.w = rates[i];
		hFavoriteContactMenuItems[i] = Menu_AddContactMenuItem(&mi);
		Menu_ConfigureItem(hFavoriteContactMenuItems[i], MCI_OPT_EXECPARAM, i);
	}

	mi.hIcolibItem = nullptr;
	mi.pszService = CLUI_FAVTOGGLESHOWOFFLINE;
	mi.position = -100000000;
	mi.name.w = LPGENW("Show even if offline");
	hShowIfOflineItem = Menu_AddContactMenuItem(&mi);
	Menu_ConfigureItem(hShowIfOflineItem, MCI_OPT_EXECPARAM, _countof(rates) + 100000000);
	return 0;
}

int UnloadFavoriteContactMenu()
{
	Menu_RemoveItem(hFavoriteContactMenu); hFavoriteContactMenu = nullptr;
	return 0;
}
