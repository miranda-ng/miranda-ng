/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-05 Miranda ICQ/IM project,
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

#include "stdafx.h"

CMPlugin g_plugin;
CLIST_INTERFACE coreCli;

HIMAGELIST himlCListClc = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////
// external functions

void InitClc();
void InitCustomMenus(void);

int ClcOptInit(WPARAM wParam, LPARAM lParam);
int CluiOptInit(WPARAM wParam, LPARAM lParam);
int CListOptInit(WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////
// returns the plugin information

PLUGININFOEX pluginInfoEx = {
	sizeof(pluginInfoEx),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {53E095A3-2695-490A-9DAD-D20479093831}
	{0x53e095a3, 0x2695, 0x490a, {0x9d, 0xad, 0xd2, 0x4, 0x79, 0x9, 0x38, 0x31}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("CList", pluginInfoEx)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns plugin's interfaces information

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_CLIST, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////
// called when number of accounts has been changed

static int OnAccountsChanged(WPARAM, LPARAM)
{
	himlCListClc = Clist_GetImageList();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// called when all modules got loaded

static int OnModulesLoaded(WPARAM, LPARAM)
{
	himlCListClc = Clist_GetImageList();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// options iniatialization

static int OnOptsInit(WPARAM wParam, LPARAM lParam)
{
	ClcOptInit(wParam, lParam);
	CluiOptInit(wParam, lParam);
	CListOptInit(wParam, lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// menu status services

static INT_PTR GetStatusMode(WPARAM, LPARAM)
{
	return g_clistApi.currentDesiredStatusMode;
}

/////////////////////////////////////////////////////////////////////////////////////////
// main clist initialization routine

int CMPlugin::Load()
{
	g_bSortByStatus = g_plugin.getByte("SortByStatus", SETTING_SORTBYSTATUS_DEFAULT);
	g_bSortByProto = g_plugin.getByte("SortByProto", SETTING_SORTBYPROTO_DEFAULT);

	InitClc();
	CreateServiceFunction(MS_CLIST_GETSTATUSMODE, GetStatusMode);

	HookEvent(ME_OPT_INITIALISE, OnOptsInit);
	HookEvent(ME_PROTO_ACCLISTCHANGED, OnAccountsChanged);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	InitCustomMenus();
	return 0;
}
