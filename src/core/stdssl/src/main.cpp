/*

Standard SSL driver for Miranda NG

Copyright (c) 2014-21  Miranda NG team (https://miranda-ng.org)

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

int LoadSslModule(void);
void UnloadSslModule(void);

CMPlugin g_plugin;

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	MIRANDA_VERSION_DWORD,
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// { 312C4F84 - 75BE - 4404 - BCB1 - C103DBE5A3B8 }
	{ 0x312C4F84, 0x75BE, 0x4404, {0xBC, 0xB1, 0xC1, 0x03, 0xDB, 0xE5, 0xA3, 0xB8 }}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(nullptr, pluginInfoEx)
{}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SSL, MIID_LAST };

int CMPlugin::Load()
{
	return LoadSslModule();
}

int CMPlugin::Unload()
{
	UnloadSslModule();
	return 0;
}
