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
	FoldersGetCustomPathT(hPluginUpdaterFolder, g_tszRoot, MAX_PATH, _T(""));
	size_t len = _tcslen(g_tszRoot);
	if (g_tszRoot[len-1] == '\\' || g_tszRoot[len-1] == '/')
		g_tszRoot[len-1] = 0;
	return 0;
}

void EmptyFolder()
{
	SHFILEOPSTRUCT file_op = {
		NULL,
		FO_DELETE,
		g_tszRoot,
		_T(""),
		FOF_NOERRORUI | FOF_SILENT | FOF_NOCONFIRMATION,
		false,
		0,
		_T("") };
	SHFileOperation(&file_op);
}

int ModulesLoaded(WPARAM, LPARAM)
{
	if (hPluginUpdaterFolder = FoldersRegisterCustomPathT(MODULEA, LPGEN("Plugin Updater"), MIRANDA_PATHT _T("\\")DEFAULT_UPDATES_FOLDER)) {
		HookEvent(ME_FOLDERS_PATH_CHANGED, OnFoldersChanged);
		OnFoldersChanged(0, 0);
	}
	else lstrcpyn(g_tszRoot, VARST( _T("%miranda_path%\\" DEFAULT_UPDATES_FOLDER)), _countof(g_tszRoot));

#if MIRANDA_VER >= 0x0A00
	if (ServiceExists(MS_ASSOCMGR_ADDNEWURLTYPE))
		AssocMgr_AddNewUrlTypeT("mirpu:", TranslateT("Plugin updater URI scheme"), hInst, IDI_PLGLIST, MODNAME "/ParseUri", 0);
#endif

	int iRestartCount = db_get_b(NULL, MODNAME, DB_SETTING_RESTART_COUNT, 2);
	if (iRestartCount > 0)
		db_set_b(NULL, MODNAME, DB_SETTING_RESTART_COUNT, iRestartCount - 1);
	else
		EmptyFolder(); // silently

	CheckUpdateOnStartup();

	CreateTimer();

	return 0;
}

int OnPreShutdown(WPARAM, LPARAM)
{
	UninitCheck();

#if MIRANDA_VER >= 0x0A00
	UninitListNew();
#endif
	return 0;
}

void InitEvents()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
}