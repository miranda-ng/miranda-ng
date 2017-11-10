/*

'File Association Manager'-Plugin for Miranda IM

Copyright (C) 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (AssocMgr-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

#pragma comment(lib, "delayimp.lib")

HINSTANCE hInst;
static HANDLE hHookModulesLoaded;
int hLangpack;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {52685CD7-0EC7-44c1-A1A6-381612418202}
	{0x52685cd7, 0xec7, 0x44c1, {0xa1, 0xa6, 0x38, 0x16, 0x12, 0x41, 0x82, 0x2}}
};

/////////////////////////////////////////////////////////////////////////////////////////
// if we run here, we're running from the command prompt

static bool bPathSet = false;

FARPROC WINAPI myDliHook(unsigned dliNotify, PDelayLoadInfo)
{
	if (dliNotify == dliNotePreLoadLibrary && !bPathSet) {
		bPathSet = true;
		SetCurrentDirectoryW(L"Libs");
		LoadLibraryW(L"ucrtbase.dll");
	}
	return NULL;
}

PfnDliHook  __pfnDliNotifyHook2 = &myDliHook;

/////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	hInst = hinstDLL;
	return TRUE;
}

static int AssocMgrModulesLoaded(WPARAM,LPARAM)
{
	InitTest();
	return 0;
}

extern "C" __declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);

	InitAssocList();
	InitDde();

	hHookModulesLoaded=HookEvent(ME_SYSTEM_MODULESLOADED,AssocMgrModulesLoaded);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	UninitTest();
	UninitDde();
	UninitAssocList();
	UnhookEvent(hHookModulesLoaded);
	return 0;
}
