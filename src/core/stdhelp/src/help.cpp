/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (�) 2012-16 Miranda NG project (http://miranda-ng.org),
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

#include "stdafx.h"

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
	Utils_OpenUrl("http://wiki.miranda-ng.org");
	return 0;
}

static INT_PTR WebsiteCommand(WPARAM, LPARAM)
{
	Utils_OpenUrl("http://miranda-ng.org");
	return 0;
}

static INT_PTR BugCommand(WPARAM, LPARAM)
{
	Utils_OpenUrl("http://trac.miranda-ng.org/newticket");
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

	CMenuItem mi;
	mi.root = Menu_CreateRoot(MO_MAIN, LPGENT("&Help"), 2000090000);
	Menu_ConfigureItem(mi.root, MCI_OPT_UID, "8824ECA5-6942-46D7-9D07-1BA600E0D02E");

	SET_UID(mi, 0xf3ebf1fa, 0x587c, 0x494d, 0xbd, 0x33, 0x7f, 0x88, 0xb3, 0x61, 0x1e, 0xd3);
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MIRANDA);
	mi.position = 2000090000;
	mi.name.a = LPGEN("&About...");
	mi.pszService = "Help/AboutCommand";
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x495df66f, 0x844e, 0x479a, 0xaf, 0x21, 0x3e, 0x42, 0xc5, 0x14, 0x7c, 0x7e);
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_HELP);
	mi.position = -500050000;
	mi.name.a = LPGEN("&Support");
	mi.pszService = "Help/IndexCommand";
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0x15e18b58, 0xec73, 0x45c2, 0xb9, 0xf4, 0x2a, 0xfe, 0xc2, 0xb7, 0xd3, 0x25);
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_MIRANDAWEB);
	mi.position = 2000050000;
	mi.name.a = LPGEN("&Miranda NG homepage");
	mi.pszService = "Help/WebsiteCommand";
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0xe7d0fe8b, 0xfdeb, 0x45b3, 0xba, 0x83, 0x3, 0x1e, 0x15, 0xda, 0x7e, 0x52);
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_EVENT_URL);
	mi.position = 2000040000;
	mi.name.a = LPGEN("&Report bug");
	mi.pszService = "Help/BugCommand";
	Menu_AddMainMenuItem(&mi);
	return 0;
}
