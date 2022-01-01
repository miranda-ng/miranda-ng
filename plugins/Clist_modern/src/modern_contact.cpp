/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

struct
{
	int status, order;
}
static statusModeOrder[] =
{
	{ ID_STATUS_OFFLINE, 500 },
	{ ID_STATUS_ONLINE, 10 },
	{ ID_STATUS_AWAY, 200 },
	{ ID_STATUS_DND, 110 },
	{ ID_STATUS_NA, 450 },
	{ ID_STATUS_OCCUPIED, 100 },
	{ ID_STATUS_FREECHAT, 0 },
	{ ID_STATUS_INVISIBLE, 20 }
};

static int LocaleId = -1;

static int GetContactStatus(MCONTACT hContact)
{
	return (GetContactCachedStatus(hContact));
}

static int GetStatusModeOrdering(int statusMode)
{
	for (auto &it : statusModeOrder)
		if (it.status == statusMode)
			return it.order;
	
	return 1000;
}

uint32_t CompareContacts2_getLMTime(MCONTACT hContact)
{
	uint32_t ret = g_plugin.getDword(hContact, "mf_lastmsg");
	if (ret != 0)
		return ret;

	DB::ECPTR pCursor(DB::EventsRev(hContact));
	while (MEVENT hDbEvent = pCursor.FetchNext()) {
		DBEVENTINFO dbei = {};
		db_event_get(hDbEvent, &dbei);
		if ((dbei.eventType == EVENTTYPE_MESSAGE || dbei.eventType == EVENTTYPE_FILE) && !(dbei.flags & DBEF_SENT))
			return dbei.timestamp;
	}
	return 0;
}

#define SAFESTRING(a) a?a:""
#define SAFETSTRING(a) a?a:L""

int GetProtoIndex(char * szName)
{
	if (szName)
		for (auto &pa : Accounts())
			if (!mir_strcmpi(szName, pa->szModuleName))
				return pa->iOrder;

	return -1;
}

int cliCompareContacts(const ClcContact *contact1, const ClcContact *contact2)
{
	ClcCacheEntry *c1 = contact1->pce, *c2 = contact2->pce;

	for (int i = 0; i < _countof(g_CluiData.bSortByOrder); i++) {
		uint8_t &by = g_CluiData.bSortByOrder[i];

		if (by == SORTBY_STATUS) { //status
			int ordera = GetStatusModeOrdering(c1->getStatus());
			int orderb = GetStatusModeOrdering(c2->getStatus());
			if (ordera == orderb)
				continue;
			return ordera - orderb;
		}

		// one is offline: offline goes below online
		if (!g_CluiData.fSortNoOfflineBottom) {
			int statusa = c1->getStatus();
			int statusb = c2->getStatus();
			if ((statusa == ID_STATUS_OFFLINE) != (statusb == ID_STATUS_OFFLINE))
				return 2 * (statusa == ID_STATUS_OFFLINE) - 1;
		}

		int r = 0;

		switch (by) {
		case SORTBY_NAME: // name
			r = mir_wstrcmpi(contact1->szText, contact2->szText);
			break;

		case SORTBY_NAME_LOCALE: // name
			if (LocaleId == -1)
				LocaleId = Langpack_GetDefaultLocale();
			r = CompareString(LocaleId, NORM_IGNORECASE, SAFETSTRING(contact1->szText), -1, SAFETSTRING(contact2->szText), -1) - 2;
			break;

		case SORTBY_LASTMSG: // last message
			if (c1->dwLastMsgTime == -1) c1->dwLastMsgTime = CompareContacts2_getLMTime(contact1->hContact);
			if (c2->dwLastMsgTime == -1) c2->dwLastMsgTime = CompareContacts2_getLMTime(contact2->hContact);
			if (c1->dwLastMsgTime == c2->dwLastMsgTime)
				continue;

			r = (c1->dwLastMsgTime < c2->dwLastMsgTime) ? 1 : -1;  // reverse sort order
			break;

		case SORTBY_PROTO:
			if (contact1->pce->szProto == nullptr || contact2->pce->szProto == nullptr)
				continue;
			r = GetProtoIndex(contact1->pce->szProto) - GetProtoIndex(contact2->pce->szProto);
			break;

		case SORTBY_LAST_ONLINE:
			r = int(c2->dwLastOnlineTime) - int(c1->dwLastOnlineTime); // reverse order 
			break;

		case SORTBY_RATE:
			r = contact2->bContactRate - contact1->bContactRate; // reverse order 
			break;

		default: // should never happen
			continue;
		}

		if (r != 0)
			return r;
	}
	return 0;
}

#undef SAFESTRING

INT_PTR ToggleHideOffline(WPARAM, LPARAM)
{
	return g_clistApi.pfnSetHideOffline(-1);
}

INT_PTR SetUseGroups(WPARAM wParam, LPARAM)
{
	int newVal = !(GetWindowLongPtr(g_clistApi.hwndContactTree, GWL_STYLE)&CLS_USEGROUPS);
	if (wParam != -1)
	{
		if (!newVal == (int)wParam) return 0;
		newVal = wParam;
	}
	Clist::UseGroups = newVal;
	SendMessage(g_clistApi.hwndContactTree, CLM_SETUSEGROUPS, newVal, 0);
	return 0;
}

INT_PTR ToggleSounds(WPARAM, LPARAM)
{
	db_set_b(0, "Skin", "UseSound",
		(uint8_t)!db_get_b(0, "Skin", "UseSound", SETTING_ENABLESOUNDS_DEFAULT));
	return 0;
}
