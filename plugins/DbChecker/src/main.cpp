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

int hLangpack = 0;
bool bServiceMode, bLaunchMiranda, bShortMode, bAutoExit;

DbToolOptions opts = { 0 };

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(pluginInfoEx),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE | STATIC_PLUGIN,
	// {A0138FC6-4C52-4501-AF93-7D3E20BCAE5B}
	{ 0xa0138fc6, 0x4c52, 0x4501, { 0xaf, 0x93, 0x7d, 0x3e, 0x20, 0xbc, 0xae, 0x5b } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(nullptr, pluginInfoEx)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// we implement service mode interface

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SERVICEMODE, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(DATABASELINK *) Profile_GetLink();

static HANDLE hService; // do not remove it!

static INT_PTR ServiceMode(WPARAM, LPARAM)
{
	bLaunchMiranda = bShortMode = bAutoExit = false;
	bServiceMode = true;
	
	opts.db = db_get_current();
	wcsncpy_s(opts.filename, VARSW(L"%miranda_userdata%\\%miranda_profilename%.dat"), _TRUNCATE);

	opts.dbChecker = opts.db->GetChecker();
	if (opts.dbChecker == nullptr)
		return SERVICE_FAILED;

	DialogBox(g_plugin.getInst(), MAKEINTRESOURCE(IDD_WIZARD), nullptr, ProgressDlgProc);
	return (bLaunchMiranda) ? SERVICE_CONTINUE : SERVICE_FAILED;
}

static INT_PTR CheckProfile(WPARAM wParam, LPARAM lParam)
{
	bShortMode = true;
	bLaunchMiranda = lParam != 0;
	bAutoExit = lParam == 2;
	bServiceMode = false;
	wcsncpy(opts.filename, (wchar_t*)wParam, _countof(opts.filename));
	return DialogBox(g_plugin.getInst(), MAKEINTRESOURCE(IDD_WIZARD), nullptr, WizardDlgProc);
}

int CMPlugin::Load(void)
{
	CreateServiceFunction(MS_DB_CHECKPROFILE, CheckProfile);
	hService = CreateServiceFunction(MS_SERVICEMODE_LAUNCH, ServiceMode);
	return 0;
}

int CMPlugin::Unload(void)
{
	DestroyServiceFunction(hService);
	return 0;
}
