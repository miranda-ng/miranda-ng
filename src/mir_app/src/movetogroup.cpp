/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (�) 2012-15 Miranda NG project (http://miranda-ng.org),
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

static HGENMENU hMoveToGroupItem = 0, hPriorityItem = 0, hFloatingItem = 0;
static LIST<HANDLE> lphGroupsItems(5);

#define MTG_MOVE "MoveToGroup/Move"

struct GroupItemSort
{
	TCHAR* name;
	int position;

	GroupItemSort(TCHAR* pname, int pos)
		: name(mir_tstrdup(pname)), position(pos)
	{
	}

	~GroupItemSort() { mir_free(name); }

	static int compare(const GroupItemSort* d1, const GroupItemSort* d2)
	{
		return _tcscoll(d1->name, d2->name);
	}
};

static TCHAR* PrepareGroupName(TCHAR* str)
{
	TCHAR* p = _tcschr(str, '&'), *d;
	if (p == NULL)
		return mir_tstrdup(str);

	d = p = (TCHAR*)mir_alloc(sizeof(TCHAR)*(2 * mir_tstrlen(str) + 1));
	while (*str) {
		if (*str == '&')
			*d++ = '&';
		*d++ = *str++;
	}

	*d++ = 0;
	return p;
}

static void AddGroupItem(HGENMENU hRoot, TCHAR* name, int pos, WPARAM param, bool checked)
{
	CMenuItem mi;
	mi.root = hRoot;
	mi.position = pos;
	mi.name.t = PrepareGroupName(name);
	mi.flags = CMIF_SYSTEM | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
	if (checked)
		mi.flags |= CMIF_CHECKED;
	mi.pszService = MTG_MOVE;
	HGENMENU result = Menu_AddContactMenuItem(&mi);
	Menu_ConfigureItem(result, MCI_OPT_EXECPARAM, param);

	lphGroupsItems.insert((HANDLE*)result);
	mir_free(mi.name.t);
}

// service
// wparam - hcontact
// lparam .popupposition from TMO_MenuItem

static int OnContactMenuBuild(WPARAM wParam, LPARAM)
{
	OBJLIST<GroupItemSort> groups(10, GroupItemSort::compare);
	for (int i = 0; i < lphGroupsItems.getCount(); i++)
		Menu_RemoveItem((HGENMENU)lphGroupsItems[i]);
	lphGroupsItems.destroy();

	ptrT szContactGroup(db_get_tsa(wParam, "CList", "Group"));

	int pos = 1000;

	AddGroupItem(hMoveToGroupItem, TranslateT("<Root group>"), pos, -1, !szContactGroup);

	pos += 100000; // Separator

	for (int i = 0;; i++) {
		char intname[20];
		_itoa(i, intname, 10);

		DBVARIANT dbv;
		if (db_get_ts(NULL, "CListGroups", intname, &dbv))
			break;

		if (dbv.ptszVal[0])
			groups.insert(new GroupItemSort(dbv.ptszVal + 1, i + 1));

		mir_free(dbv.ptszVal);
	}

	for (int i = 0; i < groups.getCount(); i++) {
		bool checked = szContactGroup && !mir_tstrcmp(szContactGroup, groups[i].name);
		AddGroupItem(hMoveToGroupItem, groups[i].name, ++pos, groups[i].position, checked);
	}

	return 0;
}

static INT_PTR MTG_DOMOVE(WPARAM wParam, LPARAM lParam)
{
	CallService(MS_CLIST_CONTACTCHANGEGROUP, wParam, lParam < 0 ? 0 : lParam);
	return 0;
}

void MTG_OnmodulesLoad()
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnContactMenuBuild);
	CreateServiceFunction(MTG_MOVE, MTG_DOMOVE);

	CMenuItem mi;
	SET_UID(mi, 0x403c548, 0x4ac6, 0x4ced, 0xa7, 0x6c, 0x4e, 0xb9, 0xc8, 0xba, 0x94, 0x5);
	mi.position = 100000;
	mi.name.a = LPGEN("&Move to group");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_GROUP);
	hMoveToGroupItem = Menu_AddContactMenuItem(&mi);
}
