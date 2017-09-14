/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

HINSTANCE g_hInst = 0;
CLIST_INTERFACE* pcli = NULL, coreCli;
HIMAGELIST himlCListClc = NULL;
int hLangpack;

/////////////////////////////////////////////////////////////////////////////////////////
// external functions

void RegisterCListFonts( void );
void InitCustomMenus( void );
void PaintClc(HWND hwnd, struct ClcData *dat, HDC hdc, RECT * rcPaint);

int ClcOptInit(WPARAM wParam, LPARAM lParam);
int CluiOptInit(WPARAM wParam, LPARAM lParam);
int CListOptInit(WPARAM wParam, LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////////////////
// dll stub

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD, LPVOID)
{
	g_hInst = hInstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// returns the plugin information

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	MIRANDA_VERSION_DWORD,
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	//{240A91DC-9464-457a-9787-FF1EA88E77E2}
	{ 0x240a91dc, 0x9464, 0x457a, { 0x97, 0x87, 0xff, 0x1e, 0xa8, 0x8e, 0x77, 0xe2 } }
};

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
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
	RegisterCListFonts();
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
	return pcli->currentDesiredStatusMode;
}

/////////////////////////////////////////////////////////////////////////////////////////
// main clist initialization routine

extern "C" __declspec(dllexport) int CListInitialise()
{
	mir_getLP(&pluginInfo);
	pcli = Clist_GetInterface();

	g_bSortByStatus = db_get_b(NULL, "CList", "SortByStatus", SETTING_SORTBYSTATUS_DEFAULT);
	g_bSortByProto = db_get_b(NULL, "CList", "SortByProto", SETTING_SORTBYPROTO_DEFAULT);

	coreCli = *pcli;
	pcli->hInst = g_hInst;
	pcli->pfnPaintClc = PaintClc;
	pcli->pfnLoadClcOptions = LoadClcOptions;
	pcli->pfnCompareContacts = CompareContacts;

	CreateServiceFunction(MS_CLIST_GETSTATUSMODE, GetStatusMode);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, OnAccountsChanged);
	HookEvent(ME_OPT_INITIALISE, OnOptsInit);

	InitCustomMenus();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// a plugin loader aware of CList exports will never call this.

extern "C" __declspec(dllexport) int Load(void)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// a plugin unloader

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}
