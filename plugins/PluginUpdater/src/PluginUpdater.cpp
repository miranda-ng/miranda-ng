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

PlugOptions opts;

HINSTANCE hInst = NULL;
TCHAR tszRoot[MAX_PATH] = {0}, tszTempPath[MAX_PATH];
int hLangpack;
DWORD g_mirandaVersion;

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
#if MIRANDA_VER < 0x0A00
	0,
#endif
	// {968DEF4A-BC60-4266-AC08-754CE721DB5F}
	{0x968def4a, 0xbc60, 0x4266, {0xac, 0x8, 0x75, 0x4c, 0xe7, 0x21, 0xdb, 0x5f}} 
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	g_mirandaVersion = mirandaVersion;
	return &pluginInfoEx;
}


#if MIRANDA_VER < 0x0A00
extern "C" __declspec(dllexport) int Load(PLUGINLINK *link)
{
	pluginLink = link;
	mir_getMMI(&mmi);
	mir_getLI(&li);
	mir_getMD5I(&md5i);
	mir_getUTFI(&utfi);
#else
extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfoEx);

	InitServices();
#endif

	db_set_b(NULL, MODNAME, DB_SETTING_NEED_RESTART, 0);

	DWORD dwLen = GetTempPath( SIZEOF(tszTempPath), tszTempPath);
	if (tszTempPath[dwLen-1] == '\\')
		tszTempPath[dwLen-1] = 0;

	LoadOptions();
	InitPopupList();
	InitNetlib();
	InitIcoLib();

	// Add cheking update menu item
	InitCheck();
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 400010000;
	mi.icolibItem = Skin_GetIconHandle("check_update");
	mi.pszName = LPGEN("Check for updates");
	mi.pszService = MODNAME"/CheckUpdates";
	Menu_AddMainMenuItem(&mi);

#if MIRANDA_VER >= 0x0A00
	InitListNew();

	mi.position++;
	mi.icolibItem = Skin_GetIconHandle("plg_list");
	mi.pszName = LPGEN("Available components list");
	mi.pszService = MODNAME"/ShowList";
	Menu_AddMainMenuItem(&mi);

	InitOptions();
#endif

	// Add hotkey
	HOTKEYDESC hkd = { sizeof(hkd) };
	hkd.pszName = "Check for updates";
	hkd.pszDescription = "Check for updates";
	hkd.pszSection = "Plugin Updater";
	hkd.pszService = MODNAME"/CheckUpdates";
	hkd.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL, VK_F10) | HKF_MIRANDA_LOCAL;
	hkd.lParam = FALSE;
	Hotkey_Register(&hkd);

	InitEvents();

	// add sounds
	SkinAddNewSoundEx("updatecompleted", LPGEN("Plugin Updater"), LPGEN("Update completed"));
	SkinAddNewSoundEx("updatefailed", LPGEN("Plugin Updater"), LPGEN("Update failed"));
	
	// Upgrade old settings
	if (-1 == db_get_b(0, MODNAME, DB_SETTING_UPDATE_MODE, -1)) {
		ptrT dbvUpdateURL(db_get_tsa(0, MODNAME, DB_SETTING_UPDATE_URL));
		if (dbvUpdateURL) {
			if (!_tcscmp(dbvUpdateURL, _T(DEFAULT_UPDATE_URL))) {
				db_set_b(0, MODNAME, DB_SETTING_UPDATE_MODE, UPDATE_MODE_STABLE);
				db_unset(0, MODNAME, DB_SETTING_UPDATE_URL);
			}
			else if (!_tcscmp(dbvUpdateURL, _T(DEFAULT_UPDATE_URL_TRUNK))) {
				db_set_b(0, MODNAME, DB_SETTING_UPDATE_MODE, UPDATE_MODE_TRUNK);
				db_unset(0, MODNAME, DB_SETTING_UPDATE_URL);
			}
			else if (!_tcscmp(dbvUpdateURL, _T(DEFAULT_UPDATE_URL_TRUNK_SYMBOLS)_T("/"))) {
				db_set_b(0, MODNAME, DB_SETTING_UPDATE_MODE, UPDATE_MODE_TRUNK_SYMBOLS);
				db_unset(0, MODNAME, DB_SETTING_UPDATE_URL);
			}
			else db_set_b(0, MODNAME, DB_SETTING_UPDATE_MODE, UPDATE_MODE_CUSTOM);
		}
	}
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	UnloadCheck();
#if MIRANDA_VER >= 0x0A00
	UnloadListNew();
#endif
	UnloadNetlib();
	return 0;
}
