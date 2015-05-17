/*
Exchange notifier plugin for Miranda IM

Copyright © 2006 Cristian Libotean, Attila Vajda

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

#include "stdafx.h"

char ModuleName[] = "ExchangeNotify";
HINSTANCE hInstance;
HICON hiMailIcon = NULL;
HWND hEmailsDlg = NULL;
int hLangpack=0;

CExchangeServer &exchangeServer = CExchangeServer();

//PLUGINLINK *pluginLink;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_DISPLAY_NAME,
	__VERSION_DWORD,
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
  {0x2fd0df15, 0x7098, 0x41ce, {0xaa, 0x92, 0xff, 0x62, 0x18, 0x06, 0xe3, 0x8b}} //{2fd0df15-7098-41ce-aa92-ff621806e38b}
}; //not used

//OLD_MIRANDAPLUGININFO_SUPPORT;

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD) 
{
//	Log("%s", "Entering function " __FUNCTION__);
//	Log("%s", "Leaving function " __FUNCTION__);
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_EXCHANGE, MIID_LAST};

extern "C" int __declspec(dllexport) Load()
{
//	Log("%s", "Entering function " __FUNCTION__);
	//pluginLink = link;
	mir_getLP( &pluginInfo );
//	Log("%s", "Initialising miranda memory functions");
//	InitializeMirandaMemFunctions();
	hiMailIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIL));
//	Log("%s", "Creating service functions ...");
	InitServices();
//	Log("%s", "Hooking events ...");	
	HookEvents();
	return 0;
}

extern "C" int __declspec(dllexport) Unload()
{
//	Log("%s", "Entering function " __FUNCTION__);
//	Log("%s", "Unhooking events ...");
	
//	Log("%s", "Destroying service functions ...");
	DestroyServices();
//	DestroyEvents();
	UnhookEvents();
	
//	Log("%s", "Leaving function " __FUNCTION__);
	return 0;
}

extern "C" bool WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID)
{
	hInstance = hinstDLL; //save global instance
	if (fdwReason == DLL_PROCESS_ATTACH)
		{
			DisableThreadLibraryCalls(hinstDLL);
		}
	return TRUE;
}