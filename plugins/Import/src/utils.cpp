/*

Import plugin for Miranda NG

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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
// Creates a group with a specified name in the
// Miranda contact list.
// If contact is specified adds it to group
// ------------------------------------------------
// Returns 1 if successful and 0 when it fails.

int CreateGroup(const wchar_t *group, MCONTACT hContact)
{
	if (group == nullptr)
		return 0;

	if (Clist_GroupExists(group)) {
		if (hContact)
			db_set_ws(hContact, "CList", "Group", group);
		else
			AddMessage(LPGENW("Skipping duplicate group %s."), group);
		return 0;
	}

	Clist_GroupCreate(NULL, group);
	if (hContact)
		db_set_ws(hContact, "CList", "Group", group);
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// rtl integers

uint32_t RLInteger(const uint8_t *p)
{
	uint32_t ret = 0;
	for (int i = 0; i < 4; i++) {
		ret <<= 8;
		ret += p[i];
	}
	return ret;
}

uint32_t RLWord(const uint8_t *p)
{
	return (p[0] << 8) + p[1];
}

/////////////////////////////////////////////////////////////////////////////////////////
// icons

static IconItem iconList[] =
{
	{ LPGEN("Import..."), "import_main", IDI_IMPORT }
};

void RegisterIcons()
{
	g_plugin.registerIcon("Import", iconList);
}
