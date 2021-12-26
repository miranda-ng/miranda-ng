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

#include "stdafx.h"

CMPlugin g_plugin;

LIST<void> g_hWindows(5, PtrKeySortT);

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {1D9BF74A-44A8-4B3F-A6E5-73069D3A8979}
	{ 0x1d9bf74a, 0x44a8, 0x4b3f, { 0xa6, 0xe5, 0x73, 0x6, 0x9d, 0x3a, 0x89, 0x79 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

static int OnIconPressed(WPARAM hContact, LPARAM lParam)
{
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;

	if (!(sicd->flags & MBCF_RIGHTBUTTON) && !mir_strcmp(sicd->szModule, MODULENAME) && g_plugin.getByte("ChangeInMW", 0)) {
		int nh = sicd->dwId;
		Srmm_SetIconFlags(hContact, MODULENAME, nh, MBF_HIDDEN);

		nh = (nh + 1) % 4;

		g_plugin.setByte(hContact, "SweepHistory", (uint8_t)nh);
		Srmm_SetIconFlags(hContact, MODULENAME, nh, 0);
	}
	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	int sweep = g_plugin.getByte("SweepHistory", 0);

	StatusIconData sid = {};
	sid.szModule = MODULENAME;

	sid.hIcon = g_plugin.getIcon(IDI_ACTG);
	if (sweep == 0)
		sid.szTooltip.w = LPGENW("Keep all events");
	else if (sweep == 1)
		sid.szTooltip.w = time_stamp_strings[g_plugin.getByte("StartupShutdownOlder", 0)];
	else if (sweep == 2)
		sid.szTooltip.w = keep_strings[g_plugin.getByte("StartupShutdownKeep", 0)];
	else if (sweep == 3)
		sid.szTooltip.w = LPGENW("Delete all events");

	sid.flags = MBF_HIDDEN | MBF_UNICODE;
	Srmm_AddIcon(&sid, &g_plugin);

	sid.dwId = 1;
	sid.hIcon = g_plugin.getIcon(IDI_ACT1);
	sid.szTooltip.w = time_stamp_strings[g_plugin.getByte("StartupShutdownOlder", 0)];
	Srmm_AddIcon(&sid, &g_plugin);

	sid.dwId = 2;
	sid.hIcon = g_plugin.getIcon(IDI_ACT2);
	sid.szTooltip.w = keep_strings[g_plugin.getByte("StartupShutdownKeep", 0)];
	Srmm_AddIcon(&sid, &g_plugin);

	sid.dwId = 3;
	sid.hIcon = g_plugin.getIcon(IDI_ACTDEL);
	sid.szTooltip.w = LPGENW("Delete all events");
	Srmm_AddIcon(&sid, &g_plugin);

	HookEvent(ME_MSG_WINDOWEVENT, OnWindowEvent);
	HookEvent(ME_MSG_ICONPRESSED, OnIconPressed);
	return 0;
}

int CMPlugin::Load()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, HSOptInitialise);

	InitIcons();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	ShutdownAction();
	return 0;
}
