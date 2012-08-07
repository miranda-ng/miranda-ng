/*
Custom profile folders plugin for Miranda IM

Copyright © 2005 Cristian Libotean

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
#include "hooked_events.h"
#include "services.h"
#include "events.h"

#include "m_folders.h"

#define MS_FOLDERS_TEST_PLUGIN "Folders/Test/Plugin"

char ModuleName[] = "Folders";
HINSTANCE hInstance;
int hLangpack;

CFoldersList &lstRegisteredFolders = CFoldersList(10); //the list

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_DISPLAY_NAME,
	VERSION,
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
  {0x2f129563, 0x2c7d, 0x4a9a, {0xb9, 0x48, 0x97, 0xdf, 0xcc, 0x0a, 0xfd, 0xd7}}
	//{2f129563-2c7d-4a9a-b948-97dfcc0afdd7}
};

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion) 
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_FOLDERS, MIID_LAST};

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	
	InitServices();
	InitEvents();
	HookEvents();
	return 0;
}

extern "C" int __declspec(dllexport) Unload()
{
//	DestroyServiceFunction(MS_HISTORY_SHOWCONTACTHISTORY);
	DestroyServices();
	DestroyEvents();
	UnhookEvents();
	return 0;
}

bool WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;
	if (fdwReason == DLL_PROCESS_ATTACH)
		DisableThreadLibraryCalls(hinstDLL);

	return TRUE;
}
