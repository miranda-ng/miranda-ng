/*
Miranda IM History Sweeper Light plugin
Copyright (C) 2002-2003  Sergey V. Gershovich
Copyright (C) 2006-2009  Boris Krasnovskiy
Copyright (C) 2010, 2011 tico-tico

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

typedef struct
{
	time_t time;
	int keep;
} CriteriaStruct;

// build criteria
time_t BuildCriteria(int dwti)
{
	time_t tim = time(0);

	switch (dwti) {
	case 0:
		tim -= 1 * 24 * 3600;
		break;
	case 1:
		tim -= 3 * 24 * 3600;
		break;
	case 2:
		tim -= 7 * 24 * 3600;
		break;
	case 3:
		tim -= 14 * 24 * 3600;
		break;
	case 4:
		tim -= 30 * 24 * 3600;
		break;
	case 5:
		tim -= 90 * 24 * 3600;
		break;
	case 6:
		tim -= 180 * 24 * 3600;
		break;
	default:
		tim -= 365 * 24 * 3600;
	}
	return tim;
}

// keep criteria
int KeepCriteria(int dwti)
{
	switch (dwti) {
	case 0:
		return 1;
	case 1:
		return 2;
	case 2:
		return 5;
	case 3:
		return 10;
	case 4:
		return 20;
	default:
		return 50;
	}
}

struct BEventData
{
	MEVENT hDBEvent;
	uint32_t CRC32;
	uint32_t Timestamp;
};

static int CompareBookmarks(const void* p1, const void* p2)
{
	return (uint8_t*)((BEventData*)p1)->hDBEvent - (uint8_t*)((BEventData*)p2)->hDBEvent;
}

static void GetBookmarks(MCONTACT hContact, BEventData** books, size_t* bookcnt)
{
	*books = nullptr;
	*bookcnt = 0;

	DBVARIANT dbv;
	if (db_get(hContact, "HistoryPlusPlus", "Bookmarks", &dbv) == 0) {
		if (dbv.cpbVal > 2 && *(uint16_t*)dbv.pbVal >= sizeof(BEventData)) {
			size_t recSize = *(uint16_t*)dbv.pbVal;
			size_t numb = (dbv.cpbVal - sizeof(uint16_t)) / recSize;

			BEventData* tbooks = (BEventData*)mir_alloc(sizeof(BEventData)*numb);
			size_t tbookcnt = 0;

			size_t i;
			uint8_t* evs = dbv.pbVal + sizeof(uint16_t);

			for (i = 0; i < numb; ++i) {
				tbooks[tbookcnt++] = *(BEventData*)evs;
				evs += recSize;
			}
			if (tbookcnt != 0) {
				qsort(tbooks, tbookcnt, sizeof(BEventData), CompareBookmarks);
				*bookcnt = tbookcnt;
				*books = tbooks;
			}
			else mir_free(tbooks);
		}

		db_free(&dbv);
	}
}

//Sweep history from specified contact
void SweepHistoryFromContact(MCONTACT hContact, CriteriaStruct Criteria, BOOL keepUnread)
{
	int lPolicy;
	if (hContact == NULL) // for system history
		lPolicy = g_plugin.getByte("SweepSHistory", 0);
	else							// for contact history (or "SweepHistory" - default action)
		lPolicy = g_plugin.getByte(hContact, "SweepHistory", g_plugin.getByte("SweepHistory", 0));

	if (lPolicy == 0) // nothing to do
		return;

	int eventsCnt = db_event_count(hContact);
	if (eventsCnt == 0)
		return;

	BOOL unsafe = g_plugin.getByte("UnsafeMode", 0);
	BEventData *books, *item, ev = { 0 };
	size_t bookcnt, btshift;

	SetCursor(LoadCursor(nullptr, IDC_WAIT));

	// switch off SAFETYMODE if necessary
	if (unsafe)
		db_set_safety_mode(FALSE);

	GetBookmarks(hContact, &books, &bookcnt);

	// Get first event
	DB::ECPTR pCursor(DB::Events(hContact));
	while (MEVENT hDBEvent = pCursor.FetchNext()) {
		DBEVENTINFO dbei = {};
		db_event_get(hDBEvent, &dbei);

		// should we stop processing?
		// lPolicy == 1 - for time criterion, lPolicy == 2 - keep N last events, lPolicy == 3 - delete all events
		if ((lPolicy == 1 && (unsigned)Criteria.time < dbei.timestamp) || (lPolicy == 2 && Criteria.keep > --eventsCnt)) break;

		bool doDelete = true;
		if (!(dbei.flags & (DBEF_SENT | DBEF_READ)) && keepUnread)
			doDelete = false;	// keep unread events

		if (bookcnt != 0) { // keep bookmarks
			ev.hDBEvent = hDBEvent;
			item = (BEventData*)bsearch(&ev, books, bookcnt, sizeof(BEventData), CompareBookmarks);
			if (item != nullptr && item->Timestamp == dbei.timestamp) {
				doDelete = FALSE;
				btshift = (--bookcnt - (item - books)) * sizeof(BEventData);
				if (btshift)
					memmove(item, item + 1, btshift);
			}
		}

		if (doDelete)
			pCursor.DeleteEvent();
	}

	mir_free(books);

	// switch ON safety mode as fast as we can to avoid  DB corruption
	if (unsafe)
		db_set_safety_mode(TRUE);

	SetCursor(LoadCursor(nullptr, IDC_ARROW));
}

// shutdown action
void ShutdownAction(void)
{
	CriteriaStruct Criteria;
	Criteria.keep = KeepCriteria(g_plugin.getByte("StartupShutdownKeep"));
	Criteria.time = BuildCriteria(g_plugin.getByte("StartupShutdownOlder"));

	SweepHistoryFromContact(NULL, Criteria, FALSE);				// sweep system history, keepunread==0

	for (auto &hContact : Contacts())
		SweepHistoryFromContact(hContact, Criteria, TRUE);		// sweep contact history, keepunread==1
}

int OnWindowEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *msgEvData = (MessageWindowEventData*)lParam;
	switch (msgEvData->uType) {
	case MSG_WINDOW_EVT_OPENING:
		g_hWindows.insert(PVOID(msgEvData->hContact));
		SetSrmmIcon(msgEvData->hContact);
		break;

	case MSG_WINDOW_EVT_CLOSE:
		if (g_plugin.getByte("SweepOnClose", 0)) {
			CriteriaStruct Criteria;

			Criteria.keep = KeepCriteria(g_plugin.getByte("StartupShutdownKeep"));
			Criteria.time = BuildCriteria(g_plugin.getByte("StartupShutdownOlder"));

			SweepHistoryFromContact(msgEvData->hContact, Criteria, TRUE);
		}

		for (auto &it : g_hWindows.rev_iter())
			if (it == PVOID(msgEvData->hContact))
				g_hWindows.removeItem(&it);
		break;
	}

	return 0;
}

void SetSrmmIcon(MCONTACT hContact)
{
	int sweep = g_plugin.getByte(hContact, "SweepHistory");

	for (int i = 0; i < 4; i++)
		Srmm_SetIconFlags(hContact, MODULENAME, i, (sweep == i) ? 0 : MBF_HIDDEN);
}
