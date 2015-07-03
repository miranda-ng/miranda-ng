/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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
	replaceStr(pimi->UniqName, pmi->pszService);
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
	if (mii == NULL)
		return 0;

	if (hTrayMainMenuItemProxy == (HGENMENU)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		mii->hSubMenu = hMainMenu;
	}

	if (hTrayStatusMenuItemProxy == (HGENMENU)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		mii->hSubMenu = hStatusMenu = Menu_GetStatusMenu();
	}

	return TRUE;
}

void InitTrayMenus(void)
{
	CreateServiceFunction("CLISTMENUSTRAY/ExecService", TrayMenuExecService);
	CreateServiceFunction("CLISTMENUSTRAY/FreeOwnerDataTrayMenu", FreeOwnerDataTrayMenu);
	CreateServiceFunction("CLISTMENUSTRAY/TrayMenuOnAddService", TrayMenuOnAddService);

	hEventPreBuildTrayMenu = CreateHookableEvent(ME_CLIST_PREBUILDTRAYMENU);

	// Tray menu
	hTrayMenuObject = Menu_AddObject("TrayMenu", LPGEN("Tray menu"), 0, "CLISTMENUSTRAY/ExecService");
	Menu_ConfigureObject(hTrayMenuObject, MCO_OPT_USERDEFINEDITEMS, TRUE);
	Menu_ConfigureObject(hTrayMenuObject, MCO_OPT_FREE_SERVICE, "CLISTMENUSTRAY/FreeOwnerDataTrayMenu");
	Menu_ConfigureObject(hTrayMenuObject, MCO_OPT_ONADD_SERVICE, "CLISTMENUSTRAY/TrayMenuOnAddService");

	// add exit command to menu
	CMenuItem mi;
	mi.flags = CMIF_DEFAULT;
	mi.position = 100000;
	mi.pszService = MS_CLIST_SHOWHIDE;
	mi.name.a = LPGEN("&Hide/show");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_SHOWHIDE);
	Menu_AddTrayMenuItem(&mi);

	mi.flags = 0;
	mi.position = 200000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_FINDUSER);
	mi.pszService = "FindAdd/FindAddCommand";
	mi.name.a = LPGEN("&Find/add contacts...");
	Menu_AddTrayMenuItem(&mi);

	mi.position = 300000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MAINMENU); // eternity #004
	mi.pszService = NULL;
	mi.name.a = LPGEN("&Main menu");
	hTrayMainMenuItemProxy = Menu_AddTrayMenuItem(&mi);

	mi.position = 300100;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_STATUS); // eternity #004
	mi.name.a = LPGEN("&Status");
	hTrayStatusMenuItemProxy = Menu_AddTrayMenuItem(&mi);

	mi.position = 400000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_OPTIONS);
	mi.pszService = "Options/OptionsCommand";
	mi.name.a = LPGEN("&Options...");
	Menu_AddTrayMenuItem(&mi);

	mi.position = 500000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MIRANDA);
	mi.pszService = "Help/AboutCommand";
	mi.name.a = LPGEN("&About");
	Menu_AddTrayMenuItem(&mi);

	mi.position = 900000;
	mi.pszService = "CloseAction";
	mi.name.a = LPGEN("E&xit");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_EXIT);
	Menu_AddTrayMenuItem(&mi);
}
