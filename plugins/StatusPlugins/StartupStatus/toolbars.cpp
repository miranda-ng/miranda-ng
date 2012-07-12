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

static HANDLE hTtbDown = 0, hTtbUp = 0;

static HANDLE* ttbButtons = NULL;
static int ttbButtonCount = 0;

/////////////////////////////////////////////////////////////////////////////////////////

int RemoveTopToolbarButtons()
{
	int profileCount, orgButtonCount;
	
	profileCount = CallService(MS_SS_GETPROFILECOUNT, 0, 0);
	orgButtonCount = ttbButtonCount;
	for (int i=0; i < orgButtonCount; i++)
		if (CallService(MS_TTB_REMOVEBUTTON, (WPARAM)ttbButtons[i], 0) != 1)
			ttbButtonCount -= 1;

	if (ttbButtonCount == 0) {
		free(ttbButtons);
		ttbButtons = NULL;
	}
	else ttbButtons = ( HANDLE* )realloc(ttbButtons, profileCount*sizeof(HANDLE));

	return 0;
}

int CreateTopToolbarButtons(WPARAM wParam, LPARAM lParam)
{
	int profileCount = CallService(MS_SS_GETPROFILECOUNT, 0, 0);
	ttbButtons = ( HANDLE* )realloc(ttbButtons, profileCount*sizeof(HANDLE));

	TTBButton ttb = { 0 };
	ttb.cbSize = sizeof(ttb);
	ttb.dwFlags = TTBBF_VISIBLE | TTBBF_SHOWTOOLTIP;
	ttb.pszService = MS_SS_LOADANDSETPROFILE;
	for (int i=0; i < profileCount; i++) {
		char setting[80];
		_snprintf(setting, sizeof(setting), "%d_%s", i, SETTING_CREATETTBBUTTON);
		if (!DBGetContactSettingByte(NULL, MODULENAME, setting, FALSE))
			continue;
		
		DBVARIANT dbv;
		_snprintf(setting, sizeof(setting), "%d_%s", i, SETTING_PROFILENAME);
		if (DBGetContactSetting(NULL, MODULENAME, setting, &dbv))
			continue;

		char profileName[128];
		strncpy(profileName, dbv.pszVal, sizeof(profileName)-1);
		ttb.name = profileName;
		ttb.hIconHandleDn = hTtbDown;
		ttb.hIconHandleUp = hTtbUp;
		ttb.wParamDown = ttb.wParamUp = i;
		ttb.pszTooltipUp = profileName;
		HANDLE ttbAddResult = TopToolbar_AddButton(&ttb);
		if (ttbAddResult) {
			ttbButtons[ttbButtonCount] = ttbAddResult;
			DBFreeVariant(&dbv);
			ttbButtonCount += 1;
		}
	}
	if (ttbButtonCount > 0)
		ttbButtons = ( HANDLE* )realloc(ttbButtons, ttbButtonCount*sizeof(HANDLE));
	else {
		free(ttbButtons);
		ttbButtons = NULL;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void RegisterButtons()
{
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(hInst, szFile, SIZEOF(szFile));

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(SKINICONDESC);
	sid.ptszDefaultFile = szFile;
	sid.pszSection = "StartupStatus";
	sid.pszName = "StartupStatus/TtbDown";
	sid.pszDescription = "Pressed toolbar icon";
	sid.iDefaultIndex = -IDI_TTBDOWN;
	sid.flags = SIDF_PATH_TCHAR;
	hTtbDown = Skin_AddIcon(&sid);

	sid.pszName = "StartupStatus/TtbUp";
	sid.pszDescription = "Released toolbar icon";
	sid.iDefaultIndex = -IDI_TTBUP;
	hTtbUp = Skin_AddIcon(&sid);
}
