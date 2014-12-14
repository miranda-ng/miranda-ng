/*
Copyright (c) 2013-14 Miranda NG project (http://miranda-ng.org)

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
	// {32579908-724B-467F-ADAD-22B6359A749A}
	{ 0x32579908, 0x724b, 0x467f, {0xad, 0xad, 0x22, 0xb6, 0x35, 0x9a, 0x74, 0x9a}}
};

/////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD, LPVOID)
{
	hInst = hModule;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD)
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

static CVkProto* vkProtoInit(const char* pszProtoName, const TCHAR *tszUserName)
{
	CVkProto *ppro = new CVkProto(pszProtoName, tszUserName);
	return ppro;
}

static int vkProtoUninit(CVkProto *ppro)
{
	delete ppro;
	return 0;
}

extern "C" int __declspec(dllexport) Load()
{
	mir_getLP(&pluginInfo);
	mir_getCLI();

	InitIcons();

	// Register protocol module
	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = "VKontakte";
	pd.fnInit = (pfnInitProto)vkProtoInit;
	pd.fnUninit = (pfnUninitProto)vkProtoUninit;
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
