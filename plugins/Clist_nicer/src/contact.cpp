/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-03 Miranda ICQ/IM project,
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
statusModeOrder[] =
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

static int GetContactStatus(MCONTACT hContact)
{
	char *szProto;

	szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr)
		return ID_STATUS_OFFLINE;
	return db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
}

int __forceinline GetStatusModeOrdering(int statusMode)
{
	for (int i = 0; i < sizeof(statusModeOrder) / sizeof(statusModeOrder[0]); i++)
		if (statusModeOrder[i].status == statusMode)
			return statusModeOrder[i].order;

	return 1000;
}

int mf_updatethread_running = TRUE;
HANDLE hThreadMFUpdate = nullptr;

static void MF_CalcFrequency(MCONTACT hContact, uint32_t dwCutoffDays, int doSleep)
{
	uint32_t  curTime = time(0);
	uint32_t  frequency, eventCount = 0;

	DBEVENTINFO dbei = {};
	DB::ECPTR cursor(DB::EventsRev(hContact));
	while (MEVENT hEvent = cursor.FetchNext()) {
		db_event_get(hEvent, &dbei);

		// record time of last event
		if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT))
			eventCount++;

		if (eventCount >= 100 || dbei.timestamp < curTime - (dwCutoffDays * 86400))
			break;

		if (doSleep && mf_updatethread_running == FALSE)
			return;
		if (doSleep)
			Sleep(100);
	}

	if (eventCount == 0) {
		frequency = 0x7fffffff;
		g_plugin.setDword(hContact, "mf_firstEvent", curTime - (dwCutoffDays * 86400));
	}
	else {
		frequency = (curTime - dbei.timestamp) / eventCount;
		g_plugin.setDword(hContact, "mf_firstEvent", dbei.timestamp);
	}

	g_plugin.setDword(hContact, "mf_freq", frequency);
	g_plugin.setDword(hContact, "mf_count", eventCount);
}

extern wchar_t g_ptszEventName[];

void MF_UpdateThread(LPVOID)
{
	Thread_SetName("CList_nicer: MF_UpdateThread");

	HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, g_ptszEventName);

	WaitForSingleObject(hEvent, 20000);
	ResetEvent(hEvent);

	while (mf_updatethread_running) {
		for (auto &hContact : Contacts()) {
			MF_CalcFrequency(hContact, 50, 1);
			if (mf_updatethread_running)
				WaitForSingleObject(hEvent, 5000);
			ResetEvent(hEvent);
		}
		if (mf_updatethread_running)
			WaitForSingleObject(hEvent, 1000000);
		ResetEvent(hEvent);
	}
	CloseHandle(hEvent);
}

void MF_InitCheck(void)
{
	uint8_t bMsgFrequency = g_plugin.getByte("fhistdata", 0);
	if (!bMsgFrequency) {
		for (auto &hContact : Contacts())
			MF_CalcFrequency(hContact, 100, 0);
		g_plugin.setByte("fhistdata", 1);
	}
}

uint32_t INTSORT_GetLastMsgTime(MCONTACT hContact)
{
	DB::ECPTR cursor(DB::EventsRev(hContact));
	while (MEVENT hDbEvent = cursor.FetchNext()) {
		DBEVENTINFO dbei = {};
		db_event_get(hDbEvent, &dbei);
		if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT))
			return dbei.timestamp;
	}
	return 0;
}

int __forceinline GetProtoIndex(char * szName)
{
	if (!szName)
		return -1;

	PROTOACCOUNT *pa = Proto_GetAccount(szName);
	return (pa == nullptr) ? -1 : pa->iOrder;
}

int __forceinline INTSORT_CompareContacts(const ClcContact* c1, const ClcContact* c2, UINT bywhat)
{
	wchar_t *namea, *nameb;
	int statusa, statusb;
	int rc;

	if (c1 == nullptr || c2 == nullptr)
		return 0;

	char *szProto1 = c1->pce->szProto;
	char *szProto2 = c2->pce->szProto;
	statusa = c1->wStatus;
	statusb = c2->wStatus;
	// make sure, sticky contacts are always at the beginning of the group/list

	if ((c1->flags & CONTACTF_STICKY) != (c2->flags & CONTACTF_STICKY))
		return 2 * (c2->flags & CONTACTF_STICKY) - 1;

	if (bywhat == SORTBY_PRIOCONTACTS) {
		if ((cfg::clcdat->exStyle & CLS_EX_DIVIDERONOFF) && ((c1->flags & CONTACTF_ONLINE) != (c2->flags & CONTACTF_ONLINE)))
			return 0;
		if ((c1->flags & CONTACTF_PRIORITY) != (c2->flags & CONTACTF_PRIORITY))
			return 2 * (c2->flags & CONTACTF_PRIORITY) - 1;
		else
			return 0;
	}

	if (bywhat == SORTBY_STATUS) {
		int ordera, orderb;

		ordera = GetStatusModeOrdering(statusa);
		orderb = GetStatusModeOrdering(statusb);
		if (ordera != orderb)
			return ordera - orderb;
		else
			return 0;
	}

	// separate contacts treated as "offline"
	if (!cfg::dat.bDontSeparateOffline && ((statusa == ID_STATUS_OFFLINE) != (statusb == ID_STATUS_OFFLINE)))
		return 2 * (statusa == ID_STATUS_OFFLINE) - 1;

	switch (bywhat) {
	case SORTBY_NAME:
		namea = (wchar_t *)c1->szText;
		nameb = (wchar_t *)c2->szText;
		return CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, namea, -1, nameb, -1) - 2;

	case SORTBY_LASTMSG:
		if (c1->pExtra && c2->pExtra)
			return c2->pExtra->dwLastMsgTime - c1->pExtra->dwLastMsgTime;
		else {
			uint32_t timestamp1 = INTSORT_GetLastMsgTime(c1->hContact);
			uint32_t timestamp2 = INTSORT_GetLastMsgTime(c2->hContact);
			return timestamp2 - timestamp1;
		}

	case SORTBY_FREQUENCY:
		if (c1->pExtra && c2->pExtra)
			return c1->pExtra->msgFrequency - c2->pExtra->msgFrequency;
		break;

	case SORTBY_PROTO:
		if (c1->bIsMeta)
			szProto1 = c1->metaProto ? c1->metaProto : c1->pce->szProto;
		if (c2->bIsMeta)
			szProto2 = c2->metaProto ? c2->metaProto : c2->pce->szProto;

		rc = GetProtoIndex(szProto1) - GetProtoIndex(szProto2);

		if (rc != 0 && (szProto1 != nullptr && szProto2 != nullptr))
			return rc;
	}
	return 0;
}

int CompareContacts(const ClcContact* c1, const ClcContact* c2)
{
	int result = INTSORT_CompareContacts(c1, c2, SORTBY_PRIOCONTACTS);
	if (result)
		return result;

	for (int i = 0; i <= 2; i++) {
		if (cfg::dat.sortOrder[i]) {
			result = INTSORT_CompareContacts(c1, c2, cfg::dat.sortOrder[i]);
			if (result != 0)
				return result;
		}
	}
	return 0;
}

#undef SAFESTRING

int SetHideOffline(int iValue)
{
	int newVal = coreCli.pfnSetHideOffline(iValue);

	SetButtonStates();
	ClcSetButtonState(IDC_TBHIDEOFFLINE, newVal);
	return newVal;
}
