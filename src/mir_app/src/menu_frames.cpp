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

#include <m_cluiframes.h>
#include "genmenu.h"

static int hFrameMenuObject;
static HANDLE hPreBuildFrameMenuEvent;

/////////////////////////////////////////////////////////////////////////////////////////
// contactmenu exec param(ownerdata)
// also used in checkservice

struct FrameMenuExecParam
{
	ptrA szServiceName;
};

INT_PTR FreeOwnerDataFrameMenu(WPARAM, LPARAM lParam)
{
	delete (FrameMenuExecParam*)lParam;
	return 0;
}

static INT_PTR AddContextFrameMenuItem(WPARAM, LPARAM lParam)
{
	TMO_MenuItem *pmi = (TMO_MenuItem*)lParam;

	FrameMenuExecParam *fmep = new FrameMenuExecParam();
	fmep->szServiceName = mir_strdup(pmi->pszService);
	return (INT_PTR)Menu_AddItem(hFrameMenuObject, pmi, fmep);
}

/////////////////////////////////////////////////////////////////////////////////////////
// called with:
// wparam - ownerdata
// lparam - lparam from winproc

INT_PTR FrameMenuExecService(WPARAM wParam, LPARAM lParam)
{
	FrameMenuExecParam *fmep = (FrameMenuExecParam*)wParam;
	if (fmep == nullptr)
		return -1;

	CallService(fmep->szServiceName, lParam, 0);
	return 0;
}

INT_PTR FrameMenuCheckService(WPARAM wParam, LPARAM)
{
	TCheckProcParam *pcpp = (TCheckProcParam*)wParam;
	if (pcpp == nullptr)
		return FALSE;

	TMO_IntMenuItem *pimi = pcpp->MenuItemHandle;
	return pimi->execParam == pcpp->wParam || pimi->execParam == -1;
}

static INT_PTR ContextFrameMenuNotify(WPARAM wParam, LPARAM lParam)
{
	NotifyEventHooks(hPreBuildFrameMenuEvent, wParam, lParam);
	return 0;
}

static INT_PTR BuildContextFrameMenu(WPARAM wParam, LPARAM lParam)
{
	ContextFrameMenuNotify(wParam, -1);

	HMENU hMenu = CreatePopupMenu();
	Menu_Build(hMenu, hFrameMenuObject, wParam, lParam);
	return (INT_PTR)hMenu;
}

/////////////////////////////////////////////////////////////////////////////////////////

void InitFramesMenus(void)
{
	CreateServiceFunction("FrameMenuExecService", FrameMenuExecService);
	CreateServiceFunction("FrameMenuCheckService", FrameMenuCheckService);
	CreateServiceFunction("FrameMenuFreeService", FreeOwnerDataFrameMenu);

	CreateServiceFunction("CList/AddContextFrameMenuItem", AddContextFrameMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDFRAMECONTEXT, BuildContextFrameMenu);
	CreateServiceFunction(MS_CLIST_FRAMEMENUNOTIFY, ContextFrameMenuNotify);
	hPreBuildFrameMenuEvent = CreateHookableEvent(ME_CLIST_PREBUILDFRAMEMENU);

	// frame menu object
	hFrameMenuObject = Menu_AddObject("FrameMenu", LPGEN("Frame menu"), "FrameMenuCheckService", "FrameMenuExecService");
	Menu_ConfigureObject(hFrameMenuObject, MCO_OPT_FREE_SERVICE, "FrameMenuFreeService");
}
