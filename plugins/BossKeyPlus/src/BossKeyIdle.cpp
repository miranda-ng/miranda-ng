/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)
Copyright (c) 2000-05 Miranda ICQ/IM project,
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

POINT mousepos;
UINT_PTR hTimer;
UINT mouseidle, minutes;

VOID CALLBACK IdleTimer(HWND hwnd, UINT umsg, UINT idEvent, DWORD dwTime);

static bool IsUserIdle()
{
	if (g_wMaskAdv & OPT_HIDEIFMIRIDLE) {
		DWORD dwTick = Miranda_GetIdle();
		return GetTickCount() - dwTick > (minutes * 60 * 1000);
	}

	LASTINPUTINFO ii = { sizeof(ii) };
	if (GetLastInputInfo(&ii))
		return GetTickCount() - ii.dwTime > (minutes * 60 * 1000);

	return FALSE;
}

VOID CALLBACK IdleTimer(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	if (hTimer == idEvent && !g_bWindowHidden && ((g_wMaskAdv & (OPT_HIDEIFWINIDLE | OPT_HIDEIFMIRIDLE) && IsUserIdle()) ||
		(g_wMaskAdv & OPT_HIDEIFSCRSVR) && IsScreenSaverRunning()))
		BossKeyHideMiranda(0, 0);
}

void InitIdleTimer()
{
	minutes = g_plugin.getByte("time", 10);
	hTimer = SetTimer(nullptr, 0, 2000, IdleTimer);
}

void UninitIdleTimer()
{
	KillTimer(nullptr, hTimer);
}
