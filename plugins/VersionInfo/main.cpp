/*
Version information plugin for Miranda IM

Copyright © 2002-2006 Luca Santarelli, Cristian Libotean

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

#define STRICT
#define WIN32_LEAN_AND_MEAN

#define _CRT_SECURE_NO_DEPRECATE

#include "common.h"

#include "CVersionInfo.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;

HICON hiVIIcon;

DWORD EnglishLocale;

BOOL bFoldersAvailable = FALSE;
HANDLE hOutputLocation = NULL; //for folders plugin

void * (* MirandaMalloc)(size_t);
void * (* MirandaRealloc)(void *, size_t);
void (* MirandaFree)(void *);

char ModuleName[] = "VersionInfo";

#ifdef _DEBUG
	BOOL verbose = FALSE;//TRUE;
#else
	BOOL verbose = FALSE;
#endif

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_DISPLAY_NAME,
	VERSION,
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	1,		//not transient
	0,
	{0x2f174488, 0x489c, 0x4fe1, {0x94, 0x0d, 0x59, 0x6c, 0xf0, 0xf3, 0x5e, 0x65}} //{2f174488-489c-4fe1-940d-596cf0f35e65}
};

OLD_MIRANDAPLUGININFO_SUPPORT;

static const MUUID interfaces[] = {MIID_VERSIONINFO, MIID_SERVICEMODE, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID *MirandaPluginInterfaces()
{
	return interfaces;
}

bool WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	LogToFileInit();
	LogToFile("Entering %s", __FUNCTION__);
	hInst=hinstDLL;
	if (fdwReason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls(hinstDLL);
	EnglishLocale = MAKELCID(MAKELANGID(0x09, 0x01), SORT_DEFAULT);	//create our english locale and use it everywhere it's needed
	LogToFile("Leaving %s", __FUNCTION__);
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	LogToFile("Entering %s", __FUNCTION__);
	LogToFile("Leaving %s", __FUNCTION__);
	return &pluginInfo;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	LogToFile("Entering %s", __FUNCTION__);
	pluginLink=link;

	LogToFile("Initialising services ...");
	InitServices();
	LogToFile("Hooking events ...");
	HookEvents();
	
	hiVIIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_MAIN));
	
	//get the name of the dll itself
	char filePath[512] = {0};
	GetModuleFileName(hInst, filePath, sizeof(filePath));
	char *fileName = NULL;
	size_t i = strlen(filePath) - 1;
	_strlwr(filePath);
	
	//check that the name begins with svc_
	while ((i > 0) && (filePath[i] != '\\')) { i--; }
	if (i > 0)
	{
		filePath[i] = 0;
		fileName = filePath + i + 1;
		
		if (strstr(fileName, "svc_") != fileName)
		{
			char buffer[1024];
			mir_snprintf(buffer, sizeof(buffer), "Please rename the plugin '%s' to 'svc_vi.dll' to enable service mode functionality.", fileName);
			MessageBox(NULL, TranslateTS(buffer), Translate("Version Information"), MB_OK | MB_ICONEXCLAMATION);
		}
	}

	//Menu item
	//if (DBGetContactSettingByte(NULL, ModuleName, "MenuItem", TRUE)) 	{
	{
		LogToFile("creating menu item ...");
		CLISTMENUITEM mi = { 0 };

		mi.cbSize = sizeof(mi);
		mi.position = mi.popupPosition = 2000089999;
		mi.flags = 0;
		mi.hIcon = hiVIIcon;
		mi.pszName = Translate("Version Information");
		mi.pszService = MS_VERSIONINFO_MENU_COMMAND;
		CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);
//		mi.pszPopupName = "Version Information";
//		mi.popupPosition = 2;
//		mi.pszName = "Test 1";
//		CallService(MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM) &mi);
	}
	LogToFile("Check riched32.dll ...");
	if (LoadLibrary("RichEd32.dll") == NULL) {
		MessageBox(NULL, "d'oh", "d'oh", MB_OK);
	}

	//get miranda's malloc, realloc and free functions
	LogToFile("Get miranda memory functions ...");
	MM_INTERFACE mmInterface;
	mmInterface.cbSize = sizeof(MM_INTERFACE);
	CallService(MS_SYSTEM_GET_MMI, 0, (LPARAM) &mmInterface);
	MirandaFree = mmInterface.mmi_free;
	MirandaMalloc = mmInterface.mmi_malloc;
	MirandaRealloc = mmInterface.mmi_realloc;
	
	LogToFile("Leaving %s", __FUNCTION__);
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	LogToFile("Entering %s", __FUNCTION__);
	
	LogToFile("Unhooking events ...");
	UnhookEvents();
	
	LogToFile("Destroying services ...");
	DestroyServices();
	
	LogToFile("Leaving %s", __FUNCTION__);
	return 0;
}