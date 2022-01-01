/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "genmenu.h"

static int hTrayMenuObject;
static HANDLE hEventPreBuildTrayMenu;
static HGENMENU hTrayMainMenuItemProxy, hTrayStatusMenuItemProxy;

/////////////////////////////////////////////////////////////////////////////////////////
// Tray menu services

MIR_APP_DLL(HMENU) Menu_BuildTrayMenu(void)
{
	NotifyEventHooks(hEventPreBuildTrayMenu, 0, 0);

	HMENU hMenu = CreatePopupMenu();
	Menu_Build(hMenu, hTrayMenuObject);
	return hMenu;
}

MIR_APP_DLL(HGENMENU) Menu_AddTrayMenuItem(TMO_MenuItem *pmi)
{
	HGENMENU pimi = Menu_AddItem(hTrayMenuObject, pmi, mir_strdup(pmi->pszService));
	replaceStr(pimi->pszUniqName, pmi->pszService);
	return pimi;
}

MIR_APP_DLL(void) Menu_DestroyNestedMenu(HMENU hMenu)
{
	int cnt = GetMenuItemCount(hMenu);
	for (int i = 0; i < cnt; ++i) {
		HMENU hSubMenu = GetSubMenu(hMenu, i);
		if (hSubMenu && ((hSubMenu == hStatusMenu) || (hSubMenu == hMainMenu)))
			RemoveMenu(hMenu, i--, MF_BYPOSITION);
	}
	DestroyMenu(hMenu);
}

// called with:
// wparam - ownerdata
// lparam - lparam from winproc
static INT_PTR TrayMenuExecService(WPARAM wParam, LPARAM lParam)
{
	if (wParam != 0)
		CallService((char*)wParam, 0, lParam);

	return 1;
}

static INT_PTR FreeOwnerDataTrayMenu(WPARAM, LPARAM lParam)
{
	mir_free((char*)lParam);
	return 0;
}

static INT_PTR TrayMenuOnAddService(WPARAM wParam, LPARAM lParam)
{
	MENUITEMINFO *mii = (MENUITEMINFO*)wParam;
	if (mii == nullptr)
		return 0;

	if (hTrayMainMenuItemProxy == (HGENMENU)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		mii->hSubMenu = hMainMenu = Menu_GetMainMenu();
	}

	if (hTrayStatusMenuItemProxy == (HGENMENU)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		mii->hSubMenu = hStatusMenu = Menu_GetStatusMenu();
	}

	return TRUE;
}

static INT_PTR ShowHideStub(WPARAM, LPARAM)
{
	return g_clistApi.pfnShowHide();
}

void InitTrayMenus(void)
{
	CreateServiceFunction("CLISTMENUSTRAY/ExecService", TrayMenuExecService);
	CreateServiceFunction("CLISTMENUSTRAY/FreeOwnerDataTrayMenu", FreeOwnerDataTrayMenu);
	CreateServiceFunction("CLISTMENUSTRAY/TrayMenuOnAddService", TrayMenuOnAddService);

	hEventPreBuildTrayMenu = CreateHookableEvent(ME_CLIST_PREBUILDTRAYMENU);

	// Tray menu
	hTrayMenuObject = Menu_AddObject("TrayMenu", LPGEN("Tray menu"), nullptr, "CLISTMENUSTRAY/ExecService");
	Menu_ConfigureObject(hTrayMenuObject, MCO_OPT_USERDEFINEDITEMS, TRUE);
	Menu_ConfigureObject(hTrayMenuObject, MCO_OPT_FREE_SERVICE, "CLISTMENUSTRAY/FreeOwnerDataTrayMenu");
	Menu_ConfigureObject(hTrayMenuObject, MCO_OPT_ONADD_SERVICE, "CLISTMENUSTRAY/TrayMenuOnAddService");

	// add exit command to menu
	CMenuItem mi(&g_plugin);

	SET_UID(mi, 0x6c202553, 0xb4d5, 0x403c, 0xa6, 0x82, 0x2, 0xd8, 0x2b, 0x42, 0xba, 0x9e);
	mi.flags = CMIF_DEFAULT;
	mi.position = 100000;
	mi.pszService = MS_CLIST_SHOWHIDE;
	mi.name.a = LPGEN("&Hide/show");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_SHOWHIDE);
	Menu_AddTrayMenuItem(&mi);
	CreateServiceFunction(mi.pszService, ShowHideStub);

	SET_UID(mi, 0x10e9b2f0, 0xeef2, 0x4684, 0xa7, 0xa7, 0xde, 0x7f, 0x2a, 0xb3, 0x5b, 0x30);
	mi.flags = 0;
	mi.position = 200000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_FINDUSER);
	mi.pszService = "FindAdd/FindAddCommand";
	mi.name.a = LPGEN("&Find/add contacts...");
	Menu_AddTrayMenuItem(&mi);

	SET_UID(mi, 0xb6d861bc, 0x9c3f, 0x40d9, 0xaf, 0x58, 0x53, 0x9f, 0x22, 0xe4, 0x77, 0x98);
	mi.position = 300000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MAINMENU); // eternity #004
	mi.pszService = "MainMenu/Command";
	mi.name.a = LPGEN("&Main menu");
	hTrayMainMenuItemProxy = Menu_AddTrayMenuItem(&mi);

	SET_UID(mi, 0xbafb2db6, 0x8994, 0x40f5, 0x8f, 0x34, 0x4, 0xde, 0x67, 0x8e, 0x6e, 0x70);
	mi.position = 300100;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_STATUS); // eternity #004
	mi.pszService = "GlobalStatus/Command";
	mi.name.a = LPGEN("&Status");
	hTrayStatusMenuItemProxy = Menu_AddTrayMenuItem(&mi);

	SET_UID(mi, 0x75107441, 0xa2ef, 0x420d, 0x85, 0xd5, 0xf7, 0x40, 0x69, 0xf0, 0xa1, 0x42);
	mi.position = 400000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_OPTIONS);
	mi.pszService = "Options/OptionsCommand";
	mi.name.a = LPGEN("&Options...");
	Menu_AddTrayMenuItem(&mi);

	SET_UID(mi, 0x8cecc833, 0x81ad, 0x499f, 0xb0, 0x44, 0x1, 0x5, 0xf7, 0xe1, 0xe6, 0x7);
	mi.position = 500000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MIRANDA);
	mi.pszService = "Help/AboutCommand";
	mi.name.a = LPGEN("&About");
	Menu_AddTrayMenuItem(&mi);

	SET_UID(mi, 0xdfde6fc7, 0xaef7, 0x40db, 0xb4, 0x15, 0xf4, 0x6d, 0xa8, 0xc4, 0x41, 0x71);
	mi.position = 900000;
	mi.pszService = "CloseAction";
	mi.name.a = LPGEN("E&xit");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_EXIT);
	Menu_AddTrayMenuItem(&mi);
}
