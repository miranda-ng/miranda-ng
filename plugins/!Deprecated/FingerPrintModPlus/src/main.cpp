/*
Fingerprint Mod+ (client version) icons module for Miranda IM

Copyright © 2006-2007 Artem Shpynov aka FYR, Bio, Faith Healer. 2009-2010 HierOS

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

//Start of header
#include "global.h"

#define LIB_REG		2
#define LIB_USE		3

HINSTANCE g_hInst;
int       hLangpack;

HANDLE hHeap					= NULL;

HANDLE compClientServA			= NULL;
HANDLE getClientIconA			= NULL;

HANDLE compClientServW			= NULL;
HANDLE getClientIconW			= NULL;
LPSTR  g_szClientDescription	= NULL;

HANDLE hStaticHooks[1]			= { NULL };

//End of header

// PluginInfo & PluginInfoEx
PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__INTERNAL_NAME_STRING,
	__VERSION_DWORD,
	"Fingerprint Mod+ (client version) icons module set extra icon of your buddies according to their client version.",
	"Artem Shpynov aka FYR, Bio, Faith Healer, HierOS",
	"crazy.hieros@gmail.com",
	__LEGAL_COPYRIGHT_STRING,
	"http://miranda-ng.org/",
	UNICODE_AWARE,
	MIID_THIS_PLUGIN
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_FINGERPRINT, MIID_LAST };

///////////////////////////////////////////////////////////////////////////////

static int OnPreShutdown(WPARAM wParam, LPARAM lParam)
{
	DestroyServiceFunction(compClientServA);
	DestroyServiceFunction(getClientIconA);
	DestroyServiceFunction(compClientServW);
	DestroyServiceFunction(getClientIconW);

	return 0;
}

extern "C" int	__declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfoEx);

	hStaticHooks[0] = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
	compClientServA = CreateServiceFunction(MS_FP_SAMECLIENTS, ServiceSameClientsA);
	getClientIconA = CreateServiceFunction(MS_FP_GETCLIENTICON, ServiceGetClientIconA);

	compClientServW = CreateServiceFunction(MS_FP_SAMECLIENTSW, ServiceSameClientsW);
	getClientIconW = CreateServiceFunction(MS_FP_GETCLIENTICONW, ServiceGetClientIconW);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

extern "C" int	__declspec(dllexport) Unload()
{
	if (g_szClientDescription != NULL)
		mir_free(g_szClientDescription);

	HeapDestroy(hHeap);
	ClearFI();

	for (size_t i = 0; i < SIZEOF(hStaticHooks); i++)
	{
		UnhookEvent(hStaticHooks[i]);
		hStaticHooks[i] = NULL;
	}
	return 0;
}
