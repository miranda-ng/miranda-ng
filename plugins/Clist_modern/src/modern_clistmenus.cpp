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

#include "hdr/modern_commonheaders.h"
#include "hdr/modern_clist.h"
#include "m_genmenu.h"
#include "m_clui.h"
#include "hdr/modern_commonprototypes.h"

int LoadFavoriteContactMenu();
int UnloadFavoriteContactMenu();

#pragma hdrstop

INT_PTR CloseAction(WPARAM, LPARAM)
{
	int k;
	g_CluiData.bSTATE = STATE_PREPEARETOEXIT;  // workaround for avatar service and other wich destroys service on OK_TOEXIT
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

void UninitCustomMenus(void)
{
	UnloadFavoriteContactMenu();
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
	if (bContactRate > SIZEOF(rates) - 1)
		bContactRate = SIZEOF(rates) - 1;

	BOOL bModifyMenu = FALSE;

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.icolibItem = iconList[bContactRate].hIcolib;
	mi.pszPopupName = (char *)-1;
	mi.position = 0;
	mi.flags = CMIF_ROOTPOPUP | CMIF_TCHAR;
	if (!bContactRate)
		mi.ptszName = FAVMENUROOTNAME;
	else {
		TCHAR *str1 = TranslateTS(FAVMENUROOTNAME), *str2 = TranslateTS(rates[bContactRate]);
		size_t bufsize = (mir_tstrlen(str1) + mir_tstrlen(str2) + 15) * sizeof(TCHAR);
		TCHAR *name = (TCHAR *)_alloca(bufsize);
		mir_sntprintf(name, (bufsize / sizeof(TCHAR)), _T("%s (%s)"), str1, str2);
		mi.ptszName = name;
		mi.flags |= CMIF_KEEPUNTRANSLATED;
	}
	if (!hFavoriteContactMenu)
		hFavoriteContactMenu = Menu_AddContactMenuItem(&mi);
	else {
		mi.flags |= CMIM_FLAGS | CMIM_NAME;
		Menu_ModifyItem(hFavoriteContactMenu, &mi);
		bModifyMenu = TRUE;
	}

	OptParam op;
	op.Handle = hFavoriteContactMenu;
	op.Setting = OPT_MENUITEMSETUNIQNAME;
	op.Value = (INT_PTR)"ModernClistMenu_ContactRate";
	CallService(MO_SETOPTIONSMENUITEM, 0, (LPARAM)&op);

	mi.hParentMenu = hFavoriteContactMenu;
	if (!hFavoriteContactMenuItems) {
		hFavoriteContactMenuItems = (HGENMENU *)malloc(sizeof(HANDLE) * SIZEOF(rates));
		memset(hFavoriteContactMenuItems, 0, sizeof(HANDLE) * SIZEOF(rates));
	}

	int i;
	for (i = 0; i < SIZEOF(rates); i++) {
		mi.icolibItem = iconList[i].hIcolib;
		mi.ptszName = rates[i];
		mi.flags = CMIF_CHILDPOPUP | CMIF_TCHAR | ((bContactRate == i) ? CMIF_CHECKED : 0);
		mi.pszService = CLUI_FAVSETRATE;
		mi.popupPosition = i;
		if (bModifyMenu && hFavoriteContactMenuItems[i]) {
			mi.flags |= CMIM_FLAGS | CMIM_ICON;
			Menu_ModifyItem(hFavoriteContactMenuItems[i], &mi);
		}
		else hFavoriteContactMenuItems[i] = Menu_AddContactMenuItem(&mi);
	}

	mi.hIcon = NULL;
	mi.ptszName = LPGENT("Show even if offline");
	mi.flags = CMIF_CHILDPOPUP | CMIF_TCHAR | (db_get_b(hContact, "CList", "noOffline", 0) ? CMIF_CHECKED : 0);
	mi.pszService = CLUI_FAVTOGGLESHOWOFFLINE;
	mi.popupPosition = i + 100000000;
	mi.position = -100000000;
	if (bModifyMenu && hShowIfOflineItem) {
		mi.flags |= CMIM_FLAGS | CMIM_ICON;
		Menu_ModifyItem(hShowIfOflineItem, &mi);
	}
	else hShowIfOflineItem = Menu_AddContactMenuItem(&mi);

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
	Icon_Register(g_hInst, LPGEN("Contact list"), iconList, SIZEOF(iconList));

	CreateServiceFunction(CLUI_FAVSETRATE, FAV_SetRate);
	CreateServiceFunction(CLUI_FAVTOGGLESHOWOFFLINE, FAV_ToggleShowOffline);
	hOnContactMenuBuild_FAV = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, FAV_OnContactMenuBuild);
	return 0;
}

int UnloadFavoriteContactMenu()
{
	free(hFavoriteContactMenuItems);
	hFavoriteContactMenuItems = NULL;

	if (hFavoriteContactMenu)
		CallService(MO_REMOVEMENUITEM, (WPARAM)hFavoriteContactMenu, 0);
	hFavoriteContactMenu = NULL;

	return 0;
}
