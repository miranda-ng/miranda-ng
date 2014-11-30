/*

Copyright (C) 2009 Ricardo Pescuma Domenecci
Copyright (C) 2012-14 Miranda NG project

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "..\..\core\commonheaders.h"

#include "m_cluiframes.h"

#include "extraicons.h"
#include "usedIcons.h"
#include "..\clist\clc.h"

// Prototypes ///////////////////////////////////////////////////////////////////////////

int SortFunc(const ExtraIcon *p1, const ExtraIcon *p2)
{
	int ret = p1->getPosition() - p2->getPosition();
	if (ret != 0)
		return ret;

	int id1 = (p1->getType() != EXTRAICON_TYPE_GROUP) ? ((BaseExtraIcon*) p1)->getID() : 0;
	int id2 = (p2->getType() != EXTRAICON_TYPE_GROUP) ? ((BaseExtraIcon*) p2)->getID() : 0;
	return id1 - id2;
}

LIST<ExtraIcon> extraIconsByHandle(10), extraIconsBySlot(10, SortFunc);
LIST<BaseExtraIcon> registeredExtraIcons(10);

BOOL clistRebuildAlreadyCalled = FALSE;
BOOL clistApplyAlreadyCalled = FALSE;

int clistFirstSlot = 0;
int clistSlotCount = 0;

// Functions ////////////////////////////////////////////////////////////////////////////

int InitOptionsCallback(WPARAM wParam, LPARAM lParam);

// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	// add our modules to the KnownModules list
	CallService("DBEditorpp/RegisterSingleModule", (WPARAM) MODULE_NAME, 0);
	CallService("DBEditorpp/RegisterSingleModule", (WPARAM) MODULE_NAME "Groups", 0);

	HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);
	return 0;
}

int GetNumberOfSlots()
{
	return clistSlotCount;
}

int ConvertToClistSlot(int slot)
{
	if (slot < 0)
		return slot;

	return clistFirstSlot + slot;
}

int ExtraImage_ExtraIDToColumnNum(int extra)
{
	return (extra < 1 || extra > EXTRA_ICON_COUNT) ? -1 : extra-1;
}

int Clist_SetExtraIcon(MCONTACT hContact, int slot, HANDLE hImage)
{
	if (cli.hwndContactTree == 0)
		return -1;

	int icol = ExtraImage_ExtraIDToColumnNum( ConvertToClistSlot(slot));
	if (icol == -1)
		return -1;

	HANDLE hItem = (HANDLE)SendMessage(cli.hwndContactTree, CLM_FINDCONTACT, hContact, 0);
	if (hItem == 0)
		return -1;

	SendMessage(cli.hwndContactTree, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(icol,hImage));
	return 0;
}

ExtraIcon* GetExtraIcon(HANDLE id)
{
	int i = (int)id;
	if (i < 1 || i > extraIconsByHandle.getCount())
		return NULL;

	return extraIconsByHandle[i-1];
}

ExtraIcon* GetExtraIconBySlot(int slot)
{
	for (int i = 0; i < extraIconsBySlot.getCount(); i++) {
		ExtraIcon *extra = extraIconsBySlot[i];
		if (extra->getSlot() == slot)
			return extra;
	}
	return NULL;
}

BaseExtraIcon* GetExtraIconByName(const char *name)
{
	for (int i=0; i < registeredExtraIcons.getCount(); i++) {
		BaseExtraIcon *extra = registeredExtraIcons[i];
		if (strcmp(name, extra->getName()) == 0)
			return extra;
	}
	return NULL;
}

static void LoadGroups(LIST<ExtraIconGroup> &groups)
{
	int count = db_get_w(NULL, MODULE_NAME "Groups", "Count", 0);
	for (int i=0; i < count; i++) {
		char setting[512];
		mir_snprintf(setting, SIZEOF(setting), "%d_count", i);
		unsigned int items = db_get_w(NULL, MODULE_NAME "Groups", setting, 0);
		if (items < 1)
			continue;

		mir_snprintf(setting, SIZEOF(setting), "__group_%d", i);
		ExtraIconGroup *group = new ExtraIconGroup(setting);

		for (unsigned int j = 0; j < items; j++) {
			mir_snprintf(setting, SIZEOF(setting), "%d_%d", i, j);
			ptrA szIconName(db_get_sa(NULL, MODULE_NAME "Groups", setting));
			if (IsEmpty(szIconName))
				continue;

			BaseExtraIcon *extra = GetExtraIconByName(szIconName);
			if (extra == NULL)
				continue;

			group->items.insert(extra);
			if (extra->getSlot() >= 0)
				group->setSlot(extra->getSlot());
		}

		if (group->items.getCount() < 2) {
			delete group;
			continue;
		}

		groups.insert(group);
	}
}

static ExtraIconGroup* IsInGroup(LIST<ExtraIconGroup> &groups, BaseExtraIcon *extra)
{
	for (int i = 0; i < groups.getCount(); i++) {
		ExtraIconGroup *group = groups[i];
		for (int j = 0; j < group->items.getCount(); j++) {
			if (extra == group->items[j])
				return group;
		}
	}
	return NULL;
}

void RebuildListsBasedOnGroups(LIST<ExtraIconGroup> &groups)
{
	extraIconsByHandle.destroy();

	for (int i=0; i < registeredExtraIcons.getCount(); i++)
		extraIconsByHandle.insert(registeredExtraIcons[i]);

	for (int k=0; k < extraIconsBySlot.getCount(); k++) {
		ExtraIcon *extra = extraIconsBySlot[k];
		if (extra->getType() == EXTRAICON_TYPE_GROUP)
			delete extra;
	}
	extraIconsBySlot.destroy();

	for (int i=0; i < groups.getCount(); i++) {
		ExtraIconGroup *group = groups[i];

		for (int j = 0; j < group->items.getCount(); j++)
			extraIconsByHandle.put(group->items[j]->getID()-1, group);

		extraIconsBySlot.insert(group);
	}

	for (int k=0; k < extraIconsByHandle.getCount(); k++) {
		ExtraIcon *extra = extraIconsByHandle[k];
		if (extra->getType() != EXTRAICON_TYPE_GROUP)
			extraIconsBySlot.insert(extra);
	}
}

///////////////////////////////////////////////////////////////////////////////

void KillModuleExtraIcons(int hLangpack)
{
	LIST<ExtraIcon> arDeleted(1);

	for (int i=registeredExtraIcons.getCount()-1; i >= 0; i--) {
		BaseExtraIcon *p = registeredExtraIcons[i];
		if (p->hLangpack == hLangpack) {
			registeredExtraIcons.remove(i);
			arDeleted.insert(p);
		}
	}

	if (arDeleted.getCount() == 0)
		return;

	LIST<ExtraIconGroup> groups(1);
	LoadGroups(groups);
	RebuildListsBasedOnGroups(groups);

	for (int k=0; k < arDeleted.getCount(); k++)
		delete arDeleted[k];
}

///////////////////////////////////////////////////////////////////////////////

int ClistExtraListRebuild(WPARAM, LPARAM)
{
	clistRebuildAlreadyCalled = TRUE;

	ResetIcons();

	for (int i=0; i < extraIconsBySlot.getCount(); i++)
		extraIconsBySlot[i]->rebuildIcons();

	return 0;
}

int ClistExtraImageApply(WPARAM hContact, LPARAM)
{
	if (hContact == NULL)
		return 0;

	clistApplyAlreadyCalled = TRUE;

	for (int i=0; i < extraIconsBySlot.getCount(); i++)
		extraIconsBySlot[i]->applyIcon(hContact);

	return 0;
}

int ClistExtraClick(WPARAM hContact, LPARAM lParam)
{
	if (hContact == NULL)
		return 0;

	int clistSlot = (int)lParam;

	for (int i=0; i < extraIconsBySlot.getCount(); i++) {
		ExtraIcon *extra = extraIconsBySlot[i];
		if (ConvertToClistSlot(extra->getSlot()) == clistSlot) {
			extra->onClick(hContact);
			break;
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Extra image list functions

HANDLE hEventExtraImageListRebuilding, hEventExtraImageApplying, hEventExtraClick;

static bool bImageCreated = false;
static int g_mutex_bSetAllExtraIconsCycle = 0;
static HIMAGELIST hExtraImageList;

HANDLE ExtraIcon_Add(HICON hIcon)
{
	if (hExtraImageList == 0 || hIcon == 0)
		return INVALID_HANDLE_VALUE;

	int res = ImageList_AddIcon(hExtraImageList, hIcon);
	return (res > 0xFFFE) ? INVALID_HANDLE_VALUE : (HANDLE)res;
}

void fnReloadExtraIcons()
{
	SendMessage(cli.hwndContactTree, CLM_SETEXTRASPACE, db_get_b(NULL,"CLUI","ExtraColumnSpace",18), 0);
	SendMessage(cli.hwndContactTree, CLM_SETEXTRAIMAGELIST, 0, 0);

	if (hExtraImageList)
		ImageList_Destroy(hExtraImageList);

	hExtraImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,1,256);

	SendMessage(cli.hwndContactTree, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)hExtraImageList);
	SendMessage(cli.hwndContactTree, CLM_SETEXTRACOLUMNS, EXTRA_ICON_COUNT, 0);
	NotifyEventHooks(hEventExtraImageListRebuilding,0,0);
	bImageCreated = true;
}

void fnSetAllExtraIcons(MCONTACT hContact)
{
	if (cli.hwndContactTree == 0)
		return;

	g_mutex_bSetAllExtraIconsCycle = 1;
	bool hcontgiven = (hContact != 0);

	if (!bImageCreated)
		cli.pfnReloadExtraIcons();

	SendMessage(cli.hwndContactTree, CLM_SETEXTRACOLUMNS, EXTRA_ICON_COUNT, 0);

	if (hContact == NULL)
		hContact = db_find_first();

	for (; hContact; hContact = db_find_next(hContact)) {
		ClcCacheEntry* pdnce = (ClcCacheEntry*)cli.pfnGetCacheEntry(hContact);
		if (pdnce == NULL)
			continue;

		NotifyEventHooks(hEventExtraImageApplying, hContact, 0);
		if (hcontgiven) break;
		Sleep(0);
	}

	g_mutex_bSetAllExtraIconsCycle = 0;
	cli.pfnInvalidateRect(cli.hwndContactTree, NULL, FALSE);
	Sleep(0);
}

///////////////////////////////////////////////////////////////////////////////
// Services

INT_PTR ExtraIcon_Register(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0)
		return 0;

	EXTRAICON_INFO *ei = (EXTRAICON_INFO *)wParam;
	if (ei->cbSize < sizeof(EXTRAICON_INFO))
		return 0;
	if (ei->type != EXTRAICON_TYPE_CALLBACK && ei->type != EXTRAICON_TYPE_ICOLIB)
		return 0;
	if (IsEmpty(ei->name) || IsEmpty(ei->description))
		return 0;
	if (ei->type == EXTRAICON_TYPE_CALLBACK && (ei->ApplyIcon == NULL || ei->RebuildIcons == NULL))
		return 0;

	ptrT tszDesc(mir_a2t(ei->description));
	TCHAR *desc = TranslateTH(lParam, tszDesc);

	BaseExtraIcon *extra = GetExtraIconByName(ei->name);
	if (extra != NULL) {
		if (ei->type != extra->getType() || ei->type != EXTRAICON_TYPE_ICOLIB)
			return 0;

		// Found one, now merge it
		if (_tcsicmp(extra->getDescription(), desc)) {
			CMString newDesc = extra->getDescription();
			newDesc += _T(" / ");
			newDesc += desc;
			extra->setDescription(newDesc.c_str());
		}

		if (!IsEmpty(ei->descIcon))
			extra->setDescIcon(ei->descIcon);

		if (ei->OnClick != NULL)
			extra->setOnClick(ei->OnClick, ei->onClickParam);

		if (extra->getSlot() > 0) {
			if (clistRebuildAlreadyCalled)
				extra->rebuildIcons();
			if (clistApplyAlreadyCalled)
				extraIconsByHandle[extra->getID() - 1]->applyIcons();
		}

		return extra->getID();
	}

	int id = registeredExtraIcons.getCount() + 1;

	switch (ei->type) {
	case EXTRAICON_TYPE_CALLBACK:
		extra = new CallbackExtraIcon(id, ei->name, desc, ei->descIcon == NULL ? "" : ei->descIcon,
			ei->RebuildIcons, ei->ApplyIcon, ei->OnClick, ei->onClickParam);
		break;
	case EXTRAICON_TYPE_ICOLIB:
		extra = new IcolibExtraIcon(id, ei->name, desc, ei->descIcon == NULL ? "" : ei->descIcon, ei->OnClick,
			ei->onClickParam);
		break;
	default:
		return 0;
	}

	char setting[512];
	mir_snprintf(setting, SIZEOF(setting), "Position_%s", ei->name);
	extra->setPosition(db_get_w(NULL, MODULE_NAME, setting, 1000));

	mir_snprintf(setting, SIZEOF(setting), "Slot_%s", ei->name);
	int slot = db_get_w(NULL, MODULE_NAME, setting, 1);
	if (slot == (WORD)-1)
		slot = -1;
	extra->setSlot(slot);

	extra->hLangpack = (int)lParam;

	registeredExtraIcons.insert(extra);
	extraIconsByHandle.insert(extra);

	LIST<ExtraIconGroup> groups(1);
	LoadGroups(groups);

	ExtraIconGroup *group = IsInGroup(groups, extra);
	if (group != NULL)
		RebuildListsBasedOnGroups(groups);
	else {
		for (int i = 0; i < groups.getCount(); i++)
			delete groups[i];

		extraIconsBySlot.insert(extra);
	}

	if (slot >= 0 || group != NULL) {
		if (clistRebuildAlreadyCalled)
			extra->rebuildIcons();

		slot = 0;
		for (int i = 0; i < extraIconsBySlot.getCount(); i++) {
			ExtraIcon *ex = extraIconsBySlot[i];
			if (ex->getSlot() < 0)
				continue;

			int oldSlot = ex->getSlot();
			ex->setSlot(slot++);

			if (clistApplyAlreadyCalled && (ex == group || ex == extra || oldSlot != slot))
				extra->applyIcons();
		}
	}

	return id;
}

INT_PTR ExtraIcon_SetIcon(WPARAM wParam, LPARAM)
{
	if (wParam == 0)
		return -1;

	EXTRAICON *ei = (EXTRAICON*)wParam;
	if (ei->cbSize < sizeof(EXTRAICON) || ei->hExtraIcon == NULL || ei->hContact == NULL)
		return -1;

	ExtraIcon *extra = GetExtraIcon(ei->hExtraIcon);
	if (extra == NULL)
		return -1;

	return extra->setIcon((int)ei->hExtraIcon, ei->hContact, ei->hImage);
}

INT_PTR ExtraIcon_SetIconByName(WPARAM wParam, LPARAM)
{
	if (wParam == 0)
		return -1;

	EXTRAICON *ei = (EXTRAICON*)wParam;
	if (ei->cbSize < sizeof(EXTRAICON) || ei->hExtraIcon == NULL || ei->hContact == NULL)
		return -1;

	ExtraIcon *extra = GetExtraIcon(ei->hExtraIcon);
	if (extra == NULL)
		return -1;

	return extra->setIconByName((int)ei->hExtraIcon, ei->hContact, ei->icoName);
}

static INT_PTR svcExtraIcon_Add(WPARAM wParam, LPARAM)
{
	return (INT_PTR)ExtraIcon_Add((HICON)wParam);
}

///////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Chat activity"), "ChatActivity",  IDI_CHAT   },
	{ LPGEN("Male"),          "gender_male",   IDI_MALE   },
	{ LPGEN("Female"),		  "gender_female", IDI_FEMALE }
};

void LoadExtraIconsModule()
{
	DWORD ret = CallService(MS_CLUI_GETCAPS, CLUICAPS_FLAGS2, 0);
	clistFirstSlot = HIWORD(ret);
	clistSlotCount = LOWORD(ret);

	// Services
	CreateServiceFunction(MS_EXTRAICON_REGISTER, ExtraIcon_Register);
	CreateServiceFunction(MS_EXTRAICON_SET_ICON, ExtraIcon_SetIcon);
	CreateServiceFunction(MS_EXTRAICON_SET_ICON_BY_NAME, &ExtraIcon_SetIconByName);

	CreateServiceFunction(MS_CLIST_EXTRA_ADD_ICON, svcExtraIcon_Add);

	hEventExtraClick = CreateHookableEvent(ME_CLIST_EXTRA_CLICK);
	hEventExtraImageApplying = CreateHookableEvent(ME_CLIST_EXTRA_IMAGE_APPLY);
	hEventExtraImageListRebuilding = CreateHookableEvent(ME_CLIST_EXTRA_LIST_REBUILD);

	// Icons
	Icon_Register(NULL, LPGEN("Contact list"), iconList, SIZEOF(iconList));

	// Hooks
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	HookEvent(ME_CLIST_EXTRA_LIST_REBUILD, ClistExtraListRebuild);
	HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, ClistExtraImageApply);
	HookEvent(ME_CLIST_EXTRA_CLICK, ClistExtraClick);

	DefaultExtraIcons_Load();
}

void UnloadExtraIconsModule(void)
{
	for (int i=0; i < registeredExtraIcons.getCount(); i++)
		delete registeredExtraIcons[i];
}
