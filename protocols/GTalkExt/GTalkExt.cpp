//***************************************************************************************
//
//   Google Extension plugin for the Miranda IM's Jabber protocol
//   Copyright (c) 2011 bems@jabber.org, George Hazan (ghazan@jabber.ru)
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//***************************************************************************************
//   GTalkExt.cpp : Defines the exported functions for the DLL application.
//***************************************************************************************

#include "stdafx.h"
#include "options.h"
#include "handlers.h"
#include "tipper_items.h"
#include "avatar.h"
#include "menu.h"

PLUGINLINK *pluginLink;

#define MIID_PLUGINIFACE {0x08B86253, 0xEC6E, 0x4d09, { 0xB7, 0xA9, 0x64, 0xAC, 0xDF, 0x06, 0x27, 0xB8 }}

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	PLUGIN_DESCRIPTION,
	PLUGIN_VERSION_DWORD,
	"Currently only mail notifications implemented",
	"bems",
	"bems@vingrad.ru",
	COPYRIGHT_STRING,
	"http://miranda-planet.com/forum/index.php?autocom=downloads&showfile=1400",
	UNICODE_AWARE,
	0,		//doesn't replace anything built-in
	MIID_PLUGINIFACE //{08B86253-EC6E-4d09-B7A9-64ACDF0627B8}
};

extern DWORD g_mirandaVersion;

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	g_mirandaVersion = mirandaVersion;
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_PLUGINIFACE, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

HANDLE hModulesLoaded = 0;
HANDLE hAccListChanged = 0;

extern "C" int __declspec(dllexport) Unload(void)
{
	UnhookOptionsInitialization();
	InitMenus(FALSE);
	InitAvaUnit(FALSE);
	if (hAccListChanged) UnhookEvent(hAccListChanged);
	if (hModulesLoaded) UnhookEvent(hModulesLoaded);
	return 0;
}

HICON g_hPopupIcon = 0;
extern HINSTANCE hInst;

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	g_hPopupIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_POPUP));

	pluginLink = link;
	if (
		!mir_getXI(&xi) ||
		!(hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded)) ||
		!(hAccListChanged = HookEvent(ME_PROTO_ACCLISTCHANGED, AccListChanged)) ||
		!InitAvaUnit(TRUE) ||
		!InitMenus(TRUE)
		)
		{Unload(); return 1;};

	AddTipperItem();

	return 0;
}