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

===============================================================================

File name      : $HeadURL: main.cpp $
Revision       : $Revision: $
Last change on : $Date: 2010-07-03 15:25:00 $
Last change by : $Author: Merlin_de $

===============================================================================
*/

#include "headers.h"

HINSTANCE hInst = 0;
PLUGINLINK *pluginLink = 0;

HMODULE hMsfteditDll = 0;
HRESULT	(WINAPI *MyCreateTextServices)(IUnknown *punkOuter, ITextHost *pITextHost, IUnknown **ppUnk);

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	MTEXT_DISPLAYNAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION,__MINOR_VERSION,__RELEASE_NUM,__BUILD_NUM),
	MTEXT_DESCRIPTION,
	MTEXT_AUTHOR,
	MTEXT_EMAIL,
	MTEXT_COPYRIGHT,
	MTEXT_WEBPAGE,
	UNICODE_AWARE,
	0,
	MTEXT_UUID
};

PLUGININFO pluginInfo =
{
	sizeof(PLUGININFO),
	pluginInfoEx.shortName,
	pluginInfoEx.version,
	pluginInfoEx.description,
	pluginInfoEx.author,
	pluginInfoEx.authorEmail,
	pluginInfoEx.copyright,
	pluginInfoEx.homepage,
	pluginInfoEx.flags,
	pluginInfoEx.replacesDefaultModule
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

static HANDLE hModulesLoaded = 0;
static int ModulesLoaded(WPARAM wParam,LPARAM lParam);

extern "C" __declspec(dllexport) PLUGININFO *MirandaPluginInfo(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return &pluginInfo;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	pluginInfoEx.cbSize = sizeof(PLUGININFOEX);
	return &pluginInfoEx;
}

extern "C" __declspec(dllexport) const MUUID *MirandaPluginInterfaces(void)
{
	static const MUUID interfaces[] = { MTEXT_UUID, MIID_LAST };
	return interfaces;
}

extern "C" __declspec(dllexport) int Load(PLUGINLINK *link)
{
	pluginLink = link;

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

//	LoadFancy();
	LoadEmfCache();
	LoadRichEdit();
	LoadTextUsers();
	LoadServices();
	//LoadOptions();

	MTextControl_RegisterClass();
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	return 0;
}

static int ModulesLoaded(WPARAM wParam,LPARAM lParam)
{
//	char fn[MAX_PATH+1];
//	GetModuleFileName(hInst, fn, MAX_PATH);
//	InitFancy();
//	InitIcons(icons, fn, iconCount);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	UnhookEvent(hModulesLoaded);
//	UnloadOptions();
	UnloadServices();
	UnloadTextUsers();
	UnloadRichEdit();
	UnloadEmfCache();
//	UnloadFancy();
	FreeLibrary(hMsfteditDll);
	return 0;
}
