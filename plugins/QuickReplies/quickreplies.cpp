/* 
Copyright (C) 2010 Unsane

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

HINSTANCE hInstance = NULL;
int hLangpack;

HANDLE hOnModulesLoaded;
HANDLE hOnPreShutdown;

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
	// {A6A031B6-D32F-4842-98C6-EC716C576B77}
	{0xa6a031b6, 0xd32f, 0x4842, {0x98, 0xc6, 0xec, 0x71, 0x6c, 0x57, 0x6b, 0x77}}
};

BOOL WINAPI DllMain(HINSTANCE hInstanceDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInstance = hInstanceDLL;

	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

extern "C" __declspec(dllexport) int Load(void)
{

	mir_getLP(&pluginInfoEx);

	hOnModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	hOnPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	DestroyServiceFunction(hQuickRepliesService);
	return 0;
}
