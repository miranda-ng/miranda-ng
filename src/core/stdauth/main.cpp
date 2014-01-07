/*

Standard Send/Recv Auth plugin for Miranda NG

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

int LoadSendRecvAuthModule(void);

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
	/* 8d0a046d-8ea9-4c55-b568-38da520564fd */
	{ 0x8d0a046d, 0x8ea9, 0x4c55, {0xb5, 0x68, 0x38, 0xda, 0x52, 0x05, 0x64, 0xfd}}
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

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SRAUTH, MIID_LAST };

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	LoadSendRecvAuthModule();
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
