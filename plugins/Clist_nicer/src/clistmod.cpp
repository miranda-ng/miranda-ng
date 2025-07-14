/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-10 Miranda ICQ/IM project,
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

int InitCustomMenus(void);

INT_PTR GetContactStatusMessage(WPARAM wParam, LPARAM lParam);

extern int       g_maxStatus;
extern HANDLE    hSvc_GetContactStatusMsg;
extern ImageItem *g_CLUIImageItem;

static INT_PTR GetStatusMode(WPARAM, LPARAM)
{
	return(g_maxStatus == ID_STATUS_OFFLINE ? g_clistApi.currentDesiredStatusMode : g_maxStatus);
}

int IconFromStatusMode(const char *szProto, int status, MCONTACT hContact)
{
	char *szFinalProto;
	int finalStatus;

	if (szProto != nullptr && !mir_strcmp(szProto, META_PROTO) && hContact != 0 && !(cfg::dat.dwFlags & CLUI_USEMETAICONS)) {
		MCONTACT hSubContact = db_mc_getMostOnline(hContact);
		szFinalProto = Proto_GetBaseAccountName(hSubContact);
		finalStatus = (status == 0) ? db_get_w(hSubContact, szFinalProto, "Status", ID_STATUS_OFFLINE) : status;
		hContact = hSubContact;
	}
	else {
		szFinalProto = (char*)szProto;
		finalStatus = status;
	}

	return coreCli.pfnIconFromStatusMode(szFinalProto, finalStatus, hContact);
}

int LoadContactListModule(void)
{
	CreateServiceFunction(MS_CLIST_GETSTATUSMODE, GetStatusMode);

	hSvc_GetContactStatusMsg = CreateServiceFunction("CList/GetContactStatusMsg", GetContactStatusMessage);
	InitCustomMenus();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int GetWindowVisibleState()
{
	RECT rc = { 0 };
	POINT pt = { 0 };
	HWND hAux = nullptr;

	if (g_clistApi.hwndContactList == nullptr) {
		SetLastError(ERROR_INVALID_HANDLE); //Wrong handle
		return -1;
	}
	//Some defaults now. The routine is designed for thin and tall windows.

	if (IsIconic(g_clistApi.hwndContactList) || !IsWindowVisible(g_clistApi.hwndContactList))
		return GWVS_HIDDEN;

	HRGN rgn = nullptr;
	POINT ptOrig;
	RECT rcClient;
	int clip = (int)cfg::dat.bClipBorder;

	GetClientRect(g_clistApi.hwndContactList, &rcClient);
	ptOrig.x = ptOrig.y = 0;
	ClientToScreen(g_clistApi.hwndContactList, &ptOrig);
	rc.left = ptOrig.x;
	rc.top = ptOrig.y;
	rc.right = rc.left + rcClient.right;
	rc.bottom = rc.top + rcClient.bottom;

	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;
	int iStepX = 4;
	int iStepY = 16;

	/*
	* use a rounded clip region to determine which pixels are covered
	* this will avoid problems with certain XP themes which are using transparency for rounded
	* window frames (reluna being one popular example).

	* the radius of 8 should be sufficient for most themes as they usually don't use bigger
	* radii.
	* also, clip at least 2 pixels from the border (same reason)
	*/

	if (g_CLUIImageItem)
		clip = 5;
	else
		clip = 0;

	bool bPartiallyCovered = false;
	int iCountedDots = 0, iNotCoveredDots = 0;
	for (int i = rc.top + clip; i < rc.bottom; i += (height / iStepY)) {
		pt.y = i;
		for (int j = rc.left + clip; j < rc.right; j += (width / iStepX)) {
			pt.x = j;
			if (hAux = WindowFromPoint(pt))
				while (GetParent(hAux) != nullptr)
					hAux = GetParent(hAux);
			
			if (hAux && hAux != g_clistApi.hwndContactList) //There's another window!
				bPartiallyCovered = true;
			else
				iNotCoveredDots++; //Let's count the not covered dots.
			iCountedDots++; //Let's keep track of how many dots we checked.
		}
	}
	if (rgn)
		DeleteObject(rgn);

	if (iNotCoveredDots == iCountedDots) //Every dot was not covered: the window is visible.
		return GWVS_VISIBLE;
	if (iNotCoveredDots == 0) //They're all covered!
		return GWVS_COVERED;
	//There are dots which are visible, but they are not as many as the ones we counted: it's partially covered.
	return GWVS_PARTIALLY_COVERED;
}

int ShowHide()
{
	BOOL bShow = FALSE;

	int iVisibleState = g_clistApi.pfnGetWindowVisibleState();

	if (IsIconic(g_clistApi.hwndContactList)) {
		SendMessage(g_clistApi.hwndContactList, WM_SYSCOMMAND, SC_RESTORE, 0);
		bShow = TRUE;
	}
	else {
		switch (iVisibleState) {
		case GWVS_PARTIALLY_COVERED:
			if (!Clist::bBringToFront)
				break;
			__fallthrough;

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
	}

	if (bShow) {
		SetWindowPos(g_clistApi.hwndContactList, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOSENDCHANGING | SWP_NOCOPYBITS);
		if (!Clist::bOnTop)
			SetWindowPos(g_clistApi.hwndContactList, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSENDCHANGING | SWP_NOCOPYBITS);
		SetForegroundWindow(g_clistApi.hwndContactList);
		//SetActiveWindow(g_clistApi.hwndContactList);
		ShowWindow(g_clistApi.hwndContactList, SW_SHOW);
		g_plugin.setByte("State", SETTING_STATE_NORMAL);

		RECT rcWindow;
		GetWindowRect(g_clistApi.hwndContactList, &rcWindow);
		if (Utils_AssertInsideScreen(&rcWindow) == 1) {
			MoveWindow(g_clistApi.hwndContactList, rcWindow.left, rcWindow.top,
				rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, TRUE);
		}
	}
	else {
		ShowWindow(g_clistApi.hwndContactList, SW_HIDE);
		g_plugin.setByte("State", SETTING_STATE_HIDDEN);
		if (g_plugin.getByte("DisableWorkingSet", 1))
			SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
	}
	return 0;
}
