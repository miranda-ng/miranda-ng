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

static int iBreakSounds = 0;

static int AutoAwaySound(WPARAM, LPARAM)
{
	return iBreakSounds;
}

///////////////////////////////////////////////////////////////////////////////

static void Proto_SetStatus(const char *szProto, unsigned status)
{
	if (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) {
		ptrW awayMsg((wchar_t*)CallService(MS_AWAYMSG_GETSTATUSMSGW, status, (LPARAM)szProto));
		CallProtoService(szProto, PS_SETAWAYMSG, status, awayMsg);
	}

	CallProtoService(szProto, PS_SETSTATUS, status, 0);
}

static int AutoAwayEvent(WPARAM, LPARAM lParam)
{
	MIRANDA_IDLE_INFO mii;
	Idle_GetInfo(mii);

	if (mii.idlesoundsoff)
		iBreakSounds = (lParam & IDF_ISIDLE) != 0;

	// we don't need to switch the status
	if (mii.aaStatus == 0) {
		Netlib_Logf(0, "%s: aaStatus == 0, no need to restore the status", MODULENAME);
		return 0;
	}

	for (auto &pa : Accounts()) {
		if (!pa->IsEnabled()) {
			Netlib_Logf(0, "%s: '%s' isn't enabled, skipping", MODULENAME, pa->szModuleName);
			continue;
		}

		if (pa->IsLocked()) {
			Netlib_Logf(0, "%s: '%s' is locked, skipping", MODULENAME, pa->szModuleName);
			continue;
		}

		int statusbits = CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0);
		int status = mii.aaStatus;
		if (!(statusbits & Proto_Status2Flag(status))) // the protocol doesnt support the given status
			if (statusbits & Proto_Status2Flag(ID_STATUS_AWAY))
				status = ID_STATUS_AWAY;

		if (lParam & IDF_ISIDLE) {
			if (pa->iRealStatus != ID_STATUS_ONLINE && pa->iRealStatus != ID_STATUS_FREECHAT) {
				Netlib_Logf(0, "%s: '%s' isn't online (%d), skipping", MODULENAME, pa->szModuleName, pa->iRealStatus);
				continue;
			}

			// save old status of account and set to given status
			Netlib_Logf(0, "%s: '%s' enters AutoAway, setting status to %d", MODULENAME, pa->szModuleName, status);
			g_plugin.setWord(pa->szModuleName, pa->iRealStatus);
			Proto_SetStatus(pa->szModuleName, status);
		}
		else {
			int oldstatus = g_plugin.getWord(pa->szModuleName, 0);
			if (oldstatus != ID_STATUS_ONLINE && oldstatus != ID_STATUS_FREECHAT) {
				Netlib_Logf(0, "%s: '%s' wasn't online (%d), skipping", MODULENAME, pa->szModuleName, oldstatus);
				continue;
			}

			// returning from idle and this accout was set away, set it back
			g_plugin.delSetting(pa->szModuleName);
			if (!mii.aaLock) {
				Netlib_Logf(0, "%s: '%s' leaving idle to %d", MODULENAME, pa->szModuleName, oldstatus);
				Proto_SetStatus(pa->szModuleName, oldstatus);
			}
			else Netlib_Logf(0, "%s: '%s' leaving idle", MODULENAME, pa->szModuleName);
		}
	}

	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_IDLE_CHANGED, AutoAwayEvent);
	return 0;
}

int LoadAutoAwayModule()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_SKIN_PLAYINGSOUND, AutoAwaySound);
	return 0;
}
