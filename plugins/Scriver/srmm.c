/*
Scriver

Copyright 2000-2012 Miranda ICQ/IM project,

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

struct MM_INTERFACE mmi;
struct LIST_INTERFACE li;
struct UTF8_INTERFACE utfi;
TIME_API tmi;

PLUGINLINK *pluginLink;
HINSTANCE g_hInst;
int hLangpack;

ITaskbarList3 * pTaskbarInterface;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
#ifdef _UNICODE
	"Scriver (Unicode)",
#else
	"Scriver",
#endif
	PLUGIN_MAKE_VERSION(2, 10, 0, 2),
	"Scriver - send and receive instant messages",
	"Miranda IM Development Team",
	"the_leech@users.berlios.de",
	"Copyright (c) 2000-2012 Miranda IM Project",
	"http://www.miranda-im.org",
	UNICODE_AWARE,
	DEFMOD_SRMESSAGE,            // replace internal version (if any)
#ifdef _UNICODE
	{0x84636f78, 0x2057, 0x4302, { 0x8a, 0x65, 0x23, 0xa1, 0x6d, 0x46, 0x84, 0x4c }} //{84636F78-2057-4302-8A65-23A16D46844C}
#else
	{0x1e91b6c9, 0xe040, 0x4a6f, { 0xab, 0x56, 0xdf, 0x76, 0x98, 0xfa, 0xcb, 0xf1 }} //{1E91B6C9-E040-4a6f-AB56-DF7698FACBF1}
#endif
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInst = hinstDLL;
	return TRUE;
}

__declspec(dllexport)
	 PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 8, 0, 0))
		return NULL;
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_SRMM, MIID_CHAT, MIID_LAST};
__declspec(dllexport)
     const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

int __declspec(dllexport) Load(PLUGINLINK * link)
{
	pluginLink = link;

	// set the memory manager
	mir_getMMI( &mmi );
	mir_getLI( &li );
	mir_getUTFI( &utfi );
	mir_getTMI(&tmi);
	mir_getLP( &pluginInfo );

	if (IsWinVer7Plus())
		CoCreateInstance(&CLSID_TaskbarList, NULL, CLSCTX_ALL, &IID_ITaskbarList3, (void**)&pTaskbarInterface);

	InitSendQueue();
	return OnLoadModule();
}

int __declspec(dllexport) Unload(void)
{
	DestroySendQueue();
	if (pTaskbarInterface)
		pTaskbarInterface->lpVtbl->Release(pTaskbarInterface);
	return OnUnloadModule();
}
