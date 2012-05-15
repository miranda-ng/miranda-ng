////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2007 Adam Strzelecki <ono+miranda@java.pl>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
////////////////////////////////////////////////////////////////////////////////

#include "gg.h"

struct tagiconList
{
	const char*	szDescr;
	const char*	szName;
	int			defIconID;
}
static const iconList[] =
{
	{ LPGEN("Protocol icon"),				"main",			IDI_GG					},
	{ LPGEN("Import list from server"),		"importserver",	IDI_IMPORT_SERVER		},
	{ LPGEN("Import list from text file"),	"importtext",	IDI_IMPORT_TEXT			},
	{ LPGEN("Remove list from server"),		"removeserver",	IDI_REMOVE_SERVER		},
	{ LPGEN("Export list to server"),		"exportserver",	IDI_EXPORT_SERVER		},
	{ LPGEN("Export list to text file"),	"exporttext",	IDI_EXPORT_TEXT			},
	{ LPGEN("Account settings"),			"settings",		IDI_SETTINGS			},
	{ LPGEN("Contact list"),				"list",			IDI_LIST				},
	{ LPGEN("Block user"),					"block",		IDI_BLOCK				},
	{ LPGEN("Previous image"),				"previous",		IDI_PREV				},
	{ LPGEN("Next image"),					"next",			IDI_NEXT				},
	{ LPGEN("Send image"), 					"image",		IDI_IMAGE				},
	{ LPGEN("Save image"),					"save",			IDI_SAVE				},
	{ LPGEN("Delete image"),				"delete",		IDI_DELETE				},
	{ LPGEN("Open new conference"),			"conference",	IDI_CONFERENCE			},
	{ LPGEN("Clear ignored conferences"),	"clearignored",	IDI_CLEAR_CONFERENCE	},
	{ LPGEN("Concurrent sessions"),			"sessions",		IDI_SESSIONS			}
};

HANDLE hIconLibItem[SIZEOF(iconList)];

void gg_icolib_init()
{
	SKINICONDESC sid = {0};
	char szFile[MAX_PATH];
	char szSectionName[100];
	int i;

	mir_snprintf(szSectionName, sizeof( szSectionName ), "%s/%s", LPGEN("Protocols"), LPGEN(GGDEF_PROTO));
	GetModuleFileNameA(hInstance, szFile, MAX_PATH);

	sid.cbSize = sizeof(SKINICONDESC);
	sid.pszDefaultFile = szFile;
	sid.pszSection = szSectionName;

	for(i = 0; i < SIZEOF(iconList); i++) {
		char szSettingName[100];
		mir_snprintf(szSettingName, sizeof(szSettingName), "%s_%s", GGDEF_PROTO, iconList[i].szName);
		sid.pszName = szSettingName;
		sid.pszDescription = (char*)iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].defIconID;
		hIconLibItem[i] = (HANDLE) CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	}
}

HICON LoadIconEx(const char* name, BOOL big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, sizeof(szSettingName), "%s_%s", GGDEF_PROTO, name);
	return (HICON)CallService(MS_SKIN2_GETICON, big, (LPARAM)szSettingName);
}

HANDLE GetIconHandle(int iconId)
{
	int i;
	for(i = 0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return hIconLibItem[i];
	return NULL;
}

void ReleaseIconEx(const char* name, BOOL big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, sizeof(szSettingName), "%s_%s", GGDEF_PROTO, name);
	CallService(big ? MS_SKIN2_RELEASEICONBIG : MS_SKIN2_RELEASEICON, 0, (LPARAM)szSettingName);
}

void WindowSetIcon(HWND hWnd, const char* name)
{
	SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIconEx(name, TRUE));
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIconEx(name, FALSE));
}

void WindowFreeIcon(HWND hWnd)
{
	CallService(MS_SKIN2_RELEASEICONBIG, SendMessage(hWnd, WM_SETICON, ICON_BIG, 0), 0);
	CallService(MS_SKIN2_RELEASEICON, SendMessage(hWnd, WM_SETICON, ICON_SMALL, 0), 0);
}
