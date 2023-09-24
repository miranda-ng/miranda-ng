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

static IconItem iconList[] = 
{
	{ LPGEN("Button smiley"), "SmileyAdd_ButtonSmiley", IDI_SMILINGICON }
};

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
	HookEvent(ME_SYSTEM_PRESHUTDOWN, MirandaShutdown);
	HookEvent(ME_OPT_INITIALISE, SmileysOptionsInitialize);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, RebuildContactMenu);
	HookEvent(ME_SMILEYADD_OPTIONSCHANGED, UpdateSrmmDlg);
	HookEvent(ME_PROTO_ACCLISTCHANGED, AccountListChanged);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DbSettingChanged);
	HookEvent(ME_COLOUR_RELOAD, ReloadColour);
	HookEvent(ME_MSG_BUTTONPRESSED, SmileyButtonPressed);
	
	HookTemporaryEvent(ME_MSG_TOOLBARLOADED, SmileyButtonCreate);

	// create the smiley services
	CreateServiceFunction(MS_SMILEYADD_REPLACESMILEYS, ReplaceSmileysCommand);
	CreateServiceFunction(MS_SMILEYADD_GETINFO2, GetInfoCommand2);
	CreateServiceFunction(MS_SMILEYADD_REGISTERCATEGORY, RegisterPack);
	CreateServiceFunction(MS_SMILEYADD_BATCHPARSE, ParseTextBatch);
	CreateServiceFunction(MS_SMILEYADD_BATCHFREE, FreeTextBatch);
	CreateServiceFunction(MS_SMILEYADD_CUSTOMCATMENU, CustomCatMenu);
	CreateServiceFunction(MS_SMILEYADD_RELOAD, ReloadPack);
	CreateServiceFunction(MS_SMILEYADD_LOADCONTACTSMILEYS, LoadContactSmileys);
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
