/*
    StartupStatus Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "../commonstatus.h"
#include "startupstatus.h"
#include "../resource.h"
#include <commctrl.h>

#include <m_icolib.h>

#define MAX_MMITEMS		6

static LIST<void> ttbButtons(1);

static IconItem iconList[] = 
{
	{ LPGEN("Pressed toolbar icon"),  "StartupStatus/TtbDown", IDI_TTBDOWN },
	{ LPGEN("Released toolbar icon"), "StartupStatus/TtbUp",   IDI_TTBUP   },
};

/////////////////////////////////////////////////////////////////////////////////////////

void RemoveTopToolbarButtons()
{
	for (int i=ttbButtons.getCount()-1; i >= 0; i--)
		CallService(MS_TTB_REMOVEBUTTON, (WPARAM)ttbButtons[i], 0);
	ttbButtons.destroy();
}

int CreateTopToolbarButtons(WPARAM wParam, LPARAM lParam)
{
	if (iconList[0].hIcolib == NULL)
		Icon_Register(hInst, "Toolbar/StartupStatus", iconList, SIZEOF(iconList));

	int profileCount = CallService(MS_SS_GETPROFILECOUNT, 0, 0);

	TTBButton ttb = { 0 };
	ttb.cbSize = sizeof(ttb);
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.pszService = MS_SS_LOADANDSETPROFILE;
	for (int i=0; i < profileCount; i++) {
		char setting[80];
		mir_snprintf(setting, SIZEOF(setting), "%d_%s", i, SETTING_CREATETTBBUTTON);
		if (!db_get_b(NULL, MODULENAME, setting, FALSE))
			continue;
		
		DBVARIANT dbv;
		mir_snprintf(setting, SIZEOF(setting), "%d_%s", i, SETTING_PROFILENAME);
		if (db_get(NULL, MODULENAME, setting, &dbv))
			continue;

		ttb.hIconHandleDn = iconList[0].hIcolib;
		ttb.hIconHandleUp = iconList[1].hIcolib;
		ttb.wParamDown = ttb.wParamUp = i;
		ttb.name = ttb.pszTooltipUp = dbv.pszVal;
		HANDLE ttbAddResult = TopToolbar_AddButton(&ttb);
		if (ttbAddResult)
			ttbButtons.insert(ttbAddResult);
		db_free(&dbv);
	}
	return 0;
}
