/*

Standard e-mail urls launcher for Miranda NG

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

int LoadSendRecvEMailModule(void);

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
	// {b774d10a-c761-11e1-8405-27e76188709b}
	{0xb774d10a, 0xc761, 0x11e1, {0x84, 0x05, 0x27, 0xe7, 0x61, 0x88, 0x70, 0x9b }}
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

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SREMAIL, MIID_LAST };

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	LoadSendRecvEMailModule();
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
