// Copyright © 2009-2010 sss
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "stdafx.h"

CMPlugin g_plugin;

BOOL bWatrackService = FALSE;
wchar_t *gbHost, *gbPassword;
uint16_t gbPort;
HNETLIBUSER ghNetlibUser;

/////////////////////////////////////////////////////////////////////////////////////////

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
	// 692E87D0-6C71-4CDC-9E36-2B69FBDC4C
	{ 0x692e87d0, 0x6c71, 0x4cdc, {0x9e, 0x36, 0x2b, 0x2d, 0x69, 0xfb, 0xdc, 0x4c }}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

static int OnModulesLoaded(WPARAM, LPARAM)
{
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szDescriptiveName.w = TranslateT("Watrack MPD connection");
	nlu.szSettingsModule = __PLUGIN_NAME;
	ghNetlibUser = Netlib_RegisterUser(&nlu);

	gbPort = g_plugin.getWord("Port", 6600);
	gbHost = db_get_wsa(0, MODULENAME, "Server", L"127.0.0.1");
	gbPassword = db_get_wsa(0, MODULENAME, "Password", L"");

	if (ServiceExists(MS_WAT_PLAYER))
		bWatrackService = TRUE;

	RegisterPlayer();
	return 0;
}

int CMPlugin::Load()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, WaMpdOptInit);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	mir_free(gbHost);
	mir_free(gbPassword);
	return 0;
}
