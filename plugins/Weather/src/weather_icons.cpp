/*
Weather Protocol plugin for Miranda IM
Copyright (c) 2012 Miranda NG Team
Copyright (c) 2005-2011 Boris Krasnovskiy All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "weather.h"

HANDLE hIcoLibIconsChanged = NULL;

struct _tag_iconList
{
	char*  szDescr;
	char*  szName;
	int    defIconID;
	HANDLE hIconLibItem;
}
static iconList[] =
{
	{	LPGEN("Protocol icon"),      "main",      IDI_ICON       },
	{	LPGEN("Update Disabled"),    "disabled",  IDI_DISABLED   },
	{	LPGEN("View Log"),           "log",       IDI_LOG        },
	{	LPGEN("Update with Clear"),  "update2",   IDI_UPDATE2    },
	{	LPGEN("View Brief"),         "brief",     IDI_S          },
	{	LPGEN("View Complete"),      "read",      IDI_READ       },
	{	LPGEN("Weather Update"),     "update",    IDI_UPDATE     },
	{	LPGEN("Weather Map"),        "map",       IDI_MAP        },
	{	LPGEN("Popup"),              "popup",     IDI_POPUP      },
	{	LPGEN("No Popup"),           "nopopup",   IDI_NOPOPUP    },
	{	LPGEN("Edit Settings"),      "edit",      IDI_EDIT       },
};

void InitIcons(void)
{
	char szSettingName[100];
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(hInst, szFile, MAX_PATH);

	SKINICONDESC sid = { sizeof(sid) };
	sid.ptszDefaultFile = szFile;
	sid.pszName = szSettingName;
	sid.pszSection = WEATHERPROTONAME;
	sid.flags = SIDF_PATH_TCHAR;

	for (int i = 0; i < SIZEOF(iconList); i++) {
		mir_snprintf(szSettingName, SIZEOF( szSettingName ), "%s_%s", WEATHERPROTONAME, iconList[i].szName);

		sid.pszDescription = iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		iconList[i].hIconLibItem = Skin_AddIcon(&sid);
}	}

HICON  LoadIconEx(const char* name, BOOL big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", WEATHERPROTONAME, name);
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

void  ReleaseIconEx(HICON hIcon)
{
	Skin_ReleaseIcon(hIcon);
}
