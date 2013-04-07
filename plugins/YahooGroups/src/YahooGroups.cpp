/*
YahooGroups plugin for Miranda IM

Copyright © 2007 Cristian Libotean

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"

char ModuleName[] = "YahooGroups";
HINSTANCE hInstance;

UINT currentCodePage = 0;
int hLangpack = 0;

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
	// {2F3FE8B9-7327-4008-A60D-93F0F4F7F0F1}	
	{0x2f3fe8b9, 0x7327, 0x4008, {0xa6, 0x0d, 0x93, 0xf0, 0xf4, 0xf7, 0xf0, 0xf1}}
};
	
extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion) 
{
	return &pluginInfo;
}

extern "C" int __declspec(dllexport) Load()
{
	mir_getLP(&pluginInfo);
	LogInit();
	
	currentCodePage = (UINT) CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
	
	InitServices();
	
	HookEvents();
	
	return 0;
}

extern "C" int __declspec(dllexport) Unload()
{
	DestroyServices();

	UnhookEvents();
	
	return 0;
}

bool WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInstance = hinstDLL;
	if (fdwReason == DLL_PROCESS_ATTACH)
		{
			DisableThreadLibraryCalls(hinstDLL);
		}
		
	return TRUE;
}
