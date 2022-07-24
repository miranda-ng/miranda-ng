/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
#include <io.h>

#include "clc.h"

struct StandardIconDescription
{
	int    id;
	LPCSTR description;
	int    resource_id;
	UINT   pf2;
	LPCSTR section;
	HANDLE hIcolib;
};

static struct StandardIconDescription mainIcons[] =
{
	{ SKINICON_OTHER_MIRANDA,         LPGEN("Miranda NG"),            -IDI_MIRANDA,        0, nullptr }, // 0
	{ SKINICON_EVENT_MESSAGE,         LPGEN("Message"),               -IDI_RECVMSG,        0, nullptr }, // 1
	{ SKINICON_EVENT_URL,             LPGEN("URL"),                   -IDI_URL,            0, nullptr }, // 2
	{ SKINICON_EVENT_FILE,            LPGEN("File"),                  -IDI_FILE,           0, nullptr }, // 3
	{ SKINICON_OTHER_USERONLINE,      LPGEN("User online"),           -IDI_USERONLINE,     0, nullptr }, // 4
	{ SKINICON_OTHER_GROUPOPEN,       LPGEN("Group (open)"),          -IDI_GROUPOPEN,      0, nullptr }, // 5
	{ SKINICON_OTHER_GROUPSHUT,       LPGEN("Group (closed)"),        -IDI_GROUPSHUT,      0, nullptr }, // 6
	{ SKINICON_OTHER_CONNECTING,      LPGEN("Connecting"),            -IDI_LOAD,           0, nullptr }, // 7
	{ SKINICON_OTHER_ADDCONTACT,      LPGEN("Add contact"),           -IDI_ADDCONTACT,     0, nullptr }, // 8
	{ SKINICON_OTHER_USERDETAILS,     LPGEN("User details"),          -IDI_USERDETAILS,    0, nullptr }, // 9
	{ SKINICON_OTHER_HISTORY,         LPGEN("History"),               -IDI_HISTORY,        0, nullptr }, // 10
	{ SKINICON_OTHER_DOWNARROW,       LPGEN("Down arrow"),            -IDI_DOWNARROW,      0, nullptr }, // 11
	{ SKINICON_OTHER_FINDUSER,        LPGEN("Find user"),             -IDI_FINDUSER,       0, nullptr }, // 12
	{ SKINICON_OTHER_OPTIONS,         LPGEN("Options"),               -IDI_OPTIONS,        0, nullptr }, // 13
	{ SKINICON_OTHER_SENDEMAIL,       LPGEN("Send e-mail"),           -IDI_SENDEMAIL,      0, nullptr }, // 14
	{ SKINICON_OTHER_DELETE,          LPGEN("Delete"),                -IDI_DELETE,         0, nullptr }, // 15
	{ SKINICON_OTHER_RENAME,          LPGEN("Rename"),                -IDI_RENAME,         0, nullptr }, // 16
	{ SKINICON_OTHER_SMS,             LPGEN("SMS"),                   -IDI_SMS,            0, nullptr }, // 17
	{ SKINICON_OTHER_SEARCHALL,       LPGEN("Search all"),            -IDI_SEARCHALL,      0, nullptr }, // 18
	{ SKINICON_OTHER_TICK,            LPGEN("Tick"),                  -IDI_TICK,           0, nullptr }, // 19
	{ SKINICON_OTHER_NOTICK,          LPGEN("No tick"),               -IDI_NOTICK,         0, nullptr }, // 20
	{ SKINICON_OTHER_HELP,            LPGEN("Help"),                  -IDI_HELP,           0, nullptr }, // 21
	{ SKINICON_OTHER_MIRANDAWEB,      LPGEN("Miranda website"),       -IDI_MIRANDAWEBSITE, 0, nullptr }, // 22
	{ SKINICON_OTHER_TYPING,          LPGEN("Typing"),                -IDI_TYPING,         0, nullptr }, // 23
	{ SKINICON_OTHER_SMALLDOT,        LPGEN("Small dot"),             -IDI_SMALLDOT,       0, nullptr }, // 24
	{ SKINICON_OTHER_FILLEDBLOB,      LPGEN("Filled blob"),           -IDI_FILLEDBLOB,     0, nullptr }, // 25
	{ SKINICON_OTHER_EMPTYBLOB,       LPGEN("Empty blob"),            -IDI_EMPTYBLOB,      0, nullptr }, // 26
	{ SKINICON_OTHER_UNICODE,         LPGEN("Unicode plugin"),        -IDI_UNICODE,        0, nullptr }, // 27
	{ SKINICON_OTHER_ANSI,            LPGEN("ANSI plugin"),           -IDI_ANSI,           0, nullptr }, // 28
	{ SKINICON_OTHER_POPUP,           LPGEN("Popups are enabled"),    -IDI_POPUP,          0, nullptr }, // 29
	{ SKINICON_OTHER_NOPOPUP,         LPGEN("Popups are disabled"),   -IDI_NOPOPUP,        0, nullptr }, // 30
	{ SKINICON_OTHER_UNDO, 	          LPGEN("Undo"),                  -IDI_UNDO,           0, nullptr }, // 31
	{ SKINICON_OTHER_WINDOW,          LPGEN("Window"),                -IDI_WINDOW,         0, nullptr }, // 32
	{ SKINICON_OTHER_WINDOWS,         LPGEN("System"),                -IDI_WINDOWS,        0, nullptr }, // 33
	{ SKINICON_OTHER_ACCMGR,          LPGEN("Accounts"),              -IDI_ACCMGR,         0, nullptr }, // 34
	{ SKINICON_OTHER_SHOWHIDE,        LPGEN("Show/Hide"),             -IDI_SHOWHIDE,       0, nullptr }, // 35
	{ SKINICON_OTHER_EXIT,            LPGEN("Exit"),                  -IDI_EXIT,           0, nullptr }, // 36
	{ SKINICON_OTHER_MAINMENU,        LPGEN("Main menu"),             -IDI_MAINMENU,       0, nullptr }, // 37
	{ SKINICON_OTHER_STATUS,          LPGEN("Status"),                -IDI_ONLINE,         0, nullptr }, // 38
	{ SKINICON_CHAT_JOIN,             LPGEN("Join chat"),             -IDI_JOINCHAT,       0, nullptr }, // 39
	{ SKINICON_CHAT_LEAVE,            LPGEN("Leave chat"),            -IDI_LEAVECHAT,      0, nullptr }, // 40
	{ SKINICON_OTHER_GROUP,           LPGEN("Move to group"),         -IDI_MOVETOGROUP,    0, nullptr }, // 41
	{ SKINICON_OTHER_ON,              LPGEN("On"),                    -IDI_ON,             0, nullptr }, // 42
	{ SKINICON_OTHER_OFF,             LPGEN("Off"),                   -IDI_OFF,            0, nullptr }, // 43
	{ SKINICON_OTHER_KEYS,            LPGEN("Password"),              -IDI_PASSWORD,       0, nullptr }, // 44
	{ SKINICON_OTHER_SOUND,           LPGEN("Sound"),                 -IDI_SOUND,          0, nullptr }, // 45
	{ SKINICON_OTHER_FRAME,           LPGEN("Frames"),                -IDI_FRAME,          0, nullptr }, // 46
	{ SKINICON_OTHER_GROUPADD,        LPGEN("Add group"),             -IDI_ADDGROUP,       0, nullptr }, // 47
	{ SKINICON_AUTH_ADD,              LPGEN("Add to list"),           -IDI_AUTH_ADD,       0, nullptr }, // 48
	{ SKINICON_AUTH_REQUEST,          LPGEN("Request authorization"), -IDI_AUTH_REQUEST,   0, nullptr }, // 49
	{ SKINICON_AUTH_GRANT,            LPGEN("Grant authorization"),   -IDI_AUTH_GRANT,     0, nullptr }, // 50
	{ SKINICON_AUTH_REVOKE,           LPGEN("Revoke authorization"),  -IDI_AUTH_REVOKE,    0, nullptr }, // 51
	{ SKINICON_FATAL,                 LPGEN("Fatal error"),           -IDI_MFATAL,         0, nullptr }, // 52
	{ SKINICON_ERROR,                 LPGEN("Error"),                 -IDI_MERROR,         0, nullptr }, // 53
	{ SKINICON_WARNING,               LPGEN("Warning"),               -IDI_MWARNING,       0, nullptr }, // 54
	{ SKINICON_INFORMATION,           LPGEN("Information"),           -IDI_MINFO,          0, nullptr }, // 55
	{ SKINICON_OTHER_EDIT,            LPGEN("Edit"),                  -IDI_TYPING,         0, nullptr }, // 56
	
	{ SKINICON_OTHER_VISIBLE_ALL,     LPGEN("Always visible"),        -IDI_ALWAYSVIS,     0, LPGEN("Contact list") },
	{ SKINICON_OTHER_INVISIBLE_ALL,   LPGEN("Always invisible"),      -IDI_NEVERVIS,      0, LPGEN("Contact list") },
	{ SKINICON_OTHER_STATUS_LOCKED,   LPGEN("Locked status"),         -IDI_STATUS_LOCKED, 0, LPGEN("Status icons") },
};

static struct StandardIconDescription statusIcons[] =
{
	{ ID_STATUS_OFFLINE,         LPGEN("Offline"),          -IDI_OFFLINE,       0x0FFFFFFF     },
	{ ID_STATUS_ONLINE,          LPGEN("Online"),           -IDI_ONLINE,        PF2_ONLINE     },
	{ ID_STATUS_AWAY,            LPGEN("Away"),             -IDI_AWAY,          PF2_SHORTAWAY  },
	{ ID_STATUS_NA,              LPGEN("Not available"),    -IDI_NA,            PF2_LONGAWAY   },
	{ ID_STATUS_OCCUPIED,        LPGEN("Occupied"),         -IDI_OCCUPIED,      PF2_LIGHTDND   },
	{ ID_STATUS_DND,             LPGEN("Do not disturb"),   -IDI_DND,           PF2_HEAVYDND   },
	{ ID_STATUS_FREECHAT,        LPGEN("Free for chat"),    -IDI_FREE4CHAT,     PF2_FREECHAT   },
	{ ID_STATUS_INVISIBLE,       LPGEN("Invisible"),        -IDI_INVISIBLE,     PF2_INVISIBLE  }
};

const char mainIconsFmt[] = "core_main_";
const char statusIconsFmt[] = "core_status_";
const char protoIconsFmt[] = LPGEN("%s icons");

#define PROTOCOLS_PREFIX LPGEN("Status icons")
#define GLOBAL_PROTO_NAME "*"

// load small icon (shared) it's not need to be destroyed
static HICON LoadSmallIconShared(HINSTANCE hInstance, LPCTSTR lpIconName)
{
	int cx = g_iIconSX;
	return (HICON)LoadImage(hInstance, lpIconName, IMAGE_ICON, cx, cx, LR_DEFAULTCOLOR | LR_SHARED);
}

// load small icon (not shared) it IS NEED to be destroyed
static HICON LoadSmallIcon(HINSTANCE hInstance, LPCTSTR lpIconName)
{
	wchar_t filename[MAX_PATH];
	if (GetModuleFileName(hInstance, filename, MAX_PATH) == 0)
		return nullptr;

	HICON hIcon = nullptr; // icon handle
	int index = -(INT_PTR)lpIconName;
	ExtractIconEx(filename, index, nullptr, &hIcon, 1);
	return hIcon;
}

// load small icon from hInstance
HICON LoadIconEx(HINSTANCE hInstance, LPCTSTR lpIconName, BOOL bShared)
{
	HICON hResIcon = bShared ? LoadSmallIconShared(hInstance, lpIconName) : LoadSmallIcon(hInstance, lpIconName);
	if (hResIcon == nullptr && g_plugin.getInst() != hInstance) // Icon not found in hInstance, let's try to load it from core
		hResIcon = bShared ? LoadSmallIconShared(g_plugin.getInst(), lpIconName) : LoadSmallIcon(g_plugin.getInst(), lpIconName);

	return hResIcon;
}

int ImageList_AddIcon_NotShared(HIMAGELIST hIml, LPCTSTR szResource)
{
	HICON hTempIcon = LoadIconEx(g_plugin.getInst(), szResource, 0);
	int res = ImageList_AddIcon(hIml, hTempIcon);
	Safe_DestroyIcon(hTempIcon);
	return res;
}

MIR_APP_DLL(int) ImageList_AddSkinIcon(HIMAGELIST hIml, int iconId)
{
	HICON hIcon = Skin_LoadIcon(iconId);
	int res = ImageList_AddIcon(hIml, hIcon);
	IcoLib_ReleaseIcon(hIcon);
	return res;
}

MIR_APP_DLL(int) ImageList_AddProtoIcon(HIMAGELIST hIml, const char *szProto, int iconId)
{
	HICON hIcon = Skin_LoadProtoIcon(szProto, iconId);
	int res = ImageList_AddIcon(hIml, hIcon);
	IcoLib_ReleaseIcon(hIcon);
	return res;
}

int ImageList_ReplaceIcon_IconLibLoaded(HIMAGELIST hIml, int nIndex, HICON hIcon)
{
	int res = ImageList_ReplaceIcon(hIml, nIndex, hIcon);
	IcoLib_ReleaseIcon(hIcon);
	return res;
}

MIR_APP_DLL(void) Window_SetIcon_IcoLib(HWND hWnd, HANDLE hIcolib)
{
	SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)IcoLib_GetIconByHandle(hIcolib, true));
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)IcoLib_GetIconByHandle(hIcolib, false));
}

MIR_APP_DLL(void) Window_SetSkinIcon_IcoLib(HWND hWnd, int iconId)
{
	SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)Skin_LoadIcon(iconId, true));
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)Skin_LoadIcon(iconId, false));
}

MIR_APP_DLL(void) Window_SetProtoIcon_IcoLib(HWND hWnd, const char *szProto, int iconId)
{
	SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)Skin_LoadProtoIcon(szProto, iconId, true));
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)Skin_LoadProtoIcon(szProto, iconId, false));
}

MIR_APP_DLL(void) Window_FreeIcon_IcoLib(HWND hWnd)
{
	IcoLib_ReleaseIcon((HICON)SendMessage(hWnd, WM_SETICON, ICON_BIG, 0), true);
	IcoLib_ReleaseIcon((HICON)SendMessage(hWnd, WM_SETICON, ICON_SMALL, 0), false);
}

MIR_APP_DLL(void) Button_SetSkin_IcoLib(HWND hwndDlg, int itemId, int iconId, const char *tooltip)
{
	HWND hWnd = GetDlgItem(hwndDlg, itemId);
	SendMessage(hWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)Skin_LoadIcon(iconId, false));
	SendMessage(hWnd, BUTTONSETASFLATBTN, TRUE, 0);
	if (tooltip)
		SendMessage(hWnd, BUTTONADDTOOLTIP, (WPARAM)tooltip, 0);
}

MIR_APP_DLL(void) Button_SetIcon_IcoLib(HWND hwndDlg, int itemId, HANDLE hIcolib, const char *tooltip)
{
	HWND hWnd = GetDlgItem(hwndDlg, itemId);
	SendMessage(hWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)IcoLib_GetIconByHandle(hIcolib, false));
	SendMessage(hWnd, BUTTONSETASFLATBTN, TRUE, 0);
	if (tooltip)
		SendMessage(hWnd, BUTTONADDTOOLTIP, (WPARAM)tooltip, 0);
}

MIR_APP_DLL(void) Button_FreeIcon_IcoLib(HWND hwndDlg, int itemId)
{
	HICON hIcon = (HICON)SendDlgItemMessage(hwndDlg, itemId, BM_SETIMAGE, IMAGE_ICON, 0);
	IcoLib_ReleaseIcon(hIcon);
}

MIR_APP_DLL(HICON) Skin_LoadProtoIcon(const char *szProto, int status, bool big)
{
	return IcoLib_GetIconByHandle(Skin_GetProtoIcon(szProto, status), big);
}

MIR_APP_DLL(HANDLE) Skin_GetProtoIcon(const char *szProto, int status)
{
	char iconName[MAX_PATH];
	INT_PTR caps2;
	if (szProto == nullptr)
		caps2 = -1;
	else if ((caps2 = CallProtoServiceInt(0, szProto, PS_GETCAPS, PFLAGNUM_2, 0)) == CALLSERVICE_NOTFOUND)
		caps2 = 0;

	if (IsStatusConnecting(status)) {
		mir_snprintf(iconName, "%s%d", mainIconsFmt, 7);
		return IcoLib_GetIconHandle(iconName);
	}

	int statusIndx = -1;
	for (int i = 0; i < _countof(statusIcons); i++)
		if (statusIcons[i].id == status) {
			statusIndx = i;
			break;
		}

	if (statusIndx == -1)
		return nullptr;

	if (!szProto) {
		// Only return a protocol specific icon if there is only one protocol
		// Otherwise return the global icon. This affects the global status menu mainly.
		if (g_arAccounts.getCount() == 1) {
			// format: core_status_%proto%statusindex
			mir_snprintf(iconName, "%s%s%d", statusIconsFmt, szProto, statusIndx);

			HANDLE hIcolib = IcoLib_GetIconHandle(iconName);
			if (hIcolib)
				return hIcolib;
		}

		// format: core_status_%s%d
		mir_snprintf(iconName, "%s%s%d", statusIconsFmt, GLOBAL_PROTO_NAME, statusIndx);
		return IcoLib_GetIconHandle(iconName);
	}

	// format: core_status_%s%d
	mir_snprintf(iconName, "%s%s%d", statusIconsFmt, szProto, statusIndx);
	if (HANDLE hIcolib = IcoLib_GetIconHandle(iconName))
		return hIcolib;

	// Queried protocol isn't in list, adding them
	if (caps2 == 0 || (caps2 & statusIcons[statusIndx].pf2)) {
		PROTOACCOUNT *pa = Proto_GetAccount(szProto);
		if (pa) {
			wchar_t szPath[MAX_PATH], szFullPath[MAX_PATH], *str;
			GetModuleFileName(nullptr, szPath, _countof(szPath));

			wchar_t tszSection[MAX_PATH];
			mir_snwprintf(tszSection, _A2W(PROTOCOLS_PREFIX)L"/%s", pa->tszAccountName);

			SKINICONDESC sid = {};
			sid.section.w = tszSection;
			sid.flags = SIDF_ALL_UNICODE;

			str = wcsrchr(szPath, '\\');
			if (str != nullptr)
				*str = 0;
			
			mir_snwprintf(szFullPath, L"%s\\Icons\\proto_%S.dll", szPath, pa->szProtoName);
			if (GetFileAttributes(szFullPath) != INVALID_FILE_ATTRIBUTES)
				sid.defaultFile.w = szFullPath;
			else {
				mir_snwprintf(szFullPath, L"%s\\Plugins\\%S.dll", szPath, szProto);
				HICON hIcon = nullptr;
				if (int(ExtractIconEx(szFullPath, statusIcons[statusIndx].resource_id, nullptr, &hIcon, 1)) > 0) {
					DestroyIcon(hIcon);
					sid.defaultFile.w = szFullPath;
					hIcon = nullptr;
				}

				if (sid.defaultFile.a == nullptr) {
					if (str != nullptr)
						*str = '\\';
					sid.defaultFile.w = szPath;
				}
			}

			// Add global icons to list
			int lowidx, highidx;
			if (caps2 == 0)
				lowidx = statusIndx, highidx = statusIndx + 1;
			else
				lowidx = 0, highidx = _countof(statusIcons);

			for (int i = lowidx; i < highidx; i++) {
				if (caps2 == 0 || (caps2 & statusIcons[i].pf2)) {
					// format: core_%s%d
					mir_snprintf(iconName, "%s%s%d", statusIconsFmt, szProto, i);
					sid.pszName = iconName;
					sid.description.w = Clist_GetStatusModeDescription(statusIcons[i].id, 0);
					sid.iDefaultIndex = statusIcons[i].resource_id;
					IcoLib_AddIcon(&sid, 0);
				}
			}
		}

		// format: core_status_%s%d
		mir_snprintf(iconName, "%s%s%d", statusIconsFmt, szProto, statusIndx);
		if (HANDLE hIcolib = IcoLib_GetIconHandle(iconName))
			return hIcolib;
	}

	mir_snprintf(iconName, "%s%s%d", statusIconsFmt, GLOBAL_PROTO_NAME, statusIndx);
	if (HANDLE hIcolib = IcoLib_GetIconHandle(iconName))
		return hIcolib;

	return nullptr;
}

MIR_APP_DLL(HANDLE) Skin_GetIconHandle(int idx)
{
	for (auto &it : mainIcons)
		if (idx == it.id)
			return it.hIcolib;

	return nullptr;
}

MIR_APP_DLL(HICON) Skin_LoadIcon(int idx, bool big)
{
	// Query for global status icons
	if (idx < SKINICON_EVENT_MESSAGE) {
		if (idx >= _countof(statusIcons))
			return nullptr;

		return Skin_LoadProtoIcon(nullptr, statusIcons[idx].id, big);
	}

	return IcoLib_GetIconByHandle(Skin_GetIconHandle(idx), big);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Initializes the icon skin module

int LoadSkinIcons(void)
{
	wchar_t modulePath[MAX_PATH];
	GetModuleFileName(g_plugin.getInst(), modulePath, _countof(modulePath));

	char iconName[MAX_PATH];
	SKINICONDESC sid = {};
	sid.defaultFile.w = modulePath;
	sid.flags = SIDF_PATH_UNICODE;
	sid.pszName = iconName;

	// Add main icons to list
	for (int i = 0; i < _countof(mainIcons); i++) {
		if (mainIcons[i].id == -1)
			continue;

		mir_snprintf(iconName, "%s%d", mainIconsFmt, i);
		sid.section.a = mainIcons[i].section == nullptr ? (char*)LPGEN("Main icons") : (char*)mainIcons[i].section;
		sid.description.a = (char*)mainIcons[i].description;
		sid.iDefaultIndex = mainIcons[i].resource_id;
		mainIcons[i].hIcolib = IcoLib_AddIcon(&sid, 0);
	}

	// Add global icons to list
	sid.section.a = PROTOCOLS_PREFIX "/" LPGEN("Global");

	// Asterisk is used, to avoid conflict with proto-plugins
	// 'coz users can't rename it to name with '*'
	for (int i = 0; i < _countof(statusIcons); i++) {
		mir_snprintf(iconName, "%s%s%d", statusIconsFmt, GLOBAL_PROTO_NAME, i);
		sid.pszName = iconName;
		sid.description.a = (char*)statusIcons[i].description;
		sid.iDefaultIndex = statusIcons[i].resource_id;
		statusIcons[i].hIcolib = IcoLib_AddIcon(&sid, 0);
	}
	return 0;
}
