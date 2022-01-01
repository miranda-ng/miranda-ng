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
#include "clc.h"

#define TOOLTIP_TOLERANCE 5

static ITaskbarList3* pTaskbarInterface;

static UINT WM_TASKBARCREATED;
static UINT WM_TASKBARBUTTONCREATED;
static UINT_PTR RefreshTimerId = 0;
static UINT_PTR CycleTimerId;

mir_cs trayLockCS;

static bool hasTips()
{
	return ServiceExists(MS_TIPPER_SHOWTIPW) && db_get_b(0, "Tipper", "TrayTip", 1);
}

#define initcheck if (!fTrayInited) return

static bool fTrayInited;

static wchar_t* sttGetXStatus(const char *szProto)
{
	if (Proto_GetStatus(szProto) > ID_STATUS_OFFLINE) {
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
		pTaskbarInterface->SetOverlayIcon(g_clistApi.hwndContactList, hIcon, szNewTip);
		lastTaskBarIcon = hIcon;
	}
}

MIR_APP_DLL(wchar_t*) Clist_TrayIconMakeTooltip(const wchar_t *szPrefix, const char *szProto)
{
	initcheck nullptr;

	mir_cslock lck(trayLockCS);
	wchar_t *szSeparator = L"\n";

	if (szProto == nullptr) {
		if (g_arAccounts.getCount() == 0)
			return nullptr;

		if (g_arAccounts.getCount() == 1)
			return Clist_TrayIconMakeTooltip(szPrefix, g_arAccounts[0]->szModuleName);

		CMStringW tszTip;

		if (szPrefix && szPrefix[0]) {
			if (!Clist::TrayAlwaysStatus) {
				wcsncpy_s(g_clistApi.szTip, MAX_TIP_SIZE, szPrefix, _TRUNCATE);
				return g_clistApi.szTip;
			}
			tszTip.Append(szPrefix);
		}

		for (int t = 0; t < g_arAccounts.getCount(); t++) {
			int i = Clist_GetAccountIndex(t);
			if (i == -1)
				continue;

			PROTOACCOUNT *pa = g_arAccounts[i];
			if (!pa->IsVisible())
				continue;

			wchar_t *szStatus = Clist_GetStatusModeDescription(pa->iRealStatus, 0);
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
					tszTip.AppendFormat(L"%-24.24s\n", ProtoXStatus.get());
				}
			}
			else tszTip.AppendFormat(L"%s %s", pa->tszAccountName, szStatus);
		}

		wcsncpy_s(g_clistApi.szTip, MAX_TIP_SIZE, tszTip, _TRUNCATE);
	}
	else {
		PROTOACCOUNT *pa = Proto_GetAccount(szProto);
		if (pa != nullptr) {
			ptrW ProtoXStatus(sttGetXStatus(szProto));
			wchar_t *szStatus = Clist_GetStatusModeDescription(pa->iRealStatus, 0);
			if (szPrefix && szPrefix[0]) {
				if (Clist::TrayAlwaysStatus) {
					if (hasTips()) {
						if (ProtoXStatus != nullptr)
							mir_snwprintf(g_clistApi.szTip, MAX_TIP_SIZE, L"%s%s<b>%-12.12s</b>\t%s%s%-24.24s", szPrefix, szSeparator, pa->tszAccountName, szStatus, szSeparator, ProtoXStatus.get());
						else
							mir_snwprintf(g_clistApi.szTip, MAX_TIP_SIZE, L"%s%s<b>%-12.12s</b>\t%s", szPrefix, szSeparator, pa->tszAccountName, szStatus);
					}
					else mir_snwprintf(g_clistApi.szTip, MAX_TIP_SIZE, L"%s%s%s %s", szPrefix, szSeparator, pa->tszAccountName, szStatus);
				}
				else mir_wstrncpy(g_clistApi.szTip, szPrefix, MAX_TIP_SIZE);
			}
			else {
				if (hasTips()) {
					if (ProtoXStatus != nullptr)
						mir_snwprintf(g_clistApi.szTip, MAX_TIP_SIZE, L"<b>%-12.12s</b>\t%s\n%-24.24s", pa->tszAccountName, szStatus, ProtoXStatus.get());
					else
						mir_snwprintf(g_clistApi.szTip, MAX_TIP_SIZE, L"<b>%-12.12s</b>\t%s", pa->tszAccountName, szStatus);
				}
				else mir_snwprintf(g_clistApi.szTip, MAX_TIP_SIZE, L"%s %s", pa->tszAccountName, szStatus);
			}
		}
	}

	return g_clistApi.szTip;
}

MIR_APP_DLL(int) Clist_TrayIconAdd(HWND hwnd, const char *szProto, const char *szIconProto, int status)
{
	initcheck 0;

	mir_cslock lck(trayLockCS);
	int i;
	for (i = 0; i < g_clistApi.trayIconCount; i++)
		if (g_clistApi.trayIcon[i].id == 0)
			break;

	trayIconInfo_t &p = g_clistApi.trayIcon[i];
	p.id = TRAYICON_ID_BASE + i;
	p.szProto = (char*)szProto;
	p.hBaseIcon = g_clistApi.pfnGetIconFromStatusMode(0, szIconProto ? szIconProto : p.szProto, status);

	NOTIFYICONDATA nid = { NOTIFYICONDATA_V2_SIZE };
	nid.hWnd = hwnd;
	nid.uID = p.id;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	nid.uCallbackMessage = TIM_CALLBACK;
	nid.hIcon = p.hBaseIcon;

	Clist_TrayIconMakeTooltip(nullptr, p.szProto);
	if (!hasTips())
		mir_wstrncpy(nid.szTip, g_clistApi.szTip, _countof(nid.szTip));
	replaceStrW(p.ptszToolTip, g_clistApi.szTip);

	Shell_NotifyIcon(NIM_ADD, &nid);
	p.isBase = 1;

	if (g_clistApi.trayIconCount == 1)
		SetTaskBarIcon(g_clistApi.trayIcon[0].hBaseIcon, g_clistApi.szTip);
	return i;
}

MIR_APP_DLL(void) Clist_TrayIconRemove(HWND hwnd, const char *szProto)
{
	initcheck;

	mir_cslock lck(trayLockCS);
	for (int i = 0; i < g_clistApi.trayIconCount; i++) {
		trayIconInfo_t *pii = &g_clistApi.trayIcon[i];
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

	if (g_clistApi.trayIconCount == 1)
		SetTaskBarIcon(nullptr, nullptr);
}

int fnTrayIconInit(HWND hwnd)
{
	initcheck 0;

	mir_cslock lck(trayLockCS);

	int netProtoCount = 0;
	int averageMode = Proto_GetAverageStatus(&netProtoCount);

	if (CycleTimerId) {
		KillTimer(nullptr, CycleTimerId);
		CycleTimerId = 0;
	}

	g_clistApi.trayIconCount = 1;

	if (netProtoCount) {
		g_clistApi.trayIcon = (trayIconInfo_t*)mir_calloc(sizeof(trayIconInfo_t) * g_arAccounts.getCount());

		int trayIconSetting = db_get_b(0, "CList", "TrayIcon", SETTING_TRAYICON_DEFAULT);
		if (trayIconSetting == SETTING_TRAYICON_SINGLE) {
			DBVARIANT dbv = { DBVT_DELETED };
			char *szProto;
			if (!db_get_s(0, "CList", "PrimaryStatus", &dbv) && (averageMode < 0 || db_get_b(0, "CList", "AlwaysPrimary", 0)))
				szProto = dbv.pszVal;
			else
				szProto = nullptr;

			Clist_TrayIconAdd(hwnd, nullptr, szProto, szProto ? Proto_GetStatus(szProto) : CallService(MS_CLIST_GETSTATUSMODE, 0, 0));
			db_free(&dbv);
		}
		else if (trayIconSetting == SETTING_TRAYICON_MULTI && (averageMode < 0 || db_get_b(0, "CList", "AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT))) {
			g_clistApi.trayIconCount = netProtoCount;
			for (int i = 0; i < g_arAccounts.getCount(); i++) {
				int j = Clist_GetAccountIndex(i);
				if (j >= 0) {
					PROTOACCOUNT *pa = g_arAccounts[j];
					if (pa->IsVisible())
						Clist_TrayIconAdd(hwnd, pa->szModuleName, nullptr, pa->iRealStatus);
				}
			}
		}
		else {
			Clist_TrayIconAdd(hwnd, nullptr, nullptr, averageMode);

			if (trayIconSetting == SETTING_TRAYICON_CYCLE && averageMode < 0)
				Clist_TraySetTimer();
		}
	}
	else {
		g_clistApi.trayIcon = (trayIconInfo_t*)mir_calloc(sizeof(trayIconInfo_t));
		Clist_TrayIconAdd(hwnd, nullptr, nullptr, CallService(MS_CLIST_GETSTATUSMODE, 0, 0));
	}

	return 0;
}

MIR_APP_DLL(int) Clist_TrayIconDestroy(HWND hwnd)
{
	initcheck 0;

	mir_cslock lck(trayLockCS);
	if (g_clistApi.trayIconCount == 1)
		SetTaskBarIcon(nullptr, nullptr);

	NOTIFYICONDATA nid = { NOTIFYICONDATA_V2_SIZE };
	nid.hWnd = hwnd;
	for (int i = 0; i < g_clistApi.trayIconCount; i++) {
		if (g_clistApi.trayIcon[i].id == 0)
			continue;
		nid.uID = g_clistApi.trayIcon[i].id;
		Shell_NotifyIcon(NIM_DELETE, &nid);
		DestroyIcon(g_clistApi.trayIcon[i].hBaseIcon);
		mir_free(g_clistApi.trayIcon[i].ptszToolTip);
	}
	mir_free(g_clistApi.trayIcon);
	g_clistApi.trayIcon = nullptr;
	g_clistApi.trayIconCount = 0;
	return 0;
}

// called when Explorer crashes and the taskbar is remade
void TrayIconTaskbarCreated(HWND hwnd)
{
	initcheck;
	Clist_TrayIconDestroy(hwnd);
	g_clistApi.pfnTrayIconInit(hwnd);
}

static VOID CALLBACK RefreshTimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	if (RefreshTimerId) {
		KillTimer(nullptr, RefreshTimerId);
		RefreshTimerId = 0;
	}

	for (auto &it : g_arAccounts)
		Clist_TrayIconUpdateBase(it->szModuleName);
}

int TrayIconUpdate(HICON hNewIcon, const wchar_t *szNewTip, const char *szPreferredProto, int isBase)
{
	initcheck - 1;
	mir_cslock lck(trayLockCS);

	NOTIFYICONDATA nid = { NOTIFYICONDATA_V2_SIZE };
	nid.hWnd = g_clistApi.hwndContactList;
	nid.uFlags = NIF_ICON | NIF_TIP;
	nid.hIcon = hNewIcon;
	if (!hNewIcon)
		return -1;

	for (int i = 0; i < g_clistApi.trayIconCount; i++) {
		if (g_clistApi.trayIcon[i].id == 0)
			continue;
		if (mir_strcmp(g_clistApi.trayIcon[i].szProto, szPreferredProto))
			continue;

		nid.uID = g_clistApi.trayIcon[i].id;
		Clist_TrayIconMakeTooltip(szNewTip, g_clistApi.trayIcon[i].szProto);
		replaceStrW(g_clistApi.trayIcon[i].ptszToolTip, g_clistApi.szTip);
		if (!hasTips())
			mir_wstrncpy(nid.szTip, g_clistApi.szTip, _countof(nid.szTip));
		Shell_NotifyIcon(NIM_MODIFY, &nid);

		if (g_clistApi.trayIconCount == 1)
			SetTaskBarIcon(hNewIcon, g_clistApi.szTip);
		else
			SetTaskBarIcon(nullptr, nullptr);

		g_clistApi.trayIcon[i].isBase = isBase;
		return i;
	}

	// if there wasn't a suitable icon, change all the icons
	for (int i = 0; i < g_clistApi.trayIconCount; i++) {
		if (g_clistApi.trayIcon[i].id == 0)
			continue;
		nid.uID = g_clistApi.trayIcon[i].id;

		Clist_TrayIconMakeTooltip(szNewTip, g_clistApi.trayIcon[i].szProto);
		replaceStrW(g_clistApi.trayIcon[i].ptszToolTip, g_clistApi.szTip);
		if (!hasTips())
			mir_wstrncpy(nid.szTip, g_clistApi.szTip, _countof(nid.szTip));
		Shell_NotifyIcon(NIM_MODIFY, &nid);

		if (g_clistApi.trayIconCount == 1)
			SetTaskBarIcon(hNewIcon, g_clistApi.szTip);
		else
			SetTaskBarIcon(nullptr, nullptr);

		g_clistApi.trayIcon[i].isBase = isBase;
		if (db_get_b(0, "CList", "TrayIcon", SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_MULTI) {
			uint32_t time1 = db_get_w(0, "CList", "CycleTime", SETTING_CYCLETIME_DEFAULT) * 200;
			uint32_t time2 = db_get_w(0, "CList", "IconFlashTime", 550) + 1000;
			uint32_t time = max(max(uint32_t(2000), time1), time2);
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

MIR_APP_DLL(int) Clist_TrayIconSetBaseInfo(HICON hIcon, const char *szPreferredProto)
{
	if (!fTrayInited) {
	LBL_Error:
		DestroyIcon(hIcon);
		return -1;
	}

	mir_cslock lck(trayLockCS);

	if (szPreferredProto) {
		for (int i = 0; i < g_clistApi.trayIconCount; i++) {
			if (g_clistApi.trayIcon[i].id == 0)
				continue;
			if (mir_strcmp(g_clistApi.trayIcon[i].szProto, szPreferredProto))
				continue;

			DestroyIcon(g_clistApi.trayIcon[i].hBaseIcon);
			g_clistApi.trayIcon[i].hBaseIcon = hIcon;
			return i;
		}
		if ((Clist_GetProtocolVisibility(szPreferredProto)) &&
			(Proto_GetAverageStatus(nullptr) == -1) &&
			(db_get_b(0, "CList", "TrayIcon", SETTING_TRAYICON_DEFAULT) == SETTING_TRAYICON_MULTI) &&
			!(db_get_b(0, "CList", "AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT)))
			goto LBL_Error;
	}

	// if there wasn't a specific icon, there will only be one suitable
	for (int i = 0; i < g_clistApi.trayIconCount; i++) {
		if (g_clistApi.trayIcon[i].id == 0)
			continue;

		DestroyIcon(g_clistApi.trayIcon[i].hBaseIcon);
		g_clistApi.trayIcon[i].hBaseIcon = hIcon;
		return i;
	}

	goto LBL_Error;
}

void TrayIconUpdateWithImageList(int iImage, const wchar_t *szNewTip, const char *szPreferredProto)
{
	HICON hIcon = ImageList_GetIcon(hCListImages, iImage, ILD_NORMAL);
	TrayIconUpdate(hIcon, szNewTip, szPreferredProto, 0);
	DestroyIcon(hIcon);
}

/////////////////////////////////////////////////////////////////////////////////////////

static VOID CALLBACK TrayCycleTimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	initcheck;
	mir_cslock lck(trayLockCS);

	int i;
	for (i = g_arAccounts.getCount() + 1; --i;) {
		g_clistApi.cycleStep = (g_clistApi.cycleStep + 1) % g_arAccounts.getCount();
		if (g_arAccounts[g_clistApi.cycleStep]->IsVisible())
			break;
	}

	if (i) {
		DestroyIcon(g_clistApi.trayIcon[0].hBaseIcon);
		g_clistApi.trayIcon[0].hBaseIcon = g_clistApi.pfnGetIconFromStatusMode(0, g_arAccounts[g_clistApi.cycleStep]->szModuleName, g_arAccounts[g_clistApi.cycleStep]->iRealStatus);
		if (g_clistApi.trayIcon[0].isBase)
			TrayIconUpdate(g_clistApi.trayIcon[0].hBaseIcon, nullptr, nullptr, 1);
	}
}

MIR_APP_DLL(void) Clist_TraySetTimer()
{
	CycleTimerId = SetTimer(nullptr, CycleTimerId, db_get_w(0, "CList", "CycleTime", SETTING_CYCLETIME_DEFAULT) * 1000, TrayCycleTimerProc);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(void) Clist_TrayIconUpdateBase(const char *szChangedProto)
{
	initcheck;
	if (szChangedProto == nullptr) return;
	if (!Clist_GetProtocolVisibility(szChangedProto)) return;

	int netProtoCount;
	mir_cslock lck(trayLockCS);
	int averageMode = Proto_GetAverageStatus(&netProtoCount);

	if (CycleTimerId) {
		KillTimer(nullptr, CycleTimerId);
		CycleTimerId = 0;
	}

	for (auto &it : g_arAccounts)
		if (!mir_strcmp(szChangedProto, it->szModuleName))
			g_clistApi.cycleStep = g_arAccounts.indexOf(&it);

	int changed = g_clistApi.pfnTrayCalcChanged(szChangedProto, averageMode, netProtoCount);
	if (changed != -1 && g_clistApi.trayIcon[changed].isBase)
		TrayIconUpdate(g_clistApi.trayIcon[changed].hBaseIcon, nullptr, g_clistApi.trayIcon[changed].szProto, 1);
}

int fnTrayCalcChanged(const char *szChangedProto, int averageMode, int netProtoCount)
{
	if (netProtoCount == 0)
		return Clist_TrayIconSetBaseInfo(ImageList_GetIcon(hCListImages, g_clistApi.pfnIconFromStatusMode(nullptr, averageMode, 0), ILD_NORMAL), nullptr);

	int trayIconSetting = db_get_b(0, "CList", "TrayIcon", SETTING_TRAYICON_DEFAULT);

	if (averageMode > 0) {
		if (trayIconSetting != SETTING_TRAYICON_MULTI)
			return Clist_TrayIconSetBaseInfo(g_clistApi.pfnGetIconFromStatusMode(0, nullptr, averageMode), nullptr);

		if (db_get_b(0, "CList", "AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT))
			return Clist_TrayIconSetBaseInfo(g_clistApi.pfnGetIconFromStatusMode(0, szChangedProto, Proto_GetStatus(szChangedProto)), (char*)szChangedProto);

		if (g_clistApi.trayIcon == nullptr || g_clistApi.trayIcon[0].szProto == nullptr)
			return Clist_TrayIconSetBaseInfo(g_clistApi.pfnGetIconFromStatusMode(0, nullptr, averageMode), nullptr);

		Clist_TrayIconDestroy(g_clistApi.hwndContactList);
		g_clistApi.pfnTrayIconInit(g_clistApi.hwndContactList);
	}
	else {
		switch (trayIconSetting) {
		case SETTING_TRAYICON_CYCLE:
			Clist_TraySetTimer();
			return Clist_TrayIconSetBaseInfo(ImageList_GetIcon(hCListImages, g_clistApi.pfnIconFromStatusMode(szChangedProto, Proto_GetStatus(szChangedProto), 0), ILD_NORMAL), nullptr);

		case SETTING_TRAYICON_MULTI:
			if (!g_clistApi.trayIcon)
				Clist_TrayIconRemove(nullptr, nullptr);
			else if ((g_clistApi.trayIconCount > 1 || netProtoCount == 1) || db_get_b(0, "CList", "AlwaysMulti", SETTING_ALWAYSMULTI_DEFAULT))
				return Clist_TrayIconSetBaseInfo(g_clistApi.pfnGetIconFromStatusMode(0, szChangedProto, Proto_GetStatus(szChangedProto)), (char*)szChangedProto);
			else {
				Clist_TrayIconDestroy(g_clistApi.hwndContactList);
				g_clistApi.pfnTrayIconInit(g_clistApi.hwndContactList);
			}
			break;

		case SETTING_TRAYICON_SINGLE:
			ptrA szProto(db_get_sa(0, "CList", "PrimaryStatus"));
			return Clist_TrayIconSetBaseInfo(g_clistApi.pfnGetIconFromStatusMode(0, szProto, szProto ? Proto_GetStatus(szProto) : CallService(MS_CLIST_GETSTATUSMODE, 0, 0)), szProto);
		}
	}

	return -1;
}

void TrayIconSetToBase(char *szPreferredProto)
{
	int i;
	initcheck;
	mir_cslock lck(trayLockCS);

	for (i = 0; i < g_clistApi.trayIconCount; i++) {
		if (g_clistApi.trayIcon[i].id == 0)
			continue;
		if (mir_strcmp(g_clistApi.trayIcon[i].szProto, szPreferredProto))
			continue;
		TrayIconUpdate(g_clistApi.trayIcon[i].hBaseIcon, nullptr, szPreferredProto, 1);
		return;
	}

	// if there wasn't a specific icon, there will only be one suitable
	for (i = 0; i < g_clistApi.trayIconCount; i++) {
		if (g_clistApi.trayIcon[i].id == 0)
			continue;
		TrayIconUpdate(g_clistApi.trayIcon[i].hBaseIcon, nullptr, szPreferredProto, 1);
		return;
	}
}

MIR_APP_DLL(void) Clist_TrayIconIconsChanged()
{
	initcheck;
	mir_cslock lck(trayLockCS);
	if (g_clistApi.hwndContactList != nullptr) {
		Clist_TrayIconDestroy(g_clistApi.hwndContactList);
		g_clistApi.pfnTrayIconInit(g_clistApi.hwndContactList);
	}
}

static UINT_PTR autoHideTimerId;
static VOID CALLBACK TrayIconAutoHideTimer(HWND hwnd, UINT, UINT_PTR idEvent, DWORD)
{
	initcheck;
	mir_cslock lck(trayLockCS);

	KillTimer(hwnd, idEvent);
	HWND hwndClui = g_clistApi.hwndContactList;
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
		if (GetActiveWindow() != g_clistApi.hwndContactList) {
			KillTimer(nullptr, autoHideTimerId);
			autoHideTimerId = SetTimer(nullptr, 0, 1000 * db_get_w(0, "CList", "HideTime", SETTING_HIDETIME_DEFAULT), TrayIconAutoHideTimer);
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// processes tray icon's messages

static uint8_t s_LastHoverIconID = 0;
static bool g_trayTooltipActive = false;
static POINT tray_hover_pos = { 0 };

static void CALLBACK TrayHideToolTipTimerProc(HWND hwnd, UINT, UINT_PTR, DWORD)
{
	if (g_trayTooltipActive) {
		POINT pt;
		GetCursorPos(&pt);
		if (abs(pt.x - tray_hover_pos.x) > TOOLTIP_TOLERANCE || abs(pt.y - tray_hover_pos.y) > TOOLTIP_TOLERANCE) {
			Tipper_Hide();
			g_trayTooltipActive = false;
			KillTimer(hwnd, TIMERID_TRAYHOVER_2);
		}
	}
	else KillTimer(hwnd, TIMERID_TRAYHOVER_2);
}

static void CALLBACK TrayToolTipTimerProc(HWND hwnd, UINT, UINT_PTR id, DWORD)
{
	if (!g_trayTooltipActive && !g_clistApi.bTrayMenuOnScreen) {
		POINT pt;
		GetCursorPos(&pt);
		if (abs(pt.x - tray_hover_pos.x) <= TOOLTIP_TOLERANCE && abs(pt.y - tray_hover_pos.y) <= TOOLTIP_TOLERANCE) {
			wchar_t* szTipCur = g_clistApi.szTip;
			{
				int n = s_LastHoverIconID - 100;
				if (n >= 0 && n < g_clistApi.trayIconCount)
					szTipCur = g_clistApi.trayIcon[n].ptszToolTip;
			}
			CLCINFOTIP ti = { sizeof(ti) };
			ti.rcItem.left = pt.x - 10;
			ti.rcItem.right = pt.x + 10;
			ti.rcItem.top = pt.y - 10;
			ti.rcItem.bottom = pt.y + 10;
			ti.isTreeFocused = GetFocus() == g_clistApi.hwndContactList ? 1 : 0;
			Tipper_ShowTip(szTipCur, &ti);

			GetCursorPos(&tray_hover_pos);
			SetTimer(g_clistApi.hwndContactList, TIMERID_TRAYHOVER_2, 600, TrayHideToolTipTimerProc);
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
		g_clistApi.pfnTrayIconInit(msg->hwnd);
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
		Clist_TrayIconDestroy(msg->hwnd);
		fTrayInited = false;
		break;

	case TIM_CALLBACK:
		if (msg->lParam == WM_RBUTTONDOWN || msg->lParam == WM_LBUTTONDOWN || msg->lParam == WM_RBUTTONDOWN && g_trayTooltipActive) {
			Tipper_Hide();
			g_trayTooltipActive = false;
		}

		if (msg->lParam == WM_MBUTTONUP)
			g_clistApi.pfnShowHide();
		else if (msg->lParam == (Clist::Tray1Click ? WM_LBUTTONUP : WM_LBUTTONDBLCLK)) {
			if ((GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
				POINT pt;
				HMENU hMenu = Menu_GetStatusMenu();

				for (int i = 0; i < g_clistApi.trayIconCount; i++) {
					if ((unsigned)g_clistApi.trayIcon[i].id == msg->wParam) {
						if (!g_clistApi.trayIcon[i].szProto)
							break;

						int ind = 0;
						for (int j = 0; j < g_arAccounts.getCount(); j++) {
							int k = Clist_GetAccountIndex(j);
							if (k >= 0) {
								if (!mir_strcmp(g_clistApi.trayIcon[i].szProto, g_arAccounts[k]->szModuleName)) {
									HMENU hm = GetSubMenu(hMenu, ind);
									if (hm) hMenu = hm;
									break;
								}

								if (g_arAccounts[k]->IsVisible())
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
			else if (Clist_EventsProcessTrayDoubleClick(msg->wParam))
				g_clistApi.pfnShowHide();
		}
		else if (msg->lParam == WM_RBUTTONUP) {
			HMENU hMenu = Menu_BuildTrayMenu();
			SetForegroundWindow(msg->hwnd);
			SetFocus(msg->hwnd);

			POINT pt;
			GetCursorPos(&pt);
			g_clistApi.bTrayMenuOnScreen = TRUE;
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
					Tipper_Hide();
					g_trayTooltipActive = false;
					ReleaseCapture();
				}
			}
			else {
				GetCursorPos(&tray_hover_pos);
				SetTimer(g_clistApi.hwndContactList, TIMERID_TRAYHOVER, 600, TrayToolTipTimerProc);
			}
			break;
		}

		*((LRESULT*)lParam) = 0;
		return TRUE;

	default:
		if (msg->message == WM_TASKBARCREATED) {
			TrayIconTaskbarCreated(msg->hwnd);
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
		return g_clistApi.trayIcon[0].id;

	UINT iconId = 0;

	for (int j = 0; j < g_clistApi.trayIconCount; j++) {
		if (g_clistApi.trayIcon[j].szProto != nullptr) {
			if (!mir_strcmp(szProto, g_clistApi.trayIcon[j].szProto)) {
				iconId = g_clistApi.trayIcon[j].id;
				break;
			}
		}
		else if (g_clistApi.trayIcon[j].isBase) {
			iconId = g_clistApi.trayIcon[j].id;
			break;
		}
	}

	return iconId;
}

MIR_APP_DLL(int) Clist_TrayNotifyA(const char *szProto, const char *szInfoTitle, const char *szInfo, uint32_t dwInfoFlags, UINT uTimeout)
{
	if (szInfo == nullptr || szInfoTitle == nullptr)
		return 1;
	if (g_clistApi.trayIcon == nullptr)
		return 2;

	NOTIFYICONDATAA nid = { 0 };
	nid.cbSize = NOTIFYICONDATAA_V2_SIZE;
	nid.hWnd = g_clistApi.hwndContactList;
	nid.uID = sttGetIcon(szProto);
	nid.uFlags = NIF_INFO;
	strncpy_s(nid.szInfo, szInfo, _TRUNCATE);
	strncpy_s(nid.szInfoTitle, szInfoTitle, _TRUNCATE);
	nid.uTimeout = uTimeout;
	nid.dwInfoFlags = dwInfoFlags;
	return Shell_NotifyIconA(NIM_MODIFY, &nid) == 0;
}

MIR_APP_DLL(int) Clist_TrayNotifyW(const char *szProto, const wchar_t *wszInfoTitle, const wchar_t *wszInfo, uint32_t dwInfoFlags, UINT uTimeout)
{
	if (wszInfo == nullptr || wszInfoTitle == nullptr)
		return 1;
	if (g_clistApi.trayIcon == nullptr)
		return 2;

	NOTIFYICONDATAW nid = { 0 };
	nid.cbSize = NOTIFYICONDATAW_V2_SIZE;
	nid.hWnd = g_clistApi.hwndContactList;
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

void InitTray(void)
{
	fTrayInited = true;

	if (IsWinVer7Plus())
		CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_ALL, IID_ITaskbarList3, (void**)&pTaskbarInterface);
}

void UninitTray(void)
{
	if (pTaskbarInterface)
		pTaskbarInterface->Release();
}
