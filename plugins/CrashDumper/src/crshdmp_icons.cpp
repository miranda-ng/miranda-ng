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

static IconItem iconList[] =
{
	{ LPGEN("Version Information"), "versionInfo", IDI_VI },
	{ LPGEN("Copy To Clipboard"), "storeToClip", IDI_VITOCLIP },
	{ LPGEN("Store to file"), "storeToFile", IDI_VITOFILE },
	{ LPGEN("Show"), "showInfo", IDI_VISHOW },
	{ LPGEN("Upload"), "uploadInfo", IDI_VIUPLOAD },
	{ LPGEN("Copy link to clipboard"), "linkToClip", IDI_LINKTOCLIP },
};

void InitIcons(void)
{
	Icon_Register(hInst, LPGEN("Crash Dumper"), iconList, SIZEOF(iconList), PluginName);
}

HICON LoadIconEx(int iconId, bool big)
{
	for (int i = 0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return Skin_GetIconByHandle(iconList[i].hIcolib, big);

	return NULL;
}

HANDLE GetIconHandle(int iconId)
{
	for (int i = 0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return iconList[i].hIcolib;

	return NULL;
}
