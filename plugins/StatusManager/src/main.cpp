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

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////
// returns plugin's extended information

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESC,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {FADD4A8A-1FD0-4398-83BD-E378B85ED8F1}
	{ 0xfadd4a8a, 0x1fd0, 0x4398, { 0x83, 0xbd, 0xe3, 0x78, 0xb8, 0x5e, 0xd8, 0xf1 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////
// interfaces

static MUUID Interfaces[2] = { MIID_LAST, MIID_LAST };

extern "C" __declspec(dllexport) MUUID* MirandaPluginInterfaces(void)
{
	if (g_AAAEnabled)
		Interfaces[0] = MIID_AUTOAWAY;
	return Interfaces;
}

/////////////////////////////////////////////////////////////////////////////////////////
// plugin's entry point

bool g_bMirandaLoaded = false;

int OnModulesLoaded(WPARAM, LPARAM)
{
	g_bMirandaLoaded = true;

	HookEvent(ME_OPT_INITIALISE, OnCommonOptionsInit);

	for (auto &pa : Accounts())
		if (IsSuitableProto(pa))
			protoList.insert(new SMProto(pa));

	return 0;
}

int OnAccChanged(WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT *pa = (PROTOACCOUNT*)lParam;
	switch (wParam) {
	case PRAC_ADDED:
		protoList.insert(new SMProto(pa));
		break;

	case PRAC_REMOVED:
		for (auto &it : protoList) {
			if (!mir_strcmp(it->m_szName, pa->szModuleName)) {
				protoList.removeItem(&it);
				break;
			}
		}
		break;
	}

	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, OnAccChanged);

	InitCommonStatus();
	InitCommonOptions();

	if (g_AAAEnabled)
		AdvancedAutoAwayLoad();
	if (g_KSEnabled)
		KeepStatusLoad();
	if (g_SSEnabled)
		StartupStatusLoad();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// plugin's exit point

extern "C" int __declspec(dllexport) Unload(void)
{
	if (g_KSEnabled)
		KeepStatusUnload();
	if (g_SSEnabled)
		StartupStatusUnload();
	if (g_AAAEnabled)
		AdvancedAutoAwayUnload();
	return 0;
}
