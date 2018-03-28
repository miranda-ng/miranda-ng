/*

Omegle plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-17 Robert Pösel, 2017-18 Miranda NG team

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
CLIST_INTERFACE* pcli;
int hLangpack;

HINSTANCE g_hInstance;
std::string g_strUserAgent;
DWORD g_mirandaVersion;

PLUGININFOEX pluginInfo = {
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

DWORD WINAPI DllMain(HINSTANCE hInstance, DWORD, LPVOID)
{
	g_hInstance = hInstance;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	g_mirandaVersion = mirandaVersion;
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Interface information

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////
// Load

static HANDLE g_hEvents[1];

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	pcli = Clist_GetInterface();

	InitIcons();
	//InitContactMenus();

	// Init native User-Agent
	{
		std::stringstream agent;
		agent << "Miranda NG/";
		agent << ((g_mirandaVersion >> 24) & 0xFF);
		agent << ".";
		agent << ((g_mirandaVersion >> 16) & 0xFF);
		agent << ".";
		agent << ((g_mirandaVersion >> 8) & 0xFF);
		agent << ".";
		agent << ((g_mirandaVersion)& 0xFF);
#ifdef _WIN64
		agent << " Omegle Protocol x64/";
#else
		agent << " Omegle Protocol/";
#endif
		agent << __VERSION_STRING_DOTS;
		g_strUserAgent = agent.str();
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload

extern "C" int __declspec(dllexport) Unload(void)
{
	//UninitContactMenus();
	for (size_t i = 0; i < _countof(g_hEvents); i++)
		UnhookEvent(g_hEvents[i]);

	return 0;
}
