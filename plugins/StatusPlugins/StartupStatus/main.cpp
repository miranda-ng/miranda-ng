/*
    StartupStatus Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "../commonstatus.h"
#include "startupstatus.h"


static HANDLE
	hCSModuleLoadedHook,
	hGetProfileService,
	hGetProfileCountService,
	hGetProfileNameService;

HINSTANCE hInst;

int hLangpack = 0;
CLIST_INTERFACE *pcli;

int CSModuleLoaded( WPARAM, LPARAM );

/////////////////////////////////////////////////////////////////////////////////////////
// dll entry point

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns plugin's extended information

PLUGININFOEX pluginInfoEx={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	{ 0x4b733944, 0x5a70, 0x4b52, { 0xab, 0x2d, 0x68, 0xb1, 0xef, 0x38, 0xff, 0xe8 } } // {4B733944-5A70-4b52-AB2D-68B1EF38FFE8}
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

/////////////////////////////////////////////////////////////////////////////////////////
// plugin's entry point

static INT_PTR SrvGetProfile( WPARAM wParam, LPARAM lParam )
{	return GetProfile(( int )wParam, *(TSettingsList*)lParam );
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP( &pluginInfoEx );
	mir_getCLI();

	if ( db_get_b(NULL, MODULENAME, SETTING_SETPROFILE, 1) ||
		  db_get_b(NULL, MODULENAME, SETTING_OFFLINECLOSE, 0))
		db_set_w(NULL, "CList", "Status", (WORD)ID_STATUS_OFFLINE);

	// docking
	if (db_get_b(NULL, MODULENAME, SETTING_SETDOCKED, 0)) {
		int docked = db_get_b(NULL, MODULENAME, SETTING_DOCKED, DOCKED_NONE);
		if ( docked == DOCKED_LEFT || docked == DOCKED_RIGHT )
			docked = -docked;

		db_set_b(NULL, MODULE_CLIST, SETTING_DOCKED, (BYTE)docked);
	}

	// Create service functions; the get functions are created here; they don't rely on commonstatus
	hGetProfileService = CreateServiceFunction(MS_SS_GETPROFILE, SrvGetProfile);
	hGetProfileCountService = CreateServiceFunction(MS_SS_GETPROFILECOUNT, GetProfileCount);
	hGetProfileNameService = CreateServiceFunction(MS_SS_GETPROFILENAME, GetProfileName);

	LoadProfileModule();

	// protocols must be loaded first
	InitCommonStatus();
	hCSModuleLoadedHook = HookEvent(ME_SYSTEM_MODULESLOADED, CSModuleLoaded);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// plugin's exit point

extern "C" int __declspec(dllexport) Unload(void)
{
	UnhookEvent(hCSModuleLoadedHook);

	DestroyServiceFunction(hGetProfileService);
	DestroyServiceFunction(hGetProfileCountService);
	DestroyServiceFunction(hGetProfileNameService);
	return 0;
}
