/* 
Copyright (C) 2010 Mataes

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

#include "common.h"

HINSTANCE hInst = NULL;

HANDLE hOptHook = NULL, hLoadHook = NULL, hPackUpdaterFolder = NULL, hCheckUpdates = NULL, hEmptyFolder = NULL, hOnPreShutdown = NULL;
TCHAR tszRoot[MAX_PATH] = {0};
int hLangpack;

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	//{29517BE5-779A-48e5-8950-CB4DE1D43172}
	{0x29517be5, 0x779a, 0x48e5, {0x89, 0x50, 0xcb, 0x4d, 0xe1, 0xd4, 0x31, 0x72}} 
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfoEx);

	TCHAR* tszFolder = Utils_ReplaceVarsT(_T("%miranda_userdata%\\"DEFAULT_UPDATES_FOLDER));
	mir_tstrncpy(tszRoot, tszFolder, SIZEOF(tszRoot));

	hPackUpdaterFolder = FoldersRegisterCustomPathT(MODULEA, LPGEN("Pack Updater"), MIRANDA_USERDATAT _T("\\")DEFAULT_UPDATES_FOLDER);
	if (hPackUpdaterFolder)
		FoldersGetCustomPathT(hPackUpdaterFolder, tszRoot, MAX_PATH, _T(""));

	mir_free(tszFolder);
	LoadOptions();
	InitPopupList();
	NetlibInit();
	IcoLibInit();

	// Add cheking update menu item
	hCheckUpdates = CreateServiceFunction(MODNAME"/CheckUpdates", MenuCommand);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = -0x7FFFFFFF;
	mi.flags = CMIF_TCHAR;
	mi.hIcon = Skin_GetIcon("check_update");
	mi.ptszName = LPGENT("Check for pack updates");
	mi.pszService = MODNAME"/CheckUpdates";
	Menu_AddMainMenuItem(&mi);
	// Add empty updates folder menu item
	hEmptyFolder = CreateServiceFunction(MODNAME"/EmptyFolder", EmptyFolder);
	memset(&mi, 0, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = -0x7FFFFFFF;
	mi.flags = CMIF_TCHAR;
	mi.hIcon = Skin_GetIcon("empty_folder");
	mi.ptszName = LPGENT("Clear pack updates folder");
	mi.pszService = MODNAME"/EmptyFolder";
	Menu_AddMainMenuItem(&mi);

	// Add options hook
	hOptHook = HookEvent(ME_OPT_INITIALISE, OptInit);
	hLoadHook = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hOnPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	if (hCheckThread)
		hCheckThread = NULL;
	NetlibUnInit();
	DestroyServiceFunction(hCheckUpdates);
	DestroyServiceFunction(hEmptyFolder);
	return 0;
}