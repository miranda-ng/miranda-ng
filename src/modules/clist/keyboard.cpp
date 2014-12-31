/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"
#include "clc.h"
#include <m_hotkeys.h>

static INT_PTR hkHideShow(WPARAM, LPARAM)
{
	cli.pfnShowHide(0, 0);
	return 0;
}
/*
INT_PTR hkSearch(WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv = {0};
	if (!db_get_s(NULL, "CList", "SearchUrl", &dbv)) {
		CallService(MS_UTILS_OPENURL, db_get_b(NULL, "CList", "HKSearchNewWnd", 0), (LPARAM)dbv.pszVal);
		db_free(&dbv);
	}
	return 0;
}
*/
static INT_PTR hkRead(WPARAM, LPARAM)
{
	if (cli.pfnEventsProcessTrayDoubleClick(0) == 0) return TRUE;
	SetForegroundWindow(cli.hwndContactList);
	SetFocus(cli.hwndContactList);
	return 0;
}

static INT_PTR hkOpts(WPARAM, LPARAM)
{
	CallService("Options/OptionsCommand", 0, 0);
	return 0;
}
/*
static INT_PTR hkCloseMiranda(WPARAM wParam, LPARAM lParam)
{
	CallService("CloseAction", 0, 0);
	return 0;
}

INT_PTR hkRestoreStatus(WPARAM wParam, LPARAM lParam)
{
	int nStatus = db_get_w(NULL, "CList", "Status", ID_STATUS_OFFLINE);
	CallService(MS_CLIST_SETSTATUSMODE, nStatus, 0);
	return 0;
}

static INT_PTR hkAllOffline(WPARAM, LPARAM)
{
	CallService(MS_CLIST_SETSTATUSMODE, ID_STATUS_OFFLINE, 0);
	return 0;
}
*/
int InitClistHotKeys(void)
{
	CreateServiceFunction("CLIST/HK/SHOWHIDE", hkHideShow);
	CreateServiceFunction("CLIST/HK/Opts", hkOpts);
	CreateServiceFunction("CLIST/HK/Read", hkRead);

	HOTKEYDESC shk = { sizeof(shk) };
	shk.dwFlags = HKD_TCHAR;
	shk.ptszDescription = LPGENT("Show/Hide contact list");
	shk.pszName = "ShowHide";
	shk.ptszSection = _T("Main");
	shk.pszService = "CLIST/HK/SHOWHIDE";
	shk.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, 'A');
	Hotkey_Register(&shk);

	shk.ptszDescription = LPGENT("Read message");
	shk.pszName = "ReadMessage";
	shk.ptszSection = _T("Main");
	shk.pszService = "CLIST/HK/Read";
	shk.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, 'I');
	Hotkey_Register(&shk);
/*
	shk.pszDescription = "Search in site";
	shk.pszName = "SearchInWeb";
	shk.pszSection = "Main";
	shk.pszService = "CLIST/HK/Search";
	shk.DefHotKey = 846;
	Hotkey_Register(&shk);
*/
	shk.ptszDescription = LPGENT("Open Options page");
	shk.pszName = "ShowOptions";
	shk.ptszSection = _T("Main");
	shk.pszService = "CLIST/HK/Opts";
	shk.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, 'O') | HKF_MIRANDA_LOCAL;
	Hotkey_Register(&shk);

	shk.ptszDescription = LPGENT("Open logging options");
	shk.pszName = "ShowLogOptions";
	shk.ptszSection = _T("Main");
	shk.pszService = "Netlib/Log/Win";
	shk.DefHotKey = 0;
	Hotkey_Register(&shk);

	shk.ptszDescription = LPGENT("Open 'Find user' dialog");
	shk.pszName = "FindUsers";
	shk.ptszSection = _T("Main");
	shk.pszService = "FindAdd/FindAddCommand";
	shk.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, 'F') | HKF_MIRANDA_LOCAL;
	Hotkey_Register(&shk);

/*
	shk.pszDescription = "Close Miranda";
	shk.pszName = "CloseMiranda";
	shk.pszSection = "Main";
	shk.pszService = "CLIST/HK/CloseMiranda";
	shk.DefHotKey = 0;
	Hotkey_Register(&shk);

	shk.pszDescription = "Restore last status";
	shk.pszName = "RestoreLastStatus";
	shk.pszSection = "Status";
	shk.pszService = "CLIST/HK/RestoreStatus";
	shk.DefHotKey = 0;
	Hotkey_Register(&shk);

	shk.pszDescription = "Set All Offline";
	shk.pszName = "AllOffline";
	shk.pszSection = "Status";
	shk.pszService = "CLIST/HK/AllOffline";
	shk.DefHotKey = 0;
	Hotkey_Register(&shk);
*/
	return 0;
}


int fnHotKeysRegister(HWND)
{
	return 0;
}

void fnHotKeysUnregister(HWND)
{
}

int fnHotKeysProcess(HWND, WPARAM, LPARAM)
{
	return TRUE;
}

int fnHotkeysProcessMessage(WPARAM, LPARAM)
{
	return FALSE;
}
