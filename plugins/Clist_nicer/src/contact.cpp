/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
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
	{ ID_STATUS_INVISIBLE, 20 },
	{ ID_STATUS_ONTHEPHONE, 150 },
	{ ID_STATUS_OUTTOLUNCH, 425 }
};

static int GetContactStatus(MCONTACT hContact)
{
	char *szProto;

	szProto = GetContactProto(hContact);
	if (szProto == NULL)
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
HANDLE hThreadMFUpdate = 0;

static void MF_CalcFrequency(MCONTACT hContact, DWORD dwCutoffDays, int doSleep)
{
	DWORD  curTime = time(NULL);
	DWORD  frequency, eventCount;
	MEVENT hEvent = db_event_last(hContact);

	eventCount = 0;

	DBEVENTINFO dbei = {};
	while (hEvent) {
		db_event_get(hEvent, &dbei);

		if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT)) { // record time of last event
			eventCount++;
		}
		if (eventCount >= 100 || dbei.timestamp < curTime - (dwCutoffDays * 86400))
			break;
		hEvent = db_event_prev(hContact, hEvent);
		if (doSleep && mf_updatethread_running == FALSE)
			return;
		if (doSleep)
			Sleep(100);
	}

	if (eventCount == 0) {
		frequency = 0x7fffffff;
		db_set_dw(hContact, "CList", "mf_firstEvent", curTime - (dwCutoffDays * 86400));
	}
	else {
		frequency = (curTime - dbei.timestamp) / eventCount;
		db_set_dw(hContact, "CList", "mf_firstEvent", dbei.timestamp);
	}

	db_set_dw(hContact, "CList", "mf_freq", frequency);
	db_set_dw(hContact, "CList", "mf_count", eventCount);
}

extern wchar_t g_ptszEventName[];

void MF_UpdateThread(LPVOID)
{
	Thread_SetName("CList_nicer: MF_UpdateThread");

	HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, g_ptszEventName);

	WaitForSingleObject(hEvent, 20000);
	ResetEvent(hEvent);

	while (mf_updatethread_running) {
		for (MCONTACT hContact = db_find_first(); hContact && mf_updatethread_running; hContact = db_find_next(hContact)) {
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
	BYTE bMsgFrequency = db_get_b(NULL, "CList", "fhistdata", 0);
	if (!bMsgFrequency) {
		for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
			MF_CalcFrequency(hContact, 100, 0);
		db_set_b(NULL, "CList", "fhistdata", 1);
	}
}

DWORD INTSORT_GetLastMsgTime(MCONTACT hContact)
{
	for (MEVENT hDbEvent = db_event_last(hContact); hDbEvent; hDbEvent = db_event_prev(hContact, hDbEvent)) {
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
	return (pa == NULL) ? -1 : pa->iOrder;
}

int __forceinline INTSORT_CompareContacts(const ClcContact* c1, const ClcContact* c2, UINT bywhat)
{
	wchar_t *namea, *nameb;
	int statusa, statusb;
	char *szProto1, *szProto2;
	int rc;

	if (c1 == 0 || c2 == 0)
		return 0;

	szProto1 = c1->proto;
	szProto2 = c2->proto;
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
			DWORD timestamp1 = INTSORT_GetLastMsgTime(c1->hContact);
			DWORD timestamp2 = INTSORT_GetLastMsgTime(c2->hContact);
			return timestamp2 - timestamp1;
		}

	case SORTBY_FREQUENCY:
		if (c1->pExtra && c2->pExtra)
			return c1->pExtra->msgFrequency - c2->pExtra->msgFrequency;
		break;

	case SORTBY_PROTO:
		if (c1->bIsMeta)
			szProto1 = c1->metaProto ? c1->metaProto : c1->proto;
		if (c2->bIsMeta)
			szProto2 = c2->metaProto ? c2->metaProto : c2->proto;

		rc = GetProtoIndex(szProto1) - GetProtoIndex(szProto2);

		if (rc != 0 && (szProto1 != NULL && szProto2 != NULL))
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
