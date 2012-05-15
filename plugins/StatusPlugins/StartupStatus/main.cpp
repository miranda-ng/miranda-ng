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

// {4B733944-5A70-4b52-AB2D-68B1EF38FFE8}
#define MIID_STARTUPSTATUS { 0x4b733944, 0x5a70, 0x4b52, { 0xab, 0x2d, 0x68, 0xb1, 0xef, 0x38, 0xff, 0xe8 } }

static HANDLE
	hCSModuleLoadedHook,
	hGetProfileService,
	hGetProfileCountService,
	hGetProfileNameService;

HINSTANCE hInst;
PLUGINLINK *pluginLink;

MM_INTERFACE mmi;
LIST_INTERFACE li;
int hLangpack = 0;

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
	#if defined( _UNICODE )
		__PLUGIN_NAME __PLATFORM_NAME " (Unicode)",
	#else
		__PLUGIN_NAME,
	#endif
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	0,
	MIID_STARTUPSTATUS
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if ( mirandaVersion < PLUGIN_MAKE_VERSION( 0, 8, 0, 0 )) {
		MessageBox(NULL, TranslateT("Startup requires Miranda IM of version 0.8, get it at www.miranda-im.org"), TranslateT("KeepStatus"), MB_OK);
		return 0;
	}

	return &pluginInfoEx;
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns plugin's interfaces information

static const MUUID interfaces[] = { MIID_STARTUPSTATUS, MIID_LAST };

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

/////////////////////////////////////////////////////////////////////////////////////////
// plugin's entry point

static INT_PTR SrvGetProfile( WPARAM wParam, LPARAM lParam )
{	return GetProfile(( int )wParam, *(TSettingsList*)lParam );
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;
	mir_getMMI(&mmi);
	mir_getLI(&li);
	mir_getLP( &pluginInfoEx );

	if ( DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETPROFILE, 1) ||
		  DBGetContactSettingByte(NULL, MODULENAME, SETTING_OFFLINECLOSE, 0))
		DBWriteContactSettingWord(NULL, "CList", "Status", (WORD)ID_STATUS_OFFLINE);

	// docking
	if (DBGetContactSettingByte(NULL, MODULENAME, SETTING_SETDOCKED, 0)) {
		int docked = DBGetContactSettingByte(NULL, MODULENAME, SETTING_DOCKED, DOCKED_NONE);
		if ( docked == DOCKED_LEFT || docked == DOCKED_RIGHT )
			docked = -docked;

		DBWriteContactSettingByte(NULL, MODULE_CLIST, SETTING_DOCKED, (BYTE)docked);
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
