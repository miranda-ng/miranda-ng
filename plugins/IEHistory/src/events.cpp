/*
IEView history viewer plugin for Miranda IM

Copyright © 2005-2006 Cristian Libotean

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

#include "events.h"

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
	CLISTMENUITEM menuItem = {0};
	menuItem.cbSize = sizeof(CLISTMENUITEM);
	menuItem.flags = 0;
	menuItem.pszContactOwner = NULL; //all contacts
	menuItem.hIcon = hIcon;

	menuItem.position = 1000090000;
	
	menuItem.pszName = Translate("View &history");
	menuItem.pszService = MS_HISTORY_SHOWCONTACTHISTORY;

	Menu_AddContactMenuItem(&menuItem);
	
/// @todo (White-Tiger#1#08/19/14): fully implement
	menuItem.pszName = Translate("&System History");
	Menu_AddMainMenuItem(&menuItem);
	//PLUGININFO pInfo = pluginInfo;
	//pInfo.shortName = "IEView History Viewer";
	hOpenWindowsList = (HANDLE) CallService(MS_UTILS_ALLOCWINDOWLIST, 0, 0);
	return 0;
}

int OnOptionsInitialize(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 100000000;
//	odp.ptszTitle = _T("IEHistory");
	odp.ptszTitle = LPGENT("History");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_HISTORY);
	odp.hInstance = hInstance;
//	odp.ptszGroup = LPGENT("Message Sessions");
//	odp.groupPosition = 910000000;
	odp.flags=ODPF_BOLDGROUPS|ODPF_TCHAR;
	Options_AddPage(wParam,&odp);
	return 0;
}
