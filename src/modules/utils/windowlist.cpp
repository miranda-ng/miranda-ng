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

#include "..\..\core\commonheaders.h"

struct TWindowListItem
{
	TWindowListItem(MCONTACT _contact, HWND _wnd) :
		hContact(_contact),
		hWnd(_wnd)
		{}

	MCONTACT hContact;
	HWND hWnd;
};

typedef OBJLIST<TWindowListItem> TWindowList;

static INT_PTR AllocWindowList(WPARAM, LPARAM)
{
	return (INT_PTR)new TWindowList(10, NumericKeySortT);
}

static INT_PTR DestroyWindowList(WPARAM wParam, LPARAM)
{
	delete (TWindowList*)wParam;
	return 0;
}

static INT_PTR AddToWindowList(WPARAM, LPARAM lParam)
{
	WINDOWLISTENTRY *pEntry = (WINDOWLISTENTRY*)lParam;
	TWindowList *pList = (TWindowList*)pEntry->hList;
	if (pList != NULL)
		pList->insert(new TWindowListItem(pEntry->hContact, pEntry->hwnd));
	return 0;
}

static INT_PTR RemoveFromWindowList(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0) return 1;
	TWindowList &pList = *(TWindowList*)wParam;
	for (int i = 0; i < pList.getCount(); i++) {
		if (pList[i].hWnd == (HWND)lParam) {
			pList.remove(i);
			return 0;
		}
	}
	return 1;
}

static INT_PTR FindInWindowList(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0) return NULL;
	TWindowList &pList = *(TWindowList*)wParam;
	TWindowListItem *p = pList.find((TWindowListItem*)&lParam);
	return (p == NULL) ? NULL : (INT_PTR)p->hWnd;
}

static INT_PTR BroadcastToWindowList(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0 || lParam == 0) return NULL;
	TWindowList &pList = *(TWindowList*)wParam;
	MSG *msg = (MSG*)lParam;
	for (int i = pList.getCount()-1; i >= 0; i--)
		SendMessage(pList[i].hWnd, msg->message, msg->wParam, msg->lParam);
	return 0;
}

static INT_PTR BroadcastToWindowListAsync(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0 || lParam == 0) return NULL;
	TWindowList &pList = *(TWindowList*)wParam;
	MSG *msg = (MSG*)lParam;
	for (int i = pList.getCount()-1; i >= 0; i--)
		PostMessage(pList[i].hWnd, msg->message, msg->wParam, msg->lParam);
	return 0;
}

int InitWindowList(void)
{
	CreateServiceFunction(MS_UTILS_ALLOCWINDOWLIST, AllocWindowList);
	CreateServiceFunction(MS_UTILS_DESTROYWINDOWLIST, DestroyWindowList);
	CreateServiceFunction(MS_UTILS_ADDTOWINDOWLIST, AddToWindowList);
	CreateServiceFunction(MS_UTILS_REMOVEFROMWINDOWLIST, RemoveFromWindowList);
	CreateServiceFunction(MS_UTILS_BROADCASTTOWINDOWLIST, BroadcastToWindowList);
	CreateServiceFunction(MS_UTILS_BROADCASTTOWINDOWLIST_ASYNC, BroadcastToWindowListAsync);
	CreateServiceFunction(MS_UTILS_FINDWINDOWINLIST, FindInWindowList);
	return 0;
}
