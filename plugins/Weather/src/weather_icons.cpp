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
	TCHAR*  szDescr;
	char*  szName;
	int    defIconID;
	HANDLE hIconLibItem;
}
static iconList[] =
{
	{	LPGENT("Protocol icon"),      "main",      IDI_ICON       },
	{	LPGENT("Update Disabled"),    "disabled",  IDI_DISABLED   },
	{	LPGENT("View Log"),           "log",       IDI_LOG        },
	{	LPGENT("Update with Clear"),  "update2",   IDI_UPDATE2    },
	{	LPGENT("View Brief"),         "brief",     IDI_S          },
	{	LPGENT("View Complete"),      "read",      IDI_READ       },
	{	LPGENT("Weather Update"),     "update",    IDI_UPDATE     },
	{	LPGENT("Weather Map"),        "map",       IDI_MAP        },
	{	LPGENT("Popup"),              "popup",     IDI_POPUP      },
	{	LPGENT("No Popup"),           "nopopup",   IDI_NOPOPUP    },
	{	LPGENT("Edit Settings"),      "edit",      IDI_EDIT       },
};

void InitIcons(void)
{
	char szSettingName[100];
	TCHAR szFile[MAX_PATH];
	GetModuleFileName(hInst, szFile, MAX_PATH);

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(sid);
	sid.ptszDefaultFile = szFile;
	sid.pszName = szSettingName;
	sid.ptszSection = _T(WEATHERPROTONAME);
	sid.flags = SIDF_ALL_TCHAR;

	for (int i = 0; i < SIZEOF(iconList); i++) {
		mir_snprintf(szSettingName, SIZEOF( szSettingName ), "%s_%s", WEATHERPROTONAME, iconList[i].szName);

		sid.ptszDescription = iconList[i].szDescr;
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
