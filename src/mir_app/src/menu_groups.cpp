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

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////         Group MENU          //////////////////////////////

int hGroupMenuObject, hSubGroupMenuObject;

static HANDLE hEventPreBuildGroupMenu, hEventPreBuildSubGroupMenu;
static HGENMENU hHideOfflineUsersMenuItem;
static HGENMENU hHideOfflineUsersOutHereMenuItem;
static HGENMENU hHideEmptyGroupsMenuItem;
static HGENMENU hDisableGroupsMenuItem;

// Groupmenu exec param(ownerdata)

struct GroupMenuExecParam
{
	char *szServiceName;
	int Param1, Param2;
};

MIR_APP_DLL(HMENU) Menu_BuildGroupMenu()
{
	NotifyEventHooks(hEventPreBuildGroupMenu, 0, 0);

	HMENU hMenu = CreatePopupMenu();
	Menu_Build(hMenu, hGroupMenuObject);
	return hMenu;
}

MIR_APP_DLL(HGENMENU) Menu_AddGroupMenuItem(TMO_MenuItem *pmi, GroupMenuParam *gmp)
{
	GroupMenuExecParam *mmep = (GroupMenuExecParam*)mir_calloc(sizeof(GroupMenuExecParam));
	if (mmep == NULL)
		return 0;

	// we need just one parametr.
	mmep->szServiceName = mir_strdup(pmi->pszService);
	if (gmp != NULL) {
		mmep->Param1 = gmp->wParam;
		mmep->Param2 = gmp->lParam;
	}

	HGENMENU hNewItem = Menu_AddItem(hGroupMenuObject, pmi, mmep);

	char buf[1024];
	mir_snprintf(buf, "%s/%s", pmi->pszService, pmi->name.a);
	Menu_ConfigureItem(hNewItem, MCI_OPT_UNIQUENAME, buf);
	return hNewItem;
}

// called with:
// wparam - ownerdata
// lparam - lparam from winproc
INT_PTR GroupMenuExecService(WPARAM wParam, LPARAM lParam)
{
	if (wParam != 0) {
		GroupMenuExecParam *mmep = (GroupMenuExecParam *)wParam;
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
	GroupMenuExecParam *mmep = (GroupMenuExecParam *)lParam;
	if (mmep != NULL) {
		mir_free(mmep->szServiceName);
		mir_free(mmep);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR HideGroupsHelper(WPARAM, LPARAM)
{
	int newVal = !(GetWindowLongPtr(cli.hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS);
	db_set_b(NULL, "CList", "HideEmptyGroups", (BYTE)newVal);
	SendMessage(cli.hwndContactTree, CLM_SETHIDEEMPTYGROUPS, newVal, 0);
	return 0;
}

INT_PTR UseGroupsHelper(WPARAM, LPARAM)
{
	int newVal = !(GetWindowLongPtr(cli.hwndContactTree, GWL_STYLE) & CLS_USEGROUPS);
	db_set_b(NULL, "CList", "UseGroups", (BYTE)newVal);
	SendMessage(cli.hwndContactTree, CLM_SETUSEGROUPS, newVal,0);
	return 0;
}

INT_PTR HideOfflineRootHelper(WPARAM, LPARAM)
{
	SendMessage(cli.hwndContactTree, CLM_SETHIDEOFFLINEROOT,
		!SendMessage(cli.hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0),
		0);
	return 0;
}

INT_PTR CreateGroupHelper(WPARAM, LPARAM)
{
	SendMessage(cli.hwndContactTree, CLM_SETHIDEEMPTYGROUPS, 0, 0);
	SendMessage(cli.hwndContactTree, CLM_SETUSEGROUPS, 1, 0);
	Clist_CreateGroup(0, 0);
	return 0;
}

static int OnBuildGroupMenu(WPARAM, LPARAM)
{
	bool bChecked = db_get_b(NULL, "CList", "HideOffline", SETTING_HIDEOFFLINE_DEFAULT) != 0;
	Menu_SetChecked(hHideOfflineUsersMenuItem, bChecked);

	bChecked = SendMessage(cli.hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0) != 0;
	Menu_SetChecked(hHideOfflineUsersOutHereMenuItem, bChecked);

	DWORD dwStyle = GetWindowLongPtr(cli.hwndContactTree, GWL_STYLE);
	Menu_SetChecked(hHideEmptyGroupsMenuItem, (dwStyle & CLS_HIDEEMPTYGROUPS) != 0);
	Menu_SetChecked(hDisableGroupsMenuItem, (dwStyle & CLS_USEGROUPS) == 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////        SubGroup MENU        //////////////////////////////

struct SubGroupMenuExecParam
{
	char *szServiceName;
	int Param1, Param2;
};

MIR_APP_DLL(HMENU) Menu_BuildSubGroupMenu(ClcGroup *group)
{
	NotifyEventHooks(hEventPreBuildSubGroupMenu, (WPARAM)group, 0);

	HMENU hMenu = CreatePopupMenu();
	Menu_Build(hMenu, hSubGroupMenuObject, (WPARAM)group, 0);
	return hMenu;
}

/////////////////////////////////////////////////////////////////////////////////////////

HMENU cliBuildGroupPopupMenu(ClcGroup *group)
{
	return Menu_BuildSubGroupMenu(group);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(HGENMENU) Menu_AddSubGroupMenuItem(TMO_MenuItem *pmi, GroupMenuParam *gmp)
{
	SubGroupMenuExecParam *mmep = (SubGroupMenuExecParam*)mir_calloc(sizeof(SubGroupMenuExecParam));
	if (mmep == NULL)
		return 0;

	// we need just one parametr.
	mmep->szServiceName = mir_strdup(pmi->pszService);
	if (gmp != NULL) {
		mmep->Param1 = gmp->wParam;
		mmep->Param2 = gmp->lParam;
	}
	
	HGENMENU hNewItem = Menu_AddItem(hSubGroupMenuObject, pmi, mmep);

	char buf[1024];
	mir_snprintf(buf, "%s/%s", pmi->pszService, pmi->name.a);
	Menu_ConfigureItem(hNewItem, MCI_OPT_UNIQUENAME, buf);
	return hNewItem;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR SubGroupMenuCheckService(WPARAM wParam, LPARAM)
{
	TCheckProcParam * CParam = (TCheckProcParam*)wParam;
	if (CParam) {
		SubGroupMenuExecParam *mmep = (SubGroupMenuExecParam *)(CParam->MenuItemOwnerData);
		if (mmep)
			mmep->Param2 = CParam->lParam;
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// called with:
// wparam - ownerdata
// lparam - lparam from winproc
static INT_PTR SubGroupMenuExecService(WPARAM wParam, LPARAM lParam)
{
	if (wParam != 0) {
		SubGroupMenuExecParam *mmep = (SubGroupMenuExecParam *)wParam;
		if (!mir_strcmp(mmep->szServiceName, "Help/AboutCommand")) {
			//bug in help.c,it used wparam as parent window handle without reason.
			mmep->Param1 = 0;
			CallService(mmep->szServiceName, mmep->Param1, lParam);
		}
		else CallService(mmep->szServiceName, mmep->Param1, lParam);
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR FreeOwnerDataSubGroupMenu(WPARAM, LPARAM lParam)
{
	SubGroupMenuExecParam *mmep = (SubGroupMenuExecParam *)lParam;
	if (mmep != NULL) {
		mir_free(mmep->szServiceName);
		mir_free(mmep);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static HGENMENU hHideOfflineUsersHereMenuItem;

static int OnBuildSubGroupMenu(WPARAM wParam, LPARAM)
{
	ClcGroup *group = (ClcGroup*)wParam;
	if (group != 0) // contact->group
		Menu_SetChecked(hHideOfflineUsersHereMenuItem, group->hideOffline != 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// wparam menu handle to pass to clc.c
// lparam WM_COMMAND HWND
static INT_PTR GroupMenuExecProxy(WPARAM wParam, LPARAM lParam)
{
	SendMessage(lParam ? (HWND)lParam : (HWND)cli.hwndContactTree, WM_COMMAND, wParam, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void InitGroupMenus(void)
{
	GroupMenuParam gmp = {};

	// Group menu
	CreateServiceFunction("CLISTMENUSGroup/ExecService", GroupMenuExecService);
	CreateServiceFunction("CLISTMENUSGroup/FreeOwnerDataGroupMenu", FreeOwnerDataGroupMenu);

	hEventPreBuildGroupMenu = CreateHookableEvent(ME_CLIST_PREBUILDGROUPMENU);
	HookEvent(ME_CLIST_PREBUILDGROUPMENU, OnBuildGroupMenu);

	hGroupMenuObject = Menu_AddObject("GroupMenu", LPGEN("Group menu"), 0, "CLISTMENUSGroup/ExecService");
	Menu_ConfigureObject(hGroupMenuObject, MCO_OPT_USERDEFINEDITEMS, TRUE);
	Menu_ConfigureObject(hGroupMenuObject, MCO_OPT_FREE_SERVICE, "CLISTMENUSGroup/FreeOwnerDataGroupMenu");
	Menu_ConfigureObject(hGroupMenuObject, MCO_OPT_ONADD_SERVICE, "CLISTMENUSGroup/GroupMenuOnAddService");
	{
		CMenuItem mi;
		mi.position = 100000;
		mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_GROUPADD);
		mi.pszService = "CLISTMENUSGroup/CreateGroupHelper";
		mi.name.a = LPGEN("&New group");
		Menu_AddGroupMenuItem(&mi);
		CreateServiceFunction(mi.pszService, CreateGroupHelper);

		mi.position = 500001;
		mi.hIcolibItem = NULL;
		mi.pszService = MS_CLIST_SETHIDEOFFLINE;
		mi.name.a = LPGEN("&Hide offline users");
		gmp.wParam = -1;
		hHideOfflineUsersMenuItem = Menu_AddGroupMenuItem(&mi, &gmp);

		mi.position = 500002;
		mi.pszService = "CLISTMENUSGroup/HideOfflineRootHelper";
		mi.name.a = LPGEN("Hide &offline users out here");
		hHideOfflineUsersOutHereMenuItem = Menu_AddGroupMenuItem(&mi);
		CreateServiceFunction(mi.pszService, HideOfflineRootHelper);

		mi.position = 500003;
		mi.pszService = "CLISTMENUSGroup/HideGroupsHelper";
		mi.name.a = LPGEN("Hide &empty groups");
		hHideEmptyGroupsMenuItem = Menu_AddGroupMenuItem(&mi);
		CreateServiceFunction(mi.pszService, HideGroupsHelper);

		mi.position = 500004;
		mi.pszService = "CLISTMENUSGroup/UseGroupsHelper";
		mi.name.a = LPGEN("Disable &groups");
		hDisableGroupsMenuItem = Menu_AddGroupMenuItem(&mi);
		CreateServiceFunction(mi.pszService, UseGroupsHelper);

		mi.position = 1900000;
		mi.pszService = "CloseAction";
		mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_EXIT);
		mi.name.a = LPGEN("E&xit");
		Menu_AddGroupMenuItem(&mi);
	}

	// SubGroup menu
	CreateServiceFunction("CLISTMENUSSubGroup/ExecService", SubGroupMenuExecService);
	CreateServiceFunction("CLISTMENUSSubGroup/FreeOwnerDataSubGroupMenu", FreeOwnerDataSubGroupMenu);
	CreateServiceFunction("CLISTMENUSSubGroup/SubGroupMenuCheckService", SubGroupMenuCheckService);
	CreateServiceFunction("CLISTMENUSSubGroup/GroupMenuExecProxy", GroupMenuExecProxy);

	hEventPreBuildSubGroupMenu = CreateHookableEvent(ME_CLIST_PREBUILDSUBGROUPMENU);
	HookEvent(ME_CLIST_PREBUILDSUBGROUPMENU, OnBuildSubGroupMenu);

	hSubGroupMenuObject = Menu_AddObject("SubGroupMenu", LPGEN("Subgroup menu"), 0, "CLISTMENUSSubGroup/ExecService");
	Menu_ConfigureObject(hSubGroupMenuObject, MCO_OPT_USERDEFINEDITEMS, TRUE);
	Menu_ConfigureObject(hSubGroupMenuObject, MCO_OPT_FREE_SERVICE, "CLISTMENUSSubGroup/FreeOwnerDataSubGroupMenu");
	Menu_ConfigureObject(hSubGroupMenuObject, MCO_OPT_CHECK_SERVICE, "CLISTMENUSSubGroup/SubGroupMenuCheckService");
	{
		CMenuItem mi;
		mi.position = 1000;
		mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_ADD);
		mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
		mi.name.a = LPGEN("&New subgroup");
		gmp.wParam = POPUP_NEWSUBGROUP;
		Menu_AddSubGroupMenuItem(&mi, &gmp);

		mi.position = 1001;
		mi.hIcolibItem = 0;
		mi.name.a = LPGEN("&Hide offline users in here");
		gmp.wParam = POPUP_GROUPHIDEOFFLINE;
		hHideOfflineUsersHereMenuItem = Menu_AddSubGroupMenuItem(&mi, &gmp);

		mi.position = 900001;
		mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_RENAME);
		mi.name.a = LPGEN("&Rename group");
		gmp.wParam = POPUP_RENAMEGROUP;
		Menu_AddSubGroupMenuItem(&mi, &gmp);

		mi.position = 900002;
		mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_DELETE);
		mi.name.a = LPGEN("&Delete group");
		gmp.wParam = POPUP_DELETEGROUP;
		Menu_AddSubGroupMenuItem(&mi, &gmp);
	}
}
