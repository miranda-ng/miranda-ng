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

#include "../commonstatus.h"
#include "keepstatus.h"
#include "../resource.h"

HANDLE hMainThread = 0;
unsigned long mainThreadId = 0;
int hLangpack = 0;

HANDLE hCSModuleLoadedHook = NULL;

HANDLE hConnectionEvent = NULL;
HANDLE hStopRecon = NULL, hEnableProto = NULL, hIsProtoEnabled = NULL, hAnnounceStat = NULL;

HINSTANCE hInst;
CLIST_INTERFACE *pcli;

/////////////////////////////////////////////////////////////////////////////////////////
// dll entry point

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
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
	{ 0xa5bb1b7a, 0xb7cd, 0x4cbb, { 0xa7, 0xdb, 0xce, 0xb4, 0xeb, 0x71, 0xda, 0x49 } } // {A5BB1B7A-B7CD-4cbb-A7DB-CEB4EB71DA49}
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

/////////////////////////////////////////////////////////////////////////////////////////
// plugin's entry point

int CSModuleLoaded(WPARAM wParam, LPARAM lParam);

INT_PTR StopReconnectingService(WPARAM wParam, LPARAM lParam);
INT_PTR EnableProtocolService(WPARAM wParam, LPARAM lParam);
INT_PTR IsProtocolEnabledService(WPARAM wParam, LPARAM lParam);
INT_PTR AnnounceStatusChangeService(WPARAM wParam, LPARAM lParam);

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfoEx);
	mir_getCLI();

	InitCommonStatus();

	hCSModuleLoadedHook = HookEvent(ME_SYSTEM_MODULESLOADED, CSModuleLoaded);

	hConnectionEvent = CreateHookableEvent(ME_KS_CONNECTIONEVENT);

	hStopRecon = CreateServiceFunction(MS_KS_STOPRECONNECTING, StopReconnectingService);
	hEnableProto = CreateServiceFunction(MS_KS_ENABLEPROTOCOL, EnableProtocolService);
	hIsProtoEnabled = CreateServiceFunction(MS_KS_ISPROTOCOLENABLED, IsProtocolEnabledService);
	hAnnounceStat = CreateServiceFunction(MS_KS_ANNOUNCESTATUSCHANGE, AnnounceStatusChangeService);

	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, THREAD_SET_CONTEXT, FALSE, 0);
	mainThreadId = GetCurrentThreadId();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// plugin's exit point

extern "C" int __declspec(dllexport) Unload(void)
{
	DestroyHookableEvent(hConnectionEvent);

	UnhookEvent(hCSModuleLoadedHook);

	if (hMainThread)
		CloseHandle(hMainThread);
	DestroyServiceFunction(hStopRecon);
	DestroyServiceFunction(hEnableProto);
	DestroyServiceFunction(hIsProtoEnabled);
	DestroyServiceFunction(hAnnounceStat);

	return 0;
}
