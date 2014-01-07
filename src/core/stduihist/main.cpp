/*

Standard ugly history viewer for Miranda NG

Copyright (C) 2012-14 George Hazan

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

int LoadHistoryModule(void);

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
	/* 5eedf3c5-3071-4234-a627-efd062a4d694 */
	{ 0x5eedf3c5, 0x3071, 0x4234, {0xa6, 0x27, 0xef, 0xd0, 0x62, 0xa4, 0xd6, 0x94}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_UIHISTORY, MIID_LAST };

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	mir_getTMI(&tmi);
	mir_getCLI();

	LoadHistoryModule();
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
