/*
Miranda IM History Sweeper Light plugin
Copyright (C) 2002-2003  Sergey V. Gershovich
Copyright (C) 2006-2009  Boris Krasnovskiy
Copyright (C) 2010, 2011 tico-tico

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

#include "historysweeperlight.h"

HINSTANCE hInst;

int hLangpack;

LIST<void> g_hWindows(5);

static PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {1D9BF74A-44A8-4B3F-A6E5-73069D3A8979}
	{ 0x1d9bf74a, 0x44a8, 0x4b3f, { 0xa6, 0xe5, 0x73, 0x6, 0x9d, 0x3a, 0x89, 0x79 } }
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

int OnIconPressed(WPARAM hContact, LPARAM lParam)
{
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;

	if (!(sicd->flags & MBCF_RIGHTBUTTON) && !mir_strcmp(sicd->szModule, ModuleName) && db_get_b(NULL, ModuleName, "ChangeInMW", 0)) {
		int nh = sicd->dwId;

		StatusIconData sid = { sizeof(sid) };
		sid.szModule = ModuleName;
		sid.dwId = nh;
		sid.flags = MBF_HIDDEN;
		Srmm_ModifyIcon(hContact, &sid);

		nh = (nh + 1) % 4;
		db_set_b(hContact, ModuleName, "SweepHistory", (BYTE)nh);

		sid.dwId = nh;
		sid.flags = 0;
		Srmm_ModifyIcon(hContact, &sid);
	}
	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	int sweep = db_get_b(NULL, ModuleName, "SweepHistory", 0);

	StatusIconData sid = { sizeof(sid) };
	sid.szModule = ModuleName;

	sid.hIcon = LoadIconEx("actG");
	if (sweep == 0)
		sid.szTooltip = LPGEN("Keep all events");
	else if (sweep == 1)
		sid.szTooltip = time_stamp_strings[db_get_b(NULL, ModuleName, "StartupShutdownOlder", 0)];
	else if (sweep == 2)
		sid.szTooltip = keep_strings[db_get_b(NULL, ModuleName, "StartupShutdownKeep", 0)];
	else if (sweep == 3)
		sid.szTooltip = LPGEN("Delete all events");

	sid.flags = MBF_HIDDEN;
	Srmm_AddIcon(&sid);

	sid.dwId = 1;
	sid.hIcon = LoadIconEx("act1");
	sid.szTooltip = time_stamp_strings[db_get_b(NULL, ModuleName, "StartupShutdownOlder", 0)];
	sid.flags = MBF_HIDDEN;
	Srmm_AddIcon(&sid);

	sid.dwId = 2;
	sid.hIcon = LoadIconEx("act2");
	sid.szTooltip = keep_strings[db_get_b(NULL, ModuleName, "StartupShutdownKeep", 0)];
	sid.flags = MBF_HIDDEN;
	Srmm_AddIcon(&sid);

	sid.dwId = 3;
	sid.hIcon = LoadIconEx("actDel");
	sid.szTooltip = LPGEN("Delete all events");
	sid.flags = MBF_HIDDEN;
	Srmm_AddIcon(&sid);

	HookEvent(ME_MSG_WINDOWEVENT, OnWindowEvent);
	HookEvent(ME_MSG_ICONPRESSED, OnIconPressed);
	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfoEx);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, HSOptInitialise);

	InitIcons();
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	ShutdownAction();
	return 0;
}
