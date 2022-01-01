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

static HGENMENU hMoveToGroupItem = nullptr, hPriorityItem = nullptr, hFloatingItem = nullptr;
static LIST<HANDLE> lphGroupsItems(5);

#define MTG_MOVE "MoveToGroup/Move"

struct GroupItemSort
{
	wchar_t* name;
	int position;

	GroupItemSort(wchar_t* pname, int pos)
		: name(mir_wstrdup(pname)), position(pos)
	{
	}

	~GroupItemSort() { mir_free(name); }

	static int compare(const GroupItemSort* d1, const GroupItemSort* d2)
	{
		return wcscoll(d1->name, d2->name);
	}
};

static wchar_t* PrepareGroupName(wchar_t* str)
{
	wchar_t* p = wcschr(str, '&'), *d;
	if (p == nullptr)
		return mir_wstrdup(str);

	d = p = (wchar_t*)mir_alloc(sizeof(wchar_t)*(2 * mir_wstrlen(str) + 1));
	while (*str) {
		if (*str == '&')
			*d++ = '&';
		*d++ = *str++;
	}

	*d++ = 0;
	return p;
}

static void AddGroupItem(HGENMENU hRoot, wchar_t* name, int pos, WPARAM param, bool checked)
{
	CMenuItem mi(&g_plugin);
	mi.root = hRoot;
	mi.position = pos;
	mi.name.w = PrepareGroupName(name);
	mi.flags = CMIF_SYSTEM | CMIF_UNICODE | CMIF_KEEPUNTRANSLATED;
	if (checked)
		mi.flags |= CMIF_CHECKED;
	mi.pszService = MTG_MOVE;
	HGENMENU result = Menu_AddContactMenuItem(&mi);
	Menu_ConfigureItem(result, MCI_OPT_EXECPARAM, param);

	lphGroupsItems.insert((HANDLE*)result);
	mir_free(mi.name.w);
}

// service
// wparam - hcontact
// lparam .popupposition from TMO_MenuItem

static int OnContactMenuBuild(WPARAM wParam, LPARAM)
{
	OBJLIST<GroupItemSort> groups(10, GroupItemSort::compare);
	for (auto &p : lphGroupsItems)
		Menu_RemoveItem((HGENMENU)p);
	lphGroupsItems.destroy();

	ptrW szContactGroup(Clist_GetGroup(wParam));

	int pos = 1000;

	AddGroupItem(hMoveToGroupItem, TranslateT("<Root group>"), pos, -1, !szContactGroup);

	pos += 100000; // Separator

	for (int i = 0;; i++) {
		char intname[20];
		_itoa(i, intname, 10);

		DBVARIANT dbv;
		if (db_get_ws(0, "CListGroups", intname, &dbv))
			break;

		if (dbv.pwszVal[0])
			groups.insert(new GroupItemSort(dbv.pwszVal + 1, i + 1));

		mir_free(dbv.pwszVal);
	}

	for (auto &p : groups) {
		bool checked = szContactGroup && !mir_wstrcmp(szContactGroup, p->name);
		AddGroupItem(hMoveToGroupItem, p->name, ++pos, p->position, checked);
	}

	return 0;
}

static INT_PTR MTG_DOMOVE(WPARAM wParam, LPARAM lParam)
{
	Clist_ContactChangeGroup(wParam, lParam < 0 ? 0 : lParam);
	return 0;
}

void InitMoveToGroup()
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnContactMenuBuild);
	CreateServiceFunction(MTG_MOVE, MTG_DOMOVE);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x403c548, 0x4ac6, 0x4ced, 0xa7, 0x6c, 0x4e, 0xb9, 0xc8, 0xba, 0x94, 0x5);
	mi.position = 100000;
	mi.name.a = LPGEN("&Move to group");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_GROUP);
	hMoveToGroupItem = Menu_AddContactMenuItem(&mi);
}
