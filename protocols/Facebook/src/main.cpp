/*

Facebook plugin for Miranda NG
Copyright © 2019-22 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "stdafx.h"

#pragma comment(lib, "Rpcrt4.lib")

CMPlugin g_plugin;

bool g_bMessageState;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {86033E58-A1E3-43AD-AE8E-305E15E72A91}
	{ 0xee0543fb, 0x711d, 0x4ac8, { 0xb6, 0xc0, 0x1d, 0xda, 0x48, 0x38, 0x10, 0x7e }}
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<FacebookProto>(MODULENAME, pluginInfoEx)
{
	SetUniqueId(DBKEY_ID);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Interface information

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////
// Load

static int OnModuleLoaded(WPARAM, LPARAM)
{
	g_bMessageState = ServiceExists(MS_MESSAGESTATE_UPDATE) != 0;
	return 0;
}

int CMPlugin::Load()
{
	HookEvent(ME_SYSTEM_MODULELOAD, OnModuleLoaded);
	HookEvent(ME_SYSTEM_MODULEUNLOAD, OnModuleLoaded);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModuleLoaded);

	// Initialize random generator (used only as fallback in utils)
	return 0;
}
