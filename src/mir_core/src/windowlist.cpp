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

struct TWindowListItem
{
	TWindowListItem(MCONTACT _contact, HWND _wnd) :
		hContact(_contact),
		hWnd(_wnd)
		{}

	MCONTACT hContact;
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

MIR_CORE_DLL(void) WindowList_Destroy(MWindowList hList)
{
	delete hList;
}

MIR_CORE_DLL(int) WindowList_Add(MWindowList hList, HWND hwnd, MCONTACT hContact)
{
	if (hList == NULL)
		return 1;

	hList->insert(new TWindowListItem(hContact, hwnd));
	return 0;
}

MIR_CORE_DLL(int) WindowList_Remove(MWindowList hList, HWND hwnd)
{
	if (hList == NULL) return 1;
	
	for (int i = 0; i < hList->getCount(); i++) {
		if ((*hList)[i].hWnd == hwnd) {
			hList->remove(i);
			return 0;
		}
	}
	return 1;
}

MIR_CORE_DLL(HWND) WindowList_Find(MWindowList hList, MCONTACT hContact)
{
	if (hList == NULL)
		return NULL;

	TWindowListItem *p = hList->find((TWindowListItem*)&hContact);
	return (p == NULL) ? NULL : p->hWnd;
}

MIR_CORE_DLL(int) WindowList_Broadcast(MWindowList hList, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (hList == NULL)
		return NULL;

	for (int i = hList->getCount()-1; i >= 0; i--)
		SendMessage((*hList)[i].hWnd, message, wParam, lParam);
	return 0;
}

MIR_CORE_DLL(int) WindowList_BroadcastAsync(MWindowList hList, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (hList == NULL)
		return NULL;

	for (int i = hList->getCount()-1; i >= 0; i--)
		PostMessage((*hList)[i].hWnd, message, wParam, lParam);
	return 0;
}
