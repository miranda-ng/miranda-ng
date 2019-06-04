/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-17 Robert Pösel, 2017-19 Miranda NG team

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

CMPlugin g_plugin;

std::string g_strUserAgent;
DWORD g_mirandaVersion;
bool g_bMessageState;
HWND g_hwndHeartbeat;

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
	// {8432B009-FF32-4727-AAE6-A9035038FD58}
	{ 0x8432b009, 0xff32, 0x4727, { 0xaa, 0xe6, 0xa9, 0x3, 0x50, 0x38, 0xfd, 0x58 } }
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<FacebookProto>(FACEBOOK_NAME, pluginInfoEx)
{
	SetUniqueId(FACEBOOK_KEY_ID);
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
	g_hwndHeartbeat = CreateWindowEx(0, L"STATIC", nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);

	HookEvent(ME_SYSTEM_MODULELOAD, OnModuleLoaded);
	HookEvent(ME_SYSTEM_MODULEUNLOAD, OnModuleLoaded);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModuleLoaded);

	InitIcons();
	InitContactMenus();

	// Init native User-Agent
	MFileVersion v;
	Miranda_GetFileVersion(&v);
	std::stringstream agent;
	agent << "Miranda NG/" << v[0] << "." << v[1] << "." << v[2];
#ifdef _WIN64
	agent << " Facebook Protocol RM x64/";
#else
	agent << " Facebook Protocol RM/";
#endif
	agent << __VERSION_STRING_DOTS;
	g_strUserAgent = agent.str();

	// Initialize random generator (used only as fallback in utils)
	srand(::time(0));

	return 0;
}

int CMPlugin::Unload()
{
	DestroyWindow(g_hwndHeartbeat);
	return 0;
}
