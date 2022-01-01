/*
Copyright (C) 2011-22 Mataes

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

HANDLE hPackUpdaterFolder = nullptr;
wchar_t tszRoot[MAX_PATH] = { 0 };

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	//{29517BE5-779A-48e5-8950-CB4DE1D43172}
	{ 0x29517be5, 0x779a, 0x48e5, { 0x89, 0x50, 0xcb, 0x4d, 0xe1, 0xd4, 0x31, 0x72 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	wchar_t *tszFolder = Utils_ReplaceVarsW(L"%miranda_userdata%\\" DEFAULT_UPDATES_FOLDER);
	mir_wstrncpy(tszRoot, tszFolder, _countof(tszRoot));

	hPackUpdaterFolder = FoldersRegisterCustomPathW(MODULEA, LPGEN("Pack Updater"), MIRANDA_USERDATAW L"\\" DEFAULT_UPDATES_FOLDER);
	if (hPackUpdaterFolder)
		FoldersGetCustomPathW(hPackUpdaterFolder, tszRoot, MAX_PATH, L"");

	mir_free(tszFolder);
	LoadOptions();
	InitPopupList();
	NetlibInit();
	IcoLibInit();

	// Add cheking update menu item
	CreateServiceFunction(MODULENAME"/CheckUpdates", MenuCommand);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x326495e8, 0xab0a, 0x47d2, 0xb2, 0x22, 0x2a, 0x8e, 0xa8, 0xae, 0x53, 0x1a);
	mi.position = -0x7FFFFFFF;
	mi.flags = CMIF_UNICODE;
	mi.hIcolibItem = IcoLib_GetIcon("check_update");
	mi.name.w = LPGENW("Check for pack updates");
	mi.pszService = MODULENAME"/CheckUpdates";
	Menu_AddMainMenuItem(&mi);

	// Add empty updates folder menu item
	CreateServiceFunction(MODULENAME"/EmptyFolder", EmptyFolder);
	memset(&mi, 0, sizeof(mi));
	SET_UID(mi, 0xc3eea590, 0xaba3, 0x454f, 0x93, 0x93, 0xbc, 0x97, 0x15, 0x2c, 0x3b, 0x3d);
	mi.position = -0x7FFFFFFF;
	mi.flags = CMIF_UNICODE;
	mi.hIcolibItem = IcoLib_GetIcon("empty_folder");
	mi.name.w = LPGENW("Clear pack updates folder");
	mi.pszService = MODULENAME"/EmptyFolder";
	Menu_AddMainMenuItem(&mi);

	// Add options hook
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	if (hCheckThread)
		hCheckThread = nullptr;
	NetlibUnInit();
	return 0;
}