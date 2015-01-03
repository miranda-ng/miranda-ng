/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (с) 2012-15 Miranda NG project (http://miranda-ng.org),
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
#include <m_protoint.h>

int g_mutex_bOnTrayRightClick = 0;
BOOL g_bMultiConnectionMode = FALSE;
static HMENU hMainMenu, hStatusMenu;
static HANDLE hTrayMenuObject;
BOOL IS_WM_MOUSE_DOWN_IN_TRAY;
BOOL g_trayTooltipActive = FALSE;
POINT tray_hover_pos = { 0 };

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

BOOL  g_MultiConnectionMode = FALSE;
char* g_szConnectingProto = NULL;

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

	if (status > 0 && status < ID_STATUS_OFFLINE)
		return 600; // 'connecting' status has the top priority
	return 0;
}

int GetStatusOrder(int currentStatus, int newStatus)
{
	int current = GetStatusVal(currentStatus);
	int newstat = GetStatusVal(newStatus);
	return (current > newstat) ? currentStatus : newStatus;
}

INT_PTR CListTray_GetGlobalStatus(WPARAM, LPARAM)
{
	g_szConnectingProto = NULL;

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
		else curstatus = GetStatusOrder(curstatus, p.dwStatus);
	}

	if (connectingCount == 0)
		g_bMultiConnectionMode = FALSE;
	else if (connectingCount > 1)
		g_bMultiConnectionMode = TRUE;
	else
		g_bMultiConnectionMode = FALSE;
	return curstatus ? curstatus : ID_STATUS_OFFLINE;
}

/////////////////////////////////////////////////////////////////////////////////////////

static UINT_PTR autoHideTimerId;

static VOID CALLBACK TrayIconAutoHideTimer(HWND hwnd, UINT, UINT_PTR idEvent, DWORD)
{
	KillTimer(hwnd, idEvent);
	HWND hwndClui = pcli->hwndContactList;
	HWND ActiveWindow = GetActiveWindow();
	if (ActiveWindow == hwndClui) return;
	if (CLUI_CheckOwnedByClui(ActiveWindow)) return;

	CListMod_HideWindow();
	SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
}

int cliTrayIconPauseAutoHide(WPARAM, LPARAM)
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
		if (hSubMenu && ((hSubMenu == hStatusMenu) || (hSubMenu == hMainMenu)))
			RemoveMenu(hMenu, i--, MF_BYPOSITION);
	}
	DestroyMenu(hMenu);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Tray menu services

HGENMENU hTrayMainMenuItemProxy, hTrayStatusMenuItemProxy, hTrayHideShowMainMenuItem;

static INT_PTR BuildTrayMenu(WPARAM, LPARAM)
{
	NotifyEventHooks(g_CluiData.hEventPreBuildTrayMenu, 0, 0);

	ListParam param = { 0 };
	param.MenuObjectHandle = hTrayMenuObject;

	HMENU hMenu = CreatePopupMenu();
	CallService(MO_BUILDMENU, (WPARAM)hMenu, (LPARAM)&param);
	return (INT_PTR)hMenu;
}

static INT_PTR AddTrayMenuItem(WPARAM, LPARAM lParam)
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

INT_PTR FreeOwnerDataTrayMenu(WPARAM, LPARAM lParam)
{
	mir_free((char*)lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Tray event handler

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

	case TIM_CREATE:
		pcli->pfnTrayIconInit(msg->hwnd);
		return TRUE;

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
			HMENU hMenu = (HMENU)BuildTrayMenu(0, 0);
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

/////////////////////////////////////////////////////////////////////////////////////////
// Tray module init

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

VOID CALLBACK cliTrayCycleTimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	if (!pcli->trayIconCount)
		return;

	PROTOACCOUNT **acc;
	int AccNum;
	ProtoEnumAccounts(&AccNum, &acc);

	// looking for the appropriate account to show its icon
	int t = pcli->cycleStep;
	do {
		pcli->cycleStep = (pcli->cycleStep + 1) % AccNum;
		if (pcli->cycleStep == t)
			return;
	}
		while (acc[pcli->cycleStep]->bIsVirtual || !acc[pcli->cycleStep]->bIsVisible);

	cliTrayIconUpdateBase(acc[pcli->cycleStep]->szModuleName);
}

/////////////////////////////////////////////////////////////////////////////////////////
// this function sets the default settings values. it also migrates the
// existing settings into the new format

void SettingsMigrate(void)
{
	BYTE TrayIcon = db_get_b(NULL, "CList", "TrayIcon", 0);
	BYTE AlwaysPrimary = db_get_b(NULL, "CList", "AlwaysPrimary", 1);
	BYTE AlwaysMulti = db_get_b(NULL, "CList", "AlwaysMulti", 0);
	char *PrimaryStatus = db_get_sa(NULL, "CList", "PrimaryStatus");

	// these strings must always be set
	db_set_s(NULL, "CList", "tiAccS", "");
	db_set_s(NULL, "CList", "tiAccV", "");

	// Нужно вычислить новый режим.
	switch (TrayIcon) {
	case 0: // global or single acc
		if (AlwaysPrimary) {
			if (!PrimaryStatus) { // global always
				db_set_b(NULL, "CList", "tiModeS", TRAY_ICON_MODE_GLOBAL);
				db_set_b(NULL, "CList", "tiModeV", TRAY_ICON_MODE_GLOBAL);
			}
			else { // single acc always
				db_set_b(NULL, "CList", "tiModeS", TRAY_ICON_MODE_ACC);
				db_set_b(NULL, "CList", "tiModeV", TRAY_ICON_MODE_ACC);
				db_set_s(NULL, "CList", "tiAccS", PrimaryStatus);
				db_set_s(NULL, "CList", "tiAccV", PrimaryStatus);
			}
		}
		else {
			if (!PrimaryStatus) { // global if differ
				db_set_b(NULL, "CList", "tiModeS", TRAY_ICON_MODE_ALL);
				db_set_b(NULL, "CList", "tiModeV", TRAY_ICON_MODE_GLOBAL);
			}
			else { // single acc if differ
				db_set_b(NULL, "CList", "tiModeS", TRAY_ICON_MODE_GLOBAL);
				db_set_b(NULL, "CList", "tiModeV", TRAY_ICON_MODE_ACC);
				db_set_s(NULL, "CList", "tiAccS", PrimaryStatus);
				db_set_s(NULL, "CList", "tiAccV", PrimaryStatus);
			}
		}
		break;

	case 1: // cycle
		db_set_b(NULL, "CList", "tiModeS", TRAY_ICON_MODE_CYCLE);
		db_set_b(NULL, "CList", "tiModeV", TRAY_ICON_MODE_CYCLE);
		break;

	case 2: // multiple
		switch (AlwaysMulti) {
		case 0: // all accs if differ
			db_set_b(NULL, "CList", "tiModeS", TRAY_ICON_MODE_GLOBAL);
			db_set_b(NULL, "CList", "tiModeV", TRAY_ICON_MODE_ALL);
			break;
		case 1: // all accs always
			db_set_b(NULL, "CList", "tiModeS", TRAY_ICON_MODE_ALL);
			db_set_b(NULL, "CList", "tiModeV", TRAY_ICON_MODE_ALL);
			break;
		}
		break;
	}
}

// calculates number of accounts to be displayed 
// and the number of the most active account

int GetGoodAccNum(bool *bDiffers)
{
	PROTOACCOUNT **acc;
	int AccNum, i;
	ProtoEnumAccounts(&AccNum, &acc);

	WORD s = 0;
	BYTE d = 0;
	for (i = AccNum, AccNum = 0; i--;) {
		if (!acc[i]->bIsVirtual && acc[i]->bIsVisible && !acc[i]->bDynDisabled && acc[i]->ppro) {
			AccNum++;
			if (!d) {
				s = acc[i]->ppro->m_iStatus;
				d = 1;
			}
			else if (s != acc[i]->ppro->m_iStatus)
				d = 2;
		}
	}

	*bDiffers = d == 2;
	return AccNum;
}

BYTE OldMode; // 

int cliTrayIconInit(HWND hwnd)
{
	BYTE Mode;

	if (pcli->trayIconCount != 0)
		return 0;

	if (pcli->cycleTimerId) {
		KillTimer(NULL, pcli->cycleTimerId);
		pcli->cycleTimerId = 0;
	}

	// Присутствуют ли в базе новые настройки? Если да, то обновление не нужно.
	if (-1 == db_get_b(NULL, "CList", "tiModeS", -1))
		SettingsMigrate();

	// Нужно узнать количество годных аккаунтов и неодинаковость их статусов.
	bool bDiffers;
	pcli->trayIconCount = GetGoodAccNum(&bDiffers);
	// Если таковых аккаунтов не нашлось вообще, то будем показывать основную иконку Миранды.
	if (!pcli->trayIconCount) {
		pcli->trayIconCount = 1;
		pcli->trayIcon = (trayIconInfo_t*)mir_calloc(sizeof(trayIconInfo_t) * pcli->trayIconCount);
		pcli->pfnTrayIconAdd(hwnd, NULL, NULL, CListTray_GetGlobalStatus(0, 0));
		OldMode = TRAY_ICON_MODE_GLOBAL;
		return 0;
	}

	if (!bDiffers)  // all equal
		OldMode = Mode = db_get_b(NULL, "CList", "tiModeS", TRAY_ICON_MODE_GLOBAL);
	else
		OldMode = Mode = db_get_b(NULL, "CList", "tiModeV", TRAY_ICON_MODE_GLOBAL);

	// Некоторые режимы всегда показывают единственную иконку.
	if (Mode < 8)
		pcli->trayIconCount = 1;

	pcli->trayIcon = (trayIconInfo_t*)mir_calloc(sizeof(trayIconInfo_t) * pcli->trayIconCount);

	// Добавляем иконки.
	switch (Mode) {
	case TRAY_ICON_MODE_GLOBAL:
		pcli->pfnTrayIconAdd(hwnd, NULL, NULL, CListTray_GetGlobalStatus(0, 0));
		break;

	case TRAY_ICON_MODE_ACC:
		{
			ptrA szProto(db_get_sa(NULL, "CList", (!bDiffers) ? "tiAccS" : "tiAccV"));
			PROTOACCOUNT *pa = ProtoGetAccount(szProto);
			if (!pa || !pa->ppro)
				pcli->pfnTrayIconAdd(hwnd, NULL, NULL, CListTray_GetGlobalStatus(0, 0));
			else
				pcli->pfnTrayIconAdd(hwnd, pa->szModuleName, NULL, pa->ppro->m_iStatus);
		}
		break;

	case TRAY_ICON_MODE_CYCLE:
		pcli->pfnTrayIconAdd(hwnd, NULL, NULL, CListTray_GetGlobalStatus(0, 0));
		pcli->cycleStep = 0;
		cliTrayCycleTimerProc(0, 0, 0, 0); // force icon update
		pcli->cycleTimerId = CLUI_SafeSetTimer(NULL, 0, db_get_w(NULL, "CList", "CycleTime", SETTING_CYCLETIME_DEFAULT) * 1000, cliTrayCycleTimerProc);
		break;

	case TRAY_ICON_MODE_ALL:
		PROTOACCOUNT **acc;
		int AccNum, i;
		ProtoEnumAccounts(&AccNum, &acc);

		for (i = AccNum; i--;) {
			if (!acc[i]->bIsVirtual && acc[i]->bIsVisible && !acc[i]->bDynDisabled && acc[i]->ppro)
				pcli->pfnTrayIconAdd(hwnd, acc[i]->szModuleName, NULL, acc[i]->ppro->m_iStatus);
		}
		break;
	}

	return 0;
}

int cliTrayIconAdd(HWND hwnd, const char *szProto, const char *szIconProto, int status)
{
	int i;

	// Поиск первой пустой записи во внутреннем списке.
	for (i = 0; i < pcli->trayIconCount; i++)
		if (pcli->trayIcon[i].id == 0)
			break;

	pcli->trayIcon[i].id = TRAYICON_ID_BASE + i;
	pcli->trayIcon[i].szProto = (char*)szProto;
	pcli->trayIcon[i].hBaseIcon = pcli->pfnGetIconFromStatusMode(NULL, szIconProto ? szIconProto : pcli->trayIcon[i].szProto, status);

	NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
	nid.hWnd = hwnd;
	nid.uID = pcli->trayIcon[i].id;
	nid.uCallbackMessage = TIM_CALLBACK;
	nid.hIcon = pcli->trayIcon[i].hBaseIcon;
	nid.uFlags =  NIF_ICON | NIF_MESSAGE | NIF_TIP | (pcli->shellVersion >= 5 ? NIF_INFO : 0);

	// if Tipper is missing or turned off for tray, use system tooltips
	if (!ServiceExists("mToolTip/ShowTip") || !db_get_b(NULL, "Tipper", "TrayTip", 1)) {
		pcli->pfnTrayIconMakeTooltip(NULL, pcli->trayIcon[i].szProto);
		pcli->trayIcon[i].ptszToolTip = mir_tstrdup(pcli->szTip);

		lstrcpyn(nid.szTip, pcli->szTip, SIZEOF(nid.szTip));
	}

	Shell_NotifyIcon(NIM_ADD, &nid);

	return 0;
}

void cliTrayIconUpdateBase(const char *szChangedProto)
{
	if (!szChangedProto)
		return;

	if (!pcli->trayIconCount)
		return;

	PROTOACCOUNT *pa = ProtoGetAccount(szChangedProto);
	if (!pa->bIsVisible || pa->bIsVirtual)
		return;

	bool bDiffers;
	GetGoodAccNum(&bDiffers);

	// if the icon number to be changed, reinitialize module from scratch
	BYTE Mode = db_get_b(NULL, "CList", (!bDiffers) ? "tiModeS" : "tiModeV", TRAY_ICON_MODE_GLOBAL);
	if (Mode != OldMode) {
		OldMode = Mode;
		pcli->pfnTrayIconIconsChanged();
	}

	HICON hIcon = NULL;
	int i = 0;
	bool bShowGglobal = true;
	switch (Mode) {
	case TRAY_ICON_MODE_GLOBAL:
		hIcon = pcli->pfnGetIconFromStatusMode(NULL, NULL, CListTray_GetGlobalStatus(0, 0));
		break;

	case TRAY_ICON_MODE_ACC:
		// Если изменяется не тот аккаунт, что выбран в настройках, то ничего не делаем.
		if (strcmp(pcli->trayIcon[i].szProto, szChangedProto))
			return;

		bShowGglobal = false;
		if (g_StatusBarData.bConnectingIcon && pa->ppro->m_iStatus >= ID_STATUS_CONNECTING && pa->ppro->m_iStatus <= ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES)
			hIcon = (HICON)CLUI_GetConnectingIconService((WPARAM)szChangedProto, 0);
		else
			hIcon = pcli->pfnGetIconFromStatusMode(NULL, szChangedProto, pa->ppro->m_iStatus);
		break;

	case TRAY_ICON_MODE_CYCLE:
		hIcon = pcli->pfnGetIconFromStatusMode(NULL, szChangedProto, pa->ppro->m_iStatus);
		break;

	case TRAY_ICON_MODE_ALL:
		// Какой индекс у аккаунта, который будем апдейтить?
		for (; i < pcli->trayIconCount; i++)
			if (!strcmp(pcli->trayIcon[i].szProto, szChangedProto))
				break;

		bShowGglobal = false;
		if (g_StatusBarData.bConnectingIcon && pa->ppro->m_iStatus >= ID_STATUS_CONNECTING && pa->ppro->m_iStatus <= ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES)
			hIcon = (HICON)CLUI_GetConnectingIconService((WPARAM)szChangedProto, 0);
		else
			hIcon = pcli->pfnGetIconFromStatusMode(NULL, szChangedProto, pa->ppro->m_iStatus);
		break;
	}

	DestroyIcon(pcli->trayIcon[i].hBaseIcon);
	pcli->trayIcon[i].hBaseIcon = hIcon;
	pcli->trayIcon[i].ptszToolTip = mir_tstrdup(pcli->szTip);

	NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
	nid.hWnd = pcli->hwndContactList;
	nid.uID = pcli->trayIcon[i].id;
	nid.hIcon = pcli->trayIcon[i].hBaseIcon;
	nid.uFlags =  NIF_ICON | NIF_TIP;

	// if Tipper is missing or turned off for tray, use system tooltips
	if (!ServiceExists("mToolTip/ShowTip") || !db_get_b(NULL, "Tipper", "TrayTip", 1)) {
		pcli->pfnTrayIconMakeTooltip(NULL, (bShowGglobal) ? NULL : pcli->trayIcon[i].szProto);
		lstrcpyn(nid.szTip, pcli->szTip, SIZEOF(nid.szTip));
	}

	Shell_NotifyIcon(NIM_MODIFY, &nid);
}
