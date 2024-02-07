/*
Miranda SmileyAdd Plugin
Copyright (C) 2005 - 2011 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2003 - 2004 Rein-Peter de Boer

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

// globals
HANDLE    g_hevOptionsChanged;
HGENMENU  hContactMenuItem;

CMPlugin g_plugin;

static IconItem iconList[] =
{
	{ LPGEN("Button smiley"), "SmileyAdd_ButtonSmiley", IDI_SMILINGICON }
};

void InitServices();

/////////////////////////////////////////////////////////////////////////////////////////

static const PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {BD542BB4-5AE4-4D0E-A435-BA8DBE39607F}
	{0xbd542bb4, 0x5ae4, 0x4d0e, {0xa4, 0x35, 0xba, 0x8d, 0xbe, 0x39, 0x60, 0x7f}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////
// Events

int AccountListChanged(WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT *acc = (PROTOACCOUNT *)lParam;

	switch (wParam) {
	case PRAC_ADDED:
		if (acc != nullptr) {
			const CMStringW &defaultFile = g_SmileyCategories.GetSmileyCategory(L"Standard")->GetFilename();
			g_SmileyCategories.AddAccountAsCategory(acc, defaultFile);
		}
		break;

	case PRAC_CHANGED:
		if (acc != nullptr && acc->szModuleName != nullptr) {
			CMStringW name(acc->szModuleName);
			SmileyCategoryType *smc = g_SmileyCategories.GetSmileyCategory(name);
			if (smc != nullptr) {
				if (acc->tszAccountName)
					name = acc->tszAccountName;
				smc->SetDisplayName(name);
			}
		}
		break;

	case PRAC_REMOVED:
		g_SmileyCategories.DeleteAccountAsCategory(acc);
		break;

	case PRAC_CHECKED:
		if (acc != nullptr) {
			if (acc->bIsEnabled) {
				const CMStringW &defaultFile = g_SmileyCategories.GetSmileyCategory(L"Standard")->GetFilename();
				g_SmileyCategories.AddAccountAsCategory(acc, defaultFile);
			}
			else g_SmileyCategories.DeleteAccountAsCategory(acc);
		}
		break;
	}
	return 0;
}

static int DbSettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (hContact == 0)
		return 0;

	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
	if (cws->value.type == DBVT_DELETED)
		return 0;

	if (strcmp(cws->szSetting, "Transport") == 0) {
		SmileyCategoryType *smc = g_SmileyCategories.GetSmileyCategory(L"Standard");
		if (smc != nullptr)
			g_SmileyCategories.AddContactTransportAsCategory(hContact, smc->GetFilename());
	}
	return 0;
}


static int ReloadColors(WPARAM, LPARAM)
{
	opt.SelWndBkgClr = db_get_dw(0, "SmileyAdd", "SelWndBkgClr", GetSysColor(COLOR_WINDOW));
	return 0;
}

static int ModulesLoaded(WPARAM, LPARAM)
{
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x5ba238de, 0xe16b, 0x4928, 0xa0, 0x70, 0xff, 0x43, 0xf6, 0x1f, 0x16, 0xd4);
	mi.position = 2000070050;
	mi.hIcolibItem = iconList[0].hIcolib;
	mi.name.a = LPGEN("Assign smiley category");
	hContactMenuItem = Menu_AddContactMenuItem(&mi);

	DownloadInit();

	// install hooks if enabled
	InstallDialogBoxHook();

	g_SmileyCategories.AddAllProtocolsAsCategory();
	g_SmileyCategories.ClearAndLoadAll();

	ColourID cid = {};
	strcpy_s(cid.dbSettingsGroup, MODULENAME);
	strcpy_s(cid.group, MODULENAME);
	strcpy_s(cid.name, LPGEN("Background color"));
	strcpy_s(cid.setting, "SelWndBkgClr");
	cid.defcolour = GetSysColor(COLOR_WINDOW);
	g_plugin.addColor(&cid);
	return 0;
}

static int MirandaShutdown(WPARAM, LPARAM)
{
	CloseSmileys();
	DestroyProxyWindow();
	return 0;
}

int CMPlugin::Load()
{
	g_plugin.registerIcon(MODULENAME, iconList);

	g_SmileyCategories.SetSmileyPackStore(&g_SmileyPacks);

	opt.Load();

	// create smiley events
	g_hevOptionsChanged = CreateHookableEvent(ME_SMILEYADD_OPTIONSCHANGED);

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_SHUTDOWN, MirandaShutdown);
	HookEvent(ME_OPT_INITIALISE, SmileysOptionsInitialize);
	HookEvent(ME_PROTO_ACCLISTCHANGED, AccountListChanged);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DbSettingChanged);
	HookEvent(ME_COLOUR_RELOAD, ReloadColors);
	HookEvent(ME_MSG_BUTTONPRESSED, SmileyButtonPressed);
	
	HookTemporaryEvent(ME_MSG_TOOLBARLOADED, SmileyButtonCreate);

	// create the smiley services
	InitServices();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	RemoveDialogBoxHook();

	DestroyHookableEvent(g_hevOptionsChanged);

	RichEditData_Destroy();

	g_SmileyCategories.ClearAll();
	g_SmileyPackCStore.ClearAndFreeAll();

	DestroyImageCache();
	DestroyGdiPlus();

	DownloadClose();
	return 0;
}
