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

#include "../stdafx.h"

struct TWindowListItem
{
	TWindowListItem(UINT_PTR _param, HWND _wnd) :
		param(_param),
		hWnd(_wnd)
		{}

	UINT_PTR param;
	HWND hWnd;
};

struct TWindowList : public OBJLIST<TWindowListItem>
{
	TWindowList() :
		OBJLIST<TWindowListItem>(10, NumericKeySortT)
		{}
};

MIR_CORE_DLL(MWindowList) WindowList_Create(void)
{
	return new TWindowList();
}

MIR_CORE_DLL(void) WindowList_Destroy(MWindowList &hList)
{
	delete hList;
	hList = nullptr;
}

MIR_CORE_DLL(int) WindowList_Add(MWindowList hList, HWND hwnd, UINT_PTR param)
{
	if (hList == nullptr)
		return 1;

	hList->insert(new TWindowListItem(param, hwnd));
	return 0;
}

MIR_CORE_DLL(int) WindowList_Remove(MWindowList hList, HWND hwnd)
{
	if (hList == nullptr) return 1;
	
	for (auto &it : *hList)
		if (it->hWnd == hwnd) {
			hList->removeItem(&it);
			return 0;
		}

	return 1;
}

MIR_CORE_DLL(HWND) WindowList_Find(MWindowList hList, UINT_PTR param)
{
	if (hList == nullptr)
		return nullptr;

	TWindowListItem *p = hList->find((TWindowListItem*)&param);
	return (p == nullptr) ? nullptr : p->hWnd;
}

MIR_CORE_DLL(int) WindowList_Broadcast(MWindowList hList, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (hList == nullptr)
		return 0;

	for (auto &it : hList->rev_iter())
		SendMessage(it->hWnd, message, wParam, lParam);
	return 0;
}

MIR_CORE_DLL(int) WindowList_BroadcastAsync(MWindowList hList, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (hList == nullptr)
		return 0;

	for (auto &it : hList->rev_iter())
		PostMessage(it->hWnd, message, wParam, lParam);
	return 0;
}
