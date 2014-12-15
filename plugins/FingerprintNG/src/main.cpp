/*
Fingerprint NG (client version) icons module for Miranda NG
Copyright © 2006-12 FYR, Bio, nullbie, ghazan, mataes, HierOS, faith_healer and all respective contributors.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

//Start of header
#include "global.h"

#define LIB_REG		2
#define LIB_USE		3

HINSTANCE g_hInst;
int hLangpack;

HANDLE hHeap = NULL;

//End of header

// PluginInfo & PluginInfoEx
PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {687364AF-58B0-4AF2-A4EE-20F40A8D9AFB}
	{ 0x687364af, 0x58b0, 0x4af2, { 0xa4, 0xee, 0x20, 0xf4, 0xa, 0x8d, 0x9a, 0xfb } }
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	g_hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

extern "C" int	__declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfoEx);

	InitFingerModule();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

extern "C" int	__declspec(dllexport) Unload()
{
	HeapDestroy(hHeap);
	ClearFI();
	return 0;
}
