/*
IEView history viewer plugin for Miranda IM

Copyright © 2005-2006 Cristian Libotean

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
#include "services.h"

int hLangpack;//Miranda NG langpack used by translate functions, filled by mir_getLP()
char ModuleName[] = "IEHistory";
HICON hIcon;
HINSTANCE hInstance;
MWindowList hOpenWindowsList = NULL;

HMODULE hUxTheme = 0;
BOOL(WINAPI *MyEnableThemeDialogTexture)(HANDLE, DWORD) = NULL;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_DISPLAY_NAME,
	__VERSION_DWORD,
	__DESC,
	__AUTHOR, __AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {2f093b88-f389-44f1-9e2a-37c29194203a}
	{ 0x2f093b88, 0xf389, 0x44f1, { 0x9e, 0x2a, 0x37, 0xc2, 0x91, 0x94, 0x20, 0x3a } }
};

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

static const MUUID interfaces[] = { MIID_UIHISTORY, MIID_LAST };

extern "C" __declspec(dllexport) const MUUID *MirandaPluginInterfaces()
{
	return interfaces;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	INITCOMMONCONTROLSEX icex;

	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_DATE_CLASSES;

	InitCommonControlsEx(&icex);

	if ((hUxTheme = LoadLibraryA("uxtheme.dll")) != 0)
		MyEnableThemeDialogTexture = (BOOL(WINAPI *)(HANDLE, DWORD))GetProcAddress(hUxTheme, "EnableThemeDialogTexture");

	/// all initialization here
	hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HISTORYICON));
	hOpenWindowsList = WindowList_Create();

	InitServices();

	/// menu items
	CMenuItem mi;
	SET_UID(mi, 0x28848d7a, 0x6995, 0x4799, 0x82, 0xd7, 0x18, 0x40, 0x3d, 0xe3, 0x71, 0xc4);
	mi.name.t = LPGENT("View &history");
	mi.flags = CMIF_TCHAR;
	mi.position = 1000090000;
	mi.hIcolibItem = hIcon;
	mi.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
	Menu_AddContactMenuItem(&mi);

	/// @todo (White-Tiger#1#08/19/14): fully implement System History someday^^
	SET_UID(mi, 0xfcb4bb2a, 0xd4d8, 0x48ab, 0x94, 0xcc, 0x5b, 0xe9, 0x8d, 0x53, 0x3e, 0xf1);
	mi.name.t = LPGENT("&System History");
	Menu_AddMainMenuItem(&mi);

	HookEvents();
	return 0;
}

extern "C" int __declspec(dllexport) Unload()
{
	DestroyServices();

	WindowList_Broadcast(hOpenWindowsList, WM_CLOSE, 0, 0);
	WindowList_Destroy(hOpenWindowsList);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID)
{
	hInstance = hinstDLL;
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hinstDLL);
		LogInit();
	}
	return TRUE;
}
