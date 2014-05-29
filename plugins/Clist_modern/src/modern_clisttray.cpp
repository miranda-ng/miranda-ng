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
static int RefreshTimerId = 0;   /////by FYR
static VOID CALLBACK RefreshTimerProc(HWND hwnd,UINT message,UINT idEvent,DWORD dwTime); ///// by FYR
static HMENU hMainMenu, hStatusMenu;
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
	switch ( status ) {
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
	if (status < ID_STATUS_OFFLINE && status>0) return 600; //connecting is most priority
	return 0;
}

int GetStatusOrder(int currentStatus, int newStatus)
{
	int current = GetStatusVal(currentStatus);
	int newstat = GetStatusVal(newStatus);
	return (current>newstat)?currentStatus:newStatus;
}

INT_PTR CListTray_GetGlobalStatus(WPARAM wparam,LPARAM lparam)
{
	PROTOACCOUNT **acc;
	int AccNum, i;

	ProtoEnumAccounts(&AccNum, &acc);

	for (i = AccNum; i--;)
	{
		if (!acc[i]->bIsVirtual && acc[i]->bIsVisible && acc[i]->ppro)
		{
			if (acc[i]->ppro->m_iStatus >= ID_STATUS_ONLINE)
				return ID_STATUS_ONLINE;
		}
	}

	return ID_STATUS_OFFLINE;
}

////////////////////////////////////////////////////////////
///// Need to refresh trays icon  after timely changing/////
////////////////////////////////////////////////////////////

static VOID CALLBACK RefreshTimerProc(HWND hwnd,UINT message,UINT idEvent,DWORD dwTime)
{
	if (RefreshTimerId) {
		KillTimer(NULL,RefreshTimerId);
		RefreshTimerId = 0;
	}

	int count;
	PROTOACCOUNT **accs;
	ProtoEnumAccounts(&count, &accs);
	for (int i=0; i < count; i++)
		if ( pcli->pfnGetProtocolVisibility(accs[i]->szModuleName))
			pcli->pfnTrayIconUpdateBase(accs[i]->szModuleName);

}
//////// End by FYR /////////


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
		if (hSubMenu && hSubMenu == hStatusMenu || hSubMenu == hMainMenu)
			RemoveMenu(hMenu, i--, MF_BYPOSITION);
	}
	DestroyMenu(hMenu);
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
			HWND h4 = pcli->hwndContactList;
			if (db_get_b(NULL, "CList", "AutoHide", SETTING_AUTOHIDE_DEFAULT)) {
				if (LOWORD(msg->wParam) == WA_INACTIVE && h2 != h4)
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
			HMENU hMenu;
			hMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
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
			POINT pt;
			HMENU hMenu;
			hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDTRAY, 0, 0);
			g_mutex_bOnTrayRightClick = 1;

			SetForegroundWindow(msg->hwnd);
			SetFocus(msg->hwnd);

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
HANDLE hTrayMenuObject;

HANDLE hTrayMainMenuItemProxy;
HANDLE hTrayStatusMenuItemProxy;
HANDLE hTrayHideShowMainMenuItem;

//traymenu exec param(ownerdata)
typedef struct{
	char *szServiceName;
	int Param1;
}
TrayMenuExecParam, *lpTrayMenuExecParam;

/*
wparam = handle to the menu item returned by MS_CLIST_ADDCONTACTMENUITEM
return 0 on success.
*/

static INT_PTR RemoveTrayMenuItem(WPARAM wParam, LPARAM lParam)
{
	CallService(MO_REMOVEMENUITEM, wParam, 0);
	return 0;
}

static INT_PTR BuildTrayMenu(WPARAM wParam, LPARAM lParam)
{
	HMENU hMenu = CreatePopupMenu();
	int tick = GetTickCount();

	NotifyEventHooks(g_CluiData.hEventPreBuildTrayMenu, 0, 0);

	ListParam param = { 0 };
	param.MenuObjectHandle = hTrayMenuObject;
	CallService(MO_BUILDMENU, (WPARAM)hMenu, (LPARAM)&param);

	tick = GetTickCount() - tick;
	return (INT_PTR)hMenu;
}

static INT_PTR AddTrayMenuItem(WPARAM wParam, LPARAM lParam)
{
	CLISTMENUITEM *mi = (CLISTMENUITEM*)lParam;

	TMO_MenuItem tmi;
	if (!pcli->pfnConvertMenu(mi, &tmi))
		return NULL;

	lpTrayMenuExecParam mmep = (lpTrayMenuExecParam)mir_alloc(sizeof(TrayMenuExecParam));
	if (mmep == NULL)
		return 0;

	//we need just one parametr.
	mmep->szServiceName = mir_strdup(mi->pszService);
	mmep->Param1 = mi->popupPosition;
	tmi.ownerdata = mmep;

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

	if (hTrayHideShowMainMenuItem == (HANDLE)lParam) {
		mii->fMask |= MIIM_STATE;
		mii->fState |= MFS_DEFAULT;
	}

	if (hTrayMainMenuItemProxy == (HANDLE)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		//mi.fType = MFT_STRING;
		mii->hSubMenu = (HMENU)CallService(MS_CLIST_MENUGETMAIN, 0, 0);
	}

	if (hTrayStatusMenuItemProxy == (HANDLE)lParam) {
		mii->fMask |= MIIM_SUBMENU;
		//mi.fType = MFT_STRING;
		mii->hSubMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
	}

	return(TRUE);
}

//called with:
//wparam - ownerdata
//lparam - lparam from winproc
INT_PTR TrayMenuExecService(WPARAM wParam, LPARAM lParam)
{
	if (wParam != 0) {
		lpTrayMenuExecParam mmep = (lpTrayMenuExecParam)wParam;
		if (!mir_strcmp(mmep->szServiceName, "Help/AboutCommand")) {
			//bug in help.c,it used wparam as parent window handle without reason.
			mmep->Param1 = 0;
		}
		CallService(mmep->szServiceName, mmep->Param1, lParam);
	}
	return(1);
}

INT_PTR FreeOwnerDataTrayMenu(WPARAM wParam, LPARAM lParam)
{
	lpTrayMenuExecParam mmep = (lpTrayMenuExecParam)lParam;
	if (mmep != NULL) {
		mir_free(mmep->szServiceName);
		mir_free(mmep);
	}

	return 0;
}

void InitTrayMenus(void)
{
	CreateServiceFunction("CLISTMENUSTRAY/ExecService", TrayMenuExecService);
	CreateServiceFunction("CLISTMENUSTRAY/FreeOwnerDataTrayMenu", FreeOwnerDataTrayMenu);
	CreateServiceFunction("CLISTMENUSTRAY/TrayMenuonAddService", TrayMenuonAddService);

	CreateServiceFunction("CList/AddTrayMenuItem", AddTrayMenuItem);
	CreateServiceFunction(MS_CLIST_REMOVETRAYMENUITEM, RemoveTrayMenuItem);
	CreateServiceFunction(MS_CLIST_MENUBUILDTRAY, BuildTrayMenu);

	// Tray menu
	hTrayMenuObject = MO_CreateMenuObject("TrayMenu", LPGEN("Tray menu"), 0, "CLISTMENUSTRAY/ExecService");
	MO_SetMenuObjectParam(hTrayMenuObject, OPT_USERDEFINEDITEMS, TRUE);
	MO_SetMenuObjectParam(hTrayMenuObject, OPT_MENUOBJECT_SET_FREE_SERVICE, "CLISTMENUSTRAY/FreeOwnerDataTrayMenu");
	MO_SetMenuObjectParam(hTrayMenuObject, OPT_MENUOBJECT_SET_ONADD_SERVICE, "CLISTMENUSTRAY/TrayMenuonAddService");

	//add exit command to menu
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 900000;
	mi.pszService = "CloseAction";
	mi.pszName = LPGEN("E&xit");
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_EXIT);
	AddTrayMenuItem(0, (LPARAM)&mi);

	mi.position = 100000;
	mi.pszService = MS_CLIST_SHOWHIDE;
	mi.pszName = LPGEN("&Hide/Show");
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_SHOWHIDE);
	hTrayHideShowMainMenuItem = (HANDLE)AddTrayMenuItem(0, (LPARAM)&mi);

	mi.position = 200000;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_FINDUSER);
	mi.pszService = "FindAdd/FindAddCommand";
	mi.pszName = LPGEN("&Find/Add contacts...");
	AddTrayMenuItem(0, (LPARAM)&mi);

	mi.position = 300000;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_MAINMENU); // eternity #004
	mi.pszService = "FakeService_1";
	mi.pszName = LPGEN("&Main menu");
	hTrayMainMenuItemProxy = (HANDLE)AddTrayMenuItem(0, (LPARAM)&mi);

	mi.position = 300100;
	mi.pszService = "FakeService_2";
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_STATUS); // eternity #004
	mi.pszName = LPGEN("&Status");
	hTrayStatusMenuItemProxy = (HANDLE)AddTrayMenuItem(0, (LPARAM)&mi);

	mi.position = 400000;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_OPTIONS);
	mi.pszService = "Options/OptionsCommand";
	mi.pszName = LPGEN("&Options...");
	AddTrayMenuItem(0, (LPARAM)&mi);

	mi.position = 500000;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_MIRANDA);
	mi.pszService = "Help/AboutCommand";
	mi.pszName = LPGEN("&About");
	AddTrayMenuItem(0, (LPARAM)&mi);

	hMainMenu = (HMENU)CallService(MS_CLIST_MENUGETMAIN, 0, 0);
	hStatusMenu = (HMENU)CallService(MS_CLIST_MENUGETSTATUS, 0, 0);
}

void UninitTrayMenu()
{
	if (hTrayMenuObject && ServiceExists(MO_REMOVEMENUOBJECT))
		CallService(MO_REMOVEMENUOBJECT, (WPARAM)hTrayMenuObject, 0);
	hTrayMenuObject = NULL;
}


VOID CALLBACK cliTrayCycleTimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	PROTOACCOUNT **acc;
	int AccNum;

	if (!pcli->trayIconCount)
		return;

	ProtoEnumAccounts(&AccNum, &acc);

	// Нужно найти подходящий аккаунт, чтобы показать его значок.
	do
		pcli->cycleStep = (pcli->cycleStep + 1) % AccNum;
	while (acc[pcli->cycleStep]->bIsVirtual || !acc[pcli->cycleStep]->bIsVisible);

	cliTrayIconUpdateBase(acc[pcli->cycleStep]->szModuleName);
}

int cliTrayIconInit(HWND hwnd)
{
	PROTOACCOUNT **acc;
	int AccNum, i;
	BYTE Mode;

	// Нулевое количество иконок означает, что инициализация не была проведена.
	if (pcli->trayIconCount != 0)
		return 0;

	if (pcli->cycleTimerId)
	{
		KillTimer(NULL, pcli->cycleTimerId);
		pcli->cycleTimerId = 0;
	}

	// Какой режим?
	Mode = db_get_b(NULL, "CList", "TrayIcon", TRAY_ICON_MODE_GLOBAL);

	ProtoEnumAccounts(&AccNum, &acc);

	// Узнаём количество иконок.
	switch (Mode)
	{
		case TRAY_ICON_MODE_GLOBAL:
		case TRAY_ICON_MODE_ACC:
		case TRAY_ICON_MODE_CYCLE:
			pcli->trayIconCount = 1;
			break;
		case TRAY_ICON_MODE_ALL:
			// Считаем количество будущих иконок в трее.
			// Виртуальные и скрытые аккаунты не нужно показывать.
			for (i = AccNum, pcli->trayIconCount = 0; i--; )
			{
				if (!acc[i]->bIsVirtual && acc[i]->bIsVisible && acc[i]->ppro)
					pcli->trayIconCount++;
			}
			break;
	}

	pcli->trayIcon = (trayIconInfo_t*)mir_calloc(sizeof(trayIconInfo_t) * pcli->trayIconCount);

	// Добавляем иконки.
	switch (Mode)
	{
		case TRAY_ICON_MODE_GLOBAL:
			pcli->pfnTrayIconAdd(hwnd, NULL, NULL, CListTray_GetGlobalStatus(0, 0));
			break;
		case TRAY_ICON_MODE_ACC:
			{
				PROTOACCOUNT *pa = ProtoGetAccount(db_get_sa(NULL, "CList", "PrimaryStatus"));
				// Если злоумышленник удалил ключ, то устанавливаем глобальную иконку.
				if (!pa || !pa->ppro)
				{
					pcli->pfnTrayIconAdd(hwnd, NULL, NULL, CListTray_GetGlobalStatus(0, 0));
					return 0;
				}
				pcli->pfnTrayIconAdd(hwnd, pa->szModuleName, NULL, pa->ppro->m_iStatus);
			}
			break;
		case TRAY_ICON_MODE_CYCLE:
			pcli->pfnTrayIconAdd(hwnd, NULL, NULL, CListTray_GetGlobalStatus(0, 0));
			pcli->cycleStep = 0;
			cliTrayCycleTimerProc(0, 0, 0, 0); // Это затем, чтобы сразу обновить иконку.
			pcli->cycleTimerId = SetTimer(NULL, 0, db_get_w(NULL, "CList", "CycleTime", SETTING_CYCLETIME_DEFAULT) * 1000, cliTrayCycleTimerProc);
			break;
		case TRAY_ICON_MODE_ALL:
			for (i = AccNum; i--; )
			{
				if (!acc[i]->bIsVirtual && acc[i]->bIsVisible && acc[i]->ppro)
					pcli->pfnTrayIconAdd(hwnd, acc[i]->szModuleName, NULL, acc[i]->ppro->m_iStatus);
			}
			break;
	}

	return 0;
}

int cliTrayIconAdd(HWND hwnd, const char *szProto, const char *szIconProto, int status)
{
	int i;
	NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };

	// Поиск первой пустой записи во внутреннем списке.
	for (i = 0; i < pcli->trayIconCount; i++)
		if (pcli->trayIcon[i].id == 0)
			break;

	pcli->trayIcon[i].id = TRAYICON_ID_BASE + i;
	pcli->trayIcon[i].szProto = (char*)szProto;
	pcli->trayIcon[i].hBaseIcon = pcli->pfnGetIconFromStatusMode(NULL, szIconProto ? szIconProto : pcli->trayIcon[i].szProto, status);

	pcli->pfnTrayIconMakeTooltip(NULL, pcli->trayIcon[i].szProto);
	pcli->trayIcon[i].ptszToolTip = mir_tstrdup(pcli->szTip);
	lstrcpyn(nid.szTip, pcli->szTip, SIZEOF(nid.szTip));

	nid.hWnd = hwnd;
	nid.uID = pcli->trayIcon[i].id;
	nid.uCallbackMessage = TIM_CALLBACK;
	nid.hIcon = pcli->trayIcon[i].hBaseIcon;

	nid.uFlags =  NIF_ICON
				| NIF_MESSAGE
				| (!ServiceExists("mToolTip/ShowTip") || !db_get_b(NULL, "Tipper", "TrayTip", 0) ? NIF_TIP : 0)
				| (pcli->shellVersion >= 5 ? NIF_INFO : 0);
	// INFO Если Tipper отсутствует или не хочет заниматься треевыми тултипами,
	// то тогда тултипы показывает система.

	Shell_NotifyIcon(NIM_ADD, &nid);

	return 0;
}

void cliTrayIconUpdateBase(const char *szChangedProto)
{
	int i;
	PROTOACCOUNT *pa;
	NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
	HICON hIcon;

	if (!szChangedProto)
		return;

	if (!pcli->trayIconCount)
		return;

	pa = ProtoGetAccount(szChangedProto);
	if (!pa->bIsVisible || pa->bIsVirtual)
		return;

	i = 0;
	switch (db_get_b(NULL, "CList", "TrayIcon", TRAY_ICON_MODE_GLOBAL))
	{
		case TRAY_ICON_MODE_GLOBAL:
			hIcon = pcli->pfnGetIconFromStatusMode(NULL, NULL, CListTray_GetGlobalStatus(0, 0));
			pcli->pfnTrayIconMakeTooltip(NULL, NULL);
			break;
		case TRAY_ICON_MODE_ACC:
			// Если изменяется не тот аккаунт, что выбран в настройках, то ничего не делаем.
			if (strcmp(pcli->trayIcon[i].szProto, szChangedProto))
				return;
			if (g_StatusBarData.bConnectingIcon && pa->ppro->m_iStatus >= ID_STATUS_CONNECTING && pa->ppro->m_iStatus <= ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES)
				hIcon = (HICON)CLUI_GetConnectingIconService((WPARAM)szChangedProto, 0);
			else
				hIcon = pcli->pfnGetIconFromStatusMode(NULL, szChangedProto, pa->ppro->m_iStatus);
			pcli->pfnTrayIconMakeTooltip(NULL, pcli->trayIcon[i].szProto);
			break;
		case TRAY_ICON_MODE_CYCLE:
			return;
			hIcon = pcli->pfnGetIconFromStatusMode(NULL, szChangedProto, pa->ppro->m_iStatus);
			pcli->pfnTrayIconMakeTooltip(NULL, NULL);
			break;
		case TRAY_ICON_MODE_ALL:
			// Какой индекс у аккаунта, который будем апдейтить?
			for ( ; i < pcli->trayIconCount; i++)
				if (!strcmp(pcli->trayIcon[i].szProto, szChangedProto))
					break;

			if (g_StatusBarData.bConnectingIcon && pa->ppro->m_iStatus >= ID_STATUS_CONNECTING && pa->ppro->m_iStatus <= ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES)
				hIcon = (HICON)CLUI_GetConnectingIconService((WPARAM)szChangedProto, 0);
			else
				hIcon = pcli->pfnGetIconFromStatusMode(NULL, szChangedProto, pa->ppro->m_iStatus);
			pcli->pfnTrayIconMakeTooltip(NULL, pcli->trayIcon[i].szProto);
			break;
	}

	DestroyIcon(pcli->trayIcon[i].hBaseIcon);
	pcli->trayIcon[i].hBaseIcon = hIcon;

	pcli->trayIcon[i].ptszToolTip = mir_tstrdup(pcli->szTip);
	lstrcpyn(nid.szTip, pcli->szTip, SIZEOF(nid.szTip));

	nid.hWnd = pcli->hwndContactList;
	nid.uID = pcli->trayIcon[i].id;
	nid.hIcon = pcli->trayIcon[i].hBaseIcon;
	nid.uFlags =  NIF_ICON
				| (!ServiceExists("mToolTip/ShowTip") || !db_get_b(NULL, "Tipper", "TrayTip", 0) ? NIF_TIP : 0);
	// Если Tipper отсутствует или не хочет заниматься треевыми тултипами,
	// то тогда тултипы показывает система.

	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

