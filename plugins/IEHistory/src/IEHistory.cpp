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

CMPlugin g_plugin;

HICON hIcon;
HINSTANCE hInstance;
MWindowList hOpenWindowsList = nullptr;

HMODULE hUxTheme = nullptr;
BOOL(WINAPI *MyEnableThemeDialogTexture)(HANDLE, uint32_t) = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_DISPLAY_NAME,
	__VERSION_DWORD,
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {2f093b88-f389-44f1-9e2a-37c29194203a}
	{ 0x2f093b88, 0xf389, 0x44f1, { 0x9e, 0x2a, 0x37, 0xc2, 0x91, 0x94, 0x20, 0x3a } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("IEHistory", pluginInfoEx),
	iLoadCount(m_szModuleName, "EventsToLoad", 0),
	bEnableRtl(m_szModuleName, "EnableRTL", 0),
	bUseWorker(m_szModuleName, "UseWorkerThread", 0),
	bShowLastFirst(m_szModuleName, "ShowLastPageFirst", 0)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_UIHISTORY, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(icex);
	icex.dwICC = ICC_DATE_CLASSES;
	InitCommonControlsEx(&icex);

	if ((hUxTheme = LoadLibraryA("uxtheme.dll")) != nullptr)
		MyEnableThemeDialogTexture = (BOOL(WINAPI *)(HANDLE, uint32_t))GetProcAddress(hUxTheme, "EnableThemeDialogTexture");

	// all initialization here
	hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HISTORYICON));
	hOpenWindowsList = WindowList_Create();

	CreateServiceFunction(MS_HISTORY_SHOWCONTACTHISTORY, ShowContactHistoryService);

	// menu items
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x28848d7a, 0x6995, 0x4799, 0x82, 0xd7, 0x18, 0x40, 0x3d, 0xe3, 0x71, 0xc4);
	mi.name.w = LPGENW("View &history");
	mi.flags = CMIF_UNICODE;
	mi.position = 1000090000;
	mi.hIcolibItem = hIcon;
	mi.pszService = MS_HISTORY_SHOWCONTACTHISTORY;
	Menu_AddContactMenuItem(&mi);

	HookEvent(ME_OPT_INITIALISE, OnOptionsInitialize);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	WindowList_Broadcast(hOpenWindowsList, WM_CLOSE, 0, 0);
	WindowList_Destroy(hOpenWindowsList);
	return 0;
}
