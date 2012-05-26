/*
Version information plugin for Miranda IM

Copyright © 2002-2006 Luca Santarelli, Cristian Libotean

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

#include "common.h"
#include "hooked_events.h"

HANDLE hModulesLoaded;
HANDLE hOptionsInitialize;

const int nExpertOnlyControls = 10;
UINT uiExpertOnlyControls[nExpertOnlyControls] = {0};

#define HOST "http://eblis.tla.ro/projects"

#define VERSIONINFO_VERSION_URL HOST "/miranda/VersionInfo/updater/VersionInfo.html"
#define VERSIONINFO_UPDATE_URL HOST "/miranda/VersionInfo/updater/VersionInfo.zip"
#define VERSIONINFO_VERSION_PREFIX "Version Information version "

int HookEvents()
{
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	hOptionsInitialize = HookEvent(ME_OPT_INITIALISE, OnOptionsInitialise);
	//hPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
	
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
	update.cpbVersion = (int) strlen((char *) update.pbVersion);
	update.szUpdateURL = UPDATER_AUTOREGISTER;
	update.szBetaVersionURL = VERSIONINFO_VERSION_URL;
	update.szBetaUpdateURL = VERSIONINFO_UPDATE_URL;
	update.pbBetaVersionPrefix = (BYTE *) VERSIONINFO_VERSION_PREFIX;
	update.cpbBetaVersionPrefix = (int) strlen(VERSIONINFO_VERSION_PREFIX);
	CallService(MS_UPDATE_REGISTER, 0, (LPARAM) &update);
	
	bFoldersAvailable = ServiceExists(MS_FOLDERS_REGISTER_PATH);
	hOutputLocation = FoldersRegisterCustomPath("VersionInfo", "Output folder", "%miranda_path%");
	
	GetStringFromDatabase("UUIDCharMark", _T(DEF_UUID_CHARMARK), PLUGIN_UUID_MARK, cPLUGIN_UUID_MARK);
	
	return 0;
}

int OnOptionsInitialise(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	
	uiExpertOnlyControls[0] = IDC_SHOWUUIDS;
	uiExpertOnlyControls[1] = IDC_SHOWINSTALLEDLANGUAGES;
	
	odp.cbSize = sizeof(odp);
	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_VERSIONINFO);
	odp.pszTitle = Translate("Version Information");
	odp.pszGroup = Translate("Services");
	odp.groupPosition = 910000000;
	odp.flags=ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcOpts;
	odp.expertOnlyControls = uiExpertOnlyControls;
	odp.nExpertOnlyControls = 2;
	
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
}