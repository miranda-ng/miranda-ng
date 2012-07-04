/*
CmdLine plugin for Miranda IM

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
#include "mimcmd_ipc.h"

char ModuleName[] = "CmdLine";
HINSTANCE hInstance;

PLUGINLINK *pluginLink;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_DISPLAY_NAME,
	VERSION,
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	0, //unicode aware
	0,
	{0x2f1a117c, 0x3c1b, 0x4c01, {0x89, 0xea, 0x6d, 0x8f, 0xd8, 0x5a, 0x9b, 0x4c}} //{2f1a117c-3c1b-4c01-89ea-6d8fd85a9b4c}
	}; //not used

OLD_MIRANDAPLUGININFO_SUPPORT;

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion) 
{
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_CMDLINE, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID *MirandaPluginInterfaces()
{
	return interfaces;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;
	
//	InitServices();
	if (InitServer())
	{
		MessageBox(0, "Could not initialize CmdLine plugin property", "Error", MB_ICONEXCLAMATION | MB_OK);
	}
	
	HookEvents();
	
	InitializeMirandaMemFunctions();
	
	return 0;
}

extern "C" int __declspec(dllexport) Unload()
{
	bWaitForUnload = 0;

	UnhookEvents();
	
	DestroyServer();
	
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