/*
Copyright 2000-2012 Miranda IM project,
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

int LoadSendRecvMessageModule(void);
int SplitmsgShutdown(void);

PLUGINLINK* pluginLink;
HINSTANCE   g_hInst;
int hLangpack;

struct MM_INTERFACE mmi;
struct UTF8_INTERFACE utfi;
struct LIST_INTERFACE li;
TIME_API tmi;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"Send/Receive Messages",
	__VERSION_DWORD,
	"Send and receive instant messages",
	"Miranda IM Development Team",
	"rainwater@miranda-im.org",
	"Copyright 2000-2012 Miranda IM project",
	"http://www.miranda-im.org",
	UNICODE_AWARE,
	DEFMOD_SRMESSAGE,            // replace internal version (if any)
#ifdef _UNICODE
	{0x657fe89b, 0xd121, 0x40c2, { 0x8a, 0xc9, 0xb9, 0xfa, 0x57, 0x55, 0xb3, 0xc }} //{657FE89B-D121-40c2-8AC9-B9FA5755B30C}
#else
	{0xd53dd778, 0x16d2, 0x49ac, { 0x8f, 0xb3, 0x6f, 0x9a, 0x96, 0x1c, 0x9f, 0xd2 }} //{D53DD778-16D2-49ac-8FB3-6F9A961C9FD2}
#endif
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInst = hinstDLL;
	return TRUE;
}

__declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion < MIRANDA_VERSION_CORE )
		return NULL;
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_SRMM, MIID_LAST};
__declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

int __declspec(dllexport) Load(PLUGINLINK * link)
{
	pluginLink = link;
	mir_getMMI(&mmi);
	mir_getUTFI(&utfi);
	mir_getLI(&li);
	mir_getTMI(&tmi);
	mir_getLP(&pluginInfo);

	return LoadSendRecvMessageModule();
}

int __declspec(dllexport) Unload(void)
{
	UnloadOptions();
	return SplitmsgShutdown();
}
