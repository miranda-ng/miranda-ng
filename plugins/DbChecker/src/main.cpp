/*
Miranda Database Tool
Copyright (C) 2001-2005  Richard Hughes

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

HINSTANCE hInst;
int hLangpack = 0;
bool bServiceMode, bLaunchMiranda, bShortMode, bAutoExit;
HANDLE hService;

DbToolOptions opts = { 0 };

PLUGININFOEX pluginInfoEx =
{
	sizeof(pluginInfoEx),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE | STATIC_PLUGIN,
	// {A0138FC6-4C52-4501-AF93-7D3E20BCAE5B}
	{ 0xa0138fc6, 0x4c52, 0x4501, { 0xaf, 0x93, 0x7d, 0x3e, 0x20, 0xbc, 0xae, 0x5b } }
};

/////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

// we implement service mode interface
extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SERVICEMODE, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR ServiceMode(WPARAM, LPARAM)
{
	bLaunchMiranda = bShortMode = bAutoExit = false;
	bServiceMode = true;
	DialogBox(hInst, MAKEINTRESOURCE(IDD_WIZARD), NULL, WizardDlgProc);
	return (bLaunchMiranda) ? SERVICE_CONTINUE : SERVICE_FAILED;
}

static INT_PTR CheckProfile(WPARAM wParam, LPARAM lParam)
{
	bShortMode = true;
	bLaunchMiranda = lParam != 0;
	bAutoExit = lParam == 2;
	bServiceMode = false;
	_tcsncpy(opts.filename, (TCHAR*)wParam, SIZEOF(opts.filename));
	return DialogBox(hInst, MAKEINTRESOURCE(IDD_WIZARD), NULL, WizardDlgProc);
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfoEx);

	CreateServiceFunction(MS_DB_CHECKPROFILE, CheckProfile);
	hService = CreateServiceFunction(MS_SERVICEMODE_LAUNCH, ServiceMode);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	DestroyServiceFunction(hService);
	return 0;
}
