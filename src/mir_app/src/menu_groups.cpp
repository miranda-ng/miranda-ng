/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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
	if (mmep == nullptr)
		return nullptr;

	// we need just one parametr.
	mmep->szServiceName = mir_strdup(pmi->pszService);
	if (gmp != nullptr) {
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
	if (mmep != nullptr) {
		mir_free(mmep->szServiceName);
		mir_free(mmep);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR HideGroupsHelper(WPARAM, LPARAM)
{
	int newVal = !(GetWindowLongPtr(g_clistApi.hwndContactTree, GWL_STYLE) & CLS_HIDEEMPTYGROUPS);
	db_set_b(0, "CList", "HideEmptyGroups", (uint8_t)newVal);
	SendMessage(g_clistApi.hwndContactTree, CLM_SETHIDEEMPTYGROUPS, newVal, 0);
	return newVal;
}

static INT_PTR UseGroupsHelper(WPARAM, LPARAM)
{
	int newVal = !(GetWindowLongPtr(g_clistApi.hwndContactTree, GWL_STYLE) & CLS_USEGROUPS);
	Clist::UseGroups = newVal;
	SendMessage(g_clistApi.hwndContactTree, CLM_SETUSEGROUPS, newVal, 0);
	return newVal;
}

static INT_PTR HideOfflineHelper(WPARAM, LPARAM)
{
	return g_clistApi.pfnSetHideOffline(-1);
}

static INT_PTR HideOfflineRootHelper(WPARAM, LPARAM)
{
	int newVal = !SendMessage(g_clistApi.hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0);
	SendMessage(g_clistApi.hwndContactTree, CLM_SETHIDEOFFLINEROOT, newVal, 0);
	return newVal;
}

static INT_PTR CreateGroupHelper(WPARAM, LPARAM)
{
	SendMessage(g_clistApi.hwndContactTree, CLM_SETHIDEEMPTYGROUPS, 0, 0);
	SendMessage(g_clistApi.hwndContactTree, CLM_SETUSEGROUPS, 1, 0);
	Clist_GroupCreate(0, nullptr);
	return 0;
}

static int OnBuildGroupMenu(WPARAM, LPARAM)
{
	bool bChecked = Clist::HideOffline;
	Menu_SetChecked(hHideOfflineUsersMenuItem, bChecked);

	bChecked = SendMessage(g_clistApi.hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0) != 0;
	Menu_SetChecked(hHideOfflineUsersOutHereMenuItem, bChecked);

	uint32_t dwStyle = GetWindowLongPtr(g_clistApi.hwndContactTree, GWL_STYLE);
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
	SubGroupMenuExecParam *mmep = (SubGroupMenuExecParam *)mir_calloc(sizeof(SubGroupMenuExecParam));
	if (mmep == nullptr)
		return nullptr;

	// we need just one parametr.
	mmep->szServiceName = mir_strdup(pmi->pszService);
	if (gmp != nullptr) {
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
	TCheckProcParam *CParam = (TCheckProcParam *)wParam;
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
	if (mmep != nullptr) {
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
	if (group != nullptr) // contact->group
		Menu_SetChecked(hHideOfflineUsersHereMenuItem, group->hideOffline != 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// wparam menu handle to pass to clc.c
// lparam WM_COMMAND HWND

static INT_PTR GroupMenuExecProxy(WPARAM wParam, LPARAM lParam)
{
	SendMessage(lParam ? (HWND)lParam : (HWND)g_clistApi.hwndContactTree, WM_COMMAND, wParam, 0);
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

	hGroupMenuObject = Menu_AddObject("GroupMenu", LPGEN("Group menu"), nullptr, "CLISTMENUSGroup/ExecService");
	Menu_ConfigureObject(hGroupMenuObject, MCO_OPT_USERDEFINEDITEMS, TRUE);
	Menu_ConfigureObject(hGroupMenuObject, MCO_OPT_FREE_SERVICE, "CLISTMENUSGroup/FreeOwnerDataGroupMenu");
	Menu_ConfigureObject(hGroupMenuObject, MCO_OPT_ONADD_SERVICE, "CLISTMENUSGroup/GroupMenuOnAddService");
	{
		CMenuItem mi(&g_plugin);

		SET_UID(mi, 0x2f75bc72, 0xd836, 0x4922, 0x9f, 0xe, 0xed, 0x9e, 0xe7, 0x2b, 0x84, 0xf0);
		mi.position = 100000;
		mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_GROUPADD);
		mi.pszService = "CLISTMENUSGroup/CreateGroupHelper";
		mi.name.a = LPGEN("&New group");
		Menu_AddGroupMenuItem(&mi);
		CreateServiceFunction(mi.pszService, CreateGroupHelper);

		SET_UID(mi, 0xe6269658, 0x69, 0x4094, 0x9b, 0x35, 0x4e, 0x80, 0x29, 0x26, 0xf, 0x8e);
		mi.position = 500001;
		mi.hIcolibItem = nullptr;
		mi.pszService = MS_CLIST_TOGGLEHIDEOFFLINE;
		mi.name.a = LPGEN("&Hide offline users");
		gmp.wParam = -1;
		hHideOfflineUsersMenuItem = Menu_AddGroupMenuItem(&mi, &gmp);
		CreateServiceFunction(mi.pszService, HideOfflineHelper);

		SET_UID(mi, 0xeded7371, 0xf6e6, 0x48c3, 0x8c, 0x9e, 0x62, 0xc1, 0xd5, 0xcb, 0x51, 0xbc);
		mi.position = 500002;
		mi.pszService = MS_CLIST_TOGGLEHIDEOFFLINEROOT;
		mi.name.a = LPGEN("Hide &offline users out here");
		hHideOfflineUsersOutHereMenuItem = Menu_AddGroupMenuItem(&mi);
		CreateServiceFunction(mi.pszService, HideOfflineRootHelper);

		SET_UID(mi, 0x4c17b9cf, 0x513a, 0x41d8, 0x8d, 0x2b, 0x89, 0x44, 0x81, 0x14, 0x0, 0x91);
		mi.position = 500003;
		mi.pszService = MS_CLIST_TOGGLEEMPTYGROUPS;
		mi.name.a = LPGEN("Hide &empty groups");
		hHideEmptyGroupsMenuItem = Menu_AddGroupMenuItem(&mi);
		CreateServiceFunction(mi.pszService, HideGroupsHelper);

		SET_UID(mi, 0xfcbdbbb1, 0xa553, 0x49ac, 0xa5, 0xdf, 0xb4, 0x81, 0x38, 0xf, 0xa0, 0xc7);
		mi.position = 500004;
		mi.pszService = MS_CLIST_TOGGLEGROUPS;
		mi.name.a = LPGEN("Disable &groups");
		hDisableGroupsMenuItem = Menu_AddGroupMenuItem(&mi);
		CreateServiceFunction(mi.pszService, UseGroupsHelper);

		SET_UID(mi, 0xfffb8733, 0xa3e4, 0x4566, 0xa6, 0x6e, 0x4c, 0x4c, 0x7c, 0xe9, 0x56, 0x99);
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

	hEventPreBuildSubGroupMenu = CreateHookableEvent(ME_CLIST_PREBUILDSUBGROUPMENU);
	HookEvent(ME_CLIST_PREBUILDSUBGROUPMENU, OnBuildSubGroupMenu);

	hSubGroupMenuObject = Menu_AddObject("SubGroupMenu", LPGEN("Subgroup menu"), nullptr, "CLISTMENUSSubGroup/ExecService");
	Menu_ConfigureObject(hSubGroupMenuObject, MCO_OPT_USERDEFINEDITEMS, TRUE);
	Menu_ConfigureObject(hSubGroupMenuObject, MCO_OPT_FREE_SERVICE, "CLISTMENUSSubGroup/FreeOwnerDataSubGroupMenu");
	Menu_ConfigureObject(hSubGroupMenuObject, MCO_OPT_CHECK_SERVICE, "CLISTMENUSSubGroup/SubGroupMenuCheckService");
	{
		CMenuItem mi(&g_plugin);

		SET_UID(mi, 0xd208f1d2, 0x7220, 0x4d37, 0xb6, 0xe4, 0xd5, 0x4a, 0xe8, 0xa3, 0xf4, 0x53);
		mi.position = 1000;
		mi.hIcolibItem = Skin_GetIconHandle(SKINICON_AUTH_ADD);
		mi.pszService = "CLISTMENUSSubGroup/GroupMenuExecProxy";
		mi.name.a = LPGEN("&New subgroup");
		gmp.wParam = POPUP_NEWSUBGROUP;
		Menu_AddSubGroupMenuItem(&mi, &gmp);
		CreateServiceFunction(mi.pszService, GroupMenuExecProxy);

		SET_UID(mi, 0xd85f5ff0, 0x12ca, 0x464d, 0x86, 0x51, 0x53, 0x36, 0x9f, 0x1d, 0x80, 0x45);
		mi.position = 1001;
		mi.hIcolibItem = nullptr;
		mi.name.a = LPGEN("&Hide offline users in here");
		gmp.wParam = POPUP_GROUPHIDEOFFLINE;
		hHideOfflineUsersHereMenuItem = Menu_AddSubGroupMenuItem(&mi, &gmp);

		SET_UID(mi, 0xf0953dd, 0x5c31, 0x48a4, 0xb4, 0x16, 0x89, 0x5, 0x97, 0xd0, 0x75, 0x3e);
		mi.position = 900001;
		mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_RENAME);
		mi.name.a = LPGEN("&Rename group");
		gmp.wParam = POPUP_RENAMEGROUP;
		Menu_AddSubGroupMenuItem(&mi, &gmp);

		SET_UID(mi, 0xb0d63cda, 0xa743, 0x4cfa, 0xa6, 0x2d, 0x50, 0xc0, 0x90, 0xe7, 0x6a, 0xc4);
		mi.position = 900002;
		mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_DELETE);
		mi.name.a = LPGEN("&Delete group");
		gmp.wParam = POPUP_DELETEGROUP;
		Menu_AddSubGroupMenuItem(&mi, &gmp);
	}
}
