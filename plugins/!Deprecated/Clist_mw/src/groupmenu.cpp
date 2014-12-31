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
#include "clist.h"
#include "clc.h"

//////////////////////////////Group MENU/////////////////////////

extern HANDLE hHideShowMainMenuItem;
HANDLE hGroupMenuObject;

HANDLE hGroupMainMenuItemProxy;
HANDLE hGroupStatusMenuItemProxy;
HANDLE hPreBuildGroupMenuEvent;

HGENMENU hHideOfflineUsersMenuItem, hHideOfflineUsersOutHereMenuItem, hHideEmptyGroupsMenuItem;
HGENMENU hDisableGroupsMenuItem, hNewGroupMenuItem, hNewSubGroupMenuItem;

int NewGroupIconidx;

extern HIMAGELIST hCListImages;
HICON LoadIconFromExternalFile(char *filename, int i, boolean UseLibrary, boolean registerit, char *IconName, char *SectName, char *Description, int internalidx, HICON DefIcon);

void InitSubGroupMenus(void);

//Groupmenu exec param(ownerdata)
typedef struct{
	char *szServiceName;
	int Param1, Param2;
} GroupMenuExecParam, *lpGroupMenuExecParam;

/*
wparam = handle to the menu item returned by MS_CLIST_ADDCONTACTMENUITEM
return 0 on success.
*/
static INT_PTR RemoveGroupMenuItem(WPARAM wParam, LPARAM lParam)
{
	CallService(MO_REMOVEMENUITEM, wParam, 0);
	return 0;
}


static INT_PTR BuildGroupMenu(WPARAM wParam, LPARAM lParam)
{
	int tick;
	HMENU hMenu;
	ListParam param = {0};
	param.MenuObjectHandle = hGroupMenuObject;

	//hMenu = hMainMenu;
	hMenu = CreatePopupMenu();
	//hMenu = wParam;
	tick = GetTickCount();

	NotifyEventHooks(hPreBuildGroupMenuEvent, 0, 0);

	CallService(MO_BUILDMENU, (WPARAM)hMenu, (LPARAM)&param);
	//DrawMenuBar((HWND)CallService("CLUI/GetHwnd",0,0));
	tick = GetTickCount() - tick;
	return (INT_PTR)hMenu;
}

static INT_PTR AddGroupMenuItem(WPARAM wParam, LPARAM lParam)
{
	TMO_MenuItem tmi;
	CLISTMENUITEM *mi = (CLISTMENUITEM *)lParam;
	if (!pcli->pfnConvertMenu(mi, &tmi))
		return 0;

	lpGroupMenuExecParam mmep = (lpGroupMenuExecParam)mir_calloc(sizeof(GroupMenuExecParam));
	if (mmep == NULL)
		return 0;

	//we need just one parametr.
	mmep->szServiceName = mir_strdup(mi->pszService);
	mmep->Param1 = mi->popupPosition;
	lpGroupMenuParam gmp = (lpGroupMenuParam)wParam;
	if (gmp != NULL) {
		mmep->Param1 = gmp->wParam;
		mmep->Param2 = gmp->lParam;
	}
	tmi.ownerdata = mmep;

	char buf[1024];
	mir_snprintf(buf, SIZEOF(buf), "%s/%s", mi->pszService, mi->pszName);

	OptParam op;
	op.Handle = (HANDLE)CallService(MO_ADDNEWMENUITEM, (WPARAM)hGroupMenuObject, (LPARAM)&tmi);
	op.Setting = OPT_MENUITEMSETUNIQNAME;
	op.Value = (INT_PTR)buf;
	CallService(MO_SETOPTIONSMENUITEM, 0, (LPARAM)&op);
	return (INT_PTR)op.Handle;

//	mainItemCount++;
//	return MENU_CUSTOMITEMMAIN|(mainMenuItem[mainItemCount-1].id);
}

INT_PTR GroupMenuCheckService(WPARAM wParam, LPARAM lParam)
{
//not used
	return 0;
}

INT_PTR GroupMenuonAddService(WPARAM wParam, LPARAM lParam)
{
	MENUITEMINFO *mii = (MENUITEMINFO *)wParam;
	if (mii == NULL) return 0;

	if (hHideShowMainMenuItem == (HANDLE)lParam) {
		mii->fMask |= MIIM_STATE;
		mii->fState |= MFS_DEFAULT;
	}

	if (hGroupMainMenuItemProxy == (HANDLE)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		mii->hSubMenu = (HMENU)CallService(MS_CLIST_MENUGETMAIN, 0, 0);
	}

	if (hGroupStatusMenuItemProxy == (HANDLE)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		mii->hSubMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
	}

	return TRUE;
}

//called with:
//wparam - ownerdata
//lparam - lparam from winproc
INT_PTR GroupMenuExecService(WPARAM wParam, LPARAM lParam)
{
	if (wParam != 0) {
		lpGroupMenuExecParam mmep = (lpGroupMenuExecParam)wParam;
		if (!strcmp(mmep->szServiceName, "Help/AboutCommand")) {
			//bug in help.c,it used wparam as parent window handle without reason.
			mmep->Param1 = 0;
			CallService(mmep->szServiceName, mmep->Param1, lParam);
		}
		else
			CallService(mmep->szServiceName, mmep->Param1, mmep->Param2);

	}
	return 1;
}

INT_PTR FreeOwnerDataGroupMenu(WPARAM wParam, LPARAM lParam)
{
	lpGroupMenuExecParam mmep = (lpGroupMenuExecParam)lParam;
	if (mmep != NULL) {
		mir_free(mmep->szServiceName);
		mir_free(mmep);
	}
	return 0;
}

INT_PTR HideGroupsHelper(WPARAM wParam, LPARAM lParam)
{
	int newVal = !(GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS);
	db_set_b(NULL, "CList", "HideEmptyGroups", (BYTE)newVal);
	SendMessage(pcli->hwndContactTree, CLM_SETHIDEEMPTYGROUPS, newVal, 0);
	return 0;
}

INT_PTR UseGroupsHelper(WPARAM wParam, LPARAM lParam)
{
	int newVal = !(GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_USEGROUPS);
	db_set_b(NULL, "CList", "UseGroups", (BYTE)newVal);
	SendMessage(pcli->hwndContactTree, CLM_SETUSEGROUPS, newVal, 0);
	return 0;
}

INT_PTR HideOfflineRootHelper(WPARAM wParam, LPARAM lParam)
{
	SendMessage(pcli->hwndContactTree, CLM_SETHIDEOFFLINEROOT, !SendMessage(pcli->hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0), 0);
	return 0;
}

static int OnBuildGroupMenu(WPARAM wParam, LPARAM lParam)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_FLAGS | (db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT) ? CMIF_CHECKED : 0);
	Menu_ModifyItem(hHideOfflineUsersMenuItem, &mi);

	mi.flags = CMIM_FLAGS | (SendMessage(pcli->hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0) ? CMIF_CHECKED : 0);
	Menu_ModifyItem(hHideOfflineUsersOutHereMenuItem, &mi);

	mi.flags = CMIM_FLAGS | (GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS ? CMIF_CHECKED : 0);
	Menu_ModifyItem(hHideEmptyGroupsMenuItem, &mi);

	mi.flags = CMIM_FLAGS | (GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_USEGROUPS ? 0 : CMIF_CHECKED);
	Menu_ModifyItem(hDisableGroupsMenuItem, &mi);
	return 0;
}

int static OnIconLibIconChanged(WPARAM wParam, LPARAM lParam)
{
	HICON hicon = LoadIconFromExternalFile("clisticons.dll", 2, TRUE, FALSE, "NewGroup", "Contact list", "New group", -IDI_NEWGROUP, 0);
	NewGroupIconidx = ImageList_ReplaceIcon(hCListImages, NewGroupIconidx, hicon);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_ICON;
	mi.hIcon = ImageList_GetIcon(hCListImages, NewGroupIconidx, 0);
	Menu_ModifyItem(hNewSubGroupMenuItem, &mi);
	Menu_ModifyItem(hNewGroupMenuItem, &mi);
	return 0;
}

void InitGroupMenus(void)
{
	HICON hicon = LoadIconFromExternalFile("clisticons.dll", 2, TRUE, TRUE, "NewGroup", "Contact list", "New group", -IDI_NEWGROUP2, 0);
	NewGroupIconidx = ImageList_AddIcon(hCListImages, hicon);

	CreateServiceFunction("CLISTMENUSGroup/ExecService", GroupMenuExecService);
	CreateServiceFunction("CLISTMENUSGroup/FreeOwnerDataGroupMenu", FreeOwnerDataGroupMenu);
	CreateServiceFunction("CLISTMENUSGroup/GroupMenuonAddService", GroupMenuonAddService);
	CreateServiceFunction("CLISTMENUSGroup/HideGroupsHelper", HideGroupsHelper);
	CreateServiceFunction("CLISTMENUSGroup/UseGroupsHelper", UseGroupsHelper);
	CreateServiceFunction("CLISTMENUSGroup/HideOfflineRootHelper", HideOfflineRootHelper);

	CreateServiceFunction("CList/AddGroupMenuItem", AddGroupMenuItem);
	CreateServiceFunction(MS_CLIST_REMOVEGROUPMENUITEM, RemoveGroupMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDGROUP, BuildGroupMenu);
	hPreBuildGroupMenuEvent = CreateHookableEvent(ME_CLIST_PREBUILDGROUPMENU);
	HookEvent(ME_CLIST_PREBUILDGROUPMENU, OnBuildGroupMenu);

	InitSubGroupMenus();

	// Group menu
	hGroupMenuObject = MO_CreateMenuObject("GroupMenu", LPGEN("Group menu"), 0, "CLISTMENUSGroup/ExecService");
	MO_SetMenuObjectParam(hGroupMenuObject, OPT_USERDEFINEDITEMS, TRUE);
	MO_SetMenuObjectParam(hGroupMenuObject, OPT_MENUOBJECT_SET_FREE_SERVICE, "CLISTMENUSGroup/FreeOwnerDataGroupMenu");
	MO_SetMenuObjectParam(hGroupMenuObject, OPT_MENUOBJECT_SET_ONADD_SERVICE, "CLISTMENUSGroup/GroupMenuonAddService");
	{
		//add exit command to menu
		GroupMenuParam gmp;

		CLISTMENUITEM mi = { sizeof(mi) };
		mi.position = 1900000;
		mi.pszService = "CloseAction";
		mi.pszName = LPGEN("E&xit");
		AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 500;
		mi.pszService = MS_CLIST_SHOWHIDE;
		mi.pszName = LPGEN("&Hide/Show");
		hHideShowMainMenuItem = (HANDLE)AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 200000;
		mi.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FINDUSER));
		mi.pszService = "FindAdd/FindAddCommand";
		mi.pszName = LPGEN("&Find/Add contacts...");
		AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 300000;
		mi.pszService = "";
		mi.pszName = LPGEN("&Main menu");
		hGroupMainMenuItemProxy = (HANDLE)AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 300100;
		mi.pszService = "";
		mi.pszName = LPGEN("&Status");
		hGroupStatusMenuItemProxy = (HANDLE)AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 400000;
		mi.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_OPTIONS));
		mi.pszService = "Options/OptionsCommand";
		mi.pszName = LPGEN("&Options...");

		AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 500000;
		mi.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CLIENTMIRANDA));
		mi.pszService = "Help/AboutCommand";
		mi.pszName = LPGEN("&About");

		//AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 100000;
		mi.hIcon = ImageList_GetIcon(hCListImages, NewGroupIconidx, 0);
		mi.pszService = MS_CLIST_GROUPCREATE;
		mi.pszName = LPGEN("&New group");
		hNewGroupMenuItem = (HGENMENU)AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 100001;
		mi.hIcon = NULL;
		mi.pszService = MS_CLIST_SETHIDEOFFLINE;
		mi.pszName = LPGEN("&Hide offline users");
		gmp.lParam = 0;
		gmp.wParam = -1;
		hHideOfflineUsersMenuItem = (HGENMENU)AddGroupMenuItem((WPARAM)&gmp, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 100002;
		mi.hIcon = NULL;
		mi.pszService = "CLISTMENUSGroup/HideOfflineRootHelper";
		mi.pszName = LPGEN("Hide &offline users out here");
		hHideOfflineUsersOutHereMenuItem = (HGENMENU)AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 100003;
		mi.hIcon = NULL;
		mi.pszService = "CLISTMENUSGroup/HideGroupsHelper";
		mi.pszName = LPGEN("Hide &empty groups");
		hHideEmptyGroupsMenuItem = (HGENMENU)AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 100004;
		mi.hIcon = NULL;
		mi.pszService = "CLISTMENUSGroup/UseGroupsHelper";
		mi.pszName = LPGEN("Disable &groups");
		hDisableGroupsMenuItem = (HGENMENU)AddGroupMenuItem(0, (LPARAM)&mi);

		HookEvent(ME_SKIN2_ICONSCHANGED, OnIconLibIconChanged);
	}
}

//////////////////////////////END Group MENU/////////////////////////




//////////////////////////////SubGroup MENU/////////////////////////
HANDLE hSubGroupMenuObject;

HANDLE hSubGroupMainMenuItemProxy;
HANDLE hSubGroupStatusMenuItemProxy;
HANDLE hPreBuildSubGroupMenuEvent;

HGENMENU hHideOfflineUsersHereMenuItem;

//SubGroupmenu exec param(ownerdata)
typedef struct{
	char *szServiceName;
	int Param1, Param2;
} SubGroupMenuExecParam, *lpSubGroupMenuExecParam;

/*
wparam = handle to the menu item returned by MS_CLIST_ADDCONTACTMENUITEM
return 0 on success.
*/
static INT_PTR RemoveSubGroupMenuItem(WPARAM wParam, LPARAM lParam)
{
	CallService(MO_REMOVEMENUITEM, wParam, 0);
	return 0;
}

static int OnBuildSubGroupMenu(WPARAM wParam, LPARAM lParam)
{
	ClcGroup *group = (ClcGroup *)wParam;
	if (wParam == 0) return 0;

	//contact->group
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_FLAGS | (group->hideOffline ? CMIF_CHECKED : 0);
	Menu_ModifyItem(hHideOfflineUsersHereMenuItem, &mi);
	return 0;
}

static INT_PTR BuildSubGroupMenu(WPARAM wParam, LPARAM lParam)
{
	int tick;
	HMENU hMenu;
	ListParam param = { 0 };
	param.MenuObjectHandle = hSubGroupMenuObject;
	param.wParam = wParam;

	//hMenu = hMainMenu;
	hMenu = CreatePopupMenu();
	//hMenu = wParam;
	tick = GetTickCount();

	NotifyEventHooks(hPreBuildSubGroupMenuEvent, wParam, 0);

	CallService(MO_BUILDMENU, (WPARAM)hMenu, (LPARAM)&param);
	//DrawMenuBar((HWND)CallService("CLUI/GetHwnd",0,0));
	tick = GetTickCount() - tick;
	return (INT_PTR)hMenu;
}

static INT_PTR AddSubGroupMenuItem(WPARAM wParam, LPARAM lParam)
{
	TMO_MenuItem tmi;
	CLISTMENUITEM *mi = (CLISTMENUITEM *)lParam;
	if (!pcli->pfnConvertMenu(mi, &tmi))
		return 0;

	lpSubGroupMenuExecParam mmep = (lpSubGroupMenuExecParam)mir_calloc(sizeof(SubGroupMenuExecParam));
	if (mmep == NULL)
		return 0;

	//we need just one parametr.
	mmep->szServiceName = mir_strdup(mi->pszService);
	mmep->Param1 = mi->popupPosition;
	lpGroupMenuParam gmp = (lpGroupMenuParam)wParam;
	if (gmp != NULL ) {
		mmep->Param1 = gmp->wParam;
		mmep->Param2 = gmp->lParam;
	}
	tmi.ownerdata = mmep;

	char buf[1024];
	mir_snprintf(buf, SIZEOF(buf), "%s/%s", mi->pszService, mi->pszName);

	OptParam op;
	op.Handle = (HANDLE)CallService(MO_ADDNEWMENUITEM, (WPARAM)hSubGroupMenuObject, (LPARAM)&tmi);
	op.Setting = OPT_MENUITEMSETUNIQNAME;
	op.Value = (INT_PTR)buf;
	CallService(MO_SETOPTIONSMENUITEM, 0, (LPARAM)&op);
	return (INT_PTR)op.Handle;
}

INT_PTR SubGroupMenuCheckService(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

INT_PTR SubGroupMenuonAddService(WPARAM wParam, LPARAM lParam)
{
	MENUITEMINFO *mii = (MENUITEMINFO *)wParam;
	if (mii == NULL)
		return 0;

	return TRUE;
}

//called with:
//wparam - ownerdata
//lparam - lparam from winproc
INT_PTR SubGroupMenuExecService(WPARAM wParam, LPARAM lParam)
{
	if (wParam != 0) {
		lpSubGroupMenuExecParam mmep = (lpSubGroupMenuExecParam)wParam;
		if (!strcmp(mmep->szServiceName, "Help/AboutCommand")) {
			//bug in help.c,it used wparam as parent window handle without reason.
			mmep->Param1 = 0;
			CallService(mmep->szServiceName, mmep->Param1, lParam);
		}
		else
			CallService(mmep->szServiceName, mmep->Param1, mmep->Param2);
	}

	return 1;
}

INT_PTR FreeOwnerDataSubGroupMenu(WPARAM wParam, LPARAM lParam)
{
	lpSubGroupMenuExecParam mmep = (lpSubGroupMenuExecParam)lParam;
	if (mmep != NULL) {
		mir_free(mmep->szServiceName);
		mir_free(mmep);
	}
	return 0;
}

//wparam menu handle to pass to clc.c
INT_PTR GroupMenuExecProxy(WPARAM wParam, LPARAM lParam)
{
	SendMessage(pcli->hwndContactTree, WM_COMMAND, wParam, 0);
	return 0;
}

void InitSubGroupMenus(void)
{
	CreateServiceFunction("CLISTMENUSSubGroup/ExecService", SubGroupMenuExecService);
	CreateServiceFunction("CLISTMENUSSubGroup/FreeOwnerDataSubGroupMenu", FreeOwnerDataSubGroupMenu);
	CreateServiceFunction("CLISTMENUSSubGroup/SubGroupMenuonAddService", SubGroupMenuonAddService);
	CreateServiceFunction("CLISTMENUSSubGroup/GroupMenuExecProxy", GroupMenuExecProxy);

	CreateServiceFunction("CList/AddSubGroupMenuItem", AddSubGroupMenuItem);
	CreateServiceFunction(MS_CLIST_REMOVESUBGROUPMENUITEM, RemoveSubGroupMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDSUBGROUP, BuildSubGroupMenu);
	hPreBuildSubGroupMenuEvent = CreateHookableEvent(ME_CLIST_PREBUILDSUBGROUPMENU);
	HookEvent(ME_CLIST_PREBUILDSUBGROUPMENU, OnBuildSubGroupMenu);

	//SubGroup menu
	hSubGroupMenuObject = MO_CreateMenuObject("SubGroupMenu", LPGEN("Subgroup menu"), 0, "CLISTMENUSSubGroup/ExecService");
	MO_SetMenuObjectParam(hSubGroupMenuObject, OPT_USERDEFINEDITEMS, TRUE);
	MO_SetMenuObjectParam(hSubGroupMenuObject, OPT_MENUOBJECT_SET_FREE_SERVICE, "CLISTMENUSSubGroup/FreeOwnerDataSubGroupMenu");
	MO_SetMenuObjectParam(hSubGroupMenuObject, OPT_MENUOBJECT_SET_ONADD_SERVICE, "CLISTMENUSSubGroup/SubGroupMenuonAddService");

	//add  exit command to menu
	GroupMenuParam gmp;

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 1000;
	mi.hIcon = ImageList_GetIcon(hCListImages, NewGroupIconidx, 0);
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.pszName = LPGEN("&New subgroup");
	gmp.lParam = 0;
	gmp.wParam = POPUP_NEWSUBGROUP;
	hNewSubGroupMenuItem = (HGENMENU)AddSubGroupMenuItem((WPARAM)&gmp, (LPARAM)&mi);

	memset(&mi, 0, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = 1001;
	mi.hIcon = NULL;
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.pszName = LPGEN("&Hide offline users in here");
	gmp.lParam = 0;
	gmp.wParam = POPUP_GROUPHIDEOFFLINE;
	hHideOfflineUsersHereMenuItem = (HGENMENU)AddSubGroupMenuItem((WPARAM)&gmp, (LPARAM)&mi);

	memset(&mi, 0, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = 900001;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_RENAME); //LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_RENAME));
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.pszName = LPGEN("&Rename group");
	gmp.lParam = 0;
	gmp.wParam = POPUP_RENAMEGROUP;
	AddSubGroupMenuItem((WPARAM)&gmp, (LPARAM)&mi);

	memset(&mi, 0, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = 900002;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_DELETE); //LoadIcon(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_DELETE));
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.pszName = LPGEN("&Delete group");
	gmp.lParam = 0;
	gmp.wParam = POPUP_DELETEGROUP;
	AddSubGroupMenuItem((WPARAM)&gmp, (LPARAM)&mi);
}

//////////////////////////////END SubGroup MENU/////////////////////////
