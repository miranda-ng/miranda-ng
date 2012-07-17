/*
Miranda Crash Dumper Plugin
Copyright (C) 2008 - 2012 Boris Krasnovskiy All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "utils.h"
#include <m_icolib.h>

struct _tag_iconList
{
	const char*  szDescr;
	const char*  szName;
	int    defIconID;
}
static const iconList[] =
{
	{ "Version Information", "versionInfo", IDI_VI       },
	{ "Copy To Clipboard",   "storeToClip", IDI_VITOCLIP },
	{ "Store to file",       "storeToFile", IDI_VITOFILE },
	{ "Show",                "showInfo",    IDI_VISHOW   },
	{ "Upload",              "uploadInfo",  IDI_VIUPLOAD },
};

static HANDLE hIconLibItem[SIZEOF(iconList)];

void InitIcons(void)
{
	char szFile[MAX_PATH];
	char szSettingName[100];
	SKINICONDESC sid = {0};

	sid.cbSize = sizeof(SKINICONDESC);
	sid.pszDefaultFile = szFile;
	sid.pszName = szSettingName;
	sid.pszSection = (char*)PluginName;

	GetModuleFileNameA(hInst, szFile, MAX_PATH);

	for (unsigned i = 0; i < SIZEOF(iconList); i++) 
	{
		mir_snprintf(szSettingName, sizeof(szSettingName), "%s_%s", PluginName, iconList[i].szName);

		sid.pszDescription = (char*)iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		hIconLibItem[i] = Skin_AddIcon(&sid);
	}	
}

HICON LoadIconEx(const char* name, bool big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, sizeof(szSettingName), "%s_%s", PluginName, name);
	return (HICON)CallService(MS_SKIN2_GETICON, big, (LPARAM)szSettingName);
}

HANDLE GetIconHandle(const char* name)
{
	unsigned i;
	for (i=0; i < SIZEOF(iconList); i++)
		if (strcmp(iconList[i].szName, name) == 0)
			return hIconLibItem[i];
	return NULL;
}

void ReleaseIconEx(const char* name)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, sizeof(szSettingName), "%s_%s", PluginName, name);
	CallService(MS_SKIN2_RELEASEICON, 0, (LPARAM)szSettingName);
}

void ReleaseIconEx(HICON hIcon)
{
	CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
}
