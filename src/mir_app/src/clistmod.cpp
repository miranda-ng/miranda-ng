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
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

#include "clc.h"
#include "genmenu.h"

int InitCListEvents(void);
void UninitCListEvents(void);
void UninitGroupServices(void);
int ContactSettingChanged(WPARAM wParam, LPARAM lParam);
int ContactAdded(WPARAM hContact, LPARAM);
int ContactDeleted(WPARAM hContact, LPARAM);
INT_PTR GetContactDisplayName(WPARAM wParam, LPARAM lParam);
INT_PTR InvalidateDisplayName(WPARAM wParam, LPARAM lParam);
int InitGroupServices(void);
INT_PTR Docking_IsDocked(WPARAM wParam, LPARAM lParam);
int LoadCLUIModule(void);
int InitClistHotKeys(void);
void ScheduleMenuUpdate(void);

HANDLE hContactDoubleClicked, hContactIconChangedEvent;
HIMAGELIST hCListImages;

extern BYTE nameOrder[];

struct ProtoIconIndex
{
	char *szProto;
	int iIconBase;
};

OBJLIST<ProtoIconIndex> protoIconIndex(5);

wchar_t* fnGetStatusModeDescription(int mode, int flags)
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
	case ID_STATUS_OUTTOLUNCH:
		descr = LPGENW("Out to lunch");
		break;
	case ID_STATUS_ONTHEPHONE:
		descr = LPGENW("On the phone");
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

static int ProtocolAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack->type != ACKTYPE_STATUS)
		return 0;

	cli.pfnCluiProtocolStatusChanged(lParam, ack->szModule);

	if ((INT_PTR)ack->hProcess < ID_STATUS_ONLINE && ack->lParam >= ID_STATUS_ONLINE) {
		DWORD caps = (DWORD)CallProtoServiceInt(0, ack->szModule, PS_GETCAPS, PFLAGNUM_1, 0);
		if (caps & PF1_SERVERCLIST) {
			for (MCONTACT hContact = db_find_first(ack->szModule); hContact; ) {
				MCONTACT hNext = db_find_next(hContact, ack->szModule);
				if (db_get_b(hContact, "CList", "Delete", 0))
					db_delete_contact(hContact);
				hContact = hNext;
			}
		}
	}

	cli.pfnTrayIconUpdateBase(ack->szModule);
	return 0;
}

HICON fnGetIconFromStatusMode(MCONTACT hContact, const char *szProto, int status)
{
	return ImageList_GetIcon(hCListImages, cli.pfnIconFromStatusMode(szProto, status, hContact), ILD_NORMAL);
}

int fnIconFromStatusMode(const char *szProto, int status, MCONTACT)
{
	int index, i;

	for (index = 0; index < _countof(statusModeList); index++)
		if (status == statusModeList[index])
			break;

	if (index == _countof(statusModeList))
		index = 0;
	if (szProto == nullptr)
		return index + 1;
	for (i = 0; i < protoIconIndex.getCount(); i++) {
		if (mir_strcmp(szProto, protoIconIndex[i].szProto) == 0)
			return protoIconIndex[i].iIconBase + index;
	}
	return 1;
}

int fnGetContactIcon(MCONTACT hContact)
{
	char *szProto = GetContactProto(hContact);
	return cli.pfnIconFromStatusMode(szProto,
		szProto == nullptr ? ID_STATUS_OFFLINE : db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE), hContact);
}

static void AddProtoIconIndex(PROTOACCOUNT *pa)
{
	ProtoIconIndex *pii = new ProtoIconIndex;
	pii->szProto = pa->szModuleName;
	for (int i = 0; i < _countof(statusModeList); i++) {
		int iImg = ImageList_AddIcon_ProtoIconLibLoaded(hCListImages, pa->szModuleName, statusModeList[i]);
		if (i == 0)
			pii->iIconBase = iImg;
	}
	protoIconIndex.insert(pii);
}

static void RemoveProtoIconIndex(PROTOACCOUNT *pa)
{
	for (int i = 0; i < protoIconIndex.getCount(); i++)
		if (mir_strcmp(protoIconIndex[i].szProto, pa->szModuleName) == 0) {
			protoIconIndex.remove(i);
			break;
		}
}

static int ContactListModulesLoaded(WPARAM, LPARAM)
{
	ScheduleMenuUpdate();

	RebuildMenuOrder();
	for (int i = 0; i < accounts.getCount(); i++)
		AddProtoIconIndex(accounts[i]);

	cli.pfnLoadContactTree();

	LoadCLUIModule();

	InitClistHotKeys();

	return 0;
}

static int ContactListAccountsChanged(WPARAM eventCode, LPARAM lParam)
{
	switch (eventCode) {
	case PRAC_ADDED:
		AddProtoIconIndex((PROTOACCOUNT*)lParam);
		break;

	case PRAC_REMOVED:
		RemoveProtoIconIndex((PROTOACCOUNT*)lParam);
		break;
	}
	cli.pfnReloadProtoMenus();
	cli.pfnTrayIconIconsChanged();
	Clist_Broadcast(INTM_RELOADOPTIONS, 0, 0);
	Clist_Broadcast(INTM_INVALIDATE, 0, 0);
	return 0;
}

MIR_APP_DLL(void) Clist_ContactDoubleClicked(MCONTACT hContact)
{
	// Try to process event myself
	if (cli.pfnEventsProcessContactDoubleClick(hContact) == 0)
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

static int CListIconsChanged(WPARAM, LPARAM)
{
	for (int i = 0; i < _countof(statusModeList); i++)
		ImageList_ReplaceIcon_IconLibLoaded(hCListImages, i + 1, Skin_LoadIcon(skinIconStatusList[i]));
	ImageList_ReplaceIcon_IconLibLoaded(hCListImages, IMAGE_GROUPOPEN, Skin_LoadIcon(SKINICON_OTHER_GROUPOPEN));
	ImageList_ReplaceIcon_IconLibLoaded(hCListImages, IMAGE_GROUPSHUT, Skin_LoadIcon(SKINICON_OTHER_GROUPSHUT));
	for (int i = 0; i < protoIconIndex.getCount(); i++)
		for (int j = 0; j < _countof(statusModeList); j++)
			ImageList_ReplaceIcon_IconLibLoaded(hCListImages, protoIconIndex[i].iIconBase + j, Skin_LoadProtoIcon(protoIconIndex[i].szProto, statusModeList[j]));
	cli.pfnTrayIconIconsChanged();
	cli.pfnInvalidateRect(cli.hwndContactList, nullptr, TRUE);
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
	register int i, j, width, height, iCountedDots = 0, iNotCoveredDots = 0;
	BOOL bPartiallyCovered = FALSE;
	HWND hAux = 0;

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

	int iVisibleState = cli.pfnGetWindowVisibleState(cli.hwndContactList, 0, 0);

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
	case -1:               //We can't get here, both cli.hwndContactList and iStepX and iStepY are right.
		return 0;
	}

	if (bShow == TRUE) {
		ShowWindow(cli.hwndContactList, SW_RESTORE);
		if (!db_get_b(0, "CList", "OnTop", SETTING_ONTOP_DEFAULT))
			SetWindowPos(cli.hwndContactList, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		else
			SetWindowPos(cli.hwndContactList, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		SetForegroundWindow(cli.hwndContactList);
		db_set_b(0, "CList", "State", SETTING_STATE_NORMAL);

		// this forces the window onto the visible screen
		RECT rcWindow;
		GetWindowRect(cli.hwndContactList, &rcWindow);
		if (Utils_AssertInsideScreen(&rcWindow) == 1)
			MoveWindow(cli.hwndContactList, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, TRUE);
	}
	else { // It needs to be hidden
		if (db_get_b(0, "CList", "ToolWindow", SETTING_TOOLWINDOW_DEFAULT) ||
			db_get_b(0, "CList", "Min2Tray", SETTING_MIN2TRAY_DEFAULT)) {
			ShowWindow(cli.hwndContactList, SW_HIDE);
			db_set_b(0, "CList", "State", SETTING_STATE_HIDDEN);
		}
		else {
			ShowWindow(cli.hwndContactList, SW_MINIMIZE);
			db_set_b(0, "CList", "State", SETTING_STATE_MINIMIZED);
		}

		if (db_get_b(0, "CList", "DisableWorkingSet", 1))
			SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
	}
	return 0;
}

void fnChangeContactIcon(MCONTACT hContact, int iIcon)
{
	Clist_BroadcastAsync(INTM_ICONCHANGED, hContact, iIcon);

	NotifyEventHooks(hContactIconChangedEvent, hContact, iIcon);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(int) Clist_ContactCompare(MCONTACT hContact1, MCONTACT hContact2)
{
	ClcData *dat = (ClcData*)GetWindowLongPtr(cli.hwndContactTree, 0);
	if (dat != nullptr) {
		ClcContact *p1, *p2;
		if (Clist_FindItem(cli.hwndContactTree, dat, hContact1, &p1, nullptr, nullptr) && Clist_FindItem(cli.hwndContactTree, dat, hContact2, &p2, nullptr, nullptr))
			return cli.pfnCompareContacts(p1, p2);
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
	HookEvent(ME_PROTO_ACK, ProtocolAck);

	hContactDoubleClicked = CreateHookableEvent(ME_CLIST_DOUBLECLICKED);
	hContactIconChangedEvent = CreateHookableEvent(ME_CLIST_CONTACTICONCHANGED);

	InitCListEvents();
	InitGroupServices();
	cli.pfnInitTray();

	hCListImages = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 13, 0);
	HookEvent(ME_SKIN_ICONSCHANGED, CListIconsChanged);

	ImageList_AddIcon_NotShared(hCListImages, MAKEINTRESOURCE(IDI_BLANK));

	// now all core skin icons are loaded via icon lib. so lets release them
	for (int i = 0; i < _countof(statusModeList); i++)
		ImageList_AddIcon_IconLibLoaded(hCListImages, skinIconStatusList[i]);

	// see IMAGE_GROUP... in clist.h if you add more images above here
	ImageList_AddIcon_IconLibLoaded(hCListImages, SKINICON_OTHER_GROUPOPEN);
	ImageList_AddIcon_IconLibLoaded(hCListImages, SKINICON_OTHER_GROUPSHUT);
	return 0;
}

void UnloadContactListModule()
{
	if (!hCListImages)
		return;

	// remove transitory contacts
	for (MCONTACT hContact = db_find_first(); hContact != 0; ) {
		MCONTACT hNext = db_find_next(hContact);
		if (db_get_b(hContact, "CList", "NotOnList", 0))
			db_delete_contact(hContact);
		hContact = hNext;
	}
	ImageList_Destroy(hCListImages);
	UninitCListEvents();
	UninitGroupServices();
	DestroyHookableEvent(hContactDoubleClicked);
}
