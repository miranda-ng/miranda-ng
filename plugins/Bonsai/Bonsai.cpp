/*
Bonsai plugin for Miranda IM

Copyright © 2006 Cristian Libotean

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

#define ICON_SIZE 16

char ModuleName[] = "Bonsai";
HINSTANCE hInstance;
int bUseANSIStrings = 0;

HBITMAP hbModified = NULL;
HBITMAP hbUnmodified = NULL;
HBITMAP hbHidden = NULL;
HBITMAP hbNewlyModified = NULL;
HIMAGELIST imgList = NULL;

PLUGINLINK *pluginLink;
int hLangpack;
struct MM_INTERFACE mmi;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_DISPLAY_NAME,
	VERSION,
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE, //unicode aware
	0,
	{0x2fe47261, 0x7f38, 0x45f4, {0x8f, 0xac, 0xb8, 0x7e, 0x97, 0x4a, 0x13, 0x50}} //{2fe47261-7f38-45f4-8fac-b87e974a1350}
};

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion) 
{
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_BONSAI, MIID_LAST};

extern "C" __declspec(dllexport) const MUUID *MirandaPluginInterfaces()
{
	return interfaces;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	
	InitCommonControls();

	LogInit();
	
	pluginLink = link;
	mir_getMMI(&mmi);
	mir_getLP(&pluginInfo);
	
	InitServices();
	
	HookEvents();
	
	hbModified = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MODIFIED));
	hbUnmodified = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_UNMODIFIED));
	hbHidden = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_HIDDEN));
	hbNewlyModified = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_NEWLYMODIFIED));
	imgList = ImageList_Create(ICON_SIZE, ICON_SIZE, ILC_COLORDDB | ILC_MASK, 2, 1);
	ImageList_AddMasked(imgList, hbUnmodified, RGB(255, 0, 255));
	ImageList_AddMasked(imgList, hbModified, RGB(255, 0, 255));
	ImageList_AddMasked(imgList, hbHidden, RGB(255, 0, 255));
	ImageList_AddMasked(imgList, hbNewlyModified, RGB(255, 0, 255));
	
	//are we running on Windows 98 ?
	OSVERSIONINFO vi = {0};
	vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&vi);
	bUseANSIStrings = (vi.dwPlatformId == VER_PLATFORM_WIN32_NT) ? 0 : 1;
	char mirVer[512];
	CallService(MS_SYSTEM_GETVERSIONTEXT, sizeof(mirVer), (LPARAM) mirVer);
	_strlwr(mirVer);
	bUseANSIStrings = (strstr(mirVer, "unicode") == NULL) ? 1 : bUseANSIStrings;
	Log("Windows 98 ? %d", bUseANSIStrings);
	
	return 0;
}

extern "C" int __declspec(dllexport) Unload()
{
	DestroyServices();

	UnhookEvents();
	
	ImageList_Destroy(imgList);
	
	return 0;
}

bool WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInstance = hinstDLL;
	if (fdwReason == DLL_PROCESS_ATTACH)
		{
			DisableThreadLibraryCalls(hinstDLL);
		}
		
	return TRUE;
}

