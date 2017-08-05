/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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
#include "clc.h"

#define TOOLTIP_TOLERANCE 5

extern HIMAGELIST hCListImages;

static UINT WM_TASKBARCREATED;
static UINT WM_TASKBARBUTTONCREATED;
static UINT_PTR RefreshTimerId = 0;   /////by FYR

mir_cs trayLockCS;

static bool hasTips()
{
	return ServiceExists("mToolTip/ShowTip") && db_get_b(0, "Tipper", "TrayTip", 1);
}

#define initcheck if (!fTrayInited) return

static bool fTrayInited;

static wchar_t* sttGetXStatus(const char *szProto)
{
	if (CallProtoServiceInt(0, szProto, PS_GETSTATUS, 0, 0) > ID_STATUS_OFFLINE) {
		wchar_t tszStatus[512];
		CUSTOM_STATUS cs = { sizeof(cs) };
		cs.flags = CSSF_MASK_MESSAGE | CSSF_UNICODE;
		cs.ptszMessage = tszStatus;
		if (CallProtoServiceInt(0, szProto, PS_GETCUSTOMSTATUSEX, 0, (LPARAM)&cs) == 0)
			return mir_wstrdup(tszStatus);
	}

	return nullptr;
}

static HICON lastTaskBarIcon;
static void SetTaskBarIcon(const HICON hIcon, const wchar_t *szNewTip)
{
	if (pTaskbarInterface) {
		pTaskbarInterface->SetOverlayIcon(cli.hwndContactList, hIcon, szNewTip);
		lastTaskBarIcon = hIcon;
	}
}

wchar_t* fnTrayIconMakeTooltip(const wchar_t *szPrefix, const char *szProto)
{
	initcheck nullptr;

	mir_cslock lck(trayLockCS);
	wchar_t *szSeparator = L"\n";

	if (szProto == nullptr) {
		if (accounts.getCount() == 0)
			return nullptr;

		if (accounts.getCount() == 1)
			return cli.pfnTrayIconMakeTooltip(szPrefix, accounts[0]->szModuleName);

		CMStringW tszTip;

		if (szPrefix && szPrefix[0]) {
			if (!db_get_b(0, "CList", "AlwaysStatus", SETTING_ALWAYSSTATUS_DEFAULT)) {
				wcsncpy_s(cli.szTip, MAX_TIP_SIZE, szPrefix, _TRUNCATE);
				return cli.szTip;
			}
			tszTip.Append(szPrefix);
		}

		for (int t = 0; t < accounts.getCount(); t++) {
			int i = cli.pfnGetAccountIndexByPos(t);
			if (i == -1)
				continue;

			PROTOACCOUNT *pa = accounts[i];
			if (!cli.pfnGetProtocolVisibility(pa->szModuleName))
				continue;

			wchar_t *szStatus = cli.pfnGetStatusModeDescription(CallProtoServiceInt(0, pa->szModuleName, PS_GETSTATUS, 0, 0), 0);
			if (!szStatus)
				continue;

			if (!tszTip.IsEmpty())
				tszTip.AppendChar('\n');
			if (hasTips()) {
				tszTip.AppendFormat(L"<b>%-12.12s</b>\t%s", pa->tszAccountName, szStatus);

				ptrW ProtoXStatus(sttGetXStatus(pa->szModuleName));
				if (ProtoXStatus != nullptr) {
					if (!tszTip.IsEmpty())
						tszTip.AppendChar('\n');
					tszTip.AppendFormat(L"%-24.24s\n", ProtoXStatus);
				}
			}
			else tszTip.AppendFormat(L"%s %s", pa->tszAccountName, szStatus);
		}

		wcsncpy_s(cli.szTip, MAX_TIP_SIZE, tszTip, _TRUNCATE);
	}
	else {
		PROTOACCOUNT *pa = Proto_GetAccount(szProto);
		if (pa != nullptr) {
			ptrW ProtoXStatus(sttGetXStatus(szProto));
			wchar_t *szStatus = cli.pfnGetStatusModeDescription(CallProtoServiceInt(0, szProto, PS_GETSTATUS, 0, 0), 0);
			if (szPrefix && szPrefix[0]) {
				if (db_get_b(0, "CList", "AlwaysStatus", SETTING_ALWAYSSTATUS_DEFAULT)) {
					if (hasTips()) {
						if (ProtoXStatus != nullptr)
							mir_snwprintf(cli.szTip, MAX_TIP_SIZE, L"%s%s<b>%-12.12s</b>\t%s%s%-24.24s", szPrefix, szSeparator, pa->tszAccountName, szStatus, szSeparator, ProtoXStatus);
						else
							mir_snwprintf(cli.szTip, MAX_TIP_SIZE, L"%s%s<b>%-12.12s</b>\t%s", szPrefix, szSeparator, pa->tszAccountName, szStatus);
					}
					else mir_snwprintf(cli.szTip, MAX_TIP_SIZE, L"%s%s%s %s", szPrefix, szSeparator, pa->tszAccountName, szStatus);
				}
				else mir_wstrncpy(cli.szTip, szPrefix, MAX_TIP_SIZE);
			}
			else {
				if (hasTips()) {
					if (ProtoXStatus != nullptr)
						mir_snwprintf(cli.szTip, MAX_TIP_SIZE, L"<b>%-12.12s</b>\t%s\n%-24.24s", pa->tszAccountName, szStatus, ProtoXStatus);
					else
						mir_snwprintf(cli.szTip, MAX_TIP_SIZE, L"<b>%-12.12s</b>\t%s", pa->tszAccountName, szStatus);
				}
				else mir_snwprintf(cli.szTip, MAX_TIP_SIZE, L"%s %s", pa->tszAccountName, szStatus);
			}
		}
	}

	return cli.szTip;
}

int fnTrayIconAdd(HWND hwnd, const char *szProto, const char *szIconProto, int status)
{
	initcheck 0;

	mir_cslock lck(trayLockCS);
	int i;
	for (i = 0; i < cli.trayIconCount; i++)
		if (cli.trayIcon[i].id == 0)
			break;

	trayIconInfo_t &p = cli.trayIcon[i];
	p.id = TRAYICON_ID_BASE + i;
	p.szProto = (char*)szProto;
	p.hBaseIcon = cli.pfnGetIconFromStatusMode(0, szIconProto ? szIconProto : p.szProto, status);

	NOTIFYICONDATA nid = { NOTIFYICONDATA_V2_SIZE };
	nid.hWnd = hwnd;
	nid.uID = p.id;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = TIM_CALLBACK;
	nid.hIcon = p.hBaseIcon;

	if (cli.shellVersion >= 5)
		nid.uFlags |= NIF_INFO;

	cli.pfnTrayIconMakeTooltip(nullptr, p.szProto);
	if (!hasTips())
		mir_wstrncpy(nid.szTip, cli.szTip, _countof(nid.szTip));
	replaceStrW(p.ptszToolTip, cli.szTip);

	Shell_NotifyIcon(NIM_ADD, &nid);
	p.isBase = 1;

	if (cli.trayIconCount == 1)
		SetTaskBarIcon(cli.trayIcon[0].hBaseIcon, cli.szTip);
	return i;
}

void fnTrayIconRemove(HWND hwnd, const char *szProto)
{
	initcheck;

	mir_cslock lck(trayLockCS);
	for (int i = 0; i < cli.trayIconCount; i++) {
		trayIconInfo_t *pii = &cli.trayIcon[i];
		if (pii->id != 0 && !mir_strcmp(szProto, pii->szProto)) {
			NOTIFYICONDATA nid = { NOTIFYICONDATA_V2_SIZE };
			nid.hWnd = hwnd;
			nid.uID = pii->id;
			Shell_NotifyIcon(NIM_DELETE, &nid);

			DestroyIcon(pii->hBaseIcon);
			mir_free(pii->ptszToolTip); pii->ptszToolTip = nullptr;
			pii->id = 0;
			break;
		}
	}

	if (cli.trayIconCount == 1)
		SetTaskBarIcon(nullptr, nullptr);
}

int fnTrayIconInit(HWND hwnd)
{
	initcheck 0;

	mir_cslock lck(trayLockCS);

	int netProtoCount = 0;
	int averageMode = cli.pfnGetAverageMode(&netProtoCount);

	if (cli.cycleTimerId) {
		KillTimer(nullptr, cli.cycleTimerId);
		cli.cycleTimerId = 0;
	}

	cli.trayIconCount = 1;

	if (netProtoCount) {
		cli.trayIcon = (trayIconInfo_t*)mir_calloc(sizeof(trayIconInfo_t) * accounts.getCount());

		int trayIconSetting = db_get_b(0, "CList", "TrayIcon", SETTING_TRAYICON_DEFAULT);
		if (trayIconSetting == SETTING_TRAYICON_SINGLE) {
			DBVARIANT dbv = { DBVT_DELETED };
			char *szProto;
			if (!db_get_s(0, "CList", "PrimaryStatus", &dbv) && (averageMode < 0 || db_get_b(0, "CList", "AlwaysPrimary", 0)))
				szProto = dbv.pszVal;
			else
				szProto = nullptr;

			cli.pfnTrayIconAdd(hwnd, nullptr, szProto, szProto ? CallProtoServiceInt(0, szProto, PS_GETSTATUS, 0, 0) : CallService(MS_CLIST_GETSTATUSMODE, 0, 0));
			db_free(&dbv);
		}
		else if (trayIconSetting == SETTING_TRAYICON_MULTI && (averageMode < 0 || db_get_b(0, "CList", "AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT))) {
			cli.trayIconCount = netProtoCount;
			for (int i = 0; i < accounts.getCount(); i++) {
				int j = cli.pfnGetAccountIndexByPos(i);
				if (j >= 0) {
					PROTOACCOUNT *pa = accounts[j];
					if (cli.pfnGetProtocolVisibility(pa->szModuleName))
						cli.pfnTrayIconAdd(hwnd, pa->szModuleName, nullptr, CallProtoServiceInt(0, pa->szModuleName, PS_GETSTATUS, 0, 0));
				}
			}
		}
		else {
			cli.pfnTrayIconAdd(hwnd, nullptr, nullptr, averageMode);

			if (trayIconSetting == SETTING_TRAYICON_CYCLE && averageMode < 0)
				cli.cycleTimerId = SetTimer(nullptr, 0, db_get_w(0, "CList", "CycleTime", SETTING_CYCLETIME_DEFAULT) * 1000, cli.pfnTrayCycleTimerProc);
		}
	}
	else {
		cli.trayIcon = (trayIconInfo_t*)mir_calloc(sizeof(trayIconInfo_t));
		cli.pfnTrayIconAdd(hwnd, nullptr, nullptr, CallService(MS_CLIST_GETSTATUSMODE, 0, 0));
	}

	return 0;
}

int fnTrayIconDestroy(HWND hwnd)
{
	initcheck 0;

	mir_cslock lck(trayLockCS);
	if (cli.trayIconCount == 1)
		SetTaskBarIcon(nullptr, nullptr);

	NOTIFYICONDATA nid = { NOTIFYICONDATA_V2_SIZE };
	nid.hWnd = hwnd;
	for (int i = 0; i < cli.trayIconCount; i++) {
		if (cli.trayIcon[i].id == 0)
			continue;
		nid.uID = cli.trayIcon[i].id;
		Shell_NotifyIcon(NIM_DELETE, &nid);
		DestroyIcon(cli.trayIcon[i].hBaseIcon);
		mir_free(cli.trayIcon[i].ptszToolTip);
	}
	mir_free(cli.trayIcon);
	cli.trayIcon = nullptr;
	cli.trayIconCount = 0;
	return 0;
}

// called when Explorer crashes and the taskbar is remade
void fnTrayIconTaskbarCreated(HWND hwnd)
{
	initcheck;
	cli.pfnTrayIconDestroy(hwnd);
	cli.pfnTrayIconInit(hwnd);
}

static VOID CALLBACK RefreshTimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	if (RefreshTimerId) {
		KillTimer(nullptr, RefreshTimerId);
		RefreshTimerId = 0;
	}
	for (int i = 0; i < accounts.getCount(); i++)
		cli.pfnTrayIconUpdateBase(accounts[i]->szModuleName);
}

int fnTrayIconUpdate(HICON hNewIcon, const wchar_t *szNewTip, const char *szPreferredProto, int isBase)
{
	initcheck - 1;
	mir_cslock lck(trayLockCS);

	NOTIFYICONDATA nid = { NOTIFYICONDATA_V2_SIZE };
	nid.hWnd = cli.hwndContactList;
	nid.uFlags = NIF_ICON | NIF_TIP;
	nid.hIcon = hNewIcon;
	if (!hNewIcon)
		return -1;

	for (int i = 0; i < cli.trayIconCount; i++) {
		if (cli.trayIcon[i].id == 0)
			continue;
		if (mir_strcmp(cli.trayIcon[i].szProto, szPreferredProto))
			continue;

		nid.uID = cli.trayIcon[i].id;
		cli.pfnTrayIconMakeTooltip(szNewTip, cli.trayIcon[i].szProto);
		mir_free(cli.trayIcon[i].ptszToolTip);
		cli.trayIcon[i].ptszToolTip = mir_wstrdup(cli.szTip);
		if (!hasTips())
			mir_wstrncpy(nid.szTip, cli.szTip, _countof(nid.szTip));
		Shell_NotifyIcon(NIM_MODIFY, &nid);

		if (cli.trayIconCount == 1)
			SetTaskBarIcon(hNewIcon, cli.szTip);
		else
			SetTaskBarIcon(nullptr, nullptr);

		cli.trayIcon[i].isBase = isBase;
		return i;
	}

	// if there wasn't a suitable icon, change all the icons
	for (int i = 0; i < cli.trayIconCount; i++) {
		if (cli.trayIcon[i].id == 0)
			continue;
		nid.uID = cli.trayIcon[i].id;

		cli.pfnTrayIconMakeTooltip(szNewTip, cli.trayIcon[i].szProto);
		mir_free(cli.trayIcon[i].ptszToolTip);
		cli.trayIcon[i].ptszToolTip = mir_wstrdup(cli.szTip);
		if (!hasTips())
			mir_wstrncpy(nid.szTip, cli.szTip, _countof(nid.szTip));
		Shell_NotifyIcon(NIM_MODIFY, &nid);

		if (cli.trayIconCount == 1)
			SetTaskBarIcon(hNewIcon, cli.szTip);
		else
			SetTaskBarIcon(nullptr, nullptr);

		cli.trayIcon[i].isBase = isBase;
		if (db_get_b(0, "CList", "TrayIcon", SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_MULTI) {
			DWORD time1 = db_get_w(0, "CList", "CycleTime", SETTING_CYCLETIME_DEFAULT) * 200;
			DWORD time2 = db_get_w(0, "CList", "IconFlashTime", 550) + 1000;
			DWORD time = max(max(2000, time1), time2);
			if (RefreshTimerId)
				KillTimer(nullptr, RefreshTimerId);

			// if unknown base was changed - than show preffered proto icon for 2 sec
			// and reset it to original one after timeout
			RefreshTimerId = SetTimer(nullptr, 0, time, RefreshTimerProc);
		}
		return i;
	}

	return -1;
}

int fnTrayIconSetBaseInfo(HICON hIcon, const char *szPreferredProto)
{
	if (!fTrayInited) {
	LBL_Error:
		DestroyIcon(hIcon);
		return -1;
	}

	mir_cslock lck(trayLockCS);

	if (szPreferredProto) {
		for (int i = 0; i < cli.trayIconCount; i++) {
			if (cli.trayIcon[i].id == 0)
				continue;
			if (mir_strcmp(cli.trayIcon[i].szProto, szPreferredProto))
				continue;

			DestroyIcon(cli.trayIcon[i].hBaseIcon);
			cli.trayIcon[i].hBaseIcon = hIcon;
			return i;
		}
		if ((cli.pfnGetProtocolVisibility(szPreferredProto)) &&
			(cli.pfnGetAverageMode(nullptr) == -1) &&
			(db_get_b(0, "CList", "TrayIcon", SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_MULTI) &&
			!(db_get_b(0, "CList", "AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT)))
			goto LBL_Error;
	}

	// if there wasn't a specific icon, there will only be one suitable
	for (int i = 0; i < cli.trayIconCount; i++) {
		if (cli.trayIcon[i].id == 0)
			continue;

		DestroyIcon(cli.trayIcon[i].hBaseIcon);
		cli.trayIcon[i].hBaseIcon = hIcon;
		return i;
	}

	goto LBL_Error;
}

void fnTrayIconUpdateWithImageList(int iImage, const wchar_t *szNewTip, char *szPreferredProto)
{
	HICON hIcon = ImageList_GetIcon(hCListImages, iImage, ILD_NORMAL);
	cli.pfnTrayIconUpdate(hIcon, szNewTip, szPreferredProto, 0);
	DestroyIcon(hIcon);
}

VOID CALLBACK fnTrayCycleTimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	initcheck;
	mir_cslock lck(trayLockCS);

	int i;
	for (i = accounts.getCount() + 1; --i;) {
		cli.cycleStep = (cli.cycleStep + 1) % accounts.getCount();
		if (cli.pfnGetProtocolVisibility(accounts[cli.cycleStep]->szModuleName))
			break;
	}

	if (i) {
		DestroyIcon(cli.trayIcon[0].hBaseIcon);
		cli.trayIcon[0].hBaseIcon = cli.pfnGetIconFromStatusMode(0, accounts[cli.cycleStep]->szModuleName,
			CallProtoServiceInt(0, accounts[cli.cycleStep]->szModuleName, PS_GETSTATUS, 0, 0));
		if (cli.trayIcon[0].isBase)
			cli.pfnTrayIconUpdate(cli.trayIcon[0].hBaseIcon, nullptr, nullptr, 1);
	}
}

void fnTrayIconUpdateBase(const char *szChangedProto)
{
	initcheck;
	if (szChangedProto == nullptr) return;
	if (!cli.pfnGetProtocolVisibility(szChangedProto)) return;

	int netProtoCount;
	mir_cslock lck(trayLockCS);
	int averageMode = cli.pfnGetAverageMode(&netProtoCount);

	if (cli.cycleTimerId) {
		KillTimer(nullptr, cli.cycleTimerId);
		cli.cycleTimerId = 0;
	}

	for (int i = 0; i < accounts.getCount(); i++)
		if (!mir_strcmp(szChangedProto, accounts[i]->szModuleName))
			cli.cycleStep = i;

	int changed = cli.pfnTrayCalcChanged(szChangedProto, averageMode, netProtoCount);
	if (changed != -1 && cli.trayIcon[changed].isBase)
		cli.pfnTrayIconUpdate(cli.trayIcon[changed].hBaseIcon, nullptr, cli.trayIcon[changed].szProto, 1);
}

int fnTrayCalcChanged(const char *szChangedProto, int averageMode, int netProtoCount)
{
	if (netProtoCount == 0)
		return cli.pfnTrayIconSetBaseInfo(ImageList_GetIcon(hCListImages, cli.pfnIconFromStatusMode(nullptr, averageMode, 0), ILD_NORMAL), nullptr);

	int trayIconSetting = db_get_b(0, "CList", "TrayIcon", SETTING_TRAYICON_DEFAULT);

	if (averageMode > 0) {
		if (trayIconSetting != SETTING_TRAYICON_MULTI)
			return cli.pfnTrayIconSetBaseInfo(cli.pfnGetIconFromStatusMode(0, nullptr, averageMode), nullptr);

		if (db_get_b(0, "CList", "AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT))
			return cli.pfnTrayIconSetBaseInfo(cli.pfnGetIconFromStatusMode(0, szChangedProto, CallProtoServiceInt(0, szChangedProto, PS_GETSTATUS, 0, 0)), (char*)szChangedProto);

		if (cli.trayIcon == nullptr || cli.trayIcon[0].szProto == nullptr)
			return cli.pfnTrayIconSetBaseInfo(cli.pfnGetIconFromStatusMode(0, nullptr, averageMode), nullptr);

		cli.pfnTrayIconDestroy(cli.hwndContactList);
		cli.pfnTrayIconInit(cli.hwndContactList);
	}
	else {
		switch (trayIconSetting) {
		case SETTING_TRAYICON_CYCLE:
			cli.cycleTimerId = SetTimer(nullptr, 0, db_get_w(0, "CList", "CycleTime", SETTING_CYCLETIME_DEFAULT) * 1000, cli.pfnTrayCycleTimerProc);
			return cli.pfnTrayIconSetBaseInfo(ImageList_GetIcon(hCListImages, cli.pfnIconFromStatusMode(szChangedProto, CallProtoServiceInt(0, szChangedProto, PS_GETSTATUS, 0, 0), 0), ILD_NORMAL), nullptr);

		case SETTING_TRAYICON_MULTI:
			if (!cli.trayIcon)
				cli.pfnTrayIconRemove(nullptr, nullptr);
			else if ((cli.trayIconCount > 1 || netProtoCount == 1) || db_get_b(0, "CList", "AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT))
				return cli.pfnTrayIconSetBaseInfo(cli.pfnGetIconFromStatusMode(0, szChangedProto, CallProtoServiceInt(0, szChangedProto, PS_GETSTATUS, 0, 0)), (char*)szChangedProto);
			else {
				cli.pfnTrayIconDestroy(cli.hwndContactList);
				cli.pfnTrayIconInit(cli.hwndContactList);
			}
			break;

		case SETTING_TRAYICON_SINGLE:
			ptrA szProto(db_get_sa(0, "CList", "PrimaryStatus"));
			return cli.pfnTrayIconSetBaseInfo(cli.pfnGetIconFromStatusMode(0, szProto, szProto ? CallProtoServiceInt(0, szProto, PS_GETSTATUS, 0, 0) : CallService(MS_CLIST_GETSTATUSMODE, 0, 0)), szProto);
		}
	}

	return -1;
}

void fnTrayIconSetToBase(char *szPreferredProto)
{
	int i;
	initcheck;
	mir_cslock lck(trayLockCS);

	for (i = 0; i < cli.trayIconCount; i++) {
		if (cli.trayIcon[i].id == 0)
			continue;
		if (mir_strcmp(cli.trayIcon[i].szProto, szPreferredProto))
			continue;
		cli.pfnTrayIconUpdate(cli.trayIcon[i].hBaseIcon, nullptr, szPreferredProto, 1);
		return;
	}

	// if there wasn't a specific icon, there will only be one suitable
	for (i = 0; i < cli.trayIconCount; i++) {
		if (cli.trayIcon[i].id == 0)
			continue;
		cli.pfnTrayIconUpdate(cli.trayIcon[i].hBaseIcon, nullptr, szPreferredProto, 1);
		return;
	}
}

void fnTrayIconIconsChanged(void)
{
	initcheck;
	mir_cslock lck(trayLockCS);

	cli.pfnTrayIconDestroy(cli.hwndContactList);
	cli.pfnTrayIconInit(cli.hwndContactList);
}

static UINT_PTR autoHideTimerId;
static VOID CALLBACK TrayIconAutoHideTimer(HWND hwnd, UINT, UINT_PTR idEvent, DWORD)
{
	initcheck;
	mir_cslock lck(trayLockCS);

	KillTimer(hwnd, idEvent);
	HWND hwndClui = cli.hwndContactList;
	if (GetActiveWindow() != hwndClui) {
		ShowWindow(hwndClui, SW_HIDE);
		SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
	}
}

int fnTrayIconPauseAutoHide(WPARAM, LPARAM)
{
	initcheck 0;
	mir_cslock lck(trayLockCS);

	if (db_get_b(0, "CList", "AutoHide", SETTING_AUTOHIDE_DEFAULT)) {
		if (GetActiveWindow() != cli.hwndContactList) {
			KillTimer(nullptr, autoHideTimerId);
			autoHideTimerId = SetTimer(nullptr, 0, 1000 * db_get_w(0, "CList", "HideTime", SETTING_HIDETIME_DEFAULT), TrayIconAutoHideTimer);
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// processes tray icon's messages

static BYTE s_LastHoverIconID = 0;
static bool g_trayTooltipActive = false;
static POINT tray_hover_pos = { 0 };

static void CALLBACK TrayHideToolTipTimerProc(HWND hwnd, UINT, UINT_PTR, DWORD)
{
	if (g_trayTooltipActive) {
		POINT pt;
		GetCursorPos(&pt);
		if (abs(pt.x - tray_hover_pos.x) > TOOLTIP_TOLERANCE || abs(pt.y - tray_hover_pos.y) > TOOLTIP_TOLERANCE) {
			CallService("mToolTip/HideTip", 0, 0);
			g_trayTooltipActive = false;
			KillTimer(hwnd, TIMERID_TRAYHOVER_2);
		}
	}
	else KillTimer(hwnd, TIMERID_TRAYHOVER_2);
}

static void CALLBACK TrayToolTipTimerProc(HWND hwnd, UINT, UINT_PTR id, DWORD)
{
	if (!g_trayTooltipActive && !cli.bTrayMenuOnScreen) {
		POINT pt;
		GetCursorPos(&pt);
		if (abs(pt.x - tray_hover_pos.x) <= TOOLTIP_TOLERANCE && abs(pt.y - tray_hover_pos.y) <= TOOLTIP_TOLERANCE) {
			wchar_t* szTipCur = cli.szTip;
			{
				int n = s_LastHoverIconID - 100;
				if (n >= 0 && n < cli.trayIconCount)
					szTipCur = cli.trayIcon[n].ptszToolTip;
			}
			CLCINFOTIP ti = { sizeof(ti) };
			ti.rcItem.left = pt.x - 10;
			ti.rcItem.right = pt.x + 10;
			ti.rcItem.top = pt.y - 10;
			ti.rcItem.bottom = pt.y + 10;
			ti.isTreeFocused = GetFocus() == cli.hwndContactList ? 1 : 0;
			if (CallService("mToolTip/ShowTipW", (WPARAM)szTipCur, (LPARAM)&ti) == CALLSERVICE_NOTFOUND)
				CallService("mToolTip/ShowTip", (WPARAM)(char*)_T2A(szTipCur), (LPARAM)&ti);

			GetCursorPos(&tray_hover_pos);
			SetTimer(cli.hwndContactList, TIMERID_TRAYHOVER_2, 600, TrayHideToolTipTimerProc);
			g_trayTooltipActive = true;
		}
	}

	KillTimer(hwnd, id);
}

INT_PTR fnTrayIconProcessMessage(WPARAM wParam, LPARAM lParam)
{
	MSG *msg = (MSG *)wParam;
	switch (msg->message) {
	case WM_CREATE: {
		WM_TASKBARCREATED = RegisterWindowMessage(L"TaskbarCreated");
		WM_TASKBARBUTTONCREATED = RegisterWindowMessage(L"TaskbarButtonCreated");
		PostMessage(msg->hwnd, TIM_CREATE, 0, 0);
		break;
	}
	case TIM_CREATE:
		cli.pfnTrayIconInit(msg->hwnd);
		break;

	case WM_ACTIVATE:
		if (db_get_b(0, "CList", "AutoHide", SETTING_AUTOHIDE_DEFAULT)) {
			if (LOWORD(msg->wParam) == WA_INACTIVE)
				autoHideTimerId = SetTimer(nullptr, 0, 1000 * db_get_w(0, "CList", "HideTime", SETTING_HIDETIME_DEFAULT), TrayIconAutoHideTimer);
			else
				KillTimer(nullptr, autoHideTimerId);
		}
		break;

	case WM_DESTROY:
		cli.pfnTrayIconDestroy(msg->hwnd);
		cli.pfnUninitTray();
		break;

	case TIM_CALLBACK:
		if (msg->lParam == WM_RBUTTONDOWN || msg->lParam == WM_LBUTTONDOWN || msg->lParam == WM_RBUTTONDOWN && g_trayTooltipActive) {
			CallService("mToolTip/HideTip", 0, 0);
			g_trayTooltipActive = false;
		}

		if (msg->lParam == WM_MBUTTONUP)
			cli.pfnShowHide();
		else if (msg->lParam == (db_get_b(0, "CList", "Tray1Click", SETTING_TRAY1CLICK_DEFAULT) ? WM_LBUTTONUP : WM_LBUTTONDBLCLK)) {
			if ((GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
				POINT pt;
				HMENU hMenu = Menu_GetStatusMenu();

				for (int i = 0; i < cli.trayIconCount; i++) {
					if ((unsigned)cli.trayIcon[i].id == msg->wParam) {
						if (!cli.trayIcon[i].szProto)
							break;

						int ind = 0;
						for (int j = 0; j < accounts.getCount(); j++) {
							int k = cli.pfnGetAccountIndexByPos(j);
							if (k >= 0) {
								if (!mir_strcmp(cli.trayIcon[i].szProto, accounts[k]->szModuleName)) {
									HMENU hm = GetSubMenu(hMenu, ind);
									if (hm) hMenu = hm;
									break;
								}

								if (cli.pfnGetProtocolVisibility(accounts[k]->szModuleName))
									++ind;
							}
						}
						break;
					}
				}

				SetForegroundWindow(msg->hwnd);
				SetFocus(msg->hwnd);
				GetCursorPos(&pt);
				TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, msg->hwnd, nullptr);
			}
			else if (cli.pfnEventsProcessTrayDoubleClick(msg->wParam))
				cli.pfnShowHide();
		}
		else if (msg->lParam == WM_RBUTTONUP) {
			HMENU hMenu = Menu_BuildTrayMenu();
			SetForegroundWindow(msg->hwnd);
			SetFocus(msg->hwnd);

			POINT pt;
			GetCursorPos(&pt);
			cli.bTrayMenuOnScreen = TRUE;
			TrackPopupMenu(hMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0, msg->hwnd, nullptr);
			Menu_DestroyNestedMenu(hMenu);
			PostMessage(msg->hwnd, WM_NULL, 0, 0);
		}
		else if (msg->lParam == WM_MOUSEMOVE) {
			s_LastHoverIconID = msg->wParam;
			if (g_trayTooltipActive) {
				POINT pt;
				GetCursorPos(&pt);
				if (abs(pt.x - tray_hover_pos.x) > TOOLTIP_TOLERANCE || abs(pt.y - tray_hover_pos.y) > TOOLTIP_TOLERANCE) {
					CallService("mToolTip/HideTip", 0, 0);
					g_trayTooltipActive = false;
					ReleaseCapture();
				}
			}
			else {
				GetCursorPos(&tray_hover_pos);
				SetTimer(cli.hwndContactList, TIMERID_TRAYHOVER, 600, TrayToolTipTimerProc);
			}
			break;
		}

		*((LRESULT*)lParam) = 0;
		return TRUE;

	default:
		if (msg->message == WM_TASKBARCREATED) {
			cli.pfnTrayIconTaskbarCreated(msg->hwnd);
			*((LRESULT*)lParam) = 0;
			return TRUE;
		}
		else if (msg->message == WM_TASKBARBUTTONCREATED) {
			SetTaskBarIcon(lastTaskBarIcon, nullptr);
			*((LRESULT*)lParam) = 0;
			return TRUE;
		}
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// processes tray icon's notifications

static int sttGetIcon(const char *szProto)
{
	if (szProto == nullptr)
		return cli.trayIcon[0].id;

	UINT iconId = 0;

	for (int j = 0; j < cli.trayIconCount; j++) {
		if (cli.trayIcon[j].szProto != nullptr) {
			if (!mir_strcmp(szProto, cli.trayIcon[j].szProto)) {
				iconId = cli.trayIcon[j].id;
				break;
			}
		}
		else if (cli.trayIcon[j].isBase) {
			iconId = cli.trayIcon[j].id;
			break;
		}
	}

	return iconId;
}

MIR_APP_DLL(int) Clist_TrayNotifyA(const char *szProto, const char *szInfoTitle, const char *szInfo, DWORD dwInfoFlags, UINT uTimeout)
{
	if (szInfo == nullptr || szInfoTitle == nullptr)
		return 1;
	if (cli.trayIcon == nullptr)
		return 2;

	NOTIFYICONDATAA nid = { 0 };
	nid.cbSize = NOTIFYICONDATAA_V2_SIZE;
	nid.hWnd = cli.hwndContactList;
	nid.uID = sttGetIcon(szProto);
	nid.uFlags = NIF_INFO;
	strncpy_s(nid.szInfo, szInfo, _TRUNCATE);
	strncpy_s(nid.szInfoTitle, szInfoTitle, _TRUNCATE);
	nid.uTimeout = uTimeout;
	nid.dwInfoFlags = dwInfoFlags;
	return Shell_NotifyIconA(NIM_MODIFY, &nid) == 0;
}

MIR_APP_DLL(int) Clist_TrayNotifyW(const char *szProto, const wchar_t *wszInfoTitle, const wchar_t *wszInfo, DWORD dwInfoFlags, UINT uTimeout)
{
	if (wszInfo == nullptr || wszInfoTitle == nullptr)
		return 1;
	if (cli.trayIcon == nullptr)
		return 2;

	NOTIFYICONDATAW nid = { 0 };
	nid.cbSize = NOTIFYICONDATAW_V2_SIZE;
	nid.hWnd = cli.hwndContactList;
	nid.uID = sttGetIcon(szProto);
	nid.uFlags = NIF_INFO;
	mir_wstrncpy(nid.szInfo, wszInfo, _countof(nid.szInfo));
	mir_wstrncpy(nid.szInfoTitle, wszInfoTitle, _countof(nid.szInfoTitle));
	nid.szInfo[_countof(nid.szInfo) - 1] = 0;
	nid.szInfoTitle[_countof(nid.szInfoTitle) - 1] = 0;
	nid.uTimeout = uTimeout;
	nid.dwInfoFlags = dwInfoFlags;
	return Shell_NotifyIconW(NIM_MODIFY, &nid) == 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static DLLVERSIONINFO dviShell = { sizeof(dviShell) };

void fnInitTray(void)
{
	if (HMODULE hLib = GetModuleHandleA("shell32")) {
		if (DLLGETVERSIONPROC proc = (DLLGETVERSIONPROC)GetProcAddress(hLib, "DllGetVersion")) {
			if (proc(&dviShell) == S_OK)
				cli.shellVersion = dviShell.dwMajorVersion;
		}
	}

	fTrayInited = TRUE;
}

void fnUninitTray(void)
{
	fTrayInited = false;
}

#undef initcheck
