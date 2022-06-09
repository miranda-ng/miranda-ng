/*

Standard User Info plugin for Miranda NG

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "stdafx.h"

int LoadUserInfoModule(void);

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	MIRANDA_VERSION_DWORD,
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {8198DC94-0BC4-448A-8495-8FE832C1D333}
	{ 0x8198dc94, 0xbc4, 0x448a, { 0x84, 0x95, 0x8f, 0xe8, 0x32, 0xc1, 0xd3, 0x33 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_UIUSERINFO, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	LoadUserInfoModule();
	return 0;
}
