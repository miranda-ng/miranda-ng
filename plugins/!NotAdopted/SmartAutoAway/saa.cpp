/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2006 Miranda ICQ/IM project,
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


PLUGINLINK *pluginLink;
HINSTANCE g_hInst;

PLUGININFOEX pluginInfo = {
    sizeof(PLUGININFOEX),
#ifdef AALOG
	SECTIONNAME " - Debug",
#else
	SECTIONNAME,
#endif
#include "version.txt"
	UNICODE_AWARE,
    DEFMOD_RNDAUTOAWAY,            // replace internal version (if any)
#if defined( _UNICODE )
	{0xec29e895,0x9f97,0x4cb1,{0xac, 0x9b, 0xb1, 0xae, 0x88, 0x25, 0x8b, 0x4a} /* EC29E895-9F97-4cb1-AC9B-B1AE88258B4A */}
#else
	{0x19c5be89,0xf7e3,0x4632,{0xa1, 0x25, 0x15, 0xf3, 0x84, 0xcd, 0x0c, 0xd3} /* 19c5be89-f7e3-4632-a125-15f384cd0cd3 */}
#endif
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
extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfo(DWORD mirandaVersion)
{

	sprintf(description,"Highly configurable AutoAway/Idle/Reconnect system");
#ifdef AALOG
	sprintf(description,"%s\r\nThis is the debug version. It will make a lot of noise through Netlib log",description);
	if (!warningShown) {
		MessageBoxA(0,description,pluginInfo.shortName,MB_ICONINFORMATION);
		warningShown = 1;
	}
#endif
	if ((mirandaVersion>=0x00030400) && (!isNewMiranda)) {
		sprintf(description,"%s\r\n%s",description,"Miranda %d.%d.%d.%d has build-in \"Idle\" support and it is highly advisable to suppress it by putting SAANI.dll in plugins directory.");
		sprintf(description,description, // Yes, I realize this is strange approach...
			mirandaVersion>>24&0xFF,
			mirandaVersion>>16&0xFF,
			mirandaVersion>>8&0xFF,
			mirandaVersion&0xFF
		);
	};

	pluginInfo.description = description;
	if ( mirandaVersion < PLUGIN_MAKE_VERSION( 0,7,0,17 )) pluginInfo.cbSize = sizeof( PLUGININFO );
    return &pluginInfo;
}

extern "C" __declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD mirandaVersion)
{
	isNewMiranda=true;
	return MirandaPluginInfo(mirandaVersion);
}

extern "C" __declspec(dllexport) const MUUID interfaces[] = {MIID_AUTOAWAY, MIID_IDLE, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID * MirandaPluginInterfaces(void)
{
	return interfaces;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK * link)
{
	pluginLink = link;
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
