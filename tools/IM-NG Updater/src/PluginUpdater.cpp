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

#define MIID_UPDATER	{0x4a47b19b, 0xde5a, 0x4436, { 0xab, 0x4b, 0xe1, 0xf3, 0xa0, 0x22, 0x5d, 0xe7}}

HINSTANCE hInst = NULL;
MM_INTERFACE mmi;
LIST_INTERFACE li;
MD5_INTERFACE md5i;
UTF8_INTERFACE utfi;

HANDLE hPluginUpdaterFolder = NULL, hCheckUpdates = NULL, hEmptyFolder = NULL;
TCHAR tszRoot[MAX_PATH] = {0};
int hLangpack;
PLUGINLINK* pluginLink;

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
	0,
	//{29517BE5-779A-48e5-8950-CB4DE1D43172}
	{0x29517be5, 0x779a, 0x48e5, {0x89, 0x50, 0xcb, 0x4d, 0xe1, 0xd4, 0x31, 0x72}} 
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

static const MUUID interfaces[] = {MIID_UPDATER, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

extern "C" __declspec(dllexport) int Load(PLUGINLINK* link)
{
	pluginLink = link;
	mir_getMMI(&mmi);
	mir_getLI(&li);
	mir_getMD5I(&md5i);
	mir_getUTFI(&utfi);

	if (ServiceExists(MS_FOLDERS_REGISTER_PATH))
		hPluginUpdaterFolder = FoldersRegisterCustomPathT(MODULEA, "Plugin Updater", MIRANDA_USERDATAT _T("\\")DEFAULT_UPDATES_FOLDER);

	if (hPluginUpdaterFolder)
		OnFoldersChanged(0, 0);
	else {
		mir_ptr<TCHAR> tszFolder( Utils_ReplaceVarsT(_T("%miranda_userdata%\\"DEFAULT_UPDATES_FOLDER)));
		lstrcpyn(tszRoot, tszFolder, SIZEOF(tszRoot));
	}

	LoadOptions();
	InitPopupList();
	NetlibInit();
	IcoLibInit();

	// Add cheking update menu item
	hCheckUpdates = CreateServiceFunction(MODNAME"/CheckUpdates", MenuCommand);

	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(mi);
	mi.position = -0x7FFFFFFF;
	mi.flags = CMIF_ICONFROMICOLIB;
	mi.icolibItem = Skin_GetIconHandle("check_update");
	mi.pszName = LPGEN("Check for plugin updates");
	mi.pszService = MODNAME"/CheckUpdates";
	Menu_AddMainMenuItem(&mi);

	// Add hotkey
	HOTKEYDESC hkd = {0};
	hkd.cbSize = sizeof(hkd);
	hkd.pszName = "Check for plugin updates";
	hkd.pszDescription = "Check for plugin updates";
	hkd.pszSection = "Plugin Updater";
	hkd.pszService = MODNAME"/CheckUpdates";
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, VK_F10) | HKF_MIRANDA_LOCAL;
	hkd.lParam = FALSE;
	Hotkey_Register(&hkd);

	// Add options hook
	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	if (CheckThread)
		CheckThread = NULL;

	NetlibUnInit();
	DestroyServiceFunction(hCheckUpdates);
	return 0;
}
