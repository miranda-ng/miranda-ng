 /*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "variables.h"
#include "buildnumber.h"

HINSTANCE hInst;
struct MM_INTERFACE mmi;
struct LIST_INTERFACE li;
PLUGINLINK *pluginLink;
DWORD g_mirandaVersion;
int hLangpack = 0;

#ifdef UNICODE
	// {59B0036E-5403-422e-883B-C9AAF425682B}
	#define MIID_VARIABLES { 0x59b0036e, 0x5403, 0x422e, { 0x88, 0x3b, 0xc9, 0xaa, 0xf4, 0x25, 0x68, 0x2b } }
#else
	// {352A6BA9-5636-4db0-B92F-7A3289F57902}
	#define MIID_VARIABLES { 0x352a6ba9, 0x5636, 0x4db0, { 0xb9, 0x2f, 0x7a, 0x32, 0x89, 0xf5, 0x79, 0x2 } }
#endif

static HANDLE hExitHook, hModulesLoadedHook;

static int Exit(WPARAM wParam, LPARAM lParam)
{
	UnhookEvent(hExitHook);
	return 0;
}

static int ModulesLoaded(WPARAM wParam, LPARAM lParam) {

	UnhookEvent(hModulesLoadedHook);
	// trigger plugin
#if !defined(WINE)
	initTriggerModule();
#endif

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MirandaPluginInfo - returns an information about a plugin

PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
#ifdef _WIN64
	"Variables (x64, Unicode)",
#else
#ifdef UNICODE
	"Variables (Unicode)",
#else
	"Variables",
#endif
#endif
	__VERSION_DWORD,
	"Adds support for dynamic variables in strings for plugins",
	"P Boon",
	"unregistered@users.sourceforge.net",
	"© 2003-2008 P. Boon, Ricardo Pescuma, George Hazan",
	"http://www.miranda-im.org/",
	UNICODE_AWARE,
	0
};

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	g_mirandaVersion = mirandaVersion;
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MirandaPluginInfoEx - returns the extended information about a plugin

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
#ifdef _WIN64
	"Variables (x64, Unicode)",
#else
#ifdef UNICODE
	"Variables (Unicode)",
#else
	"Variables",
#endif
#endif
	__VERSION_DWORD,
	"Adds support for dynamic variables in strings for plugins",
	"P Boon",
	"unregistered@users.sourceforge.net",
	"© 2003-2008 P. Boon, Ricardo Pescuma, George Hazan",
	"http://www.miranda-im.org/",
	UNICODE_AWARE,
	0,
	MIID_VARIABLES
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	g_mirandaVersion = mirandaVersion;
	return &pluginInfoEx;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MirandaPluginInterfaces - returns the protocol interface to the core

static const MUUID interfaces[] = { MIID_VARIABLES, MIID_LAST };
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces( void )
{
	return interfaces;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load - plugin's entry point

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;
	if (UnicodeCheck(pluginInfo.shortName, FALSE))
		return 0;

	mir_getMMI( &mmi );
	mir_getLI( &li );
	mir_getLP( &pluginInfoEx );

	hExitHook = HookEvent(ME_SYSTEM_OKTOEXIT, Exit);
	hModulesLoadedHook = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	LoadVarModule();
		
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload - destroys plugin from memory

extern "C" int __declspec(dllexport) Unload(void)
{
	UnloadVarModule();
	return 0;
}
