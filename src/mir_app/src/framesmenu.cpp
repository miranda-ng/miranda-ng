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

#include <m_cluiframes.h>

static int hFrameMenuObject;
static HANDLE hPreBuildFrameMenuEvent;

// contactmenu exec param(ownerdata)
// also used in checkservice
struct FrameMenuExecParam
{
	ptrA    szServiceName;
	int     Frameid;
	INT_PTR param1;
};

INT_PTR FreeOwnerDataFrameMenu(WPARAM, LPARAM lParam)
{
	delete (FrameMenuExecParam*)lParam;
	return 0;
}

static INT_PTR AddContextFrameMenuItem(WPARAM, LPARAM lParam)
{
	CLISTMENUITEM *mi = (CLISTMENUITEM*)lParam;

	TMO_MenuItem tmi;
	if (!cli.pfnConvertMenu(mi, &tmi))
		return NULL;

	tmi.root = mi->hParentMenu;

	FrameMenuExecParam *fmep = new FrameMenuExecParam();
	fmep->szServiceName = mir_strdup(mi->pszService);
	fmep->Frameid = 0; // mi->popupPosition; !!!!!!!!!!!!!!!!!!!!!!!!!!
	fmep->param1 = (INT_PTR)mi->pszContactOwner;
	tmi.ownerdata = fmep;
	return (INT_PTR)Menu_AddItem(hFrameMenuObject, &tmi);
}

// called with:
// wparam - ownerdata
// lparam - lparam from winproc
INT_PTR FrameMenuExecService(WPARAM wParam, LPARAM lParam)
{
	FrameMenuExecParam *fmep = (FrameMenuExecParam*)wParam;
	if (fmep == NULL)
		return -1;

	CallService(fmep->szServiceName, lParam, fmep->param1);
	return 0;
}

//true - ok,false ignore
INT_PTR FrameMenuCheckService(WPARAM wParam, LPARAM)
{
	TCheckProcParam *pcpp = (TCheckProcParam*)wParam;
	if (pcpp == NULL)
		return FALSE;

	TMO_MenuItem mi;
	if (Menu_GetItemInfo(pcpp->MenuItemHandle, mi) == 0) {
		FrameMenuExecParam *fmep = (FrameMenuExecParam*)mi.ownerdata;
		if (fmep != NULL) {
			//pcpp->wParam  -  frameid
			if (((WPARAM)fmep->Frameid == pcpp->wParam) || fmep->Frameid == -1)
				return TRUE;
		}
	}
	return FALSE;
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

//========================== Frames end

int InitFramesMenus(void)
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
	return 0;
}
