/*

Copyright (C) 2009 Ricardo Pescuma Domenecci
Copyright (C) 2012-22 Miranda NG team

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

#include "stdafx.h"

#include "m_cluiframes.h"

#include "extraicons.h"
#include "usedIcons.h"
#include "clc.h"

// Prototypes ///////////////////////////////////////////////////////////////////////////

int SortFunc(const ExtraIcon *p1, const ExtraIcon *p2)
{
	int ret = p1->getPosition() - p2->getPosition();
	if (ret != 0)
		return ret;

	return p1->getID() - p2->getID();
}

LIST<ExtraIcon> extraIconsBySlot(10, SortFunc);
LIST<BaseExtraIcon> registeredExtraIcons(10, PtrKeySortT);

static bool clistRebuildAlreadyCalled = false, clistApplyAlreadyCalled = false;

// Functions ////////////////////////////////////////////////////////////////////////////

int InitOptionsCallback(WPARAM wParam, LPARAM lParam);

int ConvertToClistSlot(int slot)
{
	if (slot < 0)
		return slot;

	return slot + 1;
}

int ExtraImage_ExtraIDToColumnNum(int extra)
{
	return (extra < 1 || extra > EXTRA_ICON_COUNT) ? -1 : extra - 1;
}

int Clist_SetExtraIcon(MCONTACT hContact, int slot, HANDLE hImage)
{
	if (g_clistApi.hwndContactTree == nullptr)
		return -1;

	int icol = ExtraImage_ExtraIDToColumnNum(ConvertToClistSlot(slot));
	if (icol == -1)
		return -1;

	SendMessage(g_clistApi.hwndContactTree, CLM_SETEXTRAIMAGE, hContact, MAKELPARAM(icol, hImage));
	return 0;
}

BaseExtraIcon* GetExtraIconByName(const char *name)
{
	for (auto &extra : registeredExtraIcons)
		if (mir_strcmp(name, extra->getName()) == 0)
			return extra;

	return nullptr;
}

static void LoadGroups(LIST<ExtraIconGroup> &groups)
{
	int count = db_get_w(0, EI_MODULE_NAME "Groups", "Count", 0);
	for (int i = 0; i < count; i++) {
		char setting[512];
		mir_snprintf(setting, "%d_count", i);
		unsigned int items = db_get_w(0, EI_MODULE_NAME "Groups", setting, 0);
		if (items < 1)
			continue;

		mir_snprintf(setting, "__group_%d", i);
		ExtraIconGroup *group = new ExtraIconGroup(setting);

		for (unsigned int j = 0; j < items; j++) {
			mir_snprintf(setting, "%d_%d", i, j);
			ptrA szIconName(db_get_sa(0, EI_MODULE_NAME "Groups", setting));
			if (IsEmpty(szIconName))
				continue;

			BaseExtraIcon *extra = GetExtraIconByName(szIconName);
			if (extra == nullptr)
				continue;

			group->m_items.insert(extra);
			if (extra->getSlot() >= 0)
				group->setSlot(extra->getSlot());
		}

		if (group->m_items.getCount() < 2) {
			delete group;
			continue;
		}

		groups.insert(group);
	}
}

static ExtraIconGroup* IsInGroup(LIST<ExtraIconGroup> &groups, BaseExtraIcon *extra)
{
	for (auto &group : groups)
		for (auto &it : group->m_items)
			if (extra == it)
				return group;

	return nullptr;
}

void RebuildListsBasedOnGroups(LIST<ExtraIconGroup> &groups)
{
	for (auto &extra : registeredExtraIcons)
		extra->setParent(nullptr);

	for (auto &extra : extraIconsBySlot)
		if (extra->getType() == EXTRAICON_TYPE_GROUP)
			delete extra;
	extraIconsBySlot.destroy();

	for (auto &group : groups) {
		for (auto &it : group->m_items)
			it->setParent(group);

		extraIconsBySlot.insert(group);
	}

	for (auto &extra : registeredExtraIcons)
		if (extra->getParent() == nullptr)
			extraIconsBySlot.insert(extra);
}

///////////////////////////////////////////////////////////////////////////////

static void ResetSlots(BaseExtraIcon *extra, ExtraIconGroup *group, int iOldSlot = -1)
{
	int slot = 0, oldMaxSlot = -1;
	for (auto &ex : extraIconsBySlot) {
		if (ex->getSlot() < 0)
			continue;

		int oldSlot = ex->getSlot();
		if (oldSlot > oldMaxSlot)
			oldMaxSlot = oldSlot+1;

		ex->setSlot(slot++);

		if (clistApplyAlreadyCalled && (ex == group || ex == extra || oldSlot != slot))
			ex->applyIcons();
	}

	if (iOldSlot > oldMaxSlot)
		oldMaxSlot = iOldSlot + 1;

	// slots were freed, we need to clear one or more items
	if (extra == nullptr)
		for (int i = slot; i < oldMaxSlot; i++)
			for (auto &hContact : Contacts())
				Clist_SetExtraIcon(hContact, i, INVALID_HANDLE_VALUE);

	if (!Miranda_IsTerminated()) {
		Clist_InitAutoRebuild(g_clistApi.hwndContactTree);
		eiOptionsRefresh();
	}
}

void KillModuleExtraIcons(CMPluginBase *pPlugin)
{
	LIST<ExtraIcon> arIcons(1);

	for (auto &it : registeredExtraIcons.rev_iter())
		if (it->m_pPlugin == pPlugin) {
			arIcons.insert(it);
			registeredExtraIcons.removeItem(&it);
		}

	if (arIcons.getCount() == 0)
		return;

	int iOldSlot = -1;
	for (auto &it : arIcons)
		if (it->getSlot() > iOldSlot)
			iOldSlot = it->getSlot() + 1;

	LIST<ExtraIconGroup> groups(1);
	LoadGroups(groups);
	RebuildListsBasedOnGroups(groups);
	ResetSlots(0, 0, iOldSlot);

	for (auto &it : arIcons)
		delete it;
}

///////////////////////////////////////////////////////////////////////////////

int ClistExtraListRebuild(WPARAM, LPARAM)
{
	clistRebuildAlreadyCalled = true;

	ResetIcons();

	for (auto &it : extraIconsBySlot)
		it->rebuildIcons();

	return 0;
}

int ClistExtraImageApply(WPARAM hContact, LPARAM)
{
	if (hContact == 0)
		return 0;

	clistApplyAlreadyCalled = true;

	for (auto &it : extraIconsBySlot)
		it->doApply(hContact);

	return 0;
}

int ClistExtraClick(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0)
		return 0;

	int clistSlot = (int)lParam;

	for (auto &extra : extraIconsBySlot) {
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
static HIMAGELIST hExtraImageList = nullptr;

MIR_APP_DLL(HANDLE) ExtraIcon_AddIcon(HICON hIcon)
{
	if (hExtraImageList == nullptr || hIcon == nullptr)
		return INVALID_HANDLE_VALUE;

	int res = ImageList_AddIcon(hExtraImageList, hIcon);
	return (res > 0xFFFE) ? INVALID_HANDLE_VALUE : (HANDLE)res;
}

MIR_APP_DLL(void) ExtraIcon_Reload()
{
	SendMessage(g_clistApi.hwndContactTree, CLM_SETEXTRASPACE, db_get_b(0, "CLUI", "ExtraColumnSpace", 18), 0);
	SendMessage(g_clistApi.hwndContactTree, CLM_SETEXTRAIMAGELIST, 0, 0);

	if (hExtraImageList)
		ImageList_Destroy(hExtraImageList);

	hExtraImageList = ImageList_Create(g_iIconSX, g_iIconSY, ILC_COLOR32 | ILC_MASK, 1, 256);

	SendMessage(g_clistApi.hwndContactTree, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)hExtraImageList);
	SendMessage(g_clistApi.hwndContactTree, CLM_SETEXTRACOLUMNS, EXTRA_ICON_COUNT, 0);
	NotifyEventHooks(hEventExtraImageListRebuilding, 0, 0);
	bImageCreated = true;
}

MIR_APP_DLL(void) ExtraIcon_SetAll(MCONTACT hContact)
{
	if (g_clistApi.hwndContactTree == nullptr)
		return;

	if (!bImageCreated)
		ExtraIcon_Reload();

	SendMessage(g_clistApi.hwndContactTree, CLM_SETEXTRACOLUMNS, EXTRA_ICON_COUNT, 0);

	if (hContact == 0) {
		for (auto &it : Contacts())
			NotifyEventHooks(hEventExtraImageApplying, it, 0);
	}
	else NotifyEventHooks(hEventExtraImageApplying, hContact, 0);
	
	g_clistApi.pfnInvalidateRect(g_clistApi.hwndContactTree, nullptr, FALSE);
}

///////////////////////////////////////////////////////////////////////////////
// external functions

static void EI_PostCreate(BaseExtraIcon *extra, int flags)
{
	char setting[512];
	mir_snprintf(setting, "Position_%s", extra->getName());
	extra->setPosition(db_get_w(0, EI_MODULE_NAME, setting, 1000));

	mir_snprintf(setting, "Slot_%s", extra->getName());
	int slot = db_get_w(0, EI_MODULE_NAME, setting, -100);
	if (slot == EMPTY_EXTRA_ICON)
		slot = -1;
	else if (slot == -100) {
		if (flags & EIF_DISABLED_BY_DEFAULT) {
			db_set_w(0, EI_MODULE_NAME, setting, EMPTY_EXTRA_ICON);
			slot = -1;
		}
		else slot = 1;
	}
	extra->setSlot(slot);

	registeredExtraIcons.insert(extra);

	LIST<ExtraIconGroup> groups(1);
	LoadGroups(groups);

	ExtraIconGroup *group = IsInGroup(groups, extra);
	if (group != nullptr)
		RebuildListsBasedOnGroups(groups);
	else {
		for (auto &it : groups)
			delete it;

		extraIconsBySlot.insert(extra);
	}

	if (slot >= 0 || group != nullptr) {
		if (clistRebuildAlreadyCalled)
			extra->rebuildIcons();

		ResetSlots(extra, group);
	}
}

EXTERN_C MIR_APP_DLL(HANDLE) ExtraIcon_RegisterCallback(const char *name, const char *description, HANDLE descIcon,
	MIRANDAHOOK RebuildIcons, MIRANDAHOOK ApplyIcon, MIRANDAHOOKPARAM OnClick, LPARAM onClickParam, int flags)
{
	// EXTRAICON_TYPE_CALLBACK 
	if (IsEmpty(name) || IsEmpty(description))
		return nullptr;

	if (ApplyIcon == nullptr || RebuildIcons == nullptr)
		return nullptr;

	// no way to merge
	if (GetExtraIconByName(name) != nullptr)
		return nullptr;

	ptrW tszDesc(mir_a2u(description));

	BaseExtraIcon *extra = new CallbackExtraIcon(name, tszDesc, descIcon, RebuildIcons, ApplyIcon, OnClick, onClickParam);
	extra->m_pPlugin = &GetPluginByInstance(GetInstByAddress(RebuildIcons));
	EI_PostCreate(extra, flags);
	return extra;
}

EXTERN_C MIR_APP_DLL(HANDLE) ExtraIcon_RegisterIcolib(const char *name, const char *description, HANDLE descIcon, MIRANDAHOOKPARAM OnClick, LPARAM onClickParam, int flags)
{
	if (IsEmpty(name) || IsEmpty(description))
		return nullptr;

	ptrW tszDesc(mir_a2u(description));

	BaseExtraIcon *extra = GetExtraIconByName(name);
	if (extra != nullptr) {
		if (extra->getType() != EXTRAICON_TYPE_ICOLIB)
			return nullptr;

		// Found one, now merge it
		if (descIcon)
			extra->setDescIcon(descIcon);

		if (OnClick != nullptr)
			extra->setOnClick(OnClick, onClickParam);

		if (extra->getSlot() > 0) {
			if (clistRebuildAlreadyCalled)
				extra->rebuildIcons();
			if (clistApplyAlreadyCalled)
				extra->applyIcons();
		}
	}
	else {
		extra = new IcolibExtraIcon(name, tszDesc, descIcon, OnClick, onClickParam);
		extra->m_pPlugin = &GetPluginByInstance(GetInstByAddress((void*)name));
		EI_PostCreate(extra, flags);
	}
	
	return extra;
}

///////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) ExtraIcon_SetIcon(HANDLE hExtraIcon, MCONTACT hContact, HANDLE hImage)
{
	if (hExtraIcon == nullptr || hContact == 0)
		return -1;

	BaseExtraIcon *extra = registeredExtraIcons.find((BaseExtraIcon*)hExtraIcon);
	if (extra == nullptr)
		return -1;

	if (extra->getParent())
		return extra->getParent()->internalSetIcon(extra, hContact, hImage, false);

	return extra->setIcon(hContact, hImage);
}

MIR_APP_DLL(int) ExtraIcon_SetIconByName(HANDLE hExtraIcon, MCONTACT hContact, const char *icoName)
{
	if (hExtraIcon == nullptr || hContact == 0)
		return -1;

	BaseExtraIcon *extra = registeredExtraIcons.find((BaseExtraIcon*)hExtraIcon);
	if (extra == nullptr)
		return -1;

	if (extra->getParent())
		return extra->getParent()->internalSetIcon(extra, hContact, (HANDLE)icoName, true);

	return extra->setIconByName(hContact, icoName);
}

MIR_APP_DLL(int) ExtraIcon_Clear(HANDLE hExtraIcon, MCONTACT hContact)
{
	if (hExtraIcon == nullptr || hContact == 0)
		return -1;

	BaseExtraIcon *extra = registeredExtraIcons.find((BaseExtraIcon*)hExtraIcon);
	if (extra == nullptr)
		return -1;

	if (extra->getParent())
		return extra->getParent()->internalSetIcon(extra, hContact, nullptr, false);

	return extra->setIcon(hContact, nullptr);
}

///////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Chat activity"), "ChatActivity",  IDI_CHAT     },
	{ LPGEN("Mute chat"),     "ChatMute",      IDI_OFF      },
	{ LPGEN("Male"),          "gender_male",   IDI_MALE     },
	{ LPGEN("Female"),		  "gender_female", IDI_FEMALE   },
	{ LPGEN("Database"),      "database",      IDI_DATABASE },
};

void LoadExtraIconsModule()
{
	// Events
	hEventExtraClick = CreateHookableEvent(ME_CLIST_EXTRA_CLICK);
	hEventExtraImageApplying = CreateHookableEvent(ME_CLIST_EXTRA_IMAGE_APPLY);
	hEventExtraImageListRebuilding = CreateHookableEvent(ME_CLIST_EXTRA_LIST_REBUILD);

	// Icons
	g_plugin.registerIcon(LPGEN("Contact list"), iconList);

	// Hooks
	HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);

	HookEvent(ME_CLIST_EXTRA_LIST_REBUILD, ClistExtraListRebuild);
	HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, ClistExtraImageApply);
	HookEvent(ME_CLIST_EXTRA_CLICK, ClistExtraClick);

	DefaultExtraIcons_Load();
}

void UnloadExtraIconsModule(void)
{
	for (auto &extra : extraIconsBySlot)
		if (extra->getType() == EXTRAICON_TYPE_GROUP)
			delete extra;

	for (auto &it : registeredExtraIcons)
		delete it;

	if (hExtraImageList) {
		ImageList_Destroy(hExtraImageList);
		hExtraImageList = nullptr;
	}
}
