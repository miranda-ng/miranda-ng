/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG team
Copyright (c) 2005-2011 Boris Krasnovskiy All Rights Reserved
Copyright (c) 2002-2005 Calvin Che

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
Main file for the Weather Protocol, includes loading, unloading,
upgrading, support for plugin uninsaller, and anything that doesn't
belong to any other file.
*/

#include "stdafx.h"

//============  GLOBAL VARIABLES  ============

HWND hPopupWindow;

MWindowList hDataWindowList, hWindowList;

CMPlugin	g_plugin;

VARSW g_pwszIconsName(L"%miranda_path%\\Icons\\proto_Weather.dll");

HANDLE hTBButton = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////
// plugin info

static const PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {6B612A34-DCF2-4E32-85CF-B6FD006B745E}
	{0x6b612a34, 0xdcf2, 0x4e32, {0x85, 0xcf, 0xb6, 0xfd, 0x0, 0x6b, 0x74, 0x5e}}
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CWeatherProto>(MODULENAME, pluginInfoEx)
{
	SetUniqueId("ID");
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static int OnPreShutdown(WPARAM, LPARAM)
{
	WindowList_Broadcast(hWindowList, WM_CLOSE, 0, 0);
	WindowList_Broadcast(hDataWindowList, WM_CLOSE, 0, 0);
	return 0;
}

//============  MISC FUNCTIONS  ============

static IconItem iconList[] =
{
	{	LPGEN("Protocol icon"),      "main",      IDI_ICON       },
	{	LPGEN("Update Disabled"),    "disabled",  IDI_DISABLED   },
	{	LPGEN("View Log"),           "log",       IDI_LOG        },
	{	LPGEN("Update with Clear"),  "update2",   IDI_UPDATE2    },
	{	LPGEN("View Brief"),         "brief",     IDI_S          },
	{	LPGEN("View Complete"),      "read",      IDI_READ       },
	{	LPGEN("Weather Update"),     "update",    IDI_UPDATE     },
	{	LPGEN("Weather Map"),        "map",       IDI_MAP        },
	{	LPGEN("Edit Settings"),      "edit",      IDI_EDIT       },
};

int CMPlugin::Load()
{
	g_plugin.registerIcon(MODULENAME, iconList, MODULENAME);

	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);

	// load dll with icons
	hIconsDll = LoadLibraryW(g_pwszIconsName);

	// window lists
	hDataWindowList = WindowList_Create();
	hWindowList = WindowList_Create();

	// add global menus
	CWeatherProto::GlobalMenuInit();

	// add sound event
	addSound("weatherupdated", _A2W(MODULENAME), LPGENW("Condition Changed"));
	addSound("weatheralert", _A2W(MODULENAME), LPGENW("Alert Issued"));

	// window needed for popup commands
	hPopupWindow = CreateWindowEx(WS_EX_TOOLWINDOW, L"static", _A2W(MODULENAME) L"__PopupWindow", 0, CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, nullptr, g_plugin.getInst(), nullptr);
	SetWindowLongPtr(hPopupWindow, GWLP_WNDPROC, (LONG_PTR)&CWeatherProto::PopupWndProc);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// unload function

int CMPlugin::Unload()
{
	if (hIconsDll)
		FreeModule(hIconsDll);

	DestroyWindow(hPopupWindow);

	WindowList_Destroy(hDataWindowList);
	WindowList_Destroy(hWindowList);
	return 0;
}
