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
// This implements the services that form the group chat API

#include "..\commonheaders.h"

HWND CreateNewRoom(TContainerData *pContainer, SESSION_INFO *si, BOOL bActivateTab, BOOL bPopupContainer, BOOL bWantPopup)
{
	MCONTACT hContact = si->hContact;
	if (M.FindWindow(hContact) != 0)
		return 0;

	if (hContact != 0 && M.GetByte("limittabs", 0) && !_tcsncmp(pContainer->szName, _T("default"), 6)) {
		if ((pContainer = FindMatchingContainer(_T("default"))) == NULL) {
			TCHAR szName[CONTAINER_NAMELEN + 1];
			mir_sntprintf(szName, SIZEOF(szName), _T("default"));
			if ((pContainer = CreateContainer(szName, CNT_CREATEFLAG_CLONED, hContact)) == NULL)
				return 0;
		}
	}

	TNewWindowData newData = { 0 };
	newData.hContact = hContact;
	newData.isWchar = 0;
	newData.szInitialText = NULL;
	memset(&newData.item, 0, sizeof(newData.item));

	TCHAR *contactName = pcli->pfnGetContactDisplayName(newData.hContact, 0);

	// cut nickname if larger than x chars...
	TCHAR newcontactname[128];
	if (mir_tstrlen(contactName) > 0) {
		if (M.GetByte("cuttitle", 0))
			CutContactName(contactName, newcontactname, SIZEOF(newcontactname));
		else
			_tcsncpy_s(newcontactname, contactName, _TRUNCATE);
	}
	else _tcsncpy_s(newcontactname, _T("_U_"), _TRUNCATE);

	newData.item.pszText = newcontactname;
	newData.item.mask = TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM;
	newData.item.iImage = 0;

	HWND hwndTab = GetDlgItem(pContainer->hwnd, 1159);

	// hide the active tab
	if (pContainer->hwndActive && bActivateTab)
		ShowWindow(pContainer->hwndActive, SW_HIDE);

	int iTabIndex_wanted = M.GetDword(hContact, "tabindex", pContainer->iChilds * 100);
	int iCount = TabCtrl_GetItemCount(hwndTab);

	pContainer->iTabIndex = iCount;
	if (iCount > 0) {
		TCITEM item = { 0 };
		for (int i = iCount - 1; i >= 0; i--) {
			item.mask = TCIF_PARAM;
			TabCtrl_GetItem(hwndTab, i, &item);
			HWND hwnd = (HWND)item.lParam;
			TWindowData *dat = (TWindowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (dat) {
				int relPos = M.GetDword(dat->hContact, "tabindex", i * 100);
				if (iTabIndex_wanted <= relPos)
					pContainer->iTabIndex = i;
			}
		}
	}

	int newItem = TabCtrl_InsertItem(hwndTab, pContainer->iTabIndex, &newData.item);
	SendMessage(hwndTab, EM_REFRESHWITHOUTCLIP, 0, 0);
	if (bActivateTab)
		TabCtrl_SetCurSel(hwndTab, newItem);
	newData.iTabID = newItem;
	newData.iTabImage = newData.item.iImage;
	newData.pContainer = pContainer;
	newData.iActivate = bActivateTab;
	pContainer->iChilds++;
	newData.bWantPopup = bWantPopup;
	newData.hdbEvent = (HANDLE)si;
	HWND hwndNew = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_CHANNEL), GetDlgItem(pContainer->hwnd, 1159), RoomWndProc, (LPARAM)&newData);
	if (pContainer->dwFlags & CNT_SIDEBAR) {
		TWindowData *dat = (TWindowData*)GetWindowLongPtr(hwndNew, GWLP_USERDATA);
		if (dat)
			pContainer->SideBar->addSession(dat, pContainer->iTabIndex);
	}
	SendMessage(pContainer->hwnd, WM_SIZE, 0, 0);
	// if the container is minimized, then pop it up...
	if (IsIconic(pContainer->hwnd)) {
		if (bPopupContainer) {
			SendMessage(pContainer->hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
			SetFocus(pContainer->hwndActive);
		}
		else {
			if (pContainer->dwFlags & CNT_NOFLASH)
				SendMessage(pContainer->hwnd, DM_SETICON, 0, (LPARAM)LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
			else
				FlashContainer(pContainer, 1, 0);
		}
	}
	if (bActivateTab) {
		if (PluginConfig.m_bHideOnClose && !IsWindowVisible(pContainer->hwnd)) {
			WINDOWPLACEMENT wp = { 0 };
			wp.length = sizeof(wp);
			GetWindowPlacement(pContainer->hwnd, &wp);

			BroadCastContainer(pContainer, DM_CHECKSIZE, 0, 0);			// make sure all tabs will re-check layout on activation
			if (wp.showCmd == SW_SHOWMAXIMIZED)
				ShowWindow(pContainer->hwnd, SW_SHOWMAXIMIZED);
			else {
				if (bPopupContainer)
					ShowWindow(pContainer->hwnd, SW_SHOWNORMAL);
				else
					ShowWindow(pContainer->hwnd, SW_SHOWMINNOACTIVE);
			}
			SendMessage(pContainer->hwndActive, WM_SIZE, 0, 0);
			SetFocus(hwndNew);
		}
		else {
			SetFocus(hwndNew);
			RedrawWindow(pContainer->hwnd, NULL, NULL, RDW_INVALIDATE);
			UpdateWindow(pContainer->hwnd);
			if (GetForegroundWindow() != pContainer->hwnd && bPopupContainer == TRUE)
				SetForegroundWindow(pContainer->hwnd);
		}
	}

	if (PluginConfig.m_bIsWin7 && PluginConfig.m_useAeroPeek && CSkin::m_skinEnabled && !M.GetByte("forceAeroPeek", 0))
		CWarning::show(CWarning::WARN_AEROPEEK_SKIN, MB_ICONWARNING | MB_OK);
	return hwndNew;		// return handle of the new dialog
}

void ShowRoom(SESSION_INFO *si, WPARAM, BOOL)
{
	if (si == NULL)
		return;

	if (si->hWnd != NULL) {
		ActivateExistingTab(si->pContainer, si->hWnd);
		return;
	}

	TCHAR szName[CONTAINER_NAMELEN + 2]; szName[0] = 0;
	TContainerData *pContainer = si->pContainer;
	if (pContainer == NULL) {
		GetContainerNameForContact(si->hContact, szName, CONTAINER_NAMELEN);
		if (!g_Settings.bOpenInDefault && !_tcscmp(szName, _T("default")))
			_tcsncpy(szName, _T("Chat Rooms"), CONTAINER_NAMELEN);
		szName[CONTAINER_NAMELEN] = 0;
		pContainer = FindContainerByName(szName);
	}
	if (pContainer == NULL)
		pContainer = CreateContainer(szName, FALSE, si->hContact);
	if (pContainer)
		si->hWnd = CreateNewRoom(pContainer, si, TRUE, TRUE, FALSE);
}
