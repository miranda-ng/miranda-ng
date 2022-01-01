/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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
#include "clc.h"
#include <m_hotkeys.h>

static INT_PTR hkHideShow(WPARAM, LPARAM)
{
	g_clistApi.pfnShowHide();
	return 0;
}

static INT_PTR hkRead(WPARAM, LPARAM)
{
	if (Clist_EventsProcessTrayDoubleClick(0) == 0)
		return true;
	
	SetForegroundWindow(g_clistApi.hwndContactList);
	SetFocus(g_clistApi.hwndContactList);
	return 0;
}

static INT_PTR hkOpts(WPARAM, LPARAM)
{
	CallService("Options/OptionsCommand", 0, 0);
	return 0;
}

int InitClistHotKeys(void)
{
	HOTKEYDESC shk = {};
	shk.szSection.a = "Main";

	shk.szDescription.a = LPGEN("Show/Hide contact list");
	shk.pszName = "ShowHide";
	shk.pszService = "CLIST/HK/SHOWHIDE";
	shk.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, 'A');
	g_plugin.addHotkey(&shk);
	CreateServiceFunction(shk.pszService, hkHideShow);

	shk.szDescription.a = LPGEN("Read message");
	shk.pszName = "ReadMessage";
	shk.pszService = "CLIST/HK/Read";
	shk.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, 'I');
	g_plugin.addHotkey(&shk);
	CreateServiceFunction(shk.pszService, hkRead);

	shk.szDescription.a = LPGEN("Open Options page");
	shk.pszName = "ShowOptions";
	shk.pszService = "CLIST/HK/Opts";
	shk.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, 'O') | HKF_MIRANDA_LOCAL;
	g_plugin.addHotkey(&shk);
	CreateServiceFunction(shk.pszService, hkOpts);

	shk.szDescription.a = LPGEN("Toggle groups");
	shk.pszName = "Togglegroups";
	shk.pszService = MS_CLIST_TOGGLEGROUPS;
	shk.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, 'G');
	g_plugin.addHotkey(&shk);

	shk.szDescription.a = LPGEN("Open logging options");
	shk.pszName = "ShowLogOptions";
	shk.pszService = "Netlib/Log/Win";
	shk.DefHotKey = 0;
	g_plugin.addHotkey(&shk);

	shk.szDescription.a = LPGEN("Open 'Find user' dialog");
	shk.pszName = "FindUsers";
	shk.pszService = "FindAdd/FindAddCommand";
	shk.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, 'F') | HKF_MIRANDA_LOCAL;
	g_plugin.addHotkey(&shk);
	return 0;
}
