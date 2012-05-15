/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2012 Boris Krasnovskiy
Copyright (C) 2005-2006 Aaron Myles Landwehr

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "aim.h"
#include "version.h"

char AIM_CAP_MIRANDA[16] = "MirandaA";

PLUGINLINK *pluginLink;
MD5_INTERFACE md5i;
MM_INTERFACE mmi;
UTF8_INTERFACE utfi;
LIST_INTERFACE li;
int hLangpack;

HINSTANCE hInstance;

static HANDLE hMooduleLoaded;

/////////////////////////////////////////////////////////////////////////////
// Protocol instances
static int sttCompareProtocols(const CAimProto *p1, const CAimProto *p2)
{
	return _tcscmp(p1->m_tszUserName, p2->m_tszUserName);
}

OBJLIST<CAimProto> g_Instances(1, sttCompareProtocols);

/////////////////////////////////////////////////////////////////////////////////////////
// Dll entry point

extern "C" 
BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD /*fdwReason*/,LPVOID /*lpvReserved*/)
{
	hInstance = hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Plugin information

static const PLUGININFOEX pluginInfo = 
{
	sizeof(PLUGININFOEX),
	"AIM Protocol",
	__VERSION_DWORD,
	"Provides support for AOL® Instant Messenger (AIM) protocol",
	"Boris Krasnovskiy, Aaron Myles Landwehr",
	"borkra@miranda-im.org",
	"© 2008-2011 Boris Krasnovskiy, 2005-2006 Aaron Myles Landwehr",
	"http://www.miranda-im.org",
	UNICODE_AWARE,		//not transient
	0,		//doesn't replace anything built-in
	#if defined( _UNICODE )
	{0x3750a5a3, 0xbf0d, 0x490e, {0xb6, 0x5d, 0x41, 0xac, 0x4d, 0x29, 0xae, 0xb3}} // {3750A5A3-BF0D-490e-B65D-41AC4D29AEB3}
	#else
	{0x591af304, 0xcf40, 0x47ad, {0x8c, 0xed, 0x58, 0xcd, 0x89, 0x8b, 0x1e, 0x69}} // {591AF304-CF40-47ad-8CED-58CD898B1E69}
	#endif
};

extern "C" __declspec(dllexport) const PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	if (mirandaVersion < MIRANDA_VERSION_CORE) 
	{
		MessageBox(NULL, 
			_T("The AIM protocol plugin cannot be loaded. It requires Miranda IM ") 
			_T(MIRANDA_VERSION_CORE_STRING) _T(" or later."),
			_T("Miranda"), MB_OK | MB_ICONWARNING | MB_SETFOREGROUND | MB_TOPMOST);
		return NULL;
	}

	*(unsigned long*)(&AIM_CAP_MIRANDA[8]) = _htonl(mirandaVersion);
	*(unsigned long*)(&AIM_CAP_MIRANDA[12]) = _htonl(__VERSION_DWORD);
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Interface information

static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

////////////////////////////////////////////////////////////////////////////////////////
//	OnModulesLoaded - finalizes plugin's configuration on load

static int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	aim_links_init();
	InitExtraIcons();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load

static PROTO_INTERFACE* protoInit(const char* pszProtoName, const TCHAR* tszUserName)
{
	CAimProto *ppro = new CAimProto(pszProtoName, tszUserName);
	g_Instances.insert(ppro);
	return ppro;
}

static int protoUninit(PROTO_INTERFACE* ppro)
{
	g_Instances.remove((CAimProto*)ppro);
	return 0;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;
	mir_getMMI(&mmi);
	mir_getMD5I(&md5i);
	mir_getUTFI(&utfi);
	mir_getLI(&li);
	mir_getLP(&pluginInfo);

	hMooduleLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize = sizeof(pd);
	pd.szName = "AIM";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = protoInit;
	pd.fnUninit = protoUninit;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM) & pd);

	InitThemeSupport();
	InitIcons();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload

extern "C" int __declspec(dllexport) Unload(void)
{
	aim_links_destroy();
	UnhookEvent(hMooduleLoaded);

	DestroyExtraIcons();
	return 0;
}
