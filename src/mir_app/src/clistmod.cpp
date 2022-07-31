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
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

#include "clc.h"
#include "genmenu.h"

int ContactSettingChanged(WPARAM wParam, LPARAM lParam);
int InitClistHotKeys(void);
void InitMoveToGroup(void);
void ScheduleMenuUpdate(void);

HANDLE hContactDoubleClicked, hContactIconChangedEvent;
HIMAGELIST hCListImages;

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(wchar_t*) Clist_GetStatusModeDescription(int mode, int flags)
{
	static wchar_t szMode[64];
	wchar_t* descr;
	int    noPrefixReqd = 0;
	switch (mode) {
	case ID_STATUS_OFFLINE:
		descr = LPGENW("Offline");
		noPrefixReqd = 1;
		break;
	case ID_STATUS_CONNECTING:
		descr = LPGENW("Connecting");
		noPrefixReqd = 1;
		break;
	case ID_STATUS_ONLINE:
		descr = LPGENW("Online");
		noPrefixReqd = 1;
		break;
	case ID_STATUS_AWAY:
		descr = LPGENW("Away");
		break;
	case ID_STATUS_DND:
		descr = LPGENW("Do not disturb");
		break;
	case ID_STATUS_NA:
		descr = LPGENW("Not available");
		break;
	case ID_STATUS_OCCUPIED:
		descr = LPGENW("Occupied");
		break;
	case ID_STATUS_FREECHAT:
		descr = LPGENW("Free for chat");
		break;
	case ID_STATUS_INVISIBLE:
		descr = LPGENW("Invisible");
		break;
	case ID_STATUS_IDLE:
		descr = LPGENW("Idle");
		break;
	default:
		if (IsStatusConnecting(mode)) {
			const wchar_t* connFmt = LPGENW("Connecting (attempt %d)");
			mir_snwprintf(szMode, (flags & GSMDF_UNTRANSLATED) ? connFmt : TranslateW(connFmt), mode - ID_STATUS_CONNECTING + 1);
			return szMode;
		}
		return nullptr;
	}

	return (flags & GSMDF_UNTRANSLATED) ? descr : TranslateW(descr);
}

MIR_APP_DLL(void) Clist_ContactDoubleClicked(MCONTACT hContact)
{
	// Try to process event myself
	if (EventsProcessContactDoubleClick(hContact) == 0)
		return;

	// Allow third-party plugins to process a dblclick
	if (NotifyEventHooks(hContactDoubleClicked, hContact, 0))
		return;

	// Otherwise try to execute the default action
	TIntMenuObject *pmo = GetMenuObjbyId(hContactMenuObject);
	if (pmo != nullptr) {
		NotifyEventHooks(hPreBuildContactMenuEvent, hContact, 0);

		TMO_IntMenuItem *pimi = Menu_GetDefaultItem(pmo->m_items.first);
		if (pimi != nullptr)
			Menu_ProcessCommand(pimi, hContact);
	}
}

MIR_APP_DLL(HIMAGELIST) Clist_GetImageList(void)
{
	return hCListImages;
}

MIR_APP_DLL(int) Clist_GetContactIcon(MCONTACT hContact)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
	return g_clistApi.pfnIconFromStatusMode(szProto,
		szProto == nullptr ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), hContact);
}

/////////////////////////////////////////////////////////////////////////////////////////

HICON fnGetIconFromStatusMode(MCONTACT hContact, const char *szProto, int status)
{
	return ImageList_GetIcon(hCListImages, g_clistApi.pfnIconFromStatusMode(szProto, status, hContact), ILD_NORMAL);
}

int fnIconFromStatusMode(const char *szProto, int status, MCONTACT)
{
	int index;
	for (index = 0; index < _countof(g_statuses); index++)
		if (status == g_statuses[index].iStatus)
			break;

	if (index == _countof(g_statuses))
		index = 0;
	if (szProto == nullptr)
		return index + 1;

	PROTOACCOUNT *pa = Proto_GetAccount(szProto);
	if (pa && pa->iIconBase != -1)
		return pa->iIconBase + index;

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void AddProtoIconIndex(PROTOACCOUNT *pa)
{
	for (auto &it : g_statuses) {
		int iImg = ImageList_AddProtoIcon(hCListImages, pa->szModuleName, it.iStatus);
		if (it.iStatus == ID_STATUS_OFFLINE)
			pa->iIconBase = iImg;
	}
}

static int ContactListModulesLoaded(WPARAM, LPARAM)
{
	ScheduleMenuUpdate();

	RebuildMenuOrder();
	for (auto &it : g_arAccounts)
		AddProtoIconIndex(it);

	Clist_LoadContactTree();

	LoadCLUIModule();

	InitClistHotKeys();
	InitStaticAccounts();
	InitMoveToGroup();

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0xb1f74008, 0x1fa6, 0x4e98, 0x95, 0x28, 0x5a, 0x7e, 0xab, 0xfe, 0x10, 0x61);
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_ACCMGR);
	mi.position = 1900000000;
	mi.name.a = LPGEN("&Accounts...");
	mi.pszService = MS_PROTO_SHOWACCMGR;
	Menu_AddMainMenuItem(&mi);

	SET_UID(mi, 0xc1284523, 0x548d, 0x4744, 0xb0, 0x9, 0xfb, 0xa0, 0x4, 0x8e, 0xa8, 0x67);
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_OPTIONS);
	mi.name.a = LPGEN("&Options...");
	mi.pszService = MS_OPTIONS_OPEN;
	Menu_AddMainMenuItem(&mi);

	if (g_clistApi.hMenuMain) {
		MENUITEMINFO mii = { 0 };
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_SUBMENU;

		mii.hSubMenu = Menu_GetMainMenu();
		SetMenuItemInfo(g_clistApi.hMenuMain, 0, TRUE, &mii);

		mii.hSubMenu = Menu_GetStatusMenu();
		SetMenuItemInfo(g_clistApi.hMenuMain, 1, TRUE, &mii);
	}
	return 0;
}

static int ContactListAccountsChanged(WPARAM eventCode, LPARAM lParam)
{
	PROTOACCOUNT *pa = (PROTOACCOUNT*)lParam;

	switch (eventCode) {
	case PRAC_ADDED:
		if (pa->iIconBase == -1)
			AddProtoIconIndex(pa);
		break;

	case PRAC_REMOVED:
		InvalidateProtoInCache(pa->szModuleName);
		break;
	}

	Menu_ReloadProtoMenus();
	Clist_TrayIconIconsChanged();

	g_clistApi.bAutoRebuild = true;
	Clist_Broadcast(CLM_AUTOREBUILD, 0, 0);
	Clist_Broadcast(INTM_INVALIDATE, 0, 0);
	return 0;
}

static int CListIconsChanged(WPARAM, LPARAM)
{
	for (int i = 0; i < _countof(g_statuses); i++)
		ImageList_ReplaceIcon_IconLibLoaded(hCListImages, i + 1, Skin_LoadIcon(g_statuses[i].iSkinIcon));
	ImageList_ReplaceIcon_IconLibLoaded(hCListImages, IMAGE_GROUPOPEN, Skin_LoadIcon(SKINICON_OTHER_GROUPOPEN));
	ImageList_ReplaceIcon_IconLibLoaded(hCListImages, IMAGE_GROUPSHUT, Skin_LoadIcon(SKINICON_OTHER_GROUPSHUT));

	for (auto &it : g_arAccounts) {
		if (it->iIconBase == -1)
			continue;

		for (int j = 0; j < _countof(g_statuses); j++)
			ImageList_ReplaceIcon_IconLibLoaded(hCListImages, it->iIconBase + j, Skin_LoadProtoIcon(it->szModuleName, g_statuses[j].iStatus));
	}

	Clist_TrayIconIconsChanged();
	g_clistApi.pfnInvalidateRect(g_clistApi.hwndContactList, nullptr, TRUE);
	return 0;
}

/*
Begin of Hrk's code for bug
*/
#define GWVS_HIDDEN 1
#define GWVS_VISIBLE 2
#define GWVS_COVERED 3
#define GWVS_PARTIALLY_COVERED 4

int fnGetWindowVisibleState(HWND hWnd, int iStepX, int iStepY)
{
	RECT rc, rcWin, rcWorkArea;
	POINT pt;
	int i, j, width, height, iCountedDots = 0, iNotCoveredDots = 0;
	BOOL bPartiallyCovered = FALSE;
	HWND hAux = nullptr;

	if (hWnd == nullptr) {
		SetLastError(0x00000006);       //Wrong handle
		return -1;
	}

	//Some defaults now. The routine is designed for thin and tall windows.
	if (iStepX <= 0)
		iStepX = 4;
	if (iStepY <= 0)
		iStepY = 16;

	if (IsIconic(hWnd) || !IsWindowVisible(hWnd))
		return GWVS_HIDDEN;

	if (Clist_IsDocked())
		return GWVS_VISIBLE;

	GetWindowRect(hWnd, &rcWin);

	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, FALSE);
	HMONITOR hMon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
	if (GetMonitorInfo(hMon, &mi))
		rcWorkArea = mi.rcWork;

	IntersectRect(&rc, &rcWin, &rcWorkArea);

	width = rc.right - rc.left;
	height = rc.bottom - rc.top;

	for (i = rc.top; i < rc.bottom; i += (height / iStepY)) {
		pt.y = i;
		for (j = rc.left; j < rc.right; j += (width / iStepX)) {
			pt.x = j;
			hAux = WindowFromPoint(pt);
			while (GetParent(hAux) != nullptr)
				hAux = GetParent(hAux);
			if (hAux != hWnd && hAux != nullptr)       //There's another window!
				bPartiallyCovered = TRUE;
			else
				iNotCoveredDots++;  //Let's count the not covered dots.
			iCountedDots++; //Let's keep track of how many dots we checked.
		}
	}

	if (iNotCoveredDots == iCountedDots)    //Every dot was not covered: the window is visible.
		return GWVS_VISIBLE;

	if (iNotCoveredDots == 0)  //They're all covered!
		return GWVS_COVERED;

	// There are dots which are visible, but they are not as many as the ones we counted: it's partially covered.
	return GWVS_PARTIALLY_COVERED;
}

int fnShowHide()
{
	BOOL bShow = FALSE;

	int iVisibleState = g_clistApi.pfnGetWindowVisibleState(g_clistApi.hwndContactList, 0, 0);

	//bShow is FALSE when we enter the switch.
	switch (iVisibleState) {
	case GWVS_PARTIALLY_COVERED:
		//If we don't want to bring it to top, we can use a simple break. This goes against readability ;-) but the comment explains it.
		if (!db_get_b(0, "CList", "BringToFront", SETTING_BRINGTOFRONT_DEFAULT))
			break;
	case GWVS_COVERED:     //Fall through (and we're already falling)
	case GWVS_HIDDEN:
		bShow = TRUE;
		break;
	case GWVS_VISIBLE:     //This is not needed, but goes for readability.
		bShow = FALSE;
		break;
	case -1:               //We can't get here, both g_clistApi.hwndContactList and iStepX and iStepY are right.
		return 0;
	}

	if (bShow == TRUE) {
		ShowWindow(g_clistApi.hwndContactList, SW_RESTORE);
		if (!db_get_b(0, "CList", "OnTop", SETTING_ONTOP_DEFAULT))
			SetWindowPos(g_clistApi.hwndContactList, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		else
			SetWindowPos(g_clistApi.hwndContactList, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		SetForegroundWindow(g_clistApi.hwndContactList);
		db_set_b(0, "CList", "State", SETTING_STATE_NORMAL);

		// this forces the window onto the visible screen
		RECT rcWindow;
		GetWindowRect(g_clistApi.hwndContactList, &rcWindow);
		if (Utils_AssertInsideScreen(&rcWindow) == 1)
			MoveWindow(g_clistApi.hwndContactList, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, TRUE);
	}
	else { // It needs to be hidden
		if (db_get_b(0, "CList", "ToolWindow", SETTING_TOOLWINDOW_DEFAULT) ||
			db_get_b(0, "CList", "Min2Tray", SETTING_MIN2TRAY_DEFAULT)) {
			ShowWindow(g_clistApi.hwndContactList, SW_HIDE);
			db_set_b(0, "CList", "State", SETTING_STATE_HIDDEN);
		}
		else {
			ShowWindow(g_clistApi.hwndContactList, SW_MINIMIZE);
			db_set_b(0, "CList", "State", SETTING_STATE_MINIMIZED);
		}

		if (db_get_b(0, "CList", "DisableWorkingSet", 1))
			SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
	}
	return 0;
}

MIR_APP_DLL(void) Clist_ChangeContactIcon(MCONTACT hContact, int iIcon)
{
	if (hContact == 0)
		return;

	Clist_BroadcastAsync(INTM_ICONCHANGED, hContact, iIcon);

	NotifyEventHooks(hContactIconChangedEvent, hContact, iIcon);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Clist_ContactCompare(MCONTACT hContact1, MCONTACT hContact2)
{
	ClcData *dat = (ClcData*)GetWindowLongPtr(g_clistApi.hwndContactTree, 0);
	if (dat != nullptr) {
		ClcContact *p1, *p2;
		if (Clist_FindItem(g_clistApi.hwndContactTree, dat, hContact1, &p1) && Clist_FindItem(g_clistApi.hwndContactTree, dat, hContact2, &p2))
			return g_clistApi.pfnCompareContacts(p1, p2);
	}

	return 0;
}

/***************************************************************************************/

int LoadContactListModule2(void)
{
	HookEvent(ME_SYSTEM_MODULESLOADED, ContactListModulesLoaded);
	HookEvent(ME_PROTO_ACCLISTCHANGED, ContactListAccountsChanged);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);
	HookEvent(ME_DB_CONTACT_ADDED, ContactAdded);
	HookEvent(ME_DB_CONTACT_DELETED, ContactDeleted);
	HookEvent(ME_OPT_INITIALISE, ClcOptInit);
	HookEvent(ME_SKIN_ICONSCHANGED, CListIconsChanged);

	hContactDoubleClicked = CreateHookableEvent(ME_CLIST_DOUBLECLICKED);
	hContactIconChangedEvent = CreateHookableEvent(ME_CLIST_CONTACTICONCHANGED);

	InitCListEvents();
	InitGroupServices();
	InitTray();
	return 0;
}

void UnloadContactListModule()
{
	if (!hCListImages)
		return;

	// remove transitory contacts
	if (Clist::RemoveTempContacts) {
		for (MCONTACT hContact = db_find_first(); hContact != 0; ) {
			MCONTACT hNext = db_find_next(hContact);
			if (!Contact::OnList(hContact))
				db_delete_contact(hContact);
			hContact = hNext;
		}
	}
	
	ImageList_Destroy(hCListImages);
	UninitCListEvents();
	UninitGroupServices();
	DestroyHookableEvent(hContactDoubleClicked);
}
