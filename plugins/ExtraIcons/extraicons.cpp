/*
 Copyright (C) 2009 Ricardo Pescuma Domenecci

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

#include "commons.h"

// Prototypes ///////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	0, //doesn't replace anything built-in
	{ 0x112f7d30, 0xcd19, 0x4c74, { 0xa0, 0x3b, 0xbf, 0xbb, 0x76, 0xb7, 0x5b, 0xc4 } } // {112F7D30-CD19-4c74-A03BBFBB76B75BC4}
};

HINSTANCE hInst;

int hLangpack;

vector<HANDLE> hHooks;
vector<HANDLE> hServices;
vector<BaseExtraIcon*> registeredExtraIcons;
vector<ExtraIcon*> extraIconsByHandle;
vector<ExtraIcon*> extraIconsBySlot;

char *metacontacts_proto = NULL;
BOOL clistRebuildAlreadyCalled = FALSE;
BOOL clistApplyAlreadyCalled = FALSE;

int clistFirstSlot = 0;
int clistSlotCount = 0;

int ModulesLoaded(WPARAM wParam, LPARAM lParam);
int PreShutdown(WPARAM wParam, LPARAM lParam);
int IconsChanged(WPARAM wParam, LPARAM lParam);
int ClistExtraListRebuild(WPARAM wParam, LPARAM lParam);
int ClistExtraImageApply(WPARAM wParam, LPARAM lParam);
int ClistExtraClick(WPARAM wParam, LPARAM lParam);

INT_PTR ExtraIcon_Register(WPARAM wParam, LPARAM lParam);
INT_PTR ExtraIcon_SetIcon(WPARAM wParam, LPARAM lParam);

// Functions ////////////////////////////////////////////////////////////////////////////

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static const MUUID interfaces[] = { MIID_EXTRAICONSSERVICE, MIID_LAST };
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

extern "C" int __declspec(dllexport) Load(void)
{

	mir_getLP(&pluginInfo);

	DWORD ret = CallService(MS_CLUI_GETCAPS, CLUICAPS_FLAGS2, 0);
	clistFirstSlot = HIWORD(ret);
	clistSlotCount = LOWORD(ret);


	// Icons
	IcoLib_Register("AlwaysVis", "Contact List", "Always Visible", IDI_ALWAYSVIS);
	IcoLib_Register("NeverVis", "Contact List", "Never Visible", IDI_NEVERVIS);
	IcoLib_Register("ChatActivity", "Contact List", "Chat Activity", IDI_CHAT);
	IcoLib_Register("gender_male", "Contact List", "Male", IDI_MALE);
	IcoLib_Register("gender_female", "Contact List", "Female", IDI_FEMALE);


	// Hooks
	hHooks.push_back(HookEvent(ME_SYSTEM_MODULESLOADED, &ModulesLoaded));
	hHooks.push_back(HookEvent(ME_SYSTEM_PRESHUTDOWN, &PreShutdown));
	hHooks.push_back(HookEvent(ME_CLIST_EXTRA_LIST_REBUILD, &ClistExtraListRebuild));
	hHooks.push_back(HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, &ClistExtraImageApply));
	hHooks.push_back(HookEvent(ME_CLIST_EXTRA_CLICK, &ClistExtraClick));


	// Services
	hServices.push_back(CreateServiceFunction(MS_EXTRAICON_REGISTER, &ExtraIcon_Register));
	hServices.push_back(CreateServiceFunction(MS_EXTRAICON_SET_ICON, &ExtraIcon_SetIcon));

	DefaultExtraIcons_Load();

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}

// Called when all the modules are loaded
int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	if (ServiceExists(MS_MC_GETPROTOCOLNAME))
		metacontacts_proto = (char *) CallService(MS_MC_GETPROTOCOLNAME, 0, 0);


	// add our modules to the KnownModules list
	CallService("DBEditorpp/RegisterSingleModule", (WPARAM) MODULE_NAME, 0);
	CallService("DBEditorpp/RegisterSingleModule", (WPARAM) MODULE_NAME "Groups", 0);


	// updater plugin support
	if (ServiceExists(MS_UPDATE_REGISTER))
	{
		Update upd = { 0 };
		char szCurrentVersion[30];

		upd.cbSize = sizeof(upd);
		upd.szComponentName = pluginInfo.shortName;

		upd.szUpdateURL = UPDATER_AUTOREGISTER;

		upd.szBetaVersionURL = "http://pescuma.org/miranda/extraicons_version.txt";
		upd.szBetaChangelogURL = "http://pescuma.org/miranda/extraicons#Changelog";
		upd.pbBetaVersionPrefix = (BYTE *) "Extra Icons Service ";
		upd.cpbBetaVersionPrefix = (int)strlen((char *) upd.pbBetaVersionPrefix);
		upd.szBetaUpdateURL = "http://pescuma.org/miranda/extraicons.zip";

		upd.pbVersion = (BYTE *) CreateVersionStringPluginEx((PLUGININFOEX*) &pluginInfo, szCurrentVersion);
		upd.cpbVersion = (int)strlen((char *) upd.pbVersion);

		CallService(MS_UPDATE_REGISTER, 0, (LPARAM)&upd);
	}

	hHooks.push_back(HookEvent(ME_SKIN2_ICONSCHANGED, &IconsChanged));

	InitOptions();

	return 0;
}

int IconsChanged(WPARAM wParam, LPARAM lParam)
{
	return 0;
}

int PreShutdown(WPARAM wParam, LPARAM lParam)
{
	DefaultExtraIcons_Unload();

	unsigned int i;
	for (i = 0; i < hServices.size(); i++)
		DestroyServiceFunction(hServices[i]);

	for (i = 0; i < hHooks.size(); i++)
		UnhookEvent(hHooks[i]);

	DeInitOptions();

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

int Clist_SetExtraIcon(HANDLE hContact, int slot, HANDLE hImage)
{
	IconExtraColumn iec = { 0 };
	iec.cbSize = sizeof(iec);
	iec.ColumnType = ConvertToClistSlot(slot);
	iec.hImage = (hImage == NULL ? (HANDLE) -1 : hImage);

	return CallService(MS_CLIST_EXTRA_SET_ICON, (WPARAM) hContact, (LPARAM) &iec);
}

ExtraIcon * GetExtraIcon(HANDLE id)
{
	unsigned int i = (int) id;

	if (i < 1 || i > extraIconsByHandle.size())
		return NULL;

	return extraIconsByHandle[i - 1];
}

ExtraIcon * GetExtraIconBySlot(int slot)
{
	for (unsigned int i = 0; i < extraIconsBySlot.size(); ++i)
	{
		ExtraIcon *extra = extraIconsBySlot[i];
		if (extra->getSlot() == slot)
			return extra;
	}
	return NULL;
}

BaseExtraIcon * GetExtraIconByName(const char *name)
{
	for (unsigned int i = 0; i < registeredExtraIcons.size(); ++i)
	{
		BaseExtraIcon *extra = registeredExtraIcons[i];
		if (strcmp(name, extra->getName()) == 0)
			return extra;
	}
	return NULL;
}

static void LoadGroups(vector<ExtraIconGroup *> &groups)
{
	unsigned int count = DBGetContactSettingWord(NULL, MODULE_NAME "Groups", "Count", 0);
	for (unsigned int i = 0; i < count; ++i)
	{
		char setting[512];
		mir_snprintf(setting, MAX_REGS(setting), "%d_count", i);
		unsigned int items = DBGetContactSettingWord(NULL, MODULE_NAME "Groups", setting, 0);
		if (items < 1)
			continue;

		mir_snprintf(setting, MAX_REGS(setting), "__group_%d", i);
		ExtraIconGroup *group = new ExtraIconGroup(setting);

		for (unsigned int j = 0; j < items; ++j)
		{
			mir_snprintf(setting, MAX_REGS(setting), "%d_%d", i, j);

			DBVARIANT dbv = { 0 };
			if (!DBGetContactSettingString(NULL, MODULE_NAME "Groups", setting, &dbv))
			{
				if (!IsEmpty(dbv.pszVal))
				{
					BaseExtraIcon *extra = GetExtraIconByName(dbv.pszVal);
					if (extra != NULL)
					{
						group->items.push_back(extra);

						if (extra->getSlot() >= 0)
							group->setSlot(extra->getSlot());
					}
				}
				DBFreeVariant(&dbv);
			}
		}

		if (group->items.size() < 2)
		{
			delete group;
			continue;
		}

		groups.push_back(group);
	}
}

static ExtraIconGroup * IsInGroup(vector<ExtraIconGroup *> &groups, BaseExtraIcon *extra)
{
	for (unsigned int i = 0; i < groups.size(); ++i)
	{
		ExtraIconGroup *group = groups[i];
		for (unsigned int j = 0; j < group->items.size(); ++j)
		{
			if (extra == group->items[j])
				return group;
		}
	}
	return NULL;
}

struct compareFunc : std::binary_function<const ExtraIcon *, const ExtraIcon *, bool>
{
	bool operator()(const ExtraIcon * one, const ExtraIcon * two) const
	{
		return *one < *two;
	}
};

void RebuildListsBasedOnGroups(vector<ExtraIconGroup *> &groups)
{
	unsigned int i;
	for (i = 0; i < extraIconsByHandle.size(); ++i)
		extraIconsByHandle[i] = registeredExtraIcons[i];

	for (i = 0; i < extraIconsBySlot.size(); ++i)
	{
		ExtraIcon *extra = extraIconsBySlot[i];
		if (extra->getType() != EXTRAICON_TYPE_GROUP)
			continue;

		delete extra;
	}
	extraIconsBySlot.clear();

	for (i = 0; i < groups.size(); ++i)
	{
		ExtraIconGroup *group = groups[i];

		for (unsigned int j = 0; j < group->items.size(); ++j)
			extraIconsByHandle[group->items[j]->getID() - 1] = group;

		extraIconsBySlot.push_back(group);
	}

	for (i = 0; i < extraIconsByHandle.size(); ++i)
	{
		ExtraIcon *extra = extraIconsByHandle[i];
		if (extra->getType() != EXTRAICON_TYPE_GROUP)
			extraIconsBySlot.push_back(extra);
	}

	std::sort(extraIconsBySlot.begin(), extraIconsBySlot.end(), compareFunc());
}

INT_PTR ExtraIcon_Register(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0)
		return 0;

	EXTRAICON_INFO *ei = (EXTRAICON_INFO *) wParam;
	if (ei->cbSize < (int) sizeof(EXTRAICON_INFO))
		return 0;
	if (ei->type != EXTRAICON_TYPE_CALLBACK && ei->type != EXTRAICON_TYPE_ICOLIB)
		return 0;
	if (IsEmpty(ei->name) || IsEmpty(ei->description))
		return 0;
	if (ei->type == EXTRAICON_TYPE_CALLBACK && (ei->ApplyIcon == NULL || ei->RebuildIcons == NULL))
		return 0;

	const char *desc = Translate(ei->description);

	BaseExtraIcon *extra = GetExtraIconByName(ei->name);
	if (extra != NULL)
	{
		if (ei->type != extra->getType() || ei->type != EXTRAICON_TYPE_ICOLIB)
			return 0;

		// Found one, now merge it

		if (_stricmp(extra->getDescription(), desc))
		{
			string newDesc = extra->getDescription();
			newDesc += " / ";
			newDesc += desc;
			extra->setDescription(newDesc.c_str());
		}

		if (!IsEmpty(ei->descIcon))
			extra->setDescIcon(ei->descIcon);

		if (ei->OnClick != NULL)
			extra->setOnClick(ei->OnClick, ei->onClickParam);

		if (extra->getSlot() > 0)
		{
			if (clistRebuildAlreadyCalled)
				extra->rebuildIcons();
			if (clistApplyAlreadyCalled)
				extraIconsByHandle[extra->getID() - 1]->applyIcons();
		}

		return extra->getID();
	}

	int id = (int)registeredExtraIcons.size() + 1;

	switch (ei->type)
	{
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
	mir_snprintf(setting, MAX_REGS(setting), "Position_%s", ei->name);
	extra->setPosition(DBGetContactSettingWord(NULL, MODULE_NAME, setting, 1000));

	mir_snprintf(setting, MAX_REGS(setting), "Slot_%s", ei->name);
	int slot = DBGetContactSettingWord(NULL, MODULE_NAME, setting, 1);
	if (slot == (WORD) -1)
		slot = -1;
	extra->setSlot(slot);

	registeredExtraIcons.push_back(extra);
	extraIconsByHandle.push_back(extra);

	vector<ExtraIconGroup *> groups;
	LoadGroups(groups);

	ExtraIconGroup *group = IsInGroup(groups, extra);
	if (group != NULL)
	{
		RebuildListsBasedOnGroups(groups);
	}
	else
	{
		for (unsigned int i = 0; i < groups.size(); ++i)
			delete groups[i];

		extraIconsBySlot.push_back(extra);
		std::sort(extraIconsBySlot.begin(), extraIconsBySlot.end(), compareFunc());
	}

	if (slot >= 0 || group != NULL)
	{
		if (clistRebuildAlreadyCalled)
			extra->rebuildIcons();

		slot = 0;
		for (unsigned int i = 0; i < extraIconsBySlot.size(); ++i)
		{
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

INT_PTR ExtraIcon_SetIcon(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0)
		return -1;

	EXTRAICON *ei = (EXTRAICON *) wParam;
	if (ei->cbSize < (int) sizeof(EXTRAICON))
		return -1;
	if (ei->hExtraIcon == NULL || ei->hContact == NULL)
		return -1;

	ExtraIcon *extra = GetExtraIcon(ei->hExtraIcon);
	if (extra == NULL)
		return -1;

	return extra->setIcon((int) ei->hExtraIcon, ei->hContact, ei->hImage);
}

int ClistExtraListRebuild(WPARAM wParam, LPARAM lParam)
{
	clistRebuildAlreadyCalled = TRUE;

	ResetIcons();

	for (unsigned int i = 0; i < extraIconsBySlot.size(); ++i)
		extraIconsBySlot[i]->rebuildIcons();

	return 0;
}

int ClistExtraImageApply(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return 0;

	clistApplyAlreadyCalled = TRUE;

	for (unsigned int i = 0; i < extraIconsBySlot.size(); ++i)
		extraIconsBySlot[i]->applyIcon(hContact);

	return 0;
}

int ClistExtraClick(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;
	if (hContact == NULL)
		return 0;

	int clistSlot = (int) lParam;

	for (unsigned int i = 0; i < extraIconsBySlot.size(); ++i)
	{
		ExtraIcon *extra = extraIconsBySlot[i];
		if (ConvertToClistSlot(extra->getSlot()) == clistSlot)
		{
			extra->onClick(hContact);
			break;
		}
	}

	return 0;
}
