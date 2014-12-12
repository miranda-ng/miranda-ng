/*

Copyright 2000-12 Miranda IM, 2012-14 Miranda NG project,
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

#include "commonheaders.h"
#include "statusicon.h"

HANDLE hHookIconPressedEvt;

static int OnSrmmIconChanged(WPARAM hContact, LPARAM)
{
	if (hContact == NULL)
		WindowList_Broadcast(g_dat.hMessageWindowList, DM_STATUSICONCHANGE, 0, 0);
	else {
		HWND hwnd = WindowList_Find(g_dat.hMessageWindowList, hContact);
		if (hwnd != NULL)
			PostMessage(hwnd, DM_STATUSICONCHANGE, 0, 0);
	}
	return 0;
}

void DrawStatusIcons(MCONTACT hContact, HDC hDC, const RECT &rc, int gap)
{
	int x = rc.left;
	int cx_icon = GetSystemMetrics(SM_CXSMICON);
	int cy_icon = GetSystemMetrics(SM_CYSMICON);

	int nIcon = 0;
	StatusIconData *sid;
	while ((sid = Srmm_GetNthIcon(hContact, nIcon++)) != 0 && x < rc.right) {
		HICON hIcon = ((sid->flags & MBF_DISABLED) && sid->hIconDisabled) ? sid->hIconDisabled : sid->hIcon;

		SetBkMode(hDC, TRANSPARENT);
		DrawIconEx(hDC, x, (rc.top + rc.bottom - cy_icon) >> 1, hIcon, cx_icon, cy_icon, 0, NULL, DI_NORMAL);

		x += cx_icon + gap;
	}
}

void CheckStatusIconClick(MCONTACT hContact, HWND hwndFrom, POINT pt, const RECT &rc, int gap, int click_flags)
{
	int iconNum = (pt.x - rc.left) / (GetSystemMetrics(SM_CXSMICON) + gap);
	StatusIconData *sid = Srmm_GetNthIcon(hContact, iconNum);
	if (sid == NULL)
		return;

	StatusIconClickData sicd = { sizeof(sicd) };
	ClientToScreen(hwndFrom, &pt);
	sicd.clickLocation = pt;
	sicd.dwId = sid->dwId;
	sicd.szModule = sid->szModule;
	sicd.flags = click_flags;

	NotifyEventHooks(hHookIconPressedEvt, hContact, (LPARAM)&sicd);
}

HANDLE hServiceIcon[3];

int InitStatusIcons()
{
	HookEvent(ME_MSG_ICONSCHANGED, OnSrmmIconChanged);

	hHookIconPressedEvt = CreateHookableEvent(ME_MSG_ICONPRESSED);
	return 0;
}

int DeinitStatusIcons()
{
	DestroyHookableEvent(hHookIconPressedEvt);
	return 0;
}

int GetStatusIconsCount(MCONTACT hContact)
{
	int nIcon = 0;
	while (Srmm_GetNthIcon(hContact, nIcon) != NULL)
		nIcon++;
	return nIcon;
}
