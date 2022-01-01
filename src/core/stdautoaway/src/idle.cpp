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

static UINT_PTR g_hTimer;

int IdleOptInit(WPARAM wParam, LPARAM);

static int IdleObject_IsUserIdle()
{
	if (g_plugin.bIdleMethod) {
		uint32_t dwTick = Miranda_GetIdle();
		return GetTickCount() - dwTick > (g_plugin.iIdleTime1st * 60 * 1000);
	}

	LASTINPUTINFO ii = { sizeof(ii) };
	if (GetLastInputInfo(&ii))
		return GetTickCount() - ii.dwTime > (g_plugin.iIdleTime1st * 60 * 1000);

	return FALSE;
}

static void CALLBACK IdleTimer(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	if (g_hTimer != idEvent)
		return;

	if (g_plugin.bIdleCheck && IdleObject_IsUserIdle())
		Idle_Enter(1);

	else if (g_plugin.bIdleOnSaver && IsScreenSaverRunning())
		Idle_Enter(2);

	else if (g_plugin.bIdleOnFullScr && IsFullScreen())
		Idle_Enter(3);

	else if (g_plugin.bIdleOnLock && IsWorkstationLocked())
		Idle_Enter(4);

	else if (g_plugin.bIdleOnTerminal && IsTerminalDisconnected())
		Idle_Enter(5);

	else // not idle
		Idle_Enter(-1);
}

void IdleObject_Create()
{
	g_hTimer = SetTimer(nullptr, 0, 2000, IdleTimer);
}

void IdleObject_Destroy()
{
	KillTimer(nullptr, g_hTimer);
}

void LoadIdleModule(void)
{
	IdleObject_Create();

	HookEvent(ME_OPT_INITIALISE, IdleOptInit);
}

void UnloadIdleModule()
{
	IdleObject_Destroy();
}
