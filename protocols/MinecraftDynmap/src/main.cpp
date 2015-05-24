/*

Minecraft Dynmap plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2015 Robert Pösel

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
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {40DA5EBD-4F2D-4BEA-841C-EAB77BEE6F4F}
	{ 0x40da5ebd, 0x4f2d, 0x4bea, 0x84, 0x1c, 0xea, 0xb7, 0x7b, 0xee, 0x6f, 0x4f } }
};

/////////////////////////////////////////////////////////////////////////////
// Protocol instances
static int compare_protos(const MinecraftDynmapProto *p1, const MinecraftDynmapProto *p2)
{
	return _tcscmp(p1->m_tszUserName, p2->m_tszUserName);
}

OBJLIST<MinecraftDynmapProto> g_Instances(1, compare_protos);

DWORD WINAPI DllMain(HINSTANCE hInstance,DWORD,LPVOID)
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

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

/////////////////////////////////////////////////////////////////////////////////////////
// Load

static PROTO_INTERFACE* protoInit(const char *proto_name,const TCHAR *username)
{
	MinecraftDynmapProto *proto = new MinecraftDynmapProto(proto_name, username);
	g_Instances.insert(proto);
	return proto;
}

static int protoUninit(PROTO_INTERFACE* proto)
{
	g_Instances.remove((MinecraftDynmapProto*)proto);
	return EXIT_SUCCESS;
}

static HANDLE g_hEvents[1];

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	mir_getCLI();

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = "MinecraftDynmap";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = protoInit;
	pd.fnUninit = protoUninit;
	CallService(MS_PROTO_REGISTERMODULE, 0, reinterpret_cast<LPARAM>(&pd));

	InitIcons();

	// Init native User-Agent
	{
		std::stringstream agent;
		agent << "Miranda NG/";
		agent << ((g_mirandaVersion >> 24) & 0xFF);
		agent << ".";
		agent << ((g_mirandaVersion >> 16) & 0xFF);
		agent << ".";
		agent << ((g_mirandaVersion >>  8) & 0xFF);
		agent << ".";
		agent << ((g_mirandaVersion     ) & 0xFF);
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

extern "C" int __declspec(dllexport) Unload(void)
{
	for (size_t i=0; i < SIZEOF(g_hEvents); i++)
		UnhookEvent(g_hEvents[i]);

	return 0;
}
