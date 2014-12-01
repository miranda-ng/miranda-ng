/*
Scriver

Copyright (c) 2000-09 Miranda ICQ/IM project,

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

HANDLE hHookIconPressedEvt;

void DrawStatusIcons(MCONTACT hContact, HDC hDC, RECT r, int gap)
{
	HICON hIcon;
	int x = r.left;

	int nIcon = 0;
	while (StatusIconData *si = Srmm_GetNthIcon(hContact, nIcon++)) {
		if ((si->flags & MBF_DISABLED) && si->hIconDisabled) hIcon = si->hIconDisabled;
		else hIcon = si->hIcon;

		SetBkMode(hDC, TRANSPARENT);
		DrawIconEx(hDC, x, (r.top + r.bottom - GetSystemMetrics(SM_CYSMICON)) >> 1, hIcon, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, NULL, DI_NORMAL);

		x += GetSystemMetrics(SM_CYSMICON) + gap;
	}
}

void CheckStatusIconClick(MCONTACT hContact, HWND hwndFrom, POINT pt, RECT r, int gap, int click_flags)
{
	unsigned int iconNum = (pt.x - r.left) / (GetSystemMetrics(SM_CXSMICON) + gap);
	StatusIconData *si = Srmm_GetNthIcon(hContact, iconNum);
	if (si == NULL)
		return;

	ClientToScreen(hwndFrom, &pt);

	StatusIconClickData sicd = { sizeof(sicd) };
	sicd.clickLocation = pt;
	sicd.dwId = si->dwId;
	sicd.szModule = si->szModule;
	sicd.flags = click_flags;
	NotifyEventHooks(hHookIconPressedEvt, hContact, (LPARAM)&sicd);
}

static int OnSrmmIconChanged(WPARAM hContact, LPARAM)
{
	if (hContact == NULL)
		WindowList_Broadcast(g_dat.hParentWindowList, DM_STATUSICONCHANGE, 0, 0);
	else {
		HWND hwnd = WindowList_Find(g_dat.hMessageWindowList, hContact);
		if (hwnd == NULL)
			hwnd = SM_FindWindowByContact(hContact);
		if (hwnd != NULL)
			PostMessage(GetParent(hwnd), DM_STATUSICONCHANGE, 0, 0);
	}
	return 0;
}

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
