/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

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

#include "msn_global.h"
#include "msn_proto.h"
#include "version.h"

HINSTANCE hInst;

int hLangpack;
TIME_API tmi;

/////////////////////////////////////////////////////////////////////////////////////////
// Initialization routines

void MsnLinks_Init(void);
void MsnLinks_Destroy(void);

/////////////////////////////////////////////////////////////////////////////////////////
// Global variables

int avsPresent = -1;

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
	// {97724AF9-F3FB-47d3-A3BF-EAA935C74E6D}
	{0x97724af9, 0xf3fb, 0x47d3, {0xa3, 0xbf, 0xea, 0xa9, 0x35, 0xc7, 0x4e, 0x6d}}
};

int MSN_GCEventHook(WPARAM wParam, LPARAM lParam);
int MSN_GCMenuHook(WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////
// Protocol instances
static int sttCompareProtocols(const CMsnProto *p1, const CMsnProto *p2)
{
	return _tcscmp(p1->m_tszUserName, p2->m_tszUserName);
}

OBJLIST<CMsnProto> g_Instances(1, sttCompareProtocols);

/////////////////////////////////////////////////////////////////////////////////////////
//	Main DLL function

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		hInst = hinstDLL;
		DisableThreadLibraryCalls(hinstDLL);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
//	OnModulesLoaded - finalizes plugin's configuration on load

static int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	avsPresent = ServiceExists(MS_AV_SETMYAVATART) != 0;

	MsnLinks_Init();

	return 0;
}

static CMsnProto* msnProtoInit(const char* pszProtoName, const TCHAR* tszUserName)
{
	CMsnProto *ppro = new CMsnProto(pszProtoName, tszUserName);
	g_Instances.insert(ppro);
	return ppro;
}

static int msnProtoUninit(CMsnProto* ppro)
{
	g_Instances.remove(ppro);
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Performs a primary set of actions upon plugin loading

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getTMI(&tmi);
	mir_getLP(&pluginInfo);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = "MSN";
	pd.fnInit = (pfnInitProto)msnProtoInit;
	pd.fnUninit = (pfnUninitProto)msnProtoUninit;
	pd.type = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	MsnInitIcons();
	MSN_InitContactMenu();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload a plugin

extern "C" int __declspec(dllexport) Unload(void)
{
	MSN_RemoveContactMenus();
	MsnLinks_Destroy();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MirandaPluginInfoEx - returns an information about a plugin

extern "C" __declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MirandaInterfaces - returns the protocol interface to the core

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };
