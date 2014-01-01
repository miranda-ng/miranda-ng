/*
Miranda core extensions

Copyright (c) 2000-07 Miranda ICQ/IM project,
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

#include "headers.h"
#include <win2k.h>

int LoadW7UI();
int UnloadW7UI();

HINSTANCE g_hInst;
int       hLangpack;

// {3625ACB8-794C-4727-88EA-76DBBAC6D200}
#define MIID_W7UI	{ 0x3625acb8, 0x794c, 0x4727, { 0x88, 0xea, 0x76, 0xdb, 0xba, 0xc6, 0xd2, 0x0 } }


PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"Windows 7 UI",
	PLUGIN_MAKE_VERSION(0, 0, 0, 1),
	"Provides support for Windows 7 Taskbar feautures such as Aero Peek, Overlay icons, Jump Lists, Progress Bar",
	"nullbie, persei",
	"nullbie@miranda.im",
	"2009 Victor Pavlychko, Vitaliy Igonin",
	"http://miranda-ng.org/",
	UNICODE_AWARE,	           // replace internal version (if any)
	// {D38EEB0B-B8EE-4177-B9E5-91EBE101E054}
	{ 0xd38eeb0b, 0xb8ee, 0x4177, { 0xb9, 0xe5, 0x91, 0xeb, 0xe1, 0x1, 0xe0, 0x54 } }
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

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_W7UI, MIID_LAST };

extern "C" __declspec(dllexport) int Load(void)
{
	if (!IsWinVer7Plus()) return 1;


	mir_getLP(&pluginInfo);

	LoadW7UI();

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	UnloadW7UI();
	return 0;
}
