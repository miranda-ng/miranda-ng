/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "commonheaders.h"

#define AA_MODULE "AutoAway"

static int iBreakSounds = 0;

static int AutoAwaySound(WPARAM, LPARAM lParam)
{
	return iBreakSounds;
}

///////////////////////////////////////////////////////////////////////////////

static bool Proto_IsAccountEnabled(PROTOACCOUNT *pa)
{
	return pa && ((pa->bIsEnabled && !pa->bDynDisabled) || pa->bOldProto);
}

static bool Proto_IsAccountLocked(PROTOACCOUNT *pa)
{
	return pa && db_get_b(NULL, pa->szModuleName, "LockMainStatus", 0) != 0;
}

static void Proto_SetStatus(const char *szProto, unsigned status)
{
	if (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) {
		ptrT awayMsg((TCHAR*)CallService(MS_AWAYMSG_GETSTATUSMSGW, status, (LPARAM)szProto));
		CallProtoService(szProto, PS_SETAWAYMSG, status, awayMsg);
	}

	CallProtoService(szProto, PS_SETSTATUS, status, 0);
}

static int AutoAwayEvent(WPARAM, LPARAM lParam)
{
	MIRANDA_IDLE_INFO mii = { sizeof(mii) };
	CallService(MS_IDLE_GETIDLEINFO, 0, (LPARAM)&mii);

	if (mii.idlesoundsoff)
		iBreakSounds = (lParam & IDF_ISIDLE) != 0;

	// we don't need to switch the status
	if (mii.aaStatus == 0)
		return 0;

	int numAccounts;
	PROTOACCOUNT** accounts;
	ProtoEnumAccounts(&numAccounts, &accounts);

	for (int i = 0; i < numAccounts; i++) {
		PROTOACCOUNT *pa = accounts[i];
		if (!Proto_IsAccountEnabled(pa) || Proto_IsAccountLocked(pa))
			continue;

		int currentstatus = CallProtoService(pa->szModuleName, PS_GETSTATUS, 0, 0);
		int statusbits = CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0);
		int status = mii.aaStatus;
		if (!(statusbits & Proto_Status2Flag(status))) {
			// the protocol doesnt support the given status
			if (statusbits & Proto_Status2Flag(ID_STATUS_AWAY))
				status = ID_STATUS_AWAY;
		}
		if (lParam & IDF_ISIDLE) {
			if (currentstatus != ID_STATUS_ONLINE && currentstatus != ID_STATUS_FREECHAT)
				continue;

			// save old status of account and set to given status
			db_set_w(NULL, AA_MODULE, pa->szModuleName, currentstatus);
			Proto_SetStatus(pa->szModuleName, status);
		}
		else {
			int oldstatus = db_get_w(NULL, AA_MODULE, pa->szModuleName, 0);
			if (oldstatus != ID_STATUS_ONLINE && oldstatus != ID_STATUS_FREECHAT)
				continue;

			// returning from idle and this accout was set away, set it back
			db_unset(NULL, AA_MODULE, pa->szModuleName);
			if (!mii.aaLock)
				Proto_SetStatus(pa->szModuleName, oldstatus);
		}
	}

	return 0;
}

int LoadAutoAwayModule()
{
	HookEvent(ME_SKIN_PLAYINGSOUND, AutoAwaySound);
	HookEvent(ME_IDLE_CHANGED, AutoAwayEvent);
	return 0;
}
