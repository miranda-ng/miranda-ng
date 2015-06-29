/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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
#include "../hdr/modern_commonheaders.h"
#include "m_clui.h"
#include "../hdr/modern_clist.h"
#include "../hdr/modern_clc.h"

//////////////////////////////Group MENU/////////////////////////
int hGroupMenuObject;

HGENMENU hGroupMainMenuItemProxy;
HGENMENU hHideShowMainMenuItem;
HGENMENU hGroupStatusMenuItemProxy;

HGENMENU hHideOfflineUsersMenuItem;
HGENMENU hHideOfflineUsersOutHereMenuItem;
HGENMENU hHideEmptyGroupsMenuItem;

HGENMENU hDisableGroupsMenuItem;
HGENMENU hNewGroupMenuItem;
HGENMENU hNewSubGroupMenuItem;

void InitSubGroupMenus(void);

//Groupmenu exec param(ownerdata)
typedef struct
{
	char *szServiceName;
	int Param1, Param2;
}
GroupMenuExecParam, *lpGroupMenuExecParam;

INT_PTR BuildGroupMenu(WPARAM, LPARAM)
{
	NotifyEventHooks(g_CluiData.hEventPreBuildGroupMenu, 0, 0);

	HMENU hMenu = CreatePopupMenu();
	Menu_Build(hMenu, hGroupMenuObject);
	return (INT_PTR)hMenu;
}

static INT_PTR AddGroupMenuItem(WPARAM wParam, LPARAM lParam)
{
	lpGroupMenuExecParam mmep = (lpGroupMenuExecParam)mir_calloc(sizeof(GroupMenuExecParam));
	if (mmep == NULL)
		return 0;

	TMO_MenuItem *pmi = (TMO_MenuItem*)lParam;

	// we need just one parametr.
	mmep->szServiceName = mir_strdup(pmi->pszService);
	GroupMenuParam *gmp = (GroupMenuParam*)wParam;
	if (gmp != NULL) {
		mmep->Param1 = gmp->wParam;
		mmep->Param2 = gmp->lParam;
	}
	pmi->ownerdata = mmep;

	HGENMENU hNewItem = Menu_AddItem(hGroupMenuObject, pmi);

	char buf[1024];
	mir_snprintf(buf, "%s/%s", pmi->pszService, pmi->name.a);
	Menu_ConfigureItem(hNewItem, MCI_OPT_UNIQUENAME, buf);
	return (INT_PTR)hNewItem;
}

INT_PTR GroupMenuonAddService(WPARAM wParam, LPARAM lParam)
{
	MENUITEMINFO *mii = (MENUITEMINFO*)wParam;
	if (mii == NULL)
		return 0;

	if (hHideShowMainMenuItem == (HANDLE)lParam) {
		mii->fMask |= MIIM_STATE;
		mii->fState |= MFS_DEFAULT;

	}
	if (hGroupMainMenuItemProxy == (HANDLE)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		mii->hSubMenu = Menu_GetMainMenu();
	}

	if (hGroupStatusMenuItemProxy == (HANDLE)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		mii->hSubMenu = (HMENU)Menu_GetStatusMenu();
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
		if (!mir_strcmp(mmep->szServiceName, "Help/AboutCommand")) {
			//bug in help.c,it used wparam as parent window handle without reason.
			mmep->Param1 = 0;
			CallService(mmep->szServiceName, mmep->Param1, lParam);
		}
		else CallService(mmep->szServiceName, mmep->Param1, mmep->Param2);
	}
	return 1;
}

INT_PTR FreeOwnerDataGroupMenu(WPARAM, LPARAM lParam)
{
	lpGroupMenuExecParam mmep = (lpGroupMenuExecParam)lParam;
	if (mmep != NULL) {
		mir_free(mmep->szServiceName);
		mir_free(mmep);
	}

	return 0;
}

INT_PTR HideGroupsHelper(WPARAM, LPARAM)
{
	int newVal = !(GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS);
	db_set_b(NULL, "CList", "HideEmptyGroups", (BYTE)newVal);
	SendMessage(pcli->hwndContactTree, CLM_SETHIDEEMPTYGROUPS, newVal, 0);
	return 0;
}

INT_PTR UseGroupsHelper(WPARAM, LPARAM)
{
	int newVal = !(GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_USEGROUPS);
	db_set_b(NULL, "CList", "UseGroups", (BYTE)newVal);
	SendMessage(pcli->hwndContactTree, CLM_SETUSEGROUPS, newVal,0);
	return 0;
}

INT_PTR HideOfflineRootHelper(WPARAM, LPARAM)
{
	SendMessage(pcli->hwndContactTree, CLM_SETHIDEOFFLINEROOT,
		!SendMessage(pcli->hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0),
		0);
	return 0;
}

INT_PTR CreateGroupHelper(WPARAM, LPARAM)
{
	SendMessage(pcli->hwndContactTree, CLM_SETHIDEEMPTYGROUPS, 0, 0);
	SendMessage(pcli->hwndContactTree, CLM_SETUSEGROUPS, 1, 0);
	Clist_CreateGroup(0, 0);
	return 0;
};

static int OnBuildGroupMenu(WPARAM, LPARAM)
{
	if (MirandaExiting()) return 0;

	int flags = db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT) ? CMIF_CHECKED : 0;
	Menu_ModifyItem(hHideOfflineUsersMenuItem, NULL, INVALID_HANDLE_VALUE, flags);

	flags = SendMessage(pcli->hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0) ? CMIF_CHECKED : 0;
	Menu_ModifyItem(hHideOfflineUsersOutHereMenuItem, NULL, INVALID_HANDLE_VALUE, flags);

	flags = GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS ? CMIF_CHECKED : 0;
	Menu_ModifyItem(hHideEmptyGroupsMenuItem, NULL, INVALID_HANDLE_VALUE, flags);

	flags = GetWindowLongPtr(pcli->hwndContactTree, GWL_STYLE) & CLS_USEGROUPS ? 0 : CMIF_CHECKED;
	Menu_ModifyItem(hDisableGroupsMenuItem, NULL, INVALID_HANDLE_VALUE, flags);
	return 0;
}

static IconItemT iconItem[] =
{ 
	{ LPGENT("New group"), "NewGroup", IDI_NEWGROUP2 }
};

void GroupMenus_Init(void)
{
	Icon_RegisterT(g_hInst, LPGENT("Contact list"), iconItem, _countof(iconItem));
}

void InitGroupMenus(void)
{
	CreateServiceFunction("CLISTMENUSGroup/ExecService", GroupMenuExecService);
	CreateServiceFunction("CLISTMENUSGroup/FreeOwnerDataGroupMenu", FreeOwnerDataGroupMenu);
	CreateServiceFunction("CLISTMENUSGroup/GroupMenuonAddService", GroupMenuonAddService);
	CreateServiceFunction("CLISTMENUSGroup/HideGroupsHelper", HideGroupsHelper);
	CreateServiceFunction("CLISTMENUSGroup/UseGroupsHelper", UseGroupsHelper);
	CreateServiceFunction("CLISTMENUSGroup/HideOfflineRootHelper", HideOfflineRootHelper);
	CreateServiceFunction("CLISTMENUSGroup/CreateGroupHelper", CreateGroupHelper);

	CreateServiceFunction("CList/AddGroupMenuItem", AddGroupMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDGROUP, BuildGroupMenu);

	HookEvent(ME_CLIST_PREBUILDGROUPMENU, OnBuildGroupMenu);

	InitSubGroupMenus();

	// Group menu
	hGroupMenuObject = Menu_AddObject("GroupMenu", LPGEN("Group menu"), 0, "CLISTMENUSGroup/ExecService");
	Menu_ConfigureObject(hGroupMenuObject, MCO_OPT_USERDEFINEDITEMS, TRUE);
	Menu_ConfigureObject(hGroupMenuObject, MCO_OPT_FREE_SERVICE, "CLISTMENUSGroup/FreeOwnerDataGroupMenu");
	Menu_ConfigureObject(hGroupMenuObject, MCO_OPT_ONADD_SERVICE, "CLISTMENUSGroup/GroupMenuonAddService");

	// add exit command to menu
	CMenuItem mi;
	mi.position = 1900000;
	mi.pszService = "CloseAction";
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_EXIT);
	mi.name.a = LPGEN("E&xit");
	AddGroupMenuItem(0, (LPARAM)&mi);

	mi.position = 500;
	mi.pszService = MS_CLIST_SHOWHIDE;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_SHOWHIDE);
	mi.name.a = LPGEN("&Hide/show");
	hHideShowMainMenuItem = (HGENMENU)AddGroupMenuItem(0, (LPARAM)&mi);

	mi.position = 200000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_FINDUSER);
	mi.pszService = "FindAdd/FindAddCommand";
	mi.name.a = LPGEN("&Find/add contacts...");
	AddGroupMenuItem(0, (LPARAM)&mi);

	mi.position = 300000;
	mi.pszService = "";
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MAINMENU);
	mi.name.a = LPGEN("&Main menu");
	hGroupMainMenuItemProxy = (HGENMENU)AddGroupMenuItem(0, (LPARAM)&mi);

	mi.position = 300100;
	mi.pszService = "";
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_STATUS);
	mi.name.a = LPGEN("&Status");
	hGroupStatusMenuItemProxy = (HGENMENU)AddGroupMenuItem(0, (LPARAM)&mi);

	mi.position = 400000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_OPTIONS);
	mi.pszService = "Options/OptionsCommand";
	mi.name.a = LPGEN("&Options...");
	AddGroupMenuItem(0, (LPARAM)&mi);

	mi.position = 500000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MIRANDA);
	mi.pszService = "Help/AboutCommand";
	mi.name.a = LPGEN("&About");
	AddGroupMenuItem(0, (LPARAM)&mi);

	GroupMenuParam gmp = {};
	mi.flags = 0;
	mi.position = 100000;
	mi.hIcolibItem = iconItem[0].hIcolib;
	mi.pszService = "CLISTMENUSGroup/CreateGroupHelper";
	mi.name.a = LPGEN("&New group");
	hNewGroupMenuItem = (HGENMENU)AddGroupMenuItem((WPARAM)&gmp, (LPARAM)&mi);
	DestroyIcon_protect((HICON)mi.hIcolibItem);

	mi.position = 100001;
	mi.hIcolibItem = NULL;
	mi.hIcolibItem = NULL;
	mi.pszService = MS_CLIST_SETHIDEOFFLINE;
	mi.name.a = LPGEN("&Hide offline users");
	gmp.lParam = 0; gmp.wParam = -1;
	hHideOfflineUsersMenuItem = (HGENMENU)AddGroupMenuItem((WPARAM)&gmp, (LPARAM)&mi);

	mi.position = 100002;
	mi.pszService = "CLISTMENUSGroup/HideOfflineRootHelper";
	mi.name.a = LPGEN("Hide &offline users out here");
	hHideOfflineUsersOutHereMenuItem = (HGENMENU)AddGroupMenuItem(0, (LPARAM)&mi);

	mi.position = 100003;
	mi.pszService = "CLISTMENUSGroup/HideGroupsHelper";
	mi.name.a = LPGEN("Hide &empty groups");
	hHideEmptyGroupsMenuItem = (HGENMENU)AddGroupMenuItem(0, (LPARAM)&mi);

	mi.position = 100004;
	mi.pszService = "CLISTMENUSGroup/UseGroupsHelper";
	mi.name.a = LPGEN("Disable &groups");
	hDisableGroupsMenuItem = (HGENMENU)AddGroupMenuItem(0, (LPARAM)&mi);
}

//////////////////////////////END Group MENU/////////////////////////

//////////////////////////////SubGroup MENU/////////////////////////

int hSubGroupMenuObject;

HANDLE hSubGroupMainMenuItemProxy;
HANDLE hSubGroupStatusMenuItemProxy;

HGENMENU hHideOfflineUsersHereMenuItem, hShowOfflineUsersHereMenuItem;

//SubGroupmenu exec param(ownerdata)
typedef struct
{
	char *szServiceName;
	int Param1, Param2;
}
SubGroupMenuExecParam, *lpSubGroupMenuExecParam;

static int OnBuildSubGroupMenu(WPARAM wParam, LPARAM)
{
	BOOL gray1 = FALSE;
	BOOL gray2 = FALSE;
	BOOL showOfflineinGroup = FALSE;

	ClcGroup *group = (ClcGroup *)wParam;
	if (wParam == 0) return 0;

	if (MirandaExiting()) return 0;
	//contact->group

	showOfflineinGroup = CLCItems_IsShowOfflineGroup(group);
	gray1 = (showOfflineinGroup != FALSE);
	gray2 = (group->hideOffline != FALSE);

	if (gray1 && gray2) gray1 = FALSE;  //should not be cause CLCItems_IsShowOfflineGroup return false if group->hideOffline

	Menu_EnableItem(hHideOfflineUsersHereMenuItem, !gray1);
	Menu_SetChecked(hHideOfflineUsersHereMenuItem, group->hideOffline && !gray1);

	Menu_EnableItem(hShowOfflineUsersHereMenuItem, !gray2);
	Menu_SetChecked(hShowOfflineUsersHereMenuItem, showOfflineinGroup && !gray2);
	return 0;
}

INT_PTR BuildSubGroupMenu(WPARAM wParam, LPARAM lParam)
{
	NotifyEventHooks(g_CluiData.hEventPreBuildSubGroupMenu, wParam, 0);

	HMENU hMenu = CreatePopupMenu();
	Menu_Build(hMenu, hSubGroupMenuObject, wParam, lParam);
	return (INT_PTR)hMenu;
}

HMENU cliBuildGroupPopupMenu(ClcGroup *group)
{
	return (HMENU)CallService(MS_CLIST_MENUBUILDSUBGROUP, (WPARAM)group, 0);
}

static INT_PTR AddSubGroupMenuItem(WPARAM wParam, LPARAM lParam)
{
	lpSubGroupMenuExecParam mmep = (lpSubGroupMenuExecParam)mir_calloc(sizeof(SubGroupMenuExecParam));
	if (mmep == NULL)
		return 0;

	TMO_MenuItem *pmi = (TMO_MenuItem*)lParam;
	// we need just one parametr.
	mmep->szServiceName = mir_strdup(pmi->pszService);
	GroupMenuParam *gmp = (GroupMenuParam*)wParam;
	if (gmp != NULL) {
		mmep->Param1 = gmp->wParam;
		mmep->Param2 = gmp->lParam;
	}
	pmi->ownerdata = mmep;

	HGENMENU hNewItem = Menu_AddItem(hSubGroupMenuObject, pmi);

	char buf[1024];
	mir_snprintf(buf, "%s/%s", pmi->pszService, pmi->name.a);
	Menu_ConfigureItem(hNewItem, MCI_OPT_UNIQUENAME, buf);
	return (INT_PTR)hNewItem;
}

INT_PTR SubGroupMenuCheckService(WPARAM wParam, LPARAM)
{
	TCheckProcParam * CParam = (TCheckProcParam*)wParam;
	if (CParam) {
		lpSubGroupMenuExecParam mmep = (lpSubGroupMenuExecParam)(CParam->MenuItemOwnerData);
		if (mmep)
			mmep->Param2 = CParam->lParam;
	}
	return 1;
}

INT_PTR SubGroupMenuonAddService(WPARAM wParam, LPARAM)
{
	MENUITEMINFO *mii = (MENUITEMINFO*)wParam;
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
		if (!mir_strcmp(mmep->szServiceName, "Help/AboutCommand")) {
			//bug in help.c,it used wparam as parent window handle without reason.
			mmep->Param1 = 0;
			CallService(mmep->szServiceName, mmep->Param1, lParam);
		}
		else CallService(mmep->szServiceName, mmep->Param1, lParam);
	}
	return 1;
}

INT_PTR FreeOwnerDataSubGroupMenu(WPARAM, LPARAM lParam)
{
	lpSubGroupMenuExecParam mmep = (lpSubGroupMenuExecParam)lParam;
	if (mmep != NULL){
		mir_free(mmep->szServiceName);
		mir_free(mmep);
	}

	return 0;
}

//wparam menu handle to pass to clc.c
//lparam WM_COMMAND HWND
INT_PTR GroupMenuExecProxy(WPARAM wParam, LPARAM lParam)
{
	SendMessage(lParam ? (HWND)lParam : (HWND)pcli->hwndContactTree, WM_COMMAND, wParam, 0);
	return 0;
}

void InitSubGroupMenus(void)
{
	CreateServiceFunction("CLISTMENUSSubGroup/ExecService", SubGroupMenuExecService);
	CreateServiceFunction("CLISTMENUSSubGroup/FreeOwnerDataSubGroupMenu", FreeOwnerDataSubGroupMenu);
	CreateServiceFunction("CLISTMENUSSubGroup/SubGroupMenuonAddService", SubGroupMenuonAddService);
	CreateServiceFunction("CLISTMENUSSubGroup/SubGroupMenuCheckService", SubGroupMenuCheckService);
	CreateServiceFunction("CLISTMENUSSubGroup/GroupMenuExecProxy", GroupMenuExecProxy);

	CreateServiceFunction("CList/AddSubGroupMenuItem", AddSubGroupMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDSUBGROUP, BuildSubGroupMenu);

	HookEvent(ME_CLIST_PREBUILDSUBGROUPMENU, OnBuildSubGroupMenu);

	// SubGroup menu
	hSubGroupMenuObject = Menu_AddObject("SubGroupMenu", LPGEN("Subgroup menu"), 0, "CLISTMENUSSubGroup/ExecService");
	Menu_ConfigureObject(hSubGroupMenuObject, MCO_OPT_USERDEFINEDITEMS, TRUE);
	Menu_ConfigureObject(hSubGroupMenuObject, MCO_OPT_FREE_SERVICE, "CLISTMENUSSubGroup/FreeOwnerDataSubGroupMenu");
	Menu_ConfigureObject(hSubGroupMenuObject, MCO_OPT_ONADD_SERVICE, "CLISTMENUSSubGroup/SubGroupMenuonAddService");
	Menu_ConfigureObject(hSubGroupMenuObject, MCO_OPT_CHECK_SERVICE, "CLISTMENUSSubGroup/SubGroupMenuCheckService");

	// add exit command to menu
	GroupMenuParam gmp;

	CMenuItem mi;
	mi.position = 1000;
	mi.hIcolibItem = iconItem[0].hIcolib;
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.name.a = LPGEN("&New subgroup");
	gmp.lParam = 0;
	gmp.wParam = POPUP_NEWSUBGROUP;
	hNewSubGroupMenuItem = (HGENMENU)AddSubGroupMenuItem((WPARAM)&gmp, (LPARAM)&mi);

	mi.position = 1001;
	mi.hIcolibItem = NULL;
	mi.hIcolibItem = NULL;
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.name.a = LPGEN("&Hide offline users in here");
	gmp.lParam = 0;
	gmp.wParam = POPUP_GROUPHIDEOFFLINE;
	hHideOfflineUsersHereMenuItem = (HGENMENU)AddSubGroupMenuItem((WPARAM)&gmp, (LPARAM)&mi);

	mi.position = 1002;
	mi.hIcolibItem = NULL;
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.name.a = LPGEN("&Show offline users in here");
	gmp.lParam = 0;
	gmp.wParam = POPUP_GROUPSHOWOFFLINE;
	hShowOfflineUsersHereMenuItem = (HGENMENU)AddSubGroupMenuItem((WPARAM)&gmp, (LPARAM)&mi);

	memset(&mi, 0, sizeof(mi));
	mi.position = 900001;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_RENAME);
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.name.a = LPGEN("&Rename group");
	gmp.lParam = 0;
	gmp.wParam = POPUP_RENAMEGROUP;
	AddSubGroupMenuItem((WPARAM)&gmp, (LPARAM)&mi);

	mi.position = 900002;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_DELETE);
	mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
	mi.name.a = LPGEN("&Delete group");
	gmp.lParam = 0;
	gmp.wParam = POPUP_DELETEGROUP;
	AddSubGroupMenuItem((WPARAM)&gmp, (LPARAM)&mi);
}

//////////////////////////////END SubGroup MENU/////////////////////////
