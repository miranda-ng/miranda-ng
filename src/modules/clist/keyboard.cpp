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
#include "clc.h"
#include <m_hotkeys.h>

static INT_PTR hkHideShow(WPARAM, LPARAM)
{
	cli.pfnShowHide(0,0);
	return 0;
}
/*
INT_PTR hkSearch(WPARAM wParam,LPARAM lParam)
{
	DBVARIANT dbv={0};
	if(!DBGetContactSettingString(NULL,"CList","SearchUrl",&dbv)) {
		CallService(MS_UTILS_OPENURL,DBGetContactSettingByte(NULL,"CList","HKSearchNewWnd",0),(LPARAM)dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	return 0;
}
*/
static INT_PTR hkRead(WPARAM, LPARAM)
{
	if(cli.pfnEventsProcessTrayDoubleClick(0)==0) return TRUE;
	SetForegroundWindow(cli.hwndContactList);
	SetFocus(cli.hwndContactList);
	return 0;
}

static INT_PTR hkOpts(WPARAM, LPARAM)
{
	CallService("Options/OptionsCommand",0, 0);
	return 0;
}
/*
static INT_PTR hkCloseMiranda(WPARAM wParam,LPARAM lParam)
{
	CallService("CloseAction", 0, 0);
	return 0;
}

INT_PTR hkRestoreStatus(WPARAM wParam,LPARAM lParam)
{
	int nStatus = DBGetContactSettingWord(NULL, "CList", "Status", ID_STATUS_OFFLINE);
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
	HOTKEYDESC shk = {0};

	CreateServiceFunction("CLIST/HK/SHOWHIDE",hkHideShow);
	CreateServiceFunction("CLIST/HK/Opts",hkOpts);
	CreateServiceFunction("CLIST/HK/Read",hkRead);
//	CreateServiceFunction("CLIST/HK/CloseMiranda",hkCloseMiranda);
//	CreateServiceFunction("CLIST/HK/RestoreStatus",hkRestoreStatus);
//	CreateServiceFunction("CLIST/HK/AllOffline",hkAllOffline);

	shk.cbSize=sizeof(shk);
	shk.pszDescription="Show Hide Contact List";
	shk.pszName="ShowHide";
	shk.pszSection="Main";
	shk.pszService="CLIST/HK/SHOWHIDE";
	shk.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, 'A');
	CallService(MS_HOTKEY_REGISTER,0,(LPARAM)&shk);

	shk.pszDescription="Read Message";
	shk.pszName="ReadMessage";
	shk.pszSection="Main";
	shk.pszService="CLIST/HK/Read";
	shk.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, 'I');
	CallService(MS_HOTKEY_REGISTER,0,(LPARAM)&shk);
/*
	shk.pszDescription="Search in site";
	shk.pszName="SearchInWeb";
	shk.pszSection="Main";
	shk.pszService="CLIST/HK/Search";
	shk.DefHotKey=846;
	CallService(MS_HOTKEY_REGISTER,0,(LPARAM)&shk);
*/
	shk.pszDescription = "Open Options Page";
	shk.pszName = "ShowOptions";
	shk.pszSection = "Main";
	shk.pszService = "CLIST/HK/Opts";
	shk.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, 'O') | HKF_MIRANDA_LOCAL;
	CallService(MS_HOTKEY_REGISTER,0,(LPARAM)&shk);

	shk.pszDescription = "Open Logging Options";
	shk.pszName = "ShowLogOptions";
	shk.pszSection = "Main";
	shk.pszService = "Netlib/Log/Win";
	shk.DefHotKey = 0;
	CallService(MS_HOTKEY_REGISTER,0,(LPARAM)&shk);

	shk.pszDescription="Open Find User Dialog";
	shk.pszName="FindUsers";
	shk.pszSection="Main";
	shk.pszService="FindAdd/FindAddCommand";
	shk.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, 'F') | HKF_MIRANDA_LOCAL;
	CallService(MS_HOTKEY_REGISTER,0,(LPARAM)&shk);

/*
	shk.pszDescription="Close Miranda";
	shk.pszName="CloseMiranda";
	shk.pszSection="Main";
	shk.pszService="CLIST/HK/CloseMiranda";
	shk.DefHotKey=0;
	CallService(MS_HOTKEY_REGISTER,0,(LPARAM)&shk);

	shk.pszDescription="Restore last status";
	shk.pszName="RestoreLastStatus";
	shk.pszSection="Status";
	shk.pszService="CLIST/HK/RestoreStatus";
	shk.DefHotKey=0;
	CallService(MS_HOTKEY_REGISTER,0,(LPARAM)&shk);

	shk.pszDescription="Set All Offline";
	shk.pszName="AllOffline";
	shk.pszSection="Status";
	shk.pszService="CLIST/HK/AllOffline";
	shk.DefHotKey=0;
	CallService(MS_HOTKEY_REGISTER,0,(LPARAM)&shk);
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
