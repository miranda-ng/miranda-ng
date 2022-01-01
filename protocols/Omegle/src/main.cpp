/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-17 Robert Pösel, 2017-22 Miranda NG team

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

// TODO: Make following as "globals" structure?

CMPlugin g_plugin;

uint32_t g_mirandaVersion;

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
	// {9E1D9244-606C-4ef4-99A0-1D7D23CB7601}
	{ 0x9e1d9244, 0x606c, 0x4ef4, { 0x99, 0xa0, 0x1d, 0x7d, 0x23, 0xcb, 0x76, 0x1 } }
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<OmegleProto>("Omegle", pluginInfoEx)
{
	SetUniqueId("Nick");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Interface information

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////
// Load

int CMPlugin::Load()
{
	InitIcons();
	return 0;
}
