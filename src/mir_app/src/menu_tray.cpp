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

void InitTrayMenus(void)
{
	CreateServiceFunction("CLISTMENUSTRAY/ExecService", TrayMenuExecService);
	CreateServiceFunction("CLISTMENUSTRAY/FreeOwnerDataTrayMenu", FreeOwnerDataTrayMenu);

	hEventPreBuildTrayMenu = CreateHookableEvent(ME_CLIST_PREBUILDTRAYMENU);

	// Tray menu
	hTrayMenuObject = Menu_AddObject("TrayMenu", LPGEN("Tray menu"), 0, "CLISTMENUSTRAY/ExecService");
	Menu_ConfigureObject(hTrayMenuObject, MCO_OPT_USERDEFINEDITEMS, TRUE);
	Menu_ConfigureObject(hTrayMenuObject, MCO_OPT_FREE_SERVICE, "CLISTMENUSTRAY/FreeOwnerDataTrayMenu");
	Menu_ConfigureObject(hTrayMenuObject, MCO_OPT_ONADD_SERVICE, "CLISTMENUSTRAY/TrayMenuOnAddService");
}
