/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)
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

static INT_PTR CompactMe(void* obj, WPARAM, LPARAM)
{
	CDbxMDBX *db = (CDbxMDBX*)obj;
	if (!db->Compact())
		MessageBox(0, TranslateT("Database was compacted successfully"), TranslateT("Database"), MB_OK | MB_ICONINFORMATION);
	else
		MessageBox(0, TranslateT("Database compaction failed"), TranslateT("Database"), MB_OK | MB_ICONERROR);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Compact"), "compact", IDI_COMPACT }
};

static int OnModulesLoaded(WPARAM, LPARAM)
{
	g_plugin.registerIcon(LPGEN("Database"), iconList, "mdbx");

	// main menu item
	CMenuItem mi(&g_plugin);
	mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Database"), 500000000, 0);

	SET_UID(mi, 0x98c0caf3, 0xBfe5, 0x4e31, 0xac, 0xf0, 0xab, 0x95, 0xb2, 0x9b, 0x9f, 0x73);
	mi.position++;
	mi.hIcolibItem = iconList[0].hIcolib;
	mi.name.a = LPGEN("Compact");
	mi.pszService = MS_DB_COMPACT;
	Menu_AddMainMenuItem(&mi);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDbxMDBX::InitDialogs()
{
	hService[0] = CreateServiceFunctionObj(MS_DB_COMPACT, CompactMe, this);

	hHook = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
}
