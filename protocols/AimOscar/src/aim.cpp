/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2012 Boris Krasnovskiy
Copyright (C) 2005-2006 Aaron Myles Landwehr

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "stdafx.h"

char AIM_CAP_MIRANDA[16] = "MirandaA";

int hLangpack;

HINSTANCE hInstance;

/////////////////////////////////////////////////////////////////////////////
// Protocol instances
static int sttCompareProtocols(const CAimProto *p1, const CAimProto *p2)
{
	return mir_tstrcmp(p1->m_tszUserName, p2->m_tszUserName);
}

OBJLIST<CAimProto> g_Instances(1, sttCompareProtocols);

/////////////////////////////////////////////////////////////////////////////////////////
// Dll entry point

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD /*fdwReason*/,LPVOID /*lpvReserved*/)
{
	hInstance = hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Plugin information

static const PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {3750A5A3-BF0D-490E-B65D-41AC4D29AEB3}
	{0x3750a5a3, 0xbf0d, 0x490e, {0xb6, 0x5d, 0x41, 0xac, 0x4d, 0x29, 0xae, 0xb3}}
};

extern "C" __declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	*(unsigned long*)(&AIM_CAP_MIRANDA[8]) = _htonl(mirandaVersion);
	*(unsigned long*)(&AIM_CAP_MIRANDA[12]) = _htonl(PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM));
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Interface information

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

////////////////////////////////////////////////////////////////////////////////////////
//	OnModulesLoaded - finalizes plugin's configuration on load

static int OnModulesLoaded(WPARAM, LPARAM)
{
	aim_links_init();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load

static PROTO_INTERFACE* protoInit(const char* pszProtoName, const TCHAR* tszUserName)
{
	CAimProto *ppro = new CAimProto(pszProtoName, tszUserName);
	g_Instances.insert(ppro);
	return ppro;
}

static int protoUninit(PROTO_INTERFACE* ppro)
{
	g_Instances.remove((CAimProto*)ppro);
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	PROTOCOLDESCRIPTOR pd = { 0 };
	pd.cbSize = sizeof(pd);
	pd.szName = "AIM";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = protoInit;
	pd.fnUninit = protoUninit;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM) & pd);

	InitIcons();
	InitExtraIcons();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload

extern "C" int __declspec(dllexport) Unload(void)
{
	aim_links_destroy();
	return 0;
}
