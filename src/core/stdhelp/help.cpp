/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#include "commonheaders.h"

INT_PTR CALLBACK DlgProcAbout(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hAboutDlg = NULL;

static INT_PTR AboutCommand(WPARAM wParam, LPARAM)
{
	if (IsWindow(hAboutDlg)) {
		SetForegroundWindow(hAboutDlg);
		SetFocus(hAboutDlg);
		return 0;
	}
	hAboutDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_ABOUT), (HWND)wParam, DlgProcAbout);
	return 0;
}

static INT_PTR IndexCommand(WPARAM, LPARAM)
{
	CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW | OUF_TCHAR, (LPARAM)_T("http://wiki.miranda-ng.org"));
	return 0;
}

static INT_PTR WebsiteCommand(WPARAM, LPARAM)
{
	CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW | OUF_TCHAR, (LPARAM)_T("http://miranda-ng.org"));
	return 0;
}

static INT_PTR BugCommand(WPARAM, LPARAM)
{
	CallService(MS_UTILS_OPENURL, OUF_NEWWINDOW | OUF_TCHAR, (LPARAM)_T("http://trac.miranda-ng.org/newticket"));
	return 0;
}

int ShutdownHelpModule(WPARAM, LPARAM)
{
	if (IsWindow(hAboutDlg)) DestroyWindow(hAboutDlg);
	hAboutDlg = NULL;
	return 0;
}

int LoadHelpModule(void)
{
	HookEvent(ME_SYSTEM_PRESHUTDOWN, ShutdownHelpModule);

	CreateServiceFunction("Help/AboutCommand", AboutCommand);
	CreateServiceFunction("Help/IndexCommand", IndexCommand);
	CreateServiceFunction("Help/WebsiteCommand", WebsiteCommand);
	CreateServiceFunction("Help/BugCommand", BugCommand);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.icolibItem = GetSkinIconHandle(SKINICON_OTHER_MIRANDA);
	mi.pszPopupName = LPGEN("&Help");
	mi.popupPosition = 2000090000;
	mi.position = 2000090000;
	mi.pszName = LPGEN("&About...");
	mi.pszService = "Help/AboutCommand";
	Menu_AddMainMenuItem(&mi);

	mi.icolibItem = GetSkinIconHandle(SKINICON_OTHER_HELP);
	mi.position = -500050000;
	mi.pszName = LPGEN("&Support");
	mi.pszService = "Help/IndexCommand";
	Menu_AddMainMenuItem(&mi);

	mi.icolibItem = GetSkinIconHandle(SKINICON_OTHER_MIRANDAWEB);
	mi.position = 2000050000;
	mi.pszName = LPGEN("&Miranda NG homepage");
	mi.pszService = "Help/WebsiteCommand";
	Menu_AddMainMenuItem(&mi);

	mi.icolibItem = GetSkinIconHandle(SKINICON_EVENT_URL);
	mi.position = 2000040000;
	mi.pszName = LPGEN("&Report bug");
	mi.pszService = "Help/BugCommand";
	Menu_AddMainMenuItem(&mi);
	return 0;
}
