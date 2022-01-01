/*

Minecraft Dynmap plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2015-17 Robert Pösel, 2017-22 Miranda NG team

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

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {40DA5EBD-4F2D-4BEA-841C-EAB77BEE6F4F}
	{ 0x40da5ebd, 0x4f2d, 0x4bea, 0x84, 0x1c, 0xea, 0xb7, 0x7b, 0xee, 0x6f, 0x4f }
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<MinecraftDynmapProto>("MinecraftDynmap", pluginInfoEx)
{
	SetUniqueId("Nick");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Interface information

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

/////////////////////////////////////////////////////////////////////////////////////////
// Load

static HANDLE g_hEvents[1];

int CMPlugin::Load()
{
	InitIcons();

	// Init native User-Agent
	{
		MFileVersion w;
		Miranda_GetFileVersion(&w);
		std::stringstream agent;
		agent << "Miranda NG/" << w[0] << "." << w[1] << "." << w[2] << "." << w[3];
	#ifdef _WIN64
		agent << " Minecraft Dynmap Protocol x64/";
	#else
		agent << " Minecraft Dynmap Protocol/";
	#endif
		agent << __VERSION_STRING_DOTS;
		g_strUserAgent = agent.str();
	}

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload

int CMPlugin::Unload()
{
	for (size_t i=0; i < _countof(g_hEvents); i++)
		UnhookEvent(g_hEvents[i]);

	return 0;
}

