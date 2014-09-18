/*
Copyright (c) 2014 Robert Pösel

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
#include "version.h"

HINSTANCE hInst;
int hLangpack;
CLIST_INTERFACE *pcli;

PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {2A1081D1-AEE3-4091-B70D-AE46D09F9A7F}
	{ 0x2a1081d1, 0xaee3, 0x4091, {0xb7, 0xd, 0xae, 0x46, 0xd0, 0x9f, 0x9a, 0x7f}}
};

/////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD, LPVOID)
{
	hInst = hModule;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

/////////////////////////////////////////////////////////////////////////////////////////
// OnModulesLoaded - execute some code when all plugins are initialized

static int OnModulesLoaded(WPARAM, LPARAM)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnLoad - initialize the plugin instance

static CDummyProto* dummyProtoInit(const char* pszProtoName, const TCHAR *tszUserName)
{
	CDummyProto *ppro = new CDummyProto(pszProtoName, tszUserName);
	return ppro;
}

static int dummyProtoUninit(CDummyProto *ppro)
{
	delete ppro;
	return 0;
}

extern "C" int __declspec(dllexport) Load()
{
	mir_getLP(&pluginInfo);
	mir_getCLI();

	// Register protocol module
	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = "Dummy";
	pd.fnInit = (pfnInitProto)dummyProtoInit;
	pd.fnUninit = (pfnUninitProto)dummyProtoUninit;
	pd.type = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload - destroy the plugin instance

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
