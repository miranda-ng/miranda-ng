/* 
Copyright (C) 2010 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "stdafx.h"

HANDLE hPluginUpdaterFolder;

int OnFoldersChanged(WPARAM, LPARAM)
{
	FoldersGetCustomPathW(hPluginUpdaterFolder, g_wszRoot, MAX_PATH, L"");
	size_t len = wcslen(g_wszRoot);
	if (g_wszRoot[len-1] == '\\' || g_wszRoot[len-1] == '/')
		g_wszRoot[len-1] = 0;
	return 0;
}

int ModulesLoaded(WPARAM, LPARAM)
{
	if (hPluginUpdaterFolder = FoldersRegisterCustomPathW(MODULEA, LPGEN("Plugin Updater"), MIRANDA_PATHW L"\\" DEFAULT_UPDATES_FOLDER)) {
		HookEvent(ME_FOLDERS_PATH_CHANGED, OnFoldersChanged);
		OnFoldersChanged(0, 0);
	}
	else lstrcpyn(g_wszRoot, VARSW(L"%miranda_path%\\" DEFAULT_UPDATES_FOLDER), _countof(g_wszRoot));

	if (ServiceExists(MS_ASSOCMGR_ADDNEWURLTYPE))
		AssocMgr_AddNewUrlTypeW("mirpu:", TranslateT("Plugin updater URI scheme"), g_plugin.getInst(), IDI_PLGLIST, MODULENAME "/ParseUri", 0);

	int iCompatLevel = db_get_b(0, "Compatibility", MODULENAME);
	if (iCompatLevel == 0) {
		db_set_b(0, "Compatibility", MODULENAME, 1);
		DeleteDirectoryTreeW(CMStringW(g_wszRoot) + L"\\Backups");
	}

	CreateTimer();
	return 0;
}

int OnPreShutdown(WPARAM, LPARAM)
{
	UninitCheck();
	UninitListNew();
	return 0;
}

void InitEvents()
{
	Miranda_WaitOnHandle(CheckUpdateOnStartup);

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
}
