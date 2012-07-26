 /*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is mir_free software; you can redistribute it and/or modify
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

DWORD g_mirandaVersion;
int hLangpack = 0;

// {59B0036E-5403-422e-883B-C9AAF425682B}
#define MIID_VARIABLES { 0x59b0036e, 0x5403, 0x422e, { 0x88, 0x3b, 0xc9, 0xaa, 0xf4, 0x25, 0x68, 0x2b } }

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
// MirandaPluginInfoEx - returns the extended information about a plugin

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	"Variables",
	__VERSION_DWORD,
	"Adds support for dynamic variables in strings for plugins.",
	"P Boon",
	"unregistered@users.sourceforge.net",
	"© 2003-2008 P. Boon, Ricardo Pescuma, George Hazan",
	"http://miranda-ng.org/",
	UNICODE_AWARE,
	MIID_VARIABLES
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	g_mirandaVersion = mirandaVersion;
	return &pluginInfoEx;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load - plugin's entry point

extern "C" int __declspec(dllexport) Load(void)
{
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
