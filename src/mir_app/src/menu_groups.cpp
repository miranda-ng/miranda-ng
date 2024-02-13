/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-24 Miranda NG team (https://miranda-ng.org),
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
// Group menu

int hGroupMenuObject;

static HANDLE hEventPreBuildGroupMenu;
static HGENMENU hmiHideOfflineUsers, hmiHideOfflineUsersInHere, hmiHideOfflineUsersOutHere;
static HGENMENU hmiHideEmptyGroups, hmiDisableGroups;
static HGENMENU hmiCreateSubgroup, hmiRenameGroup, hmiDeleteGroup;

// Groupmenu exec param(ownerdata)

struct GroupMenuExecParam
{
	char *szServiceName;
	int Param1, Param2;
};

HMENU Menu_BuildGroupMenu(struct ClcGroup *group)
{
	NotifyEventHooks(hEventPreBuildGroupMenu, WPARAM(group), 0);

	HMENU hMenu = CreatePopupMenu();
	Menu_Build(hMenu, hGroupMenuObject, WPARAM(group));
	return hMenu;
}

MIR_APP_DLL(HGENMENU) Menu_AddGroupMenuItem(TMO_MenuItem *pmi, GroupMenuParam *gmp)
{
	GroupMenuExecParam *mmep = (GroupMenuExecParam*)mir_calloc(sizeof(GroupMenuExecParam));
	if (mmep == nullptr)
		return nullptr;

	// we need just one parametr.
	mmep->szServiceName = mir_strdup(pmi->pszService);
	if (gmp != nullptr)
		mmep->Param2 = gmp->lParam;

	HGENMENU hNewItem = Menu_AddItem(hGroupMenuObject, pmi, mmep);

	char buf[1024];
	mir_snprintf(buf, "%s/%s", pmi->pszService, pmi->name.a);
	Menu_ConfigureItem(hNewItem, MCI_OPT_UNIQUENAME, buf);
	return hNewItem;
}

// called with:
// wparam - ClcGroup*
// lparam - lparam from winproc
static INT_PTR GroupMenuCheckService(WPARAM wParam, LPARAM)
{
	TCheckProcParam *CParam = (TCheckProcParam *)wParam;
	if (CParam) {
		GroupMenuExecParam *mmep = (GroupMenuExecParam *)(CParam->MenuItemOwnerData);
		if (mmep)
			mmep->Param1 = CParam->wParam;
	}
	return 1;
}

// called with:
// wparam - ownerdata
// lparam - lparam from winproc
INT_PTR GroupMenuExecService(WPARAM wParam, LPARAM)
{
	if (wParam != 0) {
		GroupMenuExecParam *mmep = (GroupMenuExecParam *)wParam;
		CallService(mmep->szServiceName, mmep->Param1, mmep->Param2);
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

static INT_PTR ToggleOfflineHelper(WPARAM, LPARAM)
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

static INT_PTR CreateSubGroupHelper(WPARAM wParam, LPARAM)
{
	SendMessage(g_clistApi.hwndContactTree, CLM_SETHIDEEMPTYGROUPS, 0, 0);
	SendMessage(g_clistApi.hwndContactTree, CLM_SETUSEGROUPS, 1, 0);

	auto *pGroup = (ClcGroup *)wParam;
	Clist_GroupCreate(pGroup ? pGroup->groupId : 0, nullptr);
	return 0;
}

static INT_PTR HideInGroupHelper(WPARAM wParam, LPARAM)
{
	auto *pGroup = (ClcGroup *)wParam;
	Clist_GroupSetFlags(pGroup->groupId, MAKELPARAM(pGroup->bHideOffline ? 0 : GROUPF_HIDEOFFLINE, GROUPF_HIDEOFFLINE));
	return 0;
}

static INT_PTR RenameGroupHelper(WPARAM wParam, LPARAM)
{
	auto *pGroup = (ClcGroup *)wParam;
	PostMessage(g_clistApi.hwndContactTree, CLM_EDITLABEL, pGroup->groupId | HCONTACT_ISGROUP, 0);
	return 0;
}

static INT_PTR DeleteGroupHelper(WPARAM wParam, LPARAM)
{
	auto *pGroup = (ClcGroup *)wParam;
	Clist_GroupDelete(pGroup->groupId);
	return 0;
}

static int OnBuildGroupMenu(WPARAM wParam, LPARAM)
{
	bool bChecked = Clist::HideOffline;
	Menu_SetChecked(hmiHideOfflineUsers, bChecked);

	bChecked = SendMessage(g_clistApi.hwndContactTree, CLM_GETHIDEOFFLINEROOT, 0, 0) != 0;
	Menu_SetChecked(hmiHideOfflineUsersOutHere, bChecked);

	uint32_t dwStyle = GetWindowLongPtr(g_clistApi.hwndContactTree, GWL_STYLE);
	Menu_SetChecked(hmiHideEmptyGroups, (dwStyle & CLS_HIDEEMPTYGROUPS) != 0);
	Menu_SetChecked(hmiDisableGroups, (dwStyle & CLS_USEGROUPS) == 0);

	ClcGroup *group = (ClcGroup *)wParam;

	Menu_ShowItem(hmiRenameGroup, group != 0);
	Menu_ShowItem(hmiDeleteGroup, group != 0);
	Menu_ShowItem(hmiCreateSubgroup, group != 0);
	Menu_ShowItem(hmiHideOfflineUsersInHere, group && !group->bShowOffline);
	Menu_ShowItem(hmiHideOfflineUsersOutHere, group == 0);

	if (group != nullptr) // contact->group
		Menu_SetChecked(hmiHideOfflineUsersInHere, group->bHideOffline != 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void InitGroupMenus(void)
{
	// Group menu
	CreateServiceFunction("CLISTMENUSGroup/ExecService", GroupMenuExecService);
	CreateServiceFunction("CLISTMENUSGroup/CheckService", GroupMenuCheckService);
	CreateServiceFunction("CLISTMENUSGroup/FreeOwnerDataGroupMenu", FreeOwnerDataGroupMenu);

	hEventPreBuildGroupMenu = CreateHookableEvent(ME_CLIST_PREBUILDGROUPMENU);
	HookEvent(ME_CLIST_PREBUILDGROUPMENU, OnBuildGroupMenu);

	hGroupMenuObject = Menu_AddObject("GroupMenu", LPGEN("Group menu"), nullptr, "CLISTMENUSGroup/ExecService");
	Menu_ConfigureObject(hGroupMenuObject, MCO_OPT_USERDEFINEDITEMS, TRUE);
	Menu_ConfigureObject(hGroupMenuObject, MCO_OPT_CHECK_SERVICE, "CLISTMENUSGroup/CheckService");
	Menu_ConfigureObject(hGroupMenuObject, MCO_OPT_FREE_SERVICE, "CLISTMENUSGroup/FreeOwnerDataGroupMenu");

	CMenuItem mi(&g_plugin);

	SET_UID(mi, 0x2f75bc72, 0xd836, 0x4922, 0x9f, 0xe, 0xed, 0x9e, 0xe7, 0x2b, 0x84, 0xf0);
	mi.position = 100000;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_GROUPADD);
	mi.pszService = "CLISTMENUSGroup/CreateGroupHelper";
	mi.name.a = LPGEN("New group");
	Menu_AddGroupMenuItem(&mi);
	CreateServiceFunction(mi.pszService, CreateGroupHelper);

	SET_UID(mi, 0x24bcb592, 0x660e, 0x4541, 0xa0, 0xac, 0x11, 0x4a, 0x23, 0xc8, 0x9b, 0x91);
	mi.position++;
	mi.name.a = LPGEN("New subgroup");
	mi.pszService = "CLISTMENUSGroup/CreateSubGroupHelper";
	hmiCreateSubgroup = Menu_AddGroupMenuItem(&mi);
	CreateServiceFunction(mi.pszService, CreateSubGroupHelper);

	SET_UID(mi, 0xe6269658, 0x69, 0x4094, 0x9b, 0x35, 0x4e, 0x80, 0x29, 0x26, 0xf, 0x8e);
	mi.position = 500001;
	mi.hIcolibItem = nullptr;
	mi.pszService = MS_CLIST_TOGGLEHIDEOFFLINE;
	mi.name.a = LPGEN("Hide offline users");
	hmiHideOfflineUsers = Menu_AddGroupMenuItem(&mi);
	CreateServiceFunction(mi.pszService, ToggleOfflineHelper);

	SET_UID(mi, 0xeded7371, 0xf6e6, 0x48c3, 0x8c, 0x9e, 0x62, 0xc1, 0xd5, 0xcb, 0x51, 0xbc);
	mi.position++;
	mi.pszService = MS_CLIST_TOGGLEHIDEOFFLINEROOT;
	mi.name.a = LPGEN("Hide offline users out here");
	hmiHideOfflineUsersOutHere = Menu_AddGroupMenuItem(&mi);
	CreateServiceFunction(mi.pszService, HideOfflineRootHelper);

	SET_UID(mi, 0x4c17b9cf, 0x513a, 0x41d8, 0x8d, 0x2b, 0x89, 0x44, 0x81, 0x14, 0x0, 0x91);
	mi.position++;
	mi.pszService = MS_CLIST_TOGGLEEMPTYGROUPS;
	mi.name.a = LPGEN("Hide empty groups");
	hmiHideEmptyGroups = Menu_AddGroupMenuItem(&mi);
	CreateServiceFunction(mi.pszService, HideGroupsHelper);

	SET_UID(mi, 0xfcbdbbb1, 0xa553, 0x49ac, 0xa5, 0xdf, 0xb4, 0x81, 0x38, 0xf, 0xa0, 0xc7);
	mi.position++;
	mi.pszService = MS_CLIST_TOGGLEGROUPS;
	mi.name.a = LPGEN("Disable groups");
	hmiDisableGroups = Menu_AddGroupMenuItem(&mi);
	CreateServiceFunction(mi.pszService, UseGroupsHelper);
	
	SET_UID(mi, 0xd85f5ff0, 0x12ca, 0x464d, 0x86, 0x51, 0x53, 0x36, 0x9f, 0x1d, 0x80, 0x45);
	mi.position = 900001;
	mi.hIcolibItem = nullptr;
	mi.name.a = LPGEN("Hide offline users in here");
	mi.pszService = "CLISTMENUSGroup/HideInGroupHelper";
	hmiHideOfflineUsersInHere = Menu_AddGroupMenuItem(&mi);
	CreateServiceFunction(mi.pszService, HideInGroupHelper);

	SET_UID(mi, 0xf0953dd, 0x5c31, 0x48a4, 0xb4, 0x16, 0x89, 0x5, 0x97, 0xd0, 0x75, 0x3e);
	mi.position++;
	mi.name.a = LPGEN("Rename group");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_RENAME);
	mi.pszService = "CLISTMENUSGroup/RenameGroupHelper";
	hmiRenameGroup = Menu_AddGroupMenuItem(&mi);
	CreateServiceFunction(mi.pszService, RenameGroupHelper);

	SET_UID(mi, 0xb0d63cda, 0xa743, 0x4cfa, 0xa6, 0x2d, 0x50, 0xc0, 0x90, 0xe7, 0x6a, 0xc4);
	mi.position++;
	mi.name.a = LPGEN("Delete group");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_DELETE);
	mi.pszService = "CLISTMENUSGroup/DeleteGroupHelper";
	hmiDeleteGroup = Menu_AddGroupMenuItem(&mi);
	CreateServiceFunction(mi.pszService, DeleteGroupHelper);
}
