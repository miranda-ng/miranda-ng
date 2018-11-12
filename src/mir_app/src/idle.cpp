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

#define MODULENAME "Idle"

static bool bModuleInitialized = false;

static int g_idleType;
static int g_bIsIdle;

static HANDLE hIdleEvent;

MIR_APP_DLL(void) Idle_Enter(int type)
{
	int flags = 0;

	if (g_plugin.getByte("IdlePrivate"))
		flags |= IDF_PRIVACY;

	if (!g_bIsIdle && type != -1) {
		g_bIsIdle = true;
		g_idleType = type;
		NotifyEventHooks(hIdleEvent, 0, IDF_ISIDLE | flags);
	}
	
	if (g_bIsIdle && type == -1) {
		g_bIsIdle = false;
		g_idleType = 0;
		NotifyEventHooks(hIdleEvent, 0, flags);
	}
}

MIR_APP_DLL(void) Idle_GetInfo(MIRANDA_IDLE_INFO &pInfo)
{
	pInfo.idleTime = g_plugin.getDword("IdleTime1st");
	pInfo.privacy = g_plugin.getByte("IdlePrivate");
	pInfo.aaStatus = g_plugin.getByte("AAEnable", 1) ? g_plugin.getWord("AAStatus") : 0;
	pInfo.aaLock = g_plugin.getByte("IdleStatusLock");
	pInfo.idlesoundsoff = g_plugin.getByte("IdleSoundsOff");
	pInfo.idleType = g_idleType;
}

int LoadIdleModule(void)
{
	bModuleInitialized = true;

	hIdleEvent = CreateHookableEvent(ME_IDLE_CHANGED);

	g_idleType = g_bIsIdle = 0;
	return 0;
}

void UnloadIdleModule()
{
	if (!bModuleInitialized) return;

	if (g_bIsIdle) {
		NotifyEventHooks(hIdleEvent, 0, 0);
		g_bIsIdle = false;
	}

	DestroyHookableEvent(hIdleEvent);
	hIdleEvent = nullptr;
}
