/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (�) 2012-16 Miranda NG project (http://miranda-ng.org),
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

#include "stdafx.h"
#include <m_clui.h>
#include "modern_clist.h"
#include "modern_commonprototypes.h"
#include "modern_statusbar.h"
#include <m_protoint.h>

int g_mutex_bOnTrayRightClick = 0;
BOOL g_bMultiConnectionMode = FALSE;
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

		if (IsStatusConnecting(p.dwStatus)) {
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

	case TIM_CALLBACK:
		if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && msg->lParam == WM_LBUTTONDOWN && !db_get_b(NULL, "CList", "Tray1Click", SETTING_TRAY1CLICK_DEFAULT)) {
			POINT pt;
			HMENU hMenu = Menu_GetStatusMenu();
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
		else break;
		*((LRESULT*)lParam) = 0;
		return TRUE;

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
	}
	return corecli.pfnTrayIconProcessMessage(wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Tray module init

VOID CALLBACK cliTrayCycleTimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	if (!pcli->trayIconCount)
		return;

	PROTOACCOUNT **acc;
	int AccNum;
	Proto_EnumAccounts(&AccNum, &acc);

	// looking for the appropriate account to show its icon
	int t = pcli->cycleStep;
	do {
		pcli->cycleStep = (pcli->cycleStep + 1) % AccNum;
		if (pcli->cycleStep == t)
			return;
	} while (acc[pcli->cycleStep]->bIsVirtual || !acc[pcli->cycleStep]->bIsVisible);

	cliTrayCalcChanged(acc[pcli->cycleStep]->szModuleName, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// this function sets the default settings values. it also migrates the
// existing settings into the new format

void SettingsMigrate(void)
{
	BYTE TrayIcon = db_get_b(NULL, "CList", "TrayIcon", 0);
	BYTE AlwaysPrimary = db_get_b(NULL, "CList", "AlwaysPrimary", 0);
	BYTE AlwaysMulti = db_get_b(NULL, "CList", "AlwaysMulti", 0);
	ptrA PrimaryStatus(db_get_sa(NULL, "CList", "PrimaryStatus"));

	// these strings must always be set
	if (PrimaryStatus) {
		db_set_s(NULL, "CList", "tiAccS", PrimaryStatus);
		db_set_s(NULL, "CList", "tiAccV", PrimaryStatus);
	}
	else {
		db_set_s(NULL, "CList", "tiAccS", "");
		db_set_s(NULL, "CList", "tiAccV", "");
	}

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
			}
		}
		else {
			db_set_b(NULL, "CList", "tiModeS", TRAY_ICON_MODE_GLOBAL);
			db_set_b(NULL, "CList", "tiModeV", (PrimaryStatus) ? TRAY_ICON_MODE_ACC : TRAY_ICON_MODE_GLOBAL);
		}
		break;

	case 1: // cycle
		db_set_b(NULL, "CList", "tiModeS", TRAY_ICON_MODE_CYCLE);
		db_set_b(NULL, "CList", "tiModeV", TRAY_ICON_MODE_CYCLE);
		break;

	case 2: // multiple
		db_set_b(NULL, "CList", "tiModeS", (AlwaysMulti) ? TRAY_ICON_MODE_ALL : TRAY_ICON_MODE_GLOBAL);
		db_set_b(NULL, "CList", "tiModeV", TRAY_ICON_MODE_ALL);
		break;
	}
}

// calculates number of accounts to be displayed 
// and the number of the most active account

int GetGoodAccNum(bool *bDiffers, bool *bConn)
{
	PROTOACCOUNT **acc;
	int AccNum, i;
	Proto_EnumAccounts(&AccNum, &acc);

	if (bConn)
		*bConn = FALSE;

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

			if (bConn)
				if (IsStatusConnecting(acc[i]->ppro->m_iStatus))
					*bConn = TRUE;
		}
	}

	*bDiffers = d == 2;
	return AccNum;
}

BYTE OldMode; //
UINT_PTR TimerID = 0;

int cliTrayIconInit(HWND hwnd)
{
	BYTE Mode;

	if (pcli->trayIconCount != 0)
		return 0;

	if (TimerID)
	{
		KillTimer(NULL, TimerID);
		TimerID = 0;
	}

	// ������������ �� � ���� ����� ���������? ���� ��, �� ���������� �� �����.
	if (-1 == db_get_b(NULL, "CList", "tiModeS", -1))
		SettingsMigrate();

	// ����� ������ ���������� ������ ��������� � �������������� �� ��������.
	bool bDiffers;
	pcli->trayIconCount = GetGoodAccNum(&bDiffers, NULL);
	// ���� ������� ��������� �� ������� ������, �� ����� ���������� �������� ������ �������.
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

	// ��������� ������ ������ ���������� ������������ ������.
	if (Mode < 8)
		pcli->trayIconCount = 1;

	pcli->trayIcon = (trayIconInfo_t*)mir_calloc(sizeof(trayIconInfo_t) * pcli->trayIconCount);

	// ��������� ������.
	switch (Mode) {
	case TRAY_ICON_MODE_GLOBAL:
		pcli->pfnTrayIconAdd(hwnd, NULL, NULL, CListTray_GetGlobalStatus(0, 0));
		break;

	case TRAY_ICON_MODE_ACC:
	{
		ptrA szProto(db_get_sa(NULL, "CList", (!bDiffers) ? "tiAccS" : "tiAccV"));
		if (!szProto)
			break;

		PROTOACCOUNT *pa = Proto_GetAccount(szProto);
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
		// �� ��������� ID ������� � pcli, ����� fnTrayIconUpdateBase �� ������� ���.
		TimerID = CLUI_SafeSetTimer(NULL, 0, db_get_w(NULL, "CList", "CycleTime", SETTING_CYCLETIME_DEFAULT) * 1000, cliTrayCycleTimerProc);
		break;

	case TRAY_ICON_MODE_ALL:
		PROTOACCOUNT **acc;
		int AccNum, i;
		Proto_EnumAccounts(&AccNum, &acc);

		for (i = AccNum; i--;) {
			if (!acc[i]->bIsVirtual && acc[i]->bIsVisible && !acc[i]->bDynDisabled && acc[i]->ppro)
				pcli->pfnTrayIconAdd(hwnd, acc[i]->szModuleName, NULL, acc[i]->ppro->m_iStatus);
		}
		break;
	}

	return 0;
}

int cliTrayCalcChanged(const char *szChangedProto, int, int)
{
	if (!szChangedProto)
		return -1;

	if (!pcli->trayIconCount)
		return -1;

	if (!pcli->pfnGetProtocolVisibility(szChangedProto))
		return -1;

	bool bDiffers, bConn;
	GetGoodAccNum(&bDiffers, &bConn);

	// if the icon number to be changed, reinitialize module from scratch
	BYTE Mode = db_get_b(NULL, "CList", (!bDiffers) ? "tiModeS" : "tiModeV", TRAY_ICON_MODE_GLOBAL);
	if (Mode != OldMode) {
		OldMode = Mode;
		pcli->pfnTrayIconIconsChanged();
	}

	HICON hIcon = NULL;
	int i = 0, iStatus;
	char *szProto;

	switch (Mode) {
	case TRAY_ICON_MODE_GLOBAL:
		hIcon = pcli->pfnGetIconFromStatusMode(NULL, NULL, CListTray_GetGlobalStatus(0, 0));
		pcli->pfnTrayIconMakeTooltip(NULL, NULL);
		break;

	case TRAY_ICON_MODE_ACC:
		// � ���� ������ ������������ ������ ���������� ������������ ��������, � �� ������ ��� szChangedProto.
		szProto = db_get_sa(NULL, "CList", bDiffers ? "tiAccV" : "tiAccS");
		if (szProto == NULL)
			break;

		iStatus = CallProtoService(szProto, PS_GETSTATUS, 0, 0);
		if (g_StatusBarData.bConnectingIcon && IsStatusConnecting(iStatus))
			hIcon = (HICON)CLUI_GetConnectingIconService((WPARAM)szProto, 0);
		else
			hIcon = pcli->pfnGetIconFromStatusMode(NULL, szProto, CallProtoService(szProto, PS_GETSTATUS, 0, 0));

		pcli->pfnTrayIconMakeTooltip(NULL, szProto);
		break;

	case TRAY_ICON_MODE_CYCLE:
		iStatus = CallProtoService(szChangedProto, PS_GETSTATUS, 0, 0);
		if (g_StatusBarData.bConnectingIcon && IsStatusConnecting(iStatus))
			hIcon = (HICON)CLUI_GetConnectingIconService((WPARAM)szChangedProto, 0);
		else if (!bConn)
			hIcon = pcli->pfnGetIconFromStatusMode(NULL, szChangedProto, CallProtoService(szChangedProto, PS_GETSTATUS, 0, 0));
		pcli->pfnTrayIconMakeTooltip(NULL, NULL);
		break;

	case TRAY_ICON_MODE_ALL:
		// ����� ������ � ��������, ������� ����� ���������?
		for (; i < pcli->trayIconCount; i++)
			if (!mir_strcmp(pcli->trayIcon[i].szProto, szChangedProto))
				break;

		iStatus = CallProtoService(szChangedProto, PS_GETSTATUS, 0, 0);
		if (g_StatusBarData.bConnectingIcon && IsStatusConnecting(iStatus))
			hIcon = (HICON)CLUI_GetConnectingIconService((WPARAM)szChangedProto, 0);
		else
			hIcon = pcli->pfnGetIconFromStatusMode(NULL, szChangedProto, CallProtoService(szChangedProto, PS_GETSTATUS, 0, 0));
		pcli->pfnTrayIconMakeTooltip(NULL, pcli->trayIcon[i].szProto);
		break;
	}

	trayIconInfo_t &p = pcli->trayIcon[i];
	DestroyIcon(p.hBaseIcon);
	p.hBaseIcon = hIcon;
	replaceStrT(p.ptszToolTip, pcli->szTip);

	NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
	nid.hWnd = pcli->hwndContactList;
	nid.uID = p.id;
	nid.hIcon = p.hBaseIcon;
	nid.uFlags = NIF_ICON | NIF_TIP;

	// if Tipper is missing or turned off for tray, use system tooltips
	if (!ServiceExists("mToolTip/ShowTip") || !db_get_b(NULL, "Tipper", "TrayTip", 1))
		mir_tstrncpy(nid.szTip, pcli->szTip, _countof(nid.szTip));

	Shell_NotifyIcon(NIM_MODIFY, &nid);

	return -1;
}