/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (c) 2012-14 Miranda NG project,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
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
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// The code that creates and animates the tray icon.

#include "commonheaders.h"

static BOOL    isAnimThreadRunning = TRUE;
static HANDLE  hTrayAnimThread = 0;
static HICON   hIconTrayCurrent = 0;
HANDLE  g_hEvent = 0;

static TCHAR g_eventName[100];

static void TrayAnimThread(LPVOID)
{
	int     iAnimMode = (PluginConfig.m_AnimTrayIcons[0] && PluginConfig.m_AnimTrayIcons[1] && PluginConfig.m_AnimTrayIcons[2] &&
						 PluginConfig.m_AnimTrayIcons[3]);
	DWORD   dwElapsed = 0, dwAnimStep = 0;
	HICON   hIconDefault = iAnimMode ? PluginConfig.m_AnimTrayIcons[0] : PluginConfig.g_iconContainer;
	DWORD   idleTimer = 0;
	HANDLE  hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, g_eventName);

	do {
		if (isAnimThreadRunning && PluginConfig.m_UnreadInTray == 0) {
			if (hIconTrayCurrent != hIconDefault)
				FlashTrayIcon(hIconDefault);                        // restore default icon
			PluginConfig.m_TrayFlashState = 0;

			dwElapsed = 0;
			dwAnimStep = 0;
			WaitForSingleObject(hEvent, 30000);
			ResetEvent(hEvent);
			idleTimer += 2000;
		}
		if (!isAnimThreadRunning) {
			if (hIconTrayCurrent != hIconDefault)
				FlashTrayIcon(hIconDefault);                        // restore default icon
			PluginConfig.m_TrayFlashState = 0;
			break;
		}
		if (PluginConfig.m_UnreadInTray) {
			if (iAnimMode) {
				dwAnimStep++;
				if (dwAnimStep > 3)
					dwAnimStep = 0;
				FlashTrayIcon(PluginConfig.m_AnimTrayIcons[dwAnimStep]);                        // restore default icon
			}
			else {                                 // simple flashing
				dwElapsed += 200;
				if (dwElapsed >= 600) {
					PluginConfig.m_TrayFlashState = !PluginConfig.m_TrayFlashState;
					dwElapsed = 0;
					FlashTrayIcon(PluginConfig.m_TrayFlashState ? 0 : hIconDefault);                        // restore default icon
				}
			}
			Sleep(200);
			idleTimer += 200;
		}
		if (idleTimer >= 2000) {
			idleTimer = 0;
		}
	}
	while (isAnimThreadRunning);
	CloseHandle(hEvent);
}

void TSAPI CreateTrayMenus(int mode)
{
	if (mode) {
		mir_sntprintf(g_eventName, SIZEOF(g_eventName), _T("tsr_evt_%d"), GetCurrentThreadId());
		g_hEvent = CreateEvent(NULL, FALSE, FALSE, g_eventName);
		isAnimThreadRunning = TRUE;
		hTrayAnimThread = mir_forkthread(TrayAnimThread, NULL);

		PluginConfig.g_hMenuTrayUnread = CreatePopupMenu();
		PluginConfig.g_hMenuFavorites = CreatePopupMenu();
		PluginConfig.g_hMenuRecent = CreatePopupMenu();
		PluginConfig.g_hMenuTrayContext = GetSubMenu(PluginConfig.g_hMenuContext, 6);
		ModifyMenu(PluginConfig.g_hMenuTrayContext, 0, MF_BYPOSITION | MF_POPUP,
				   (UINT_PTR)PluginConfig.g_hMenuFavorites, TranslateT("Favorites"));
		ModifyMenu(PluginConfig.g_hMenuTrayContext, 2, MF_BYPOSITION | MF_POPUP,
				   (UINT_PTR)PluginConfig.g_hMenuRecent, TranslateT("Recent Sessions"));
		LoadFavoritesAndRecent();
	}
	else {
		isAnimThreadRunning = FALSE;
		SetEvent(g_hEvent);
		WaitForSingleObject(hTrayAnimThread, 5000);
		CloseHandle(g_hEvent);
		g_hEvent = 0;
		hTrayAnimThread = 0;
		if (PluginConfig.g_hMenuTrayUnread != 0) {
			DestroyMenu(PluginConfig.g_hMenuTrayUnread);
			PluginConfig.g_hMenuTrayUnread = 0;
		}
		if (PluginConfig.g_hMenuFavorites != 0) {
			DestroyMenu(PluginConfig.g_hMenuFavorites);
			PluginConfig.g_hMenuFavorites = 0;
		}
		if (PluginConfig.g_hMenuRecent != 0) {
			DestroyMenu(PluginConfig.g_hMenuRecent);
			PluginConfig.g_hMenuRecent = 0;
		}
	}
}
/*
 * create a system tray icon, create all necessary submenus
 */

void TSAPI CreateSystrayIcon(int create)
{
	NOTIFYICONDATA nim;

	nim.cbSize = sizeof(nim);
	nim.hWnd = PluginConfig.g_hwndHotkeyHandler;
	nim.uID = 100;
	nim.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nim.hIcon = PluginConfig.g_iconContainer;
	nim.uCallbackMessage = DM_TRAYICONNOTIFY;
	_tcsncpy_s(nim.szTip, _T("tabSRMM"), _TRUNCATE);
	if (create && !nen_options.bTrayExist) {
		Shell_NotifyIcon(NIM_ADD, &nim);
		nen_options.bTrayExist = TRUE;
		hIconTrayCurrent = 0;
		SetEvent(g_hEvent);
	}
	else if (create == FALSE && nen_options.bTrayExist) {
		Shell_NotifyIcon(NIM_DELETE, &nim);
		nen_options.bTrayExist = FALSE;
	}
}

/*
 * flash the tray icon
 * mode = 0 - continue to flash
 * mode = 1 - restore the original icon
 */

void TSAPI FlashTrayIcon(HICON hIcon)
{
	NOTIFYICONDATA nim;

	hIconTrayCurrent = hIcon;

	if (nen_options.bTraySupport) {
		nim.cbSize = sizeof(nim);
		nim.hWnd = PluginConfig.g_hwndHotkeyHandler;
		nim.uID = 100;
		nim.uFlags = NIF_ICON;
		nim.hIcon = hIcon;
		Shell_NotifyIcon(NIM_MODIFY, &nim);
	}
}

/*
 * add a contact to recent or favorites menu
 * mode = 1, add
 * mode = 0, only modify it..
 * hMenu specifies the menu handle (the menus are identical...)
 * cares about updating the menu entry. It sets the hIcon (proto status icon) in
 * dwItemData of the the menu entry, so that the WM_DRAWITEM handler can retrieve it
 * w/o costly service calls.
 *
 * Also, the function does housekeeping on the Recent Sessions menu to enforce the
 * maximum number of allowed entries (20 at the moment). The oldest (topmost) entry
 * is deleted, if necessary.
 */

void TSAPI AddContactToFavorites(MCONTACT hContact, const TCHAR *szNickname, const char *szProto, TCHAR *szStatus, WORD wStatus, HICON hIcon, BOOL mode, HMENU hMenu)
{
	MENUITEMINFO	mii = {0};
	TCHAR			szMenuEntry[80];
	TCHAR			szFinalNick[100];

	if (szNickname == NULL)
		_tcsncpy_s(szFinalNick, pcli->pfnGetContactDisplayName(hContact, 0), _TRUNCATE);
	else
		_tcsncpy_s(szFinalNick, szNickname, _TRUNCATE);

	if (szProto == NULL)
		szProto = GetContactProto(hContact);
	if (szProto) {
		if (wStatus == 0)
			wStatus = db_get_w((MCONTACT)hContact, szProto, "Status", ID_STATUS_OFFLINE);
		if (szStatus == NULL)
			szStatus = pcli->pfnGetStatusModeDescription(wStatus, 0);
	}
	else
		return;

	if (hIcon == 0)
		hIcon = LoadSkinnedProtoIcon(szProto, wStatus);

	PROTOACCOUNT *acc = ProtoGetAccount(szProto);
	if (acc && acc->tszAccountName) {
		mii.cbSize = sizeof(mii);
		mir_sntprintf(szMenuEntry, SIZEOF(szMenuEntry), _T("%s: %s (%s)"), acc->tszAccountName, szFinalNick, szStatus);
		if (mode) {
			if (hMenu == PluginConfig.g_hMenuRecent) {
				if (CheckMenuItem(hMenu, (UINT_PTR)hContact, MF_BYCOMMAND | MF_UNCHECKED) == 0) {
					DeleteMenu(hMenu, (UINT_PTR)hContact, MF_BYCOMMAND);
					goto addnew;                                            // move to the end of the menu...
				}
				if (GetMenuItemCount(PluginConfig.g_hMenuRecent) > nen_options.wMaxRecent) {           // throw out oldest entry in the recent menu...
					UINT uid = GetMenuItemID(hMenu, 0);
					if (uid) {
						DeleteMenu(hMenu, (UINT_PTR)0, MF_BYPOSITION);
						db_set_dw((MCONTACT)uid, SRMSGMOD_T, "isRecent", 0);
					}
				}
	addnew:
				db_set_dw(hContact, SRMSGMOD_T, "isRecent", time(NULL));
				AppendMenu(hMenu, MF_BYCOMMAND, (UINT_PTR)hContact, szMenuEntry);
			}
			else if (hMenu == PluginConfig.g_hMenuFavorites) {            // insert the item sorted...
				MENUITEMINFO mii2 = {0};
				TCHAR szBuffer[142];
				int i, c = GetMenuItemCount(PluginConfig.g_hMenuFavorites);
				mii2.fMask = MIIM_STRING;
				mii2.cbSize = sizeof(mii2);
				if (c == 0)
					InsertMenu(PluginConfig.g_hMenuFavorites, 0, MF_BYPOSITION, (UINT_PTR)hContact, szMenuEntry);
				else {
					for (i=0; i <= c; i++) {
						mii2.cch = 0;
						mii2.dwTypeData = NULL;
						GetMenuItemInfo(PluginConfig.g_hMenuFavorites, i, TRUE, &mii2);
						mii2.cch++;
						mii2.dwTypeData = szBuffer;
						GetMenuItemInfo(PluginConfig.g_hMenuFavorites, i, TRUE, &mii2);
						if (_tcsncmp((TCHAR*)mii2.dwTypeData, szMenuEntry, 140) > 0 || i == c) {
							InsertMenu(PluginConfig.g_hMenuFavorites, i, MF_BYPOSITION, (UINT_PTR)hContact, szMenuEntry);
							break;
						}
					}
				}
			}
		}
		mii.fMask = MIIM_BITMAP | MIIM_DATA;
		if (!mode) {
			mii.fMask |= MIIM_STRING;
			mii.dwTypeData = (LPTSTR)szMenuEntry;
			mii.cch = (int)mir_tstrlen(szMenuEntry) + 1;
		}
		mii.hbmpItem = HBMMENU_CALLBACK;
		mii.dwItemData = (ULONG_PTR)hIcon;
		SetMenuItemInfo(hMenu, (UINT)hContact, FALSE, &mii);
	}
}

/*
 * called by CreateSysTrayIcon(), usually on startup or when you activate tray support
 * at runtime.
 * scans the contact db for favorites or recent session entries and builds the menus.
 */

typedef struct _recentEntry {
	DWORD dwTimestamp;
	MCONTACT hContact;
} RCENTRY;

void TSAPI LoadFavoritesAndRecent()
{
	DWORD	dwRecent;
	int iIndex = 0, i, j;

	RCENTRY *recentEntries = new RCENTRY[nen_options.wMaxRecent + 1];
	if (recentEntries == NULL)
		return;

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		if (M.GetByte(hContact, "isFavorite", 0))
			AddContactToFavorites(hContact, NULL, NULL, NULL, 0, 0, 1, PluginConfig.g_hMenuFavorites);
		if ((dwRecent = M.GetDword(hContact, "isRecent", 0)) != 0 && iIndex < nen_options.wMaxRecent) {
			recentEntries[iIndex].dwTimestamp = dwRecent;
			recentEntries[iIndex++].hContact = hContact;
		}
	}

	if (iIndex == 0) {
		delete[] recentEntries;
		return;
	}

	for (i=0; i < iIndex - 1; i++) {
		for (j = 0; j < iIndex - 1; j++) {
			if (recentEntries[j].dwTimestamp > recentEntries[j+1].dwTimestamp) {
				RCENTRY rceTemp = recentEntries[j];
				recentEntries[j] = recentEntries[j+1];
				recentEntries[j+1] = rceTemp;
			}
		}
	}
	for (i=0; i < iIndex; i++)
		AddContactToFavorites(recentEntries[i].hContact, NULL, NULL, NULL, 0, 0, 1, PluginConfig.g_hMenuRecent);

	delete[] recentEntries;
}
