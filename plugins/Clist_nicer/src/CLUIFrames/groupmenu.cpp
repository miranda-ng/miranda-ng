/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include <commonheaders.h>

//////////////////////////////Group MENU/////////////////////////
HANDLE hGroupMenuObject;
HANDLE hPreBuildGroupMenuEvent;

HGENMENU hGroupMainMenuItemProxy;
HGENMENU hHideShowMainMenuItem;
HGENMENU hGroupStatusMenuItemProxy;
HGENMENU hAppearanceMenuItemProxy;
HGENMENU hEventAreaMenuItemProxy;

static HMENU hMenuOldContext;

HGENMENU hHideOfflineUsersMenuItem;
HGENMENU hHideOfflineUsersOutHereMenuItem;
HGENMENU hHideEmptyGroupsMenuItem;
HGENMENU hDisableGroupsMenuItem;
HGENMENU hNewGroupMenuItem;
HGENMENU hNewSubGroupMenuItem;

void InitSubGroupMenus(void);

//Groupmenu exec param(ownerdata)
typedef struct {
	char *szServiceName;
	int Param1,Param2;
} GroupMenuExecParam, *lpGroupMenuExecParam;

//wparam = handle to the menu item returned by MS_CLIST_ADDCONTACTMENUITEM
//return 0 on success.
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
		return NULL;

	lpGroupMenuExecParam mmep = (lpGroupMenuExecParam)mir_alloc(sizeof(GroupMenuExecParam));
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
}

INT_PTR GroupMenuCheckService(WPARAM wParam, LPARAM lParam)
{
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
		//mi.fType = MFT_STRING;
		mii->hSubMenu = (HMENU)CallService(MS_CLIST_MENUGETMAIN, 0, 0);
	}

	if (hGroupStatusMenuItemProxy == (HANDLE)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		//mi.fType = MFT_STRING;
		mii->hSubMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
	}
	if (hAppearanceMenuItemProxy == (HANDLE)lParam) {
		hMenuOldContext = GetSubMenu(LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT)), 3);
		TranslateMenu(hMenuOldContext);

		CheckMenuItem(hMenuOldContext, POPUP_FRAME, MF_BYCOMMAND | (cfg::dat.dwFlags & CLUI_FRAME_CLISTSUNKEN ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(hMenuOldContext, POPUP_BUTTONS, MF_BYCOMMAND | (cfg::dat.dwFlags & CLUI_FRAME_SHOWBOTTOMBUTTONS ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(hMenuOldContext, POPUP_SHOWMETAICONS, MF_BYCOMMAND | (cfg::dat.dwFlags & CLUI_USEMETAICONS ? MF_CHECKED : MF_UNCHECKED));
		CheckMenuItem(hMenuOldContext, POPUP_SHOWSTATUSICONS, MF_BYCOMMAND | (cfg::dat.dwFlags & CLUI_FRAME_STATUSICONS ? MF_CHECKED : MF_UNCHECKED));

		mii->fMask |= MIIM_SUBMENU;
		mii->hSubMenu = (HMENU)hMenuOldContext;
	}
	return (TRUE);
};

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
	int newVal = !(GetWindowLongPtr((HWND)CallService(MS_CLUI_GETHWNDTREE, 0, 0), GWL_STYLE) & CLS_HIDEEMPTYGROUPS);
	cfg::writeByte("CList", "HideEmptyGroups", (BYTE)newVal);
	SendMessage((HWND)CallService(MS_CLUI_GETHWNDTREE, 0, 0), CLM_SETHIDEEMPTYGROUPS, newVal, 0);
	return 0;
}

INT_PTR UseGroupsHelper(WPARAM wParam, LPARAM lParam)
{
	int newVal = !(GetWindowLongPtr((HWND)CallService(MS_CLUI_GETHWNDTREE, 0, 0), GWL_STYLE) & CLS_USEGROUPS);
	cfg::writeByte("CList", "UseGroups", (BYTE)newVal);
	SendMessage((HWND)CallService(MS_CLUI_GETHWNDTREE, 0, 0), CLM_SETUSEGROUPS, newVal,0);
	return 0;
}

INT_PTR HideOfflineRootHelper(WPARAM wParam, LPARAM lParam)
{
	SendMessage((HWND)CallService(MS_CLUI_GETHWNDTREE, 0, 0), CLM_SETHIDEOFFLINEROOT,
		!SendMessage((HWND)CallService(MS_CLUI_GETHWNDTREE, 0, 0), CLM_GETHIDEOFFLINEROOT, 0, 0),
		0);
	return 0;
}

static int OnBuildGroupMenu(WPARAM wParam, LPARAM lParam)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_FLAGS | (cfg::getByte("CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT) ? CMIF_CHECKED : 0);
	Menu_ModifyItem(hHideOfflineUsersMenuItem, &mi);

	mi.flags = CMIM_FLAGS | (SendMessage(pcli->hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0) ? CMIF_CHECKED : 0);
	Menu_ModifyItem(hHideOfflineUsersOutHereMenuItem, &mi);

	mi.flags = CMIM_FLAGS | (GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS ? CMIF_CHECKED : 0);
	Menu_ModifyItem(hHideEmptyGroupsMenuItem, &mi);

	mi.flags = CMIM_FLAGS | (GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_USEGROUPS ? 0 : CMIF_CHECKED);
	Menu_ModifyItem(hDisableGroupsMenuItem, &mi);

	mi.flags = CMIM_FLAGS;
	Menu_ModifyItem(hGroupMainMenuItemProxy, &mi);

	mi.flags = CMIM_FLAGS;
	Menu_ModifyItem(hAppearanceMenuItemProxy, &mi);
	return 0;
}

IconItemT iconItem[] = {
	{ LPGENT("New group"),    "new_group", IDI_ADDGROUP },
	{ LPGENT("Contact list"), "clist",     IDI_CLIST }
};

void InitIconLibMenuIcons(void)
{
	Icon_RegisterT(g_hInst, LPGENT("Contact list"), iconItem, SIZEOF(iconItem));
}

void InitGroupMenus(void)
{
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

	//Group menu
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
		mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_EXIT);
		mi.pszName = LPGEN("E&xit");
		AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 500;
		mi.pszService = MS_CLIST_SHOWHIDE;
		mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_SHOWHIDE);
		mi.pszName = LPGEN("&Hide/show");
		hHideShowMainMenuItem = (HGENMENU)AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 200000;
		mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_FINDUSER);
		mi.pszService = "FindAdd/FindAddCommand";
		mi.pszName = LPGEN("&Find/add contacts...");
		AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 300000;
		mi.pszService = "";
		mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_MAINMENU);
		mi.pszName = LPGEN("&Main menu");
		hGroupMainMenuItemProxy = (HGENMENU)AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 300100;
		mi.pszService = "";
		mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_STATUS);
		mi.pszName = LPGEN("&Status");
		hGroupStatusMenuItemProxy = (HGENMENU)AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 390100;
		mi.pszService = "";
		mi.icolibItem = iconItem[1].hIcolib;
		mi.pszName = LPGEN("Appearance");
		hAppearanceMenuItemProxy = (HGENMENU)AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 400000;
		mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_OPTIONS);
		mi.pszService = "Options/OptionsCommand";
		mi.pszName = LPGEN("&Options...");
		AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 500000;
		mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_MIRANDA);
		mi.pszService = "CLN/About";
		mi.pszName = LPGEN("&About the contact list...");
		AddGroupMenuItem(0, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 100000;
		mi.icolibItem = iconItem[0].hIcolib;
		mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
		mi.pszName = LPGEN("&New group");
		gmp.lParam = 0;
		gmp.wParam = POPUP_NEWGROUP;
		hNewGroupMenuItem = (HGENMENU)AddGroupMenuItem((WPARAM)&gmp, (LPARAM)&mi);

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
	}
}

HANDLE hSubGroupMenuObject;

HANDLE hSubGroupMainMenuItemProxy;
//HANDLE hHideShowMainMenuItem;
HANDLE hSubGroupStatusMenuItemProxy;
HANDLE hPreBuildSubGroupMenuEvent;
HGENMENU hHideOfflineUsersHereMenuItem;

//SubGroupmenu exec param(ownerdata)
typedef struct {
	char *szServiceName;
	int Param1, Param2;
} SubGroupMenuExecParam, *lpSubGroupMenuExecParam;

//wparam = handle to the menu item returned by MS_CLIST_ADDCONTACTMENUITEM
//return 0 on success.
static INT_PTR RemoveSubGroupMenuItem(WPARAM wParam, LPARAM lParam)
{
	CallService(MO_REMOVEMENUITEM, wParam, 0);
	return 0;
}

static int OnBuildSubGroupMenu(WPARAM wParam, LPARAM lParam)
{
	ClcGroup *group = (ClcGroup *)wParam;
	if (group == 0)
		return 0;

	//contact->group
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_FLAGS | (group->hideOffline ? CMIF_CHECKED : 0);
	Menu_ModifyItem(hHideOfflineUsersHereMenuItem, &mi);
	return 0;
}

static INT_PTR BuildSubGroupMenu(WPARAM wParam, LPARAM lParam)
{
	ListParam param = {0};
	param.MenuObjectHandle = hSubGroupMenuObject;
	param.wParam = wParam;

	HMENU hMenu = CreatePopupMenu();
	int tick = GetTickCount();
	NotifyEventHooks(hPreBuildSubGroupMenuEvent, wParam, 0);

	CallService(MO_BUILDMENU, (WPARAM)hMenu, (LPARAM)&param);
	tick = GetTickCount() - tick;
	return (INT_PTR)hMenu;
}

static INT_PTR AddSubGroupMenuItem(WPARAM wParam, LPARAM lParam)
{
	TMO_MenuItem tmi;
	CLISTMENUITEM *mi = (CLISTMENUITEM *)lParam;
	if (!pcli->pfnConvertMenu(mi, &tmi))
		return NULL;

	lpSubGroupMenuExecParam mmep = (lpSubGroupMenuExecParam)mir_alloc(sizeof(SubGroupMenuExecParam));
	if (mmep == NULL)
		return 0;

	//we need just one parametr.
	lpGroupMenuParam gmp = (lpGroupMenuParam)wParam;
	mmep->szServiceName = mir_strdup(mi->pszService);
	mmep->Param1 = mi->popupPosition;
	if (gmp != NULL) {
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

INT_PTR SubGroupMenuCheckService(WPARAM wParam, LPARAM lParam) {
	//not used
	return 0;
};

INT_PTR SubGroupMenuonAddService(WPARAM wParam, LPARAM lParam) {

	MENUITEMINFO *mii = (MENUITEMINFO *)wParam;

	if (mii == NULL) return 0;

	/*
	if (hHideShowMainMenuItem == (HANDLE)lParam) {
		mii->fMask |= MIIM_STATE;
		mii->fState |= MFS_DEFAULT;
	}
	if (hSubGroupMainMenuItemProxy == (HANDLE)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		//mi.fType = MFT_STRING;
		mii->hSubMenu = (HMENU)CallService(MS_CLIST_MENUGETMAIN, 0, 0);
	}

	if (hSubGroupStatusMenuItemProxy == (HANDLE)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		//mi.fType = MFT_STRING;
		mii->hSubMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
	}
	*/
	return TRUE;
};

//called with:
//wparam - ownerdata
//lparam - lparam from winproc
INT_PTR SubGroupMenuExecService(WPARAM wParam, LPARAM lParam) {
	if (wParam!=0) {
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
	SendMessage((HWND)CallService(MS_CLUI_GETHWNDTREE, 0, 0), WM_COMMAND, wParam, 0);
	return 0;
}

void InitSubGroupMenus(void)
{
	CreateServiceFunction("CLISTMENUSSubGroup/ExecService", SubGroupMenuExecService);
	CreateServiceFunction("CLISTMENUSSubGroup/FreeOwnerDataSubGroupMenu", FreeOwnerDataSubGroupMenu);
	CreateServiceFunction("CLISTMENUSSubGroup/SubGroupMenuonAddService", SubGroupMenuonAddService);
	CreateServiceFunction("CLISTMENUSSubGroup/GroupMenuExecProxy", GroupMenuExecProxy);

	//CreateServiceFunction("CLISTMENUSSubGroup/HideSubGroupsHelper", HideSubGroupsHelper);
	//CreateServiceFunction("CLISTMENUSSubGroup/UseSubGroupsHelper", UseSubGroupsHelper);
	//CreateServiceFunction("CLISTMENUSSubGroup/HideOfflineRootHelper", HideOfflineRootHelper);

	CreateServiceFunction("CList/AddSubGroupMenuItem", AddSubGroupMenuItem);
	CreateServiceFunction(MS_CLIST_REMOVESUBGROUPMENUITEM, RemoveSubGroupMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDSUBGROUP, BuildSubGroupMenu);
	hPreBuildSubGroupMenuEvent = CreateHookableEvent(ME_CLIST_PREBUILDSUBGROUPMENU);
	HookEvent(ME_CLIST_PREBUILDSUBGROUPMENU, OnBuildSubGroupMenu);

	// SubGroup menu
	hSubGroupMenuObject = MO_CreateMenuObject("SubGroupMenu", LPGEN("Subgroup menu"), 0, "CLISTMENUSSubGroup/ExecService");
	MO_SetMenuObjectParam(hSubGroupMenuObject, OPT_USERDEFINEDITEMS, TRUE);
	MO_SetMenuObjectParam(hSubGroupMenuObject, OPT_MENUOBJECT_SET_FREE_SERVICE, "CLISTMENUSSubGroup/FreeOwnerDataSubGroupMenu");
	MO_SetMenuObjectParam(hSubGroupMenuObject, OPT_MENUOBJECT_SET_ONADD_SERVICE, "CLISTMENUSSubGroup/SubGroupMenuonAddService");

	{
		//add  exit command to menu
		GroupMenuParam gmp;

		CLISTMENUITEM mi = { sizeof(mi) };
		mi.position = 1000;
		mi.icolibItem = iconItem[0].hIcolib;
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
		mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_RENAME);
		mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
		mi.pszName = LPGEN("&Rename group");
		gmp.lParam = 0;
		gmp.wParam = POPUP_RENAMEGROUP;
		AddSubGroupMenuItem((WPARAM)&gmp, (LPARAM)&mi);

		memset(&mi, 0, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.position = 900002;
		mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_DELETE);
		mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
		mi.pszName = LPGEN("&Delete group");
		gmp.lParam = 0;
		gmp.wParam = POPUP_DELETEGROUP;
		AddSubGroupMenuItem((WPARAM)&gmp, (LPARAM)&mi);
	}
}

//////////////////////////////END SubGroup MENU/////////////////////////
