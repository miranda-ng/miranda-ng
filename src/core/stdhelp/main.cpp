/*

Standard Help/About plugin for Myranda IM

Copyright (C) 2012 George Hazan

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "commonheaders.h"

int LoadHelpModule(void);

CLIST_INTERFACE* pcli;
TIME_API tmi;
HINSTANCE hInst;
int hLangpack;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	MIRANDA_VERSION_DWORD,
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// 1e64fd80-299e-48a0-9441-de2868563b6f
	{ 0x1e64fd80, 0x299e, 0x48a0, {0x94, 0x41, 0xde, 0x28, 0x68, 0x56, 0x3b, 0x6f}}
};

static const MUUID interfaces[] = { MIID_UIHELP, MIID_LAST };

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	mir_getTMI(&tmi);

	pcli = ( CLIST_INTERFACE* )CallService(MS_CLIST_RETRIEVE_INTERFACE, 0, (LPARAM)hInst);

	LoadHelpModule();
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
