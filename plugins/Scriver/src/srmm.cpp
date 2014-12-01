/*
Scriver

Copyright (c) 2000-12 Miranda ICQ/IM project,

all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "commonheaders.h"

int OnLoadModule(void);
int OnUnloadModule(void);

int hLangpack;
TIME_API tmi;
HINSTANCE g_hInst;
CLIST_INTERFACE *pcli;

ITaskbarList3 *pTaskbarInterface;

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
	// {84636F78-2057-4302-8A65-23A16D46844C}
	{ 0x84636f78, 0x2057, 0x4302, { 0x8a, 0x65, 0x23, 0xa1, 0x6d, 0x46, 0x84, 0x4c } }
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SRMM, MIID_CHAT, MIID_LAST };

extern "C" __declspec(dllexport) int Load(void)
{
	// set the memory manager
	mir_getTMI(&tmi);
	mir_getLP(&pluginInfo);
	mir_getCLI();

	if (IsWinVer7Plus())
		CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList3, (void**)&pTaskbarInterface);

	return OnLoadModule();
}

extern "C" __declspec(dllexport) int Unload(void)
{
	if (pTaskbarInterface)
		pTaskbarInterface->Release();
	return OnUnloadModule();
}
