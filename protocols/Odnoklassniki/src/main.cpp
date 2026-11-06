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
	// {B3D27A51-8E4C-4F09-A6D2-5C1E93F07B48}
	{ 0xb3d27a51, 0x8e4c, 0x4f09, { 0xa6, 0xd2, 0x5c, 0x1e, 0x93, 0xf0, 0x7b, 0x48 } }
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<COdnoklassnikiProto>("Odnoklassniki", pluginInfoEx)
{
	SetUniqueId(DB_KEY_ID);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Protocol icon"), "main", IDI_OK }
};

int CMPlugin::Load()
{
	registerIcon("Protocols/Odnoklassniki", iconList, "Odnoklassniki");
	return 0;
}
