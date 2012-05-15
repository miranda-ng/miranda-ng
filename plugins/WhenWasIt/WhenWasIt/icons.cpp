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

#include "icons.h"

//HICON hiDlg = NULL;
HICON hiMainMenu = NULL;
HICON hiCheckMenu = NULL;
HICON hiListMenu = NULL;
HICON hiAddBirthdayContact = NULL;
HICON hiRefreshUserDetails = NULL;

HICON hiImportBirthdays = NULL;
HICON hiExportBirthdays = NULL;

const int cDTB = 10;
HICON hiDTB[cDTB] = {NULL};
HICON hiDTBMore = NULL;

HANDLE hClistImages[cDTB + 1];

HANDLE hWWIExtraIcons = (HANDLE) -1;

#define GET_DTB_ICON(index) (hiDTB[index] = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DTB ## index)))

int LoadIcons()
{
	//hiDlgIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_BIRTHDAYS_DLG));
	hiCheckMenu = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHECK));
	hiListMenu = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LIST));
	hiAddBirthdayContact = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ADD));
	hiRefreshUserDetails = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_REFRESH_USERDETAILS));
	
	hiImportBirthdays = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_IMPORT_BIRTHDAYS));
	hiExportBirthdays = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EXPORT_BIRTHDAYS));
	
	GET_DTB_ICON(0);
	GET_DTB_ICON(1);
	GET_DTB_ICON(2);
	GET_DTB_ICON(3);
	GET_DTB_ICON(4);
	GET_DTB_ICON(5);
	GET_DTB_ICON(6);
	GET_DTB_ICON(7);
	GET_DTB_ICON(8);
	GET_DTB_ICON(9);
	hiDTBMore = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DTBMORE));
	
	AddIcons();
	GetIcons();
	
	if (ServiceExists(MS_EXTRAICON_REGISTER))
	{
		hWWIExtraIcons = ExtraIcon_Register("WhenWasIt", Translate("WhenWasIt birthday reminder"), "MenuCheck", OnExtraIconListRebuild, OnExtraImageApply);
	}	
	
	return 0;
}

int AddIcon(HICON icon, char *name, char *description)
{
	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(SKINICONDESC);
	sid.pszSection = "WhenWasIt";
	sid.cx = sid.cy = 16;
	sid.pszDescription = description;
	sid.pszName = name;
	sid.hDefaultIcon = icon;
	
	return CallService(MS_SKIN2_ADDICON, 0, (LPARAM) &sid);
}

int AddIcons()
{
	if (ServiceExists(MS_SKIN2_ADDICON)) //if icolib is installed
		{
//			AddIcon(hiDlgIcon, "Dlg", "Popup and dialog ");
			AddIcon(hiCheckMenu, "MenuCheck", "Check birthdays menu item");
			AddIcon(hiListMenu, "MenuList", "List birthdays menu item");
			AddIcon(hiAddBirthdayContact, "AddBirthday", "Add/change birthday");
			AddIcon(hiRefreshUserDetails, "RefreshUserDetails", "Refresh user details");
			
			AddIcon(hiImportBirthdays, "ImportBirthdays", "Import birthdays");
			AddIcon(hiExportBirthdays, "ExportBirthdays", "Export birthdays");
			
			int i;
			char name[1024];
			char description[1024];
			AddIcon(hiDTB[0], "DTB0", "Birthday today");
			AddIcon(hiDTB[1], "DTB1", "1 day to birthday");
			for (i = 2; i < cDTB; i++)
				{
					sprintf(name, "DTB%d", i);
					sprintf(description, "%d days to birthday", i);
					AddIcon(hiDTB[i], name, description);
				}
			sprintf(description, "More than %d days to birthday", cDTB - 1);
			AddIcon(hiDTBMore, "DTBMore", description);
		}
	return 0;
}

HICON GetIcon(char *name)
{
	return (HICON) CallService(MS_SKIN2_GETICON, 0, (LPARAM) name);	
}

void FreeIcon(HICON &icon)
{
	DestroyIcon(icon);
	icon = NULL;
}

void FreeIcons()
{
	static int bFreed = 0;
	if (!bFreed)
		{
//			FreeIcon(hiDlg);
			FreeIcon(hiCheckMenu);
			FreeIcon(hiListMenu);
			FreeIcon(hiAddBirthdayContact);
			FreeIcon(hiRefreshUserDetails);
			
			FreeIcon(hiImportBirthdays);
			FreeIcon(hiExportBirthdays);
			
			int i;
			for (i = 0; i < cDTB; i++)
				{
					FreeIcon(hiDTB[i]);
				}
			FreeIcon(hiDTBMore);
		}
	bFreed = 1; //only free them once (ours).
}

int GetIcons()
{
	if (ServiceExists(MS_SKIN2_GETICON))
		{
			//FreeIcons();
//			hiDlgIcon = Get("Dlg");
			hiCheckMenu = GetIcon("MenuCheck");
			hiListMenu = GetIcon("MenuList");
			hiAddBirthdayContact = GetIcon("AddBirthday");
			hiRefreshUserDetails = GetIcon("RefreshUserDetails");
			
			hiImportBirthdays = GetIcon("ImportBirthdays");
			hiExportBirthdays = GetIcon("ExportBirthdays");
			
			int i;
			char buffer[1024];
			for (i = 0; i < cDTB; i++)
				{
					sprintf(buffer, "DTB%d", i);
					hiDTB[i] = GetIcon(buffer);
				}
			hiDTBMore = GetIcon("DTBMore");
		}
	return 0;
}

HICON GetDTBIcon(int dtb)
{
	if ((dtb >= cDTB) || (dtb < 0))
		{
			return hiDTBMore;
		}
	return hiDTB[dtb];
}

HICON GetDABIcon(int dab)
{
	return GetDTBIcon(dab);
}

HANDLE GetClistIcon(int dtb)
{
	if (dtb >= cDTB)
		{
			return hClistImages[cDTB];
		}
	return hClistImages[dtb];
}

HANDLE RebuildCListIcon(HICON icon)
{
	INT_PTR tmp = CallService(MS_CLIST_EXTRA_ADD_ICON, (WPARAM) icon, 0);
	if (tmp != CALLSERVICE_NOTFOUND)
		{
			return (HANDLE) tmp;
		}
	return (HANDLE) -1;
}

int RebuildAdvIconList()
{
	int i;
	for (i = 0; i < cDTB; i++)
		{
			hClistImages[i] = RebuildCListIcon(hiDTB[i]);
		}
	hClistImages[cDTB] = RebuildCListIcon(hiDTBMore);
	return 0;
}
