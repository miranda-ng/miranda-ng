/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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

HWND hAboutDlg=NULL;
static HANDLE hBugEvent = NULL;

static INT_PTR AboutCommand(WPARAM wParam,LPARAM)
{
	if (IsWindow(hAboutDlg)) {
		SetForegroundWindow(hAboutDlg);
		SetFocus(hAboutDlg);
		return 0;
	}
	hAboutDlg=CreateDialog(hMirandaInst,MAKEINTRESOURCE(IDD_ABOUT),(HWND)wParam,DlgProcAbout);
	return 0;
}

static INT_PTR IndexCommand(WPARAM, LPARAM)
{
	CallService(MS_UTILS_OPENURL,1,(LPARAM)Translate("http://wiki.miranda-im.org/"));
	return 0;
}

static INT_PTR WebsiteCommand(WPARAM, LPARAM)
{
	CallService(MS_UTILS_OPENURL,1,(LPARAM)"http://www.miranda-im.org");
	return 0;
}

static int BugCommandEvent(WPARAM wParam, LPARAM lParam) {
    char *szUrl = (char*)lParam;

    if (szUrl) {
        CallService(MS_UTILS_OPENURL,1,(LPARAM)szUrl);
    }
    return 0;
}

static INT_PTR BugCommand(WPARAM, LPARAM)
{
    NotifyEventHooks(hBugEvent, 0, (LPARAM)"http://code.google.com/p/miranda/issues/list");
	return 0;
}


int ShutdownHelpModule(WPARAM, LPARAM)
{
	if (IsWindow(hAboutDlg)) DestroyWindow(hAboutDlg);
	hAboutDlg=NULL;
	return 0;
}

int LoadHelpModule(void)
{
	HookEvent(ME_SYSTEM_PRESHUTDOWN,ShutdownHelpModule);

	CreateServiceFunction("Help/AboutCommand",AboutCommand);
	CreateServiceFunction("Help/IndexCommand",IndexCommand);
	CreateServiceFunction("Help/WebsiteCommand",WebsiteCommand);
	CreateServiceFunction("Help/BugCommand",BugCommand);

	hBugEvent = CreateHookableEvent(ME_HELP_BUGREPORT);
	SetHookDefaultForHookableEvent(hBugEvent, BugCommandEvent);

	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof(mi);
	mi.flags = CMIF_ICONFROMICOLIB;
	mi.icolibItem = GetSkinIconHandle(SKINICON_OTHER_MIRANDA);
	mi.pszPopupName = LPGEN("&Help");
	mi.popupPosition = 2000090000;
	mi.position = 2000090000;
	mi.pszName = LPGEN("&About...");
	mi.pszService = "Help/AboutCommand";
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

	mi.icolibItem = GetSkinIconHandle(SKINICON_OTHER_HELP);
	mi.position = -500050000;
	mi.pszName = LPGEN("&Support");
	mi.pszService = "Help/IndexCommand";
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

	mi.icolibItem = GetSkinIconHandle(SKINICON_OTHER_MIRANDAWEB);
	mi.position = 2000050000;
	mi.pszName = LPGEN("&Miranda IM Homepage");
	mi.pszService = "Help/WebsiteCommand";
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);

	mi.icolibItem = GetSkinIconHandle(SKINICON_EVENT_URL);
	mi.position = 2000040000;
	mi.pszName = LPGEN("&Report Bug");
	mi.pszService = "Help/BugCommand";
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&mi);
	return 0;
}
