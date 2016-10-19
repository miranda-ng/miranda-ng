/*
	KeepStatus Plugin for Miranda-IM (www.miranda-im.org)
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

#include "stdafx.h"

unsigned long mainThreadId = 0;

HANDLE hMainThread = 0,
	hCSModuleLoadedHook,
	hGetProfileService,
	hGetProfileCountService,
	hGetProfileNameService,
	hStateChangedEvent;

HANDLE hConnectionEvent = NULL,
	hStopRecon = NULL,
	hEnableProto = NULL,
	hIsProtoEnabled = NULL,
	hAnnounceStat = NULL;

HINSTANCE hInst;
int hLangpack = 0;
CLIST_INTERFACE *pcli;

/////////////////////////////////////////////////////////////////////////////////////////
// dll entry point

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		hInst = hinstDLL;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns plugin's extended information

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {FADD4A8A-1FD0-4398-83BD-E378B85ED8F1}
	{ 0xfadd4a8a, 0x1fd0, 0x4398, { 0x83, 0xbd, 0xe3, 0x78, 0xb8, 0x5e, 0xd8, 0xf1 } }
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

/////////////////////////////////////////////////////////////////////////////////////////
// interfaces

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_AUTOAWAY, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////
// plugin's entry point

INT_PTR StopReconnectingService(WPARAM wParam, LPARAM lParam);
INT_PTR EnableProtocolService(WPARAM wParam, LPARAM lParam);
INT_PTR IsProtocolEnabledService(WPARAM wParam, LPARAM lParam);
INT_PTR AnnounceStatusChangeService(WPARAM wParam, LPARAM lParam);

static INT_PTR SrvGetProfile(WPARAM wParam, LPARAM lParam)
{
	return GetProfile((int)wParam, *(TSettingsList*)lParam);
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfoEx);
	pcli = Clist_GetInterface();

	//common
	InitCommonStatus();

	/* KeepStatus */
	hCSModuleLoadedHook = HookEvent(ME_SYSTEM_MODULESLOADED, KSCSModuleLoaded);

	hConnectionEvent = CreateHookableEvent(ME_KS_CONNECTIONEVENT);

	hStopRecon = CreateServiceFunction(MS_KS_STOPRECONNECTING, StopReconnectingService);
	hEnableProto = CreateServiceFunction(MS_KS_ENABLEPROTOCOL, EnableProtocolService);
	hIsProtoEnabled = CreateServiceFunction(MS_KS_ISPROTOCOLENABLED, IsProtocolEnabledService);
	hAnnounceStat = CreateServiceFunction(MS_KS_ANNOUNCESTATUSCHANGE, AnnounceStatusChangeService);

	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, THREAD_SET_CONTEXT, FALSE, 0);
	mainThreadId = GetCurrentThreadId();

	/* StartupStatus */
	hCSModuleLoadedHook = HookEvent(ME_SYSTEM_MODULESLOADED, SSCSModuleLoaded);

	if (db_get_b(NULL, SSMODULENAME, SETTING_SETPROFILE, 1) ||
		db_get_b(NULL, SSMODULENAME, SETTING_OFFLINECLOSE, 0))
		db_set_w(NULL, "CList", "Status", (WORD)ID_STATUS_OFFLINE);

	// docking
	if (db_get_b(NULL, SSMODULENAME, SETTING_SETDOCKED, 0)) {
		int docked = db_get_b(NULL, SSMODULENAME, SETTING_DOCKED, DOCKED_NONE);
		if (docked == DOCKED_LEFT || docked == DOCKED_RIGHT)
			docked = -docked;

		db_set_b(NULL, MODULE_CLIST, SETTING_DOCKED, (BYTE)docked);
	}

	// Create service functions; the get functions are created here; they don't rely on commonstatus
	hGetProfileService = CreateServiceFunction(MS_SS_GETPROFILE, SrvGetProfile);
	hGetProfileCountService = CreateServiceFunction(MS_SS_GETPROFILECOUNT, GetProfileCount);
	hGetProfileNameService = CreateServiceFunction(MS_SS_GETPROFILENAME, GetProfileName);

	LoadProfileModule();

	/* AdvancedAutoAway */
	hCSModuleLoadedHook = HookEvent(ME_SYSTEM_MODULESLOADED, AAACSModuleLoaded);
	hStateChangedEvent = CreateHookableEvent(ME_AAA_STATECHANGED);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// plugin's exit point

extern "C" int __declspec(dllexport) Unload(void)
{
	UnhookEvent(hCSModuleLoadedHook);

	// StartupStatus
	DestroyHookableEvent(hConnectionEvent);

	if (hMainThread)
		CloseHandle(hMainThread);
	DestroyServiceFunction(hStopRecon);
	DestroyServiceFunction(hEnableProto);
	DestroyServiceFunction(hIsProtoEnabled);
	DestroyServiceFunction(hAnnounceStat);

	// StartupStatus
	DestroyServiceFunction(hGetProfileService);
	DestroyServiceFunction(hGetProfileCountService);
	DestroyServiceFunction(hGetProfileNameService);

	return 0;
}
