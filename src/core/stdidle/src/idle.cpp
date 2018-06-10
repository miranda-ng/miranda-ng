/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org),
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

Settings S;

static bool bModuleInitialized = false;

static UINT_PTR g_hTimer;
static int g_idleType;
static int g_bIsIdle;

static HANDLE hIdleEvent;

void CALLBACK IdleTimer(HWND hwnd, UINT umsg, UINT_PTR idEvent, DWORD dwTime);
int IdleOptInit(WPARAM wParam, LPARAM);

void IdleObject_Create()
{
	g_idleType = g_bIsIdle = 0;
	g_hTimer = SetTimer(nullptr, 0, 2000, IdleTimer);
}

void IdleObject_Destroy()
{
	if (g_bIsIdle) {
		NotifyEventHooks(hIdleEvent, 0, 0);
		g_bIsIdle = false;
	}
	KillTimer(nullptr, g_hTimer);
}

static int IdleObject_IsUserIdle()
{
	if (S.bIdleMethod) {
		DWORD dwTick;
		CallService(MS_SYSTEM_GETIDLE, 0, (LPARAM)&dwTick);
		return GetTickCount() - dwTick > (S.iIdleTime1st * 60 * 1000);
	}

	LASTINPUTINFO ii = { sizeof(ii) };
	if (GetLastInputInfo(&ii))
		return GetTickCount() - ii.dwTime > (S.iIdleTime1st * 60 * 1000);

	return FALSE;
}

static void IdleObject_Tick()
{
	bool idle = false;
	int idleType = 0, flags = 0;

	if (S.bIdleCheck && IdleObject_IsUserIdle()) {
		idleType = 1; idle = true;
	}
	else if (S.bIdleOnSaver && IsScreenSaverRunning()) {
		idleType = 2; idle = true;
	}
	else if (S.bIdleOnFullScr && IsFullScreen()) {
		idleType = 5; idle = true;
	}
	else if (S.bIdleOnLock && IsWorkstationLocked()) {
		idleType = 3; idle = true;
	}
	else if (S.bIdleOnTerminal && IsTerminalDisconnected()) {
		idleType = 4; idle = true;
	}

	if (S.bIdlePrivate)
		flags |= IDF_PRIVACY;

	if (!g_bIsIdle && idle) {
		g_bIsIdle = true;
		g_idleType = idleType;
		NotifyEventHooks(hIdleEvent, 0, IDF_ISIDLE | flags);
	}
	
	if (g_bIsIdle && !idle) {
		g_bIsIdle = false;
		g_idleType = 0;
		NotifyEventHooks(hIdleEvent, 0, flags);
	}
}

void CALLBACK IdleTimer(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	if (g_hTimer == idEvent)
		IdleObject_Tick();
}

static INT_PTR IdleGetInfo(WPARAM, LPARAM lParam)
{
	MIRANDA_IDLE_INFO *mii = (MIRANDA_IDLE_INFO*)lParam;
	if (!mii)
		return 1;

	mii->idleTime = S.iIdleTime1st;
	mii->privacy = S.bIdlePrivate;
	mii->aaStatus = (S.bAAEnable) ? S.bAAStatus : 0;
	mii->aaLock = S.bIdleStatusLock;
	mii->idlesoundsoff = S.bIdleSoundsOff;
	mii->idleType = g_idleType;
	return 0;
}

int LoadIdleModule(void)
{
	bModuleInitialized = TRUE;

	hIdleEvent = CreateHookableEvent(ME_IDLE_CHANGED);
	IdleObject_Create();
	CreateServiceFunction(MS_IDLE_GETIDLEINFO, IdleGetInfo);
	HookEvent(ME_OPT_INITIALISE, IdleOptInit);
	return 0;
}

void UnloadIdleModule()
{
	if (!bModuleInitialized) return;

	IdleObject_Destroy();
	DestroyHookableEvent(hIdleEvent);
	hIdleEvent = nullptr;
}
