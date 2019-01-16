// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018-19 Miranda NG team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------

#include "stdafx.h"

bool g_bPopupService;

HWND g_hwndHeartbeat;

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	{ 0xEFB2355B, 0x82B3, 0x4759, { 0xb7, 0xd8, 0x95, 0xf8, 0xe9, 0x50, 0x62, 0x91 } } // {EFB2355B-82B3-4759-B7D8-95F8E9506291}
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CIcqProto>(MODULENAME, pluginInfoEx)
{
	SetUniqueId(DB_KEY_UIN);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

int ModuleLoad(WPARAM, LPARAM)
{
	g_bPopupService = ServiceExists(MS_POPUP_ADDPOPUPT);
	return 0;
}

int CMPlugin::Load()
{
	g_hwndHeartbeat = CreateWindowEx(0, L"STATIC", nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);

	HookEvent(ME_SYSTEM_MODULELOAD, ModuleLoad);
	HookEvent(ME_SYSTEM_MODULEUNLOAD, ModuleLoad);
	ModuleLoad(0, 0);
	return 0;
};

int CMPlugin::Unload()
{
	DestroyWindow(g_hwndHeartbeat);
	return 0;
}
