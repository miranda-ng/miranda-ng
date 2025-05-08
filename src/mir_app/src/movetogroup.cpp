/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org),
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
#include "clc.h"

static HGENMENU hMoveToGroupItem = nullptr, hPriorityItem = nullptr, hFloatingItem = nullptr;
static LIST<HANDLE> lphGroupsItems(5);

#define MTG_MOVE "MoveToGroup/Move"

struct GroupItemSort
{
	wchar_t* name;
	int position;
	HGENMENU hMenu = 0;

	GroupItemSort(wchar_t* pname, int pos) :
		name(mir_wstrdup(pname)), position(pos)
	{
	}

	~GroupItemSort() { mir_free(name); }

	static int compare(const GroupItemSort* d1, const GroupItemSort* d2)
	{
		return wcscoll(d1->name, d2->name);
	}
};

static HGENMENU AddGroupItem(HGENMENU hRoot, wchar_t* name, int pos, WPARAM param, bool checked)
{
	CMStringW wszName(name);
	wszName.Replace(L"&", L"&&");

	CMenuItem mi(&g_plugin);
	mi.root = hRoot;
	mi.position = pos;
	mi.name.w = wszName.GetBuffer();
	mi.flags = CMIF_SYSTEM | CMIF_UNICODE | CMIF_KEEPUNTRANSLATED;
	if (checked)
		mi.flags |= CMIF_CHECKED;
	mi.pszService = MTG_MOVE;
	
	HGENMENU result = Menu_AddContactMenuItem(&mi);
	Menu_ConfigureItem(result, MCI_OPT_EXECPARAM, param);

	lphGroupsItems.insert((HANDLE*)result);
	return result;
}

// service
// wparam - hcontact
// lparam .popupposition from TMO_MenuItem

static int OnContactMenuBuild(WPARAM wParam, LPARAM)
{
	for (auto &p : lphGroupsItems)
		Menu_RemoveItem((HGENMENU)p);
	lphGroupsItems.destroy();

	ptrW szContactGroup(Clist_GetGroup(wParam));

	int pos = 1000;

	AddGroupItem(hMoveToGroupItem, TranslateT("<Root group>"), pos, -1, !szContactGroup);

	pos += 100000; // Separator

	OBJLIST<GroupItemSort> groups(10, GroupItemSort::compare);
	for (MGROUP hGroup = 1;; hGroup++) {
		if (auto *pwszGroupName = Clist_GroupGetName(hGroup))
			groups.insert(new GroupItemSort(pwszGroupName, hGroup));
		else
			break;
	}

	for (auto &p : groups) {
		bool checked = szContactGroup && !mir_wstrcmp(szContactGroup, p->name);

		HGENMENU hRoot = hMoveToGroupItem;
		if (auto *pSlash = wcsrchr(p->name, '\\')) {
			*pSlash = 0;
			for (auto &it : groups) {
				if (!mir_wstrcmp(it->name, p->name)) {
					hRoot = it->hMenu;
					break;
				}
			}
			*pSlash = '\\';
			p->hMenu = AddGroupItem(hRoot, pSlash + 1, ++pos, p->position, checked);
		}
		else p->hMenu = AddGroupItem(hRoot, p->name, ++pos, p->position, checked);
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
