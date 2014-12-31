/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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

#include "commonheaders.h"
#include "m_clui.h"

#define TIM_CALLBACK   (WM_USER+1857)
#define TIM_CREATE     (WM_USER+1858)

extern INT_PTR ( *saveTrayIconProcessMessage )(WPARAM wParam, LPARAM lParam);
void DestroyTrayMenu(HMENU hMenu);

INT_PTR TrayIconProcessMessage(WPARAM wParam, LPARAM lParam)
{
	MSG *msg = (MSG*)wParam;
	switch(msg->message) {
	case WM_DRAWITEM:
		return CallService(MS_CLIST_MENUDRAWITEM,msg->wParam,msg->lParam);
		break;

	case WM_MEASUREITEM:
		return CallService(MS_CLIST_MENUMEASUREITEM,msg->wParam,msg->lParam);
		break;

	case TIM_CALLBACK:
		if (msg->lParam == WM_RBUTTONUP) {
			POINT pt;
			HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDTRAY,0,0);

			SetForegroundWindow(msg->hwnd);
			SetFocus(msg->hwnd);
			GetCursorPos(&pt);
			TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, msg->hwnd, NULL);
			DestroyTrayMenu(hMenu);
		}
		else break;

		*((LRESULT*)lParam) = 0;
		return TRUE;
	}

	return saveTrayIconProcessMessage( wParam, lParam );
}

//////////////////////////////TRAY MENU/////////////////////////
HANDLE hTrayMenuObject;

HANDLE hTrayMainMenuItemProxy;
HANDLE hHideShowMainMenuItem;
HANDLE hTrayStatusMenuItemProxy;
HANDLE hPreBuildTrayMenuEvent;

//traymenu exec param(ownerdata)
typedef struct
{
	char *szServiceName;
	INT_PTR Param1;
}
	TrayMenuExecParam,*lpTrayMenuExecParam;

/*
wparam = handle to the menu item returned by MS_CLIST_ADDCONTACTMENUITEM
return 0 on success.
*/
static INT_PTR RemoveTrayMenuItem(WPARAM wParam, LPARAM lParam)
{
	CallService(MO_REMOVEMENUITEM,wParam,0);
	return 0;
}

static INT_PTR BuildTrayMenu(WPARAM wParam, LPARAM lParam)
{
	int tick;
	HMENU hMenu;
	ListParam param = { 0 };
	param.MenuObjectHandle = hTrayMenuObject;

	//hMenu = hMainMenu;
	hMenu = CreatePopupMenu();
	//hMenu = wParam;
	tick = GetTickCount();

	NotifyEventHooks(hPreBuildTrayMenuEvent,0,0);

	CallService(MO_BUILDMENU,(WPARAM)hMenu,(LPARAM)&param);
	//DrawMenuBar((HWND)CallService("CLUI/GetHwnd",0,0));
	tick = GetTickCount()-tick;
	return (INT_PTR)hMenu;
}

static INT_PTR AddTrayMenuItem(WPARAM wParam, LPARAM lParam)
{
	TMO_MenuItem tmi;
	CLISTMENUITEM *mi = (CLISTMENUITEM*)lParam;
	if ( !pcli->pfnConvertMenu(mi, &tmi))
		return NULL;

	lpTrayMenuExecParam mmep = (lpTrayMenuExecParam)mir_alloc(sizeof(TrayMenuExecParam));
	if (mmep == NULL)
		return 0;

	mmep->szServiceName = mir_strdup(mi->pszService);
	mmep->Param1 = mi->popupPosition;
	tmi.ownerdata = mmep;

	OptParam op;
	op.Handle = (HANDLE)CallService(MO_ADDNEWMENUITEM,(WPARAM)hTrayMenuObject,(LPARAM)&tmi);
	op.Setting = OPT_MENUITEMSETUNIQNAME;
	op.Value = (INT_PTR)mi->pszService;
	CallService(MO_SETOPTIONSMENUITEM,0,(LPARAM)&op);
	return (INT_PTR)op.Handle;
}

INT_PTR TrayMenuCheckService(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

INT_PTR TrayMenuonAddService(WPARAM wParam, LPARAM lParam)
{
	MENUITEMINFO *mii = (MENUITEMINFO* )wParam;
	if (mii == NULL) return 0;

	if (hHideShowMainMenuItem == (HANDLE)lParam) {
		mii->fMask |= MIIM_STATE;
		mii->fState |= MFS_DEFAULT;
	}

	if (hTrayMainMenuItemProxy == (HANDLE)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		mii->hSubMenu = (HMENU)CallService(MS_CLIST_MENUGETMAIN,0,0);
	}

	if (hTrayStatusMenuItemProxy == (HANDLE)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		mii->hSubMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS,0,0);
	}

	return(TRUE);
}


//called with:
//wparam - ownerdata
//lparam - lparam from winproc
INT_PTR TrayMenuExecService(WPARAM wParam, LPARAM lParam)
{
	if (wParam != 0)
	{
		lpTrayMenuExecParam mmep = (lpTrayMenuExecParam)wParam;
		if ( !strcmp(mmep->szServiceName,"Help/AboutCommand"))
		{
			//bug in help.c,it used wparam as parent window handle without reason.
			mmep->Param1 = 0;
		}
		CallService(mmep->szServiceName,mmep->Param1,lParam);
	}
	return(1);
}

INT_PTR FreeOwnerDataTrayMenu (WPARAM wParam, LPARAM lParam)
{

	lpTrayMenuExecParam mmep;
	mmep = (lpTrayMenuExecParam)lParam;
	if (mmep != NULL){
		mir_free(mmep->szServiceName);
		mir_free(mmep);
	}

	return 0;
}

void InitTrayMenus(void)
{
	//Tray menu
	hTrayMenuObject = MO_CreateMenuObject("TrayMenu", LPGEN("Tray menu"), 0, "CLISTMENUSTRAY/ExecService");
	MO_SetMenuObjectParam(hTrayMenuObject, OPT_USERDEFINEDITEMS, TRUE);
	MO_SetMenuObjectParam(hTrayMenuObject, OPT_MENUOBJECT_SET_FREE_SERVICE, "CLISTMENUSTRAY/FreeOwnerDataTrayMenu");
	MO_SetMenuObjectParam(hTrayMenuObject, OPT_MENUOBJECT_SET_ONADD_SERVICE, "CLISTMENUSTRAY/TrayMenuonAddService");

	CreateServiceFunction("CLISTMENUSTRAY/ExecService",TrayMenuExecService);
	CreateServiceFunction("CLISTMENUSTRAY/FreeOwnerDataTrayMenu",FreeOwnerDataTrayMenu);
	CreateServiceFunction("CLISTMENUSTRAY/TrayMenuonAddService",TrayMenuonAddService);

	CreateServiceFunction("CList/AddTrayMenuItem",AddTrayMenuItem);
	CreateServiceFunction(MS_CLIST_REMOVETRAYMENUITEM,RemoveTrayMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDTRAY,BuildTrayMenu);
	hPreBuildTrayMenuEvent = CreateHookableEvent(ME_CLIST_PREBUILDTRAYMENU);

	//add  exit command to menu
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 900000;
	mi.pszService = "CloseAction";
	mi.pszName = LPGEN("E&xit");
	AddTrayMenuItem(0,(LPARAM)&mi);

	memset(&mi,0,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = 100000;
	mi.pszService = MS_CLIST_SHOWHIDE;
	mi.pszName = LPGEN("&Hide/Show");
	hHideShowMainMenuItem = (HANDLE)AddTrayMenuItem(0,(LPARAM)&mi);

	memset(&mi,0,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = 200000;
	mi.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FINDUSER));
	mi.pszService = "FindAdd/FindAddCommand";
	mi.pszName = LPGEN("&Find/Add contacts...");
	AddTrayMenuItem(0,(LPARAM)&mi);

	memset(&mi,0,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = 300000;
	mi.pszService = "";
	mi.pszName = LPGEN("&Main menu");
	hTrayMainMenuItemProxy = (HANDLE)AddTrayMenuItem(0,(LPARAM)&mi);

	memset(&mi,0,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = 300100;
	mi.pszService = "";
	mi.pszName = LPGEN("&Status");
	hTrayStatusMenuItemProxy = (HANDLE)AddTrayMenuItem(0,(LPARAM)&mi);

	memset(&mi,0,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = 400000;
	mi.hIcon = LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_OPTIONS));
	mi.pszService = "Options/OptionsCommand";
	mi.pszName = LPGEN("&Options...");
	AddTrayMenuItem(0,(LPARAM)&mi);

	memset(&mi,0,sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = 500000;
	mi.hIcon = LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_CLIENTMIRANDA));
	mi.pszService = "Help/AboutCommand";
	mi.pszName = LPGEN("&About");
	AddTrayMenuItem(0,(LPARAM)&mi);
}
