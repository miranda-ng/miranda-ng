/*
WhenWasIt (birthday reminder) plugin for Miranda IM

Copyright © 2006 Cristian Libotean

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

HANDLE hCheckMenu, hListMenu, hAddBirthdayContact, hRefreshUserDetails;
HANDLE hImportBirthdays, hExportBirthdays;

const int cDTB = 10;
HANDLE hDTB[cDTB] = { NULL };
HANDLE hDTBMore = NULL;

HANDLE hWWIExtraIcons = (HANDLE)-1;

static HANDLE AddIcon(char *name, char *description, TCHAR *tszPath, int iDefaultIdx)
{
	SKINICONDESC sid = { 0 };
	sid.flags = SIDF_PATH_TCHAR;
	sid.section.a = LPGEN("WhenWasIt");
	sid.description.a = description;
	sid.pszName = name;
	sid.defaultFile.t = tszPath;
	sid.iDefaultIndex = -iDefaultIdx;
	return IcoLib_AddIcon(&sid);
}

int AddIcons()
{
	TCHAR tszPath[MAX_PATH];
	GetModuleFileName(hInstance, tszPath, _countof(tszPath));

	hCheckMenu = AddIcon("MenuCheck", LPGEN("Check birthdays menu item"), tszPath, IDI_CHECK);
	hListMenu = AddIcon("MenuList", LPGEN("List birthdays menu item"), tszPath, IDI_LIST);
	hAddBirthdayContact = AddIcon("AddBirthday", LPGEN("Add/change birthday"), tszPath, IDI_ADD);
	hRefreshUserDetails = AddIcon("RefreshUserDetails", LPGEN("Refresh user details"), tszPath, IDI_REFRESH_USERDETAILS);

	hImportBirthdays = AddIcon("ImportBirthdays", LPGEN("Import birthdays"), tszPath, IDI_IMPORT_BIRTHDAYS);
	hExportBirthdays = AddIcon("ExportBirthdays", LPGEN("Export birthdays"), tszPath, IDI_EXPORT_BIRTHDAYS);

	char name[1024];
	char description[1024];
	hDTB[0] = AddIcon("DTB0", LPGEN("Birthday today"), tszPath, IDI_DTB0);
	hDTB[1] = AddIcon("DTB1", LPGEN("1 day to birthday"), tszPath, IDI_DTB1);
	for (int i = 2; i < cDTB; i++) {
		mir_snprintf(name, "DTB%d", i);
		mir_snprintf(description, _countof(description), Translate("%d days to birthday"), i);
		hDTB[i] = AddIcon(name, description, tszPath, IDI_DTB0 + i);
	}
	mir_snprintf(description, _countof(description), Translate("More than %d days to birthday"), cDTB - 1);
	hDTBMore = AddIcon("DTBMore", description, tszPath, IDI_DTBMORE);

	hWWIExtraIcons = ExtraIcon_RegisterIcolib("WhenWasIt", LPGEN("WhenWasIt birthday reminder"), "MenuCheck");
	return 0;
}

HANDLE GetDTBIconHandle(int dtb)
{
	return ((dtb >= cDTB || dtb < 0) ? hDTBMore : hDTB[dtb]);
}
