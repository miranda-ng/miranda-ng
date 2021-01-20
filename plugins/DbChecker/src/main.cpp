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
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// we implement service mode interface

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SERVICEMODE, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static HANDLE hService; // do not remove it!

static INT_PTR ServiceMode(WPARAM, LPARAM)
{
	auto *opts = new DbToolOptions();
	opts->db = db_get_current();
	opts->dbChecker = opts->db->GetChecker();
	if (opts->dbChecker == nullptr) {
		delete opts;
		return SERVICE_FAILED;
	}
	wcsncpy_s(opts->filename, VARSW(L"%miranda_userdata%\\%miranda_profilename%.dat"), _TRUNCATE);

	CWizardDlg(opts).DoModal();
	return SERVICE_FAILED;
}

static INT_PTR CheckProfile(WPARAM wParam, LPARAM lParam)
{
	auto *opts = new DbToolOptions();
	opts->bAutoExit = lParam == 2;

	if (wParam) {
		wcsncpy(opts->filename, (wchar_t *)wParam, _countof(opts->filename));
		opts->bOwnsDb = true;
	}
	else {
		opts->db = db_get_current();
		opts->dbChecker = opts->db->GetChecker();
		if (opts->dbChecker == nullptr) {
			delete opts;
			return 2;
		}

		wcsncpy_s(opts->filename, VARSW(L"%miranda_userdata%\\%miranda_profilename%.dat"), _TRUNCATE);
	}

	return CWizardDlg(opts).DoModal();
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	CMenuItem mi(&g_plugin);
	mi.root = g_plugin.addRootMenu(MO_MAIN, LPGENW("Database"), 500000000);

	// Main menu items
	SET_UID(mi, 0x371f9e52, 0x39f5, 0x4fee, 0x84, 0xbb, 0xfe, 0x9c, 0xf, 0xd6, 0x85, 0x79); // {371F9E52-39F5-4FEE-84BB-FE9C0FD68579}
	mi.name.a = LPGEN("Check database");
	mi.pszService = MS_DB_CHECKPROFILE;
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_DBTOOL);
	mi.position = 500000003;
	Menu_AddMainMenuItem(&mi);
	return 0;
}

static IconItem iconList[] = {
	{ LPGEN("Check database"), "DbChecker", IDI_DBTOOL },
};

int CMPlugin::Load(void)
{
	g_plugin.registerIcon("DbChecker", iconList);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	CreateServiceFunction(MS_DB_CHECKPROFILE, CheckProfile);
	hService = CreateServiceFunction(MS_SERVICEMODE_LAUNCH, ServiceMode);
	return 0;
}

int CMPlugin::Unload(void)
{
	DestroyServiceFunction(hService);
	return 0;
}
