/*
Copyright (C) 2026 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

CMPlugin g_plugin;

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {6E5C1F2A-4B7D-4E39-9A0C-3D8B2F71C5E4}
	{ 0x6e5c1f2a, 0x4b7d, 0x4e39, { 0x9a, 0x0c, 0x3d, 0x8b, 0x2f, 0x71, 0xc5, 0xe4 } }
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CKicqProto>("KICQ", pluginInfoEx)
{
	SetUniqueId("UIN");
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Protocol icon"), "main", IDI_KICQ }
};

int CMPlugin::Load()
{
	registerIcon("Protocols/KICQ", iconList, "KICQ");
	return 0;
}
