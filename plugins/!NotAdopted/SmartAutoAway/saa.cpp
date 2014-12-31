/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-06 Miranda ICQ/IM project,
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

#include "saa.h"

int InitVariables( WPARAM wParam, LPARAM lParam );

HINSTANCE g_hInst;

int hLangpack;

PLUGININFOEX pluginInfo = {
    sizeof(PLUGININFOEX),
#ifdef AALOG
	SECTIONNAME " - Debug",
#else
	SECTIONNAME,
#endif
    PLUGIN_MAKE_VERSION(0, 0, 3, 3),
    "",
    "YB",
    "y_b" /* these */ "@" /* fucking spammers */ "saaplugin.no-ip.info" /* gather everything */,
    "© 2005-2009 YB; 2000-2006 Miranda-IM",
    "http://saaplugin.no-ip.info/",
	UNICODE_AWARE,
	// EC29E895-9F97-4cb1-AC9B-B1AE88258B4A
	{0xec29e895,0x9f97,0x4cb1,{0xac, 0x9b, 0xb1, 0xae, 0x88, 0x25, 0x8b, 0x4a}}
};

static	HMODULE           hUxTheme = 0;
BOOL (WINAPI *enableThemeDialogTexture)(HANDLE, DWORD) = 0;


extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    g_hInst = hinstDLL;
    return TRUE;
}
static	char description[1023];

#ifdef AALOG
	static BOOL warningShown = 0;
#endif
static bool isNewMiranda = false;

extern "C" __declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD mirandaVersion)
{
	isNewMiranda=true;
	return &pluginInfo;
}


extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_AUTOAWAY, MIID_IDLE, MIID_LAST};

extern "C" int __declspec(dllexport) Load()
{
	LoadAutoAwayModule();
    hUxTheme = GetModuleHandle(_T("uxtheme.dll"));
    if(hUxTheme)
       enableThemeDialogTexture = (BOOL (WINAPI *)(HANDLE, DWORD))GetProcAddress(hUxTheme, "EnableThemeDialogTexture");
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
//	AutoAwayShutdown(0,0);
//#ifdef AALOG
//	CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)"Killing Timer");
//#endif
//	KillTimer(NULL, hIdleTimer);
//#ifdef AALOG
//	CallService("Netlib/Log" ,(WPARAM)hNetlib ,(LPARAM)"DestroyHookableEvent(hIdleEvent)");
//#endif
//	DestroyHookableEvent(hIdleEvent);

    if(hUxTheme != 0)
        FreeLibrary(hUxTheme);

    return 1; //if 1 we dont want to shutdown because we have hooked ME_SYSTEM_SHUTDOWN
}
