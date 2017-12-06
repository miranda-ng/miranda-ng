/*
	KeepStatus Plugin for Miranda-IM (www.miranda-im.org)
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

#include "stdafx.h"

HINSTANCE hInst;
int hLangpack = 0;
CLIST_INTERFACE *pcli;

/////////////////////////////////////////////////////////////////////////////////////////
// dll entry point

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		hInst = hinstDLL;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns plugin's extended information

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {FADD4A8A-1FD0-4398-83BD-E378B85ED8F1}
	{ 0xfadd4a8a, 0x1fd0, 0x4398, { 0x83, 0xbd, 0xe3, 0x78, 0xb8, 0x5e, 0xd8, 0xf1 } }
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

/////////////////////////////////////////////////////////////////////////////////////////
// interfaces

MUUID Interfaces[2] = {0};
MUUID miidAutoAway = MIID_AUTOAWAY;


MUUID* GetInterfaces(void)
{
	if (IsSubPluginEnabled(AAAMODULENAME))
		Interfaces[0] = miidAutoAway;
	return Interfaces;
};

extern "C" __declspec(dllexport) MUUID* MirandaInterfaces = GetInterfaces();

/////////////////////////////////////////////////////////////////////////////////////////
// plugin's entry point

int OnModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_OPT_INITIALISE, CSubPluginsOptionsDlg::OnOptionsInit);
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfoEx);
	pcli = Clist_GetInterface();

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	InitCommonStatus();

	if (IsSubPluginEnabled(KSMODULENAME))
		KeepStatusLoad();
	if (IsSubPluginEnabled(SSMODULENAME))
		StartupStatusLoad();
	if (IsSubPluginEnabled(AAAMODULENAME))
		AdvancedAutoAwayLoad();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// plugin's exit point

extern "C" int __declspec(dllexport) Unload(void)
{
	KeepStatusUnload();
	StartupStatusUnload();
	AdvancedAutoAwayUnload();

	return 0;
}
