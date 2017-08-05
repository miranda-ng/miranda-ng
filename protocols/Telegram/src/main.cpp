/*
Copyright (c) 2015 Miranda NG project (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

#include <delayimp.h>
#pragma comment(lib, "delayimp.lib")

FARPROC WINAPI delayHook(unsigned dliNotify, PDelayLoadInfo dli)
{
	switch (dliNotify)
	{
	case dliNotePreLoadLibrary:
		return (FARPROC)LoadLibraryA(dli->szDll);
	}
	return NULL;
}

extern "C" PfnDliHook __pfnDliNotifyHook2 = delayHook;


int hLangpack;
HINSTANCE g_hInstance;
CLIST_INTERFACE *pcli;
char g_szMirVer[100];
HANDLE hQueue;

PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {43F56D31-CB19-471A-8A8A-91FD6E9A3741}
	{ 0x43f56d31, 0xcb19, 0x471a, { 0x8a, 0x8a, 0x91, 0xfd, 0x6e, 0x9a, 0x37, 0x41 } }

};

DWORD WINAPI DllMain(HINSTANCE hInstance, DWORD, LPVOID)
{
	g_hInstance = hInstance;

	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

extern "C" int __declspec(dllexport) Load(void)
{
	hQueue = CreateTimerQueue();
	mir_getLP(&pluginInfo);
	mir_getCLI();

	CallService(MS_SYSTEM_GETVERSIONTEXT, sizeof(g_szMirVer), LPARAM(g_szMirVer));

	PROTOCOLDESCRIPTOR pd = { 0 };
	pd.cbSize = sizeof(pd);
	pd.szName = MODULE;
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = (pfnInitProto)CTelegramProto::InitAccount;
	pd.fnUninit = (pfnUninitProto)CTelegramProto::UninitAccount;
	Proto_RegisterModule(&pd);


	HookEvent(ME_SYSTEM_MODULESLOADED, &CTelegramProto::OnModulesLoaded);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	DeleteTimerQueue(hQueue);
	return 0;
}


int CTelegramProto::OnModulesLoaded(WPARAM, LPARAM)
{
	return 0;
}