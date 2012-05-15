/*
Custom profile folders plugin for Miranda IM

Copyright © 2005 Cristian Libotean

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

#include "hooked_events.h"
#include "events.h"

#define HOST "http://eblis.tla.ro/projects"

#if defined(WIN64) || defined(_WIN64)
#define FOLDERS_VERSION_URL HOST "/miranda/Folders/updater/x64/Folders.html"
#define FOLDERS_UPDATE_URL HOST "/miranda/Folders/updater/x64/Folders.zip"
#else
#define FOLDERS_VERSION_URL HOST "/miranda/Folders/updater/Folders.html"
#define FOLDERS_UPDATE_URL HOST "/miranda/Folders/updater/Folders.zip"
#endif
#define FOLDERS_VERSION_PREFIX "Custom profile folders version "

HANDLE hModulesLoaded;
HANDLE hOptionsInitialize;

int HookEvents()
{
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	hOptionsInitialize = HookEvent(ME_OPT_INITIALISE, OnOptionsInitialize);
	return 0;
}

int UnhookEvents()
{
	UnhookEvent(hModulesLoaded);
	UnhookEvent(hOptionsInitialize);
	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	char buffer[1024];
	Update update = {0};
	update.cbSize = sizeof(Update);
	update.szComponentName = __PLUGIN_DISPLAY_NAME;
	update.pbVersion = (BYTE *) CreateVersionString(VERSION, buffer);
	update.cpbVersion = (int)strlen((char *) update.pbVersion);
	update.szUpdateURL = UPDATER_AUTOREGISTER;
	update.szBetaVersionURL = FOLDERS_VERSION_URL;
	update.szBetaUpdateURL = FOLDERS_UPDATE_URL;
	update.pbBetaVersionPrefix = (BYTE *) FOLDERS_VERSION_PREFIX;
	update.cpbBetaVersionPrefix = (int)strlen(FOLDERS_VERSION_PREFIX);
	CallService(MS_UPDATE_REGISTER, 0, (LPARAM) &update);
	
	return 0;
}

int OnOptionsInitialize(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };

	odp.cbSize = sizeof(odp);
	odp.position = 100000000;
	odp.hInstance = hInstance;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_FOLDERS);
	odp.pszTitle = Translate("Folders");
	odp.pszGroup = Translate("Customize");
	odp.groupPosition = 910000000;
	odp.flags=ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcOpts;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	
	return 0;
}