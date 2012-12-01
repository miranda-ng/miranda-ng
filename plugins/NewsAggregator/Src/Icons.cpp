/* 
Copyright (C) 2012 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "common.h"

struct _tag_iconList
{
	char*  szDescr;
	char*  szName;
	int    defIconID;
	HANDLE hIconLibItem;
}

static iconList[] =
{
	{	LPGEN("Protocol icon"),   "main",        IDI_ICON        },
	{	LPGEN("Check All Feeds"), "checkall",    IDI_CHECKALL    },
	{	LPGEN("Add Feed"),        "addfeed",     IDI_ADDFEED     },
	{	LPGEN("Import Feeds"),    "importfeeds", IDI_IMPORTFEEDS },
	{	LPGEN("Export Feeds"),    "exportfeeds", IDI_EXPORTFEEDS },
	{	LPGEN("Check Feed"),      "checkfeed",   IDI_CHECKALL    }
};

VOID InitIcons()
{
	char szSettingName[100];
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(hInst, szFile, MAX_PATH);

	SKINICONDESC sid = { sizeof(sid) };
	sid.flags = SIDF_ALL_TCHAR;
	sid.ptszDefaultFile = szFile;
	sid.pszName = szSettingName;
	sid.ptszSection = _T("News Aggregator");

	for (int i = 0; i < SIZEOF(iconList); i++) {
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", MODULE, iconList[i].szName);

		sid.pszDescription = iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		iconList[i].hIconLibItem = Skin_AddIcon(&sid);
	}	
}

HICON LoadIconEx(const char* name, BOOL big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", MODULE, name);
	return Skin_GetIcon(szSettingName, big);
}

HANDLE  GetIconHandle(const char* name)
{
	unsigned i;
	for (i=0; i < SIZEOF(iconList); i++)
		if (strcmp(iconList[i].szName, name) == 0)
			return iconList[i].hIconLibItem;
	return NULL;
}