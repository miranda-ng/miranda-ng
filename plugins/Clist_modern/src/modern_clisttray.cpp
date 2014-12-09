/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#include "hdr/modern_commonheaders.h"
#include <m_clui.h>
#include "hdr/modern_clist.h"
#include "hdr/modern_commonprototypes.h"
#include "hdr/modern_statusbar.h"

int cliShowHide(WPARAM wParam, LPARAM lParam);
int g_mutex_bOnTrayRightClick = 0;

BOOL g_bMultiConnectionMode = FALSE;
static VOID CALLBACK RefreshTimerProc(HWND hwnd,UINT message,UINT idEvent,DWORD dwTime); ///// by FYR
static HMENU hMainMenu, hStatusMenu;
static HANDLE hTrayMenuObject;
BOOL IS_WM_MOUSE_DOWN_IN_TRAY;

BOOL g_trayTooltipActive = FALSE;
POINT tray_hover_pos = {0};

// don't move to win2k.h, need new and old versions to work on 9x/2000/XP
#define NIF_STATE       0x00000008
#define NIF_INFO        0x00000010

#ifndef _INC_SHLWAPI

typedef struct _DllVersionInfo {
	DWORD cbSize;
	DWORD dwMajorVersion;                   // Major version
	DWORD dwMinorVersion;                   // Minor version
	DWORD dwBuildNumber;                    // Build number
	DWORD dwPlatformID;                     // DLLVER_PLATFORM_*
} DLLVERSIONINFO;

#define DLLVER_PLATFORM_WINDOWS         0x00000001      // Windows 95
#define DLLVER_PLATFORM_NT              0x00000002      // Windows NT
typedef HRESULT (CALLBACK* DLLGETVERSIONPROC)(DLLVERSIONINFO *);

#endif

void mir_strset(TCHAR ** dest, TCHAR *source)
{
	if (*dest) mir_free_and_nil(*dest);
	if (source) *dest = mir_tstrdup(source);
}

static DLLVERSIONINFO dviShell;
BOOL g_MultiConnectionMode = FALSE;
char * g_szConnectingProto = NULL;
int GetStatusVal(int status)
{
	switch (status) {
		case ID_STATUS_OFFLINE:               return 50;
		case ID_STATUS_ONLINE:                return 100;
		case ID_STATUS_FREECHAT:              return 110;
		case ID_STATUS_INVISIBLE:             return 120;
		case ID_STATUS_AWAY:                  return 200;
		case ID_STATUS_DND:                   return 210;
		case ID_STATUS_NA:                    return 220;
		case ID_STATUS_OCCUPIED:              return 230;
		case ID_STATUS_ONTHEPHONE:            return 400;
		case ID_STATUS_OUTTOLUNCH:            return 410;
	}

	if (status < ID_STATUS_OFFLINE && status > 0) return 600; //connecting is most priority
	return 0;
}

int GetStatusOrder(int currentStatus, int newStatus)
{
	int current = GetStatusVal(currentStatus);
	int newstat = GetStatusVal(newStatus);
	return (current > newstat) ? currentStatus : newStatus;
}

INT_PTR CListTray_GetGlobalStatus(WPARAM wparam, LPARAM lparam)
{
	int curstatus = 0;
	int connectingCount = 0;
	for (int i = 0; i < pcli->hClcProtoCount; i++) {
		ClcProtoStatus &p = pcli->clcProto[i];
		if (!pcli->pfnGetProtocolVisibility(p.szProto))
			continue;
		
		if (p.dwStatus >= ID_STATUS_CONNECTING  && p.dwStatus < ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES) {
			connectingCount++;
			if (connectingCount == 1)
				g_szConnectingProto = p.szProto;
		}
		curstatus = GetStatusOrder(curstatus, p.dwStatus);
	}
	if (connectingCount == 0) {
		//g_szConnectingProto = NULL;
		g_bMultiConnectionMode = FALSE;
	}
	else if (connectingCount > 1)
		g_bMultiConnectionMode = TRUE;
	else
		g_bMultiConnectionMode = FALSE;
	return curstatus ? curstatus : ID_STATUS_OFFLINE;
}

/////////////////////////////////////////////////////////////////////////////////////////

int cliTrayCalcChanged(const char *szChangedProto, int averageMode, int netProtoCount)
{
	HWND hwnd = pcli->hwndContactList;
	HICON hIcon;
	int status;

	if (netProtoCount > 1) {
		if (averageMode >= ID_STATUS_OFFLINE) {
			if (db_get_b(NULL, "CList", "TrayIcon", SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_MULTI) {
				if (db_get_b(NULL, "CList", "AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT))
					return pcli->pfnTrayIconSetBaseInfo(cliGetIconFromStatusMode(NULL, szChangedProto, averageMode), szChangedProto);

				if (pcli->trayIcon == NULL || pcli->trayIcon[0].szProto == NULL)
					return pcli->pfnTrayIconSetBaseInfo(cliGetIconFromStatusMode(NULL, NULL, averageMode), NULL);

				pcli->pfnTrayIconDestroy(hwnd);
				pcli->pfnTrayIconInit(hwnd);
			}
			else {
				if (db_get_b(NULL, "CList", "TrayIcon", SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_SINGLE &&
					 db_get_b(NULL, "CList", "AlwaysPrimary", SETTING_ALWAYSPRIMARY_DEFAULT)) {
					ptrA szProto(db_get_sa(NULL, "CList", "PrimaryStatus"));
					return pcli->pfnTrayIconSetBaseInfo(cliGetIconFromStatusMode(NULL, szProto, averageMode), NULL);
				}
				return pcli->pfnTrayIconSetBaseInfo(cliGetIconFromStatusMode(NULL, NULL, averageMode), NULL);
			}
		}
		else {
			switch (db_get_b(NULL, "CList", "TrayIcon", SETTING_TRAYICON_DEFAULT)) {
			case SETTING_TRAYICON_SINGLE:
				status = CallProtoService(szChangedProto, PS_GETSTATUS, 0, 0);

				if (g_StatusBarData.bConnectingIcon && status >= ID_STATUS_CONNECTING && status <= ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES) {
					// 1 check if multi connecting icon
					CListTray_GetGlobalStatus(0, 0);
					if (g_bMultiConnectionMode) {
						if (_strcmpi(szChangedProto, g_szConnectingProto))
							return -1;

						hIcon = (HICON)CLUI_GetConnectingIconService(NULL, 1);
					}
					else hIcon = (HICON)CLUI_GetConnectingIconService((WPARAM)szChangedProto, 0);
				}
				else {
					ptrA szProto(db_get_sa(NULL, "CList", "PrimaryStatus"));
					hIcon = cliGetIconFromStatusMode(NULL, szProto, (szProto) ? CallProtoService(szProto, PS_GETSTATUS, 0, 0) : CallService(MS_CLIST_GETSTATUSMODE, 0, 0));
				}
				if (hIcon)
					return pcli->pfnTrayIconSetBaseInfo(hIcon, NULL);
				break;

			case SETTING_TRAYICON_CYCLE:
				status = szChangedProto ? CallProtoService(szChangedProto, PS_GETSTATUS, 0, 0) : averageMode;
				if (g_StatusBarData.bConnectingIcon && CListTray_GetGlobalStatus(0, 0) &&
					((status >= ID_STATUS_CONNECTING && status <= ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES) || g_bMultiConnectionMode)) {
					//stop cycling
					if (pcli->cycleTimerId)
						KillTimer(NULL, pcli->cycleTimerId);
					pcli->cycleTimerId = 0;

					// 1 check if multi connecting icon
					if (g_bMultiConnectionMode) {
						if (_strcmpi(szChangedProto, g_szConnectingProto))
							return -1;
						hIcon = (HICON)CLUI_GetConnectingIconService((WPARAM)"", 1);
					}
					else hIcon = (HICON)CLUI_GetConnectingIconService((WPARAM)szChangedProto, 0);
					if (hIcon)
						return pcli->pfnTrayIconSetBaseInfo(hIcon, NULL);
				}
				else {
					pcli->cycleTimerId = CLUI_SafeSetTimer(NULL, 0, db_get_w(NULL, "CList", "CycleTime", SETTING_CYCLETIME_DEFAULT) * 1000, pcli->pfnTrayCycleTimerProc);
					return pcli->pfnTrayIconSetBaseInfo(cliGetIconFromStatusMode(NULL, szChangedProto, status), NULL);
				}
				break;

			case SETTING_TRAYICON_MULTI:
				if (!pcli->trayIcon)
					pcli->pfnTrayIconRemove(NULL, NULL);
				else if (db_get_b(NULL, "CList", "AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT)) {
					if (!pcli->pfnGetProtocolVisibility(szChangedProto))
						return -1;

					status = CallProtoService(szChangedProto, PS_GETSTATUS, 0, 0);
					if (g_StatusBarData.bConnectingIcon && status >= ID_STATUS_CONNECTING && status <= ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES)
						hIcon = (HICON)CLUI_GetConnectingIconService((WPARAM)szChangedProto, 0);
					else
						hIcon = cliGetIconFromStatusMode(NULL, szChangedProto, CallProtoService(szChangedProto, PS_GETSTATUS, 0, 0));
					if (hIcon)
						return pcli->pfnTrayIconSetBaseInfo(hIcon, szChangedProto);
				}
				else if (pcli->pfnGetProtocolVisibility(szChangedProto)) {
					int avg = pcli->pfnGetAverageMode(NULL);
					int i = pcli->pfnTrayIconSetBaseInfo(cliGetIconFromStatusMode(NULL, szChangedProto, CallProtoService(szChangedProto, PS_GETSTATUS, 0, 0)), szChangedProto);
					if (i < 0) {
						Netlib_Logf(NULL, "Connection icon disabled for %s", szChangedProto);
						pcli->pfnTrayIconDestroy(hwnd);
						pcli->pfnTrayIconInit(hwnd);
						return -1;
					}

					status = CallProtoService(szChangedProto, PS_GETSTATUS, 0, 0);
					if (g_StatusBarData.bConnectingIcon && status >= ID_STATUS_CONNECTING && status <= ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES)
						if (hIcon = (HICON)CLUI_GetConnectingIconService((WPARAM)szChangedProto, 0))
							return pcli->pfnTrayIconSetBaseInfo(hIcon, szChangedProto);

					return i;
				}
				break;
			}
		}
	}
	else if (pcli->pfnGetProtocolVisibility(szChangedProto)) {
		status = CallProtoService(szChangedProto, PS_GETSTATUS, 0, 0);

		if (g_StatusBarData.bConnectingIcon && status >= ID_STATUS_CONNECTING && status <= ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES) {
			if (hIcon = (HICON)CLUI_GetConnectingIconService((WPARAM)szChangedProto, 0))
				return pcli->pfnTrayIconSetBaseInfo(hIcon, NULL);
		}
		else if (status >= ID_STATUS_OFFLINE && status <= ID_STATUS_IDLE) {
			ptrA szProto(db_get_sa(NULL, "CList", "PrimaryStatus"));
			return pcli->pfnTrayIconSetBaseInfo(cliGetIconFromStatusMode(NULL, szProto, status), NULL);
		}
	}

	return -1;
}

static UINT_PTR autoHideTimerId;

static VOID CALLBACK TrayIconAutoHideTimer(HWND hwnd, UINT message, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(hwnd, idEvent);
	HWND hwndClui = pcli->hwndContactList;
	HWND ActiveWindow = GetActiveWindow();
	if (ActiveWindow == hwndClui) return;
	if (CLUI_CheckOwnedByClui(ActiveWindow)) return;

	CListMod_HideWindow(hwndClui, SW_HIDE);
	SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
}

int cliTrayIconPauseAutoHide(WPARAM wParam, LPARAM lParam)
{
	if (db_get_b(NULL, "CList", "AutoHide", SETTING_AUTOHIDE_DEFAULT)) {
		if (GetActiveWindow() != pcli->hwndContactList && GetWindow(GetParent(GetActiveWindow()), GW_OWNER) != pcli->hwndContactList) {
			KillTimer(NULL, autoHideTimerId);
			autoHideTimerId = CLUI_SafeSetTimer(NULL, 0, 1000 * db_get_w(NULL, "CList", "HideTime", SETTING_HIDETIME_DEFAULT), TrayIconAutoHideTimer);
		}
	}

	return 0;
}

void DestroyTrayMenu(HMENU hMenu)
{
	int cnt = GetMenuItemCount(hMenu);
	for (int i = 0; i < cnt; ++i) {
		HMENU hSubMenu = GetSubMenu(hMenu, i);
		if (hSubMenu && (hSubMenu == hStatusMenu || hSubMenu == hMainMenu))
			RemoveMenu(hMenu, i--, MF_BYPOSITION);
	}
	DestroyMenu(hMenu);
}

static HMENU BuildTrayMenu()
{
	ListParam param = { 0 };
	param.MenuObjectHandle = hTrayMenuObject;

	HMENU hMenu = CreatePopupMenu();
	CallService(MO_BUILDMENU, (WPARAM)hMenu, (LPARAM)&param);
	return hMenu;
}

INT_PTR cli_TrayIconProcessMessage(WPARAM wParam, LPARAM lParam)
{
	MSG *msg = (MSG*)wParam;
	switch (msg->message) {
	case WM_EXITMENULOOP:
		if (pcli->bTrayMenuOnScreen)
			pcli->bTrayMenuOnScreen = FALSE;
		break;

	case WM_ACTIVATE:
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		{
			HWND h1 = (HWND)msg->lParam;
			HWND h2 = h1 ? GetParent(h1) : NULL;
			if (db_get_b(NULL, "CList", "AutoHide", SETTING_AUTOHIDE_DEFAULT)) {
				if (LOWORD(msg->wParam) == WA_INACTIVE && h2 != pcli->hwndContactList)
					autoHideTimerId = CLUI_SafeSetTimer(NULL, 0, 1000 * db_get_w(NULL, "CList", "HideTime", SETTING_HIDETIME_DEFAULT), TrayIconAutoHideTimer);
				else {
					KillTimer(NULL, autoHideTimerId);
					autoHideTimerId = 0;
				}
			}
			else if (autoHideTimerId) {
				KillTimer(NULL, autoHideTimerId);
				autoHideTimerId = 0;
			}
		}
		return FALSE; //to avoid autohideTimer in core

	case TIM_CALLBACK:
		if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && msg->lParam == WM_LBUTTONDOWN && !db_get_b(NULL, "CList", "Tray1Click", SETTING_TRAY1CLICK_DEFAULT)) {
			POINT pt;
			HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
			g_mutex_bOnTrayRightClick = 1;
			IS_WM_MOUSE_DOWN_IN_TRAY = 1;
			SetForegroundWindow(msg->hwnd);
			SetFocus(msg->hwnd);
			GetCursorPos(&pt);
			pcli->bTrayMenuOnScreen = TRUE;
			TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, msg->hwnd, NULL);
			PostMessage(msg->hwnd, WM_NULL, 0, 0);
			g_mutex_bOnTrayRightClick = 0;
			IS_WM_MOUSE_DOWN_IN_TRAY = 0;
		}
		else if (msg->lParam == WM_MBUTTONDOWN || msg->lParam == WM_LBUTTONDOWN || msg->lParam == WM_RBUTTONDOWN) {
			IS_WM_MOUSE_DOWN_IN_TRAY = 1;
		}
		else if (msg->lParam == WM_RBUTTONUP) {
			HMENU hMenu = BuildTrayMenu();
			g_mutex_bOnTrayRightClick = 1;

			SetForegroundWindow(msg->hwnd);
			SetFocus(msg->hwnd);

			POINT pt;
			GetCursorPos(&pt);
			pcli->bTrayMenuOnScreen = TRUE;
			TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, msg->hwnd, NULL);
			DestroyTrayMenu(hMenu);
			PostMessage(msg->hwnd, WM_NULL, 0, 0);
		}
		else break;
		*((LRESULT*)lParam) = 0;
		return TRUE;
	}
	return corecli.pfnTrayIconProcessMessage(wParam, lParam);
}

//////////////////////////////TRAY MENU/////////////////////////

HGENMENU hTrayMainMenuItemProxy, hTrayStatusMenuItemProxy, hTrayHideShowMainMenuItem;

static INT_PTR BuildTrayMenu(WPARAM wParam, LPARAM lParam)
{
	NotifyEventHooks(g_CluiData.hEventPreBuildTrayMenu, 0, 0);

	ListParam param = { 0 };
	param.MenuObjectHandle = hTrayMenuObject;

	HMENU hMenu = CreatePopupMenu();
	CallService(MO_BUILDMENU, (WPARAM)hMenu, (LPARAM)&param);
	return (INT_PTR)hMenu;
}

static INT_PTR AddTrayMenuItem(WPARAM wParam, LPARAM lParam)
{
	CLISTMENUITEM *mi = (CLISTMENUITEM*)lParam;

	TMO_MenuItem tmi;
	if (!pcli->pfnConvertMenu(mi, &tmi))
		return NULL;

	tmi.ownerdata = mir_strdup(mi->pszService);

	OptParam op;
	op.Handle = (HANDLE)CallService(MO_ADDNEWMENUITEM, (WPARAM)hTrayMenuObject, (LPARAM)&tmi);
	op.Setting = OPT_MENUITEMSETUNIQNAME;
	op.Value = (INT_PTR)mi->pszService;
	CallService(MO_SETOPTIONSMENUITEM, 0, (LPARAM)&op);
	return (INT_PTR)op.Handle;
}

INT_PTR TrayMenuonAddService(WPARAM wParam, LPARAM lParam)
{
	MENUITEMINFO *mii = (MENUITEMINFO*)wParam;
	if (mii == NULL)
		return 0;

	if (hTrayHideShowMainMenuItem == (HGENMENU)lParam) {
		mii->fMask |= MIIM_STATE;
		mii->fState |= MFS_DEFAULT;
	}

	if (hTrayMainMenuItemProxy == (HGENMENU)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		mii->hSubMenu = (HMENU)CallService(MS_CLIST_MENUGETMAIN, 0, 0);
	}

	if (hTrayStatusMenuItemProxy == (HGENMENU)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		mii->hSubMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
	}

	return(TRUE);
}

// called with:
// wparam - ownerdata
// lparam - lparam from winproc
INT_PTR TrayMenuExecService(WPARAM wParam, LPARAM lParam)
{
	if (wParam != 0)
		CallService((char*)wParam, 0, lParam);

	return 1;
}

INT_PTR FreeOwnerDataTrayMenu(WPARAM wParam, LPARAM lParam)
{
	mir_free((char*)lParam);
	return 0;
}

void InitTrayMenus(void)
{
	CreateServiceFunction("CLISTMENUSTRAY/ExecService", TrayMenuExecService);
	CreateServiceFunction("CLISTMENUSTRAY/FreeOwnerDataTrayMenu", FreeOwnerDataTrayMenu);
	CreateServiceFunction("CLISTMENUSTRAY/TrayMenuonAddService", TrayMenuonAddService);

	CreateServiceFunction("CList/AddTrayMenuItem", AddTrayMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDTRAY, BuildTrayMenu);

	// Tray menu
	hTrayMenuObject = MO_CreateMenuObject("TrayMenu", LPGEN("Tray menu"), 0, "CLISTMENUSTRAY/ExecService");
	MO_SetMenuObjectParam(hTrayMenuObject, OPT_USERDEFINEDITEMS, TRUE);
	MO_SetMenuObjectParam(hTrayMenuObject, OPT_MENUOBJECT_SET_FREE_SERVICE, "CLISTMENUSTRAY/FreeOwnerDataTrayMenu");
	MO_SetMenuObjectParam(hTrayMenuObject, OPT_MENUOBJECT_SET_ONADD_SERVICE, "CLISTMENUSTRAY/TrayMenuonAddService");

	// add exit command to menu
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 900000;
	mi.pszService = "CloseAction";
	mi.pszName = LPGEN("E&xit");
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_EXIT);
	Menu_AddTrayMenuItem(&mi);

	mi.position = 100000;
	mi.pszService = MS_CLIST_SHOWHIDE;
	mi.pszName = LPGEN("&Hide/show");
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_SHOWHIDE);
	hTrayHideShowMainMenuItem = Menu_AddTrayMenuItem(&mi);

	mi.position = 200000;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_FINDUSER);
	mi.pszService = "FindAdd/FindAddCommand";
	mi.pszName = LPGEN("&Find/add contacts...");
	Menu_AddTrayMenuItem(&mi);

	mi.position = 300000;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_MAINMENU); // eternity #004
	mi.pszService = "FakeService_1";
	mi.pszName = LPGEN("&Main menu");
	hTrayMainMenuItemProxy = Menu_AddTrayMenuItem(&mi);

	mi.position = 300100;
	mi.pszService = "FakeService_2";
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_STATUS); // eternity #004
	mi.pszName = LPGEN("&Status");
	hTrayStatusMenuItemProxy = Menu_AddTrayMenuItem(&mi);

	mi.position = 400000;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_OPTIONS);
	mi.pszService = "Options/OptionsCommand";
	mi.pszName = LPGEN("&Options...");
	Menu_AddTrayMenuItem(&mi);

	mi.position = 500000;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_MIRANDA);
	mi.pszService = "Help/AboutCommand";
	mi.pszName = LPGEN("&About");
	Menu_AddTrayMenuItem(&mi);

	hMainMenu = (HMENU)CallService(MS_CLIST_MENUGETMAIN, 0, 0);
	hStatusMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
}

void UninitTrayMenu()
{
	if (hTrayMenuObject) {
		CallService(MO_REMOVEMENUOBJECT, (WPARAM)hTrayMenuObject, 0);
		hTrayMenuObject = NULL;
	}
}
