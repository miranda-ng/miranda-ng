/*
Miranda Text Control - Plugin for Miranda IM

Copyright	© 2005 Victor Pavlychko (nullbie@gmail.com),
			© 2010 Merlin_de

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

HINSTANCE hInst = 0;
int hLangpack;

HMODULE hMsfteditDll = 0;
HRESULT	(WINAPI *MyCreateTextServices)(IUnknown *punkOuter, ITextHost *pITextHost, IUnknown **ppUnk);

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {69B9443B-DC58-4876-AD39-E3F418A133C5}
	{0x69b9443b, 0xdc58, 0x4876, {0xad, 0x39, 0xe3, 0xf4, 0x18, 0xa1, 0x33, 0xc5}}
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfoEx);

	//6.0A SDK is missing RichEd20.lib for x64
	//http://connect.microsoft.com/VisualStudio/feedback/details/551071/the-6-0a-sdk-is-missing-riched20-lib-for-x64
	MyCreateTextServices = 0;
	hMsfteditDll = LoadLibrary(_T("msftedit.dll"));
	if (hMsfteditDll) {
		MyCreateTextServices = (HRESULT (WINAPI *)(
			IUnknown *punkOuter,
			ITextHost *pITextHost,
			IUnknown **ppUnk))
			GetProcAddress(hMsfteditDll, "CreateTextServices");
	}

	LoadEmfCache();
	LoadRichEdit();
	LoadTextUsers();
	LoadServices();

	MTextControl_RegisterClass();
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	UnloadTextUsers();
	UnloadRichEdit();
	UnloadEmfCache();
	FreeLibrary(hMsfteditDll);
	return 0;
}
