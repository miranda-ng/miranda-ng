/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)
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

LONG CDb3Mmap::GetEventCount(MCONTACT contactID)
{
	mir_cslock lck(m_csDbAccess);
	DBContact *dbc = (DBContact*)DBRead(GetContactOffset(contactID), nullptr);
	return (dbc->signature != DBCONTACT_SIGNATURE) ? -1 : dbc->eventCount;
}

MEVENT CDb3Mmap::AddEvent(MCONTACT contactID, const DBEVENTINFO *dbei)
{
	if (dbei == nullptr) return 0;
	if (dbei->timestamp == 0) return 0;

	DBEvent dbe;
	dbe.signature = DBEVENT_SIGNATURE;
	dbe.contactID = contactID; // store native or subcontact's id

	MCONTACT contactNotifyID = contactID;
	DBCachedContact *ccSub = nullptr;
	if (contactID != 0) {
		DBCachedContact *cc = m_cache->GetCachedContact(contactID);
		if (cc == nullptr)
			return 0;

		if (cc->IsSub()) {
			ccSub = cc;
			// set default sub to the event's source
			if (!(dbei->flags & DBEF_SENT))
				db_mc_setDefault(cc->parentID, contactID, false);
			contactID = cc->parentID; // and add an event to a metahistory
			if (db_mc_isEnabled())
				contactNotifyID = contactID;
		}
	}

	if (NotifyEventHooks(g_hevEventFiltered, contactNotifyID, (LPARAM)dbei))
		return 0;

	dbe.timestamp = dbei->timestamp;
	dbe.flags = dbei->flags;
	dbe.wEventType = dbei->eventType;
	dbe.cbBlob = dbei->cbBlob;
	BYTE *pBlob = dbei->pBlob;

	mir_ptr<BYTE> pCryptBlob;
	if (m_bEncrypted) {
		size_t len;
		BYTE *pResult = m_crypto->encodeBuffer(pBlob, dbe.cbBlob, &len);
		if (pResult != nullptr) {
			pCryptBlob = pBlob = pResult;
			dbe.cbBlob = (DWORD)len;
			dbe.flags |= DBEF_ENCRYPTED;
		}
	}

	mir_cslockfull lck(m_csDbAccess);

	DWORD ofsContact = GetContactOffset(contactID);
	DBContact dbc = *(DBContact*)DBRead(ofsContact, nullptr);
	if (dbc.signature != DBCONTACT_SIGNATURE)
		return 0;

	DWORD ofsNew = CreateNewSpace(offsetof(DBEvent, blob) + dbe.cbBlob);

	dbe.ofsModuleName = GetModuleNameOfs(dbei->szModule);
	// find where to put it - sort by timestamp
	if (dbc.eventCount == 0) {
		dbe.ofsPrev = dbe.ofsNext = 0;
		dbc.ofsFirstEvent = dbc.ofsLastEvent = ofsNew;
	}
	else {
		DBEvent *dbeTest = (DBEvent*)DBRead(dbc.ofsFirstEvent, nullptr);
		// Should new event be placed before first event in chain?
		if (dbe.timestamp < dbeTest->timestamp) {
			dbe.ofsPrev = 0;
			dbe.ofsNext = dbc.ofsFirstEvent;
			dbc.ofsFirstEvent = ofsNew;
			dbeTest = (DBEvent*)DBRead(dbe.ofsNext, nullptr);
			dbeTest->ofsPrev = ofsNew;
			DBWrite(dbe.ofsNext, dbeTest, sizeof(DBEvent));
		}
		else {
			// Loop through the chain, starting at the end
			DWORD ofsThis = dbc.ofsLastEvent;
			dbeTest = (DBEvent*)DBRead(ofsThis, nullptr);
			for (;;) {
				// If the new event's timesstamp is equal to or greater than the
				// current dbevent, it will be inserted after. If not, continue
				// with the previous dbevent in chain.
				if (dbe.timestamp >= dbeTest->timestamp) {
					dbe.ofsPrev = ofsThis;
					dbe.ofsNext = dbeTest->ofsNext;
					dbeTest->ofsNext = ofsNew;
					DBWrite(ofsThis, dbeTest, sizeof(DBEvent));
					if (dbe.ofsNext == 0)
						dbc.ofsLastEvent = ofsNew;
					else {
						dbeTest = (DBEvent*)DBRead(dbe.ofsNext, nullptr);
						dbeTest->ofsPrev = ofsNew;
						DBWrite(dbe.ofsNext, dbeTest, sizeof(DBEvent));
					}
					break;
				}
				ofsThis = dbeTest->ofsPrev;
				dbeTest = (DBEvent*)DBRead(ofsThis, nullptr);
			}
		}
	}
	dbc.eventCount++;

	bool neednotify;
	if (!(dbe.flags & (DBEF_READ | DBEF_SENT))) {
		if (dbe.timestamp < dbc.tsFirstUnread || dbc.tsFirstUnread == 0) {
			dbc.tsFirstUnread = dbe.timestamp;
			dbc.ofsFirstUnread = ofsNew;
		}
		neednotify = true;
	}
	else if (dbe.flags & DBEF_TEMPORARY) {
		neednotify = false;
	}
	else neednotify = m_safetyMode;

	if (ccSub != nullptr) {
		DBContact *pSub = (DBContact*)DBRead(ccSub->dwOfsContact, nullptr);
		pSub->eventCount++;
	}

	DBWrite(ofsContact, &dbc, sizeof(DBContact));
	DBWrite(ofsNew, &dbe, offsetof(DBEvent, blob));
	DBWrite(ofsNew + offsetof(DBEvent, blob), pBlob, dbe.cbBlob);
	DBFlush(0);
	lck.unlock();

	log1("add event @ %08x", ofsNew);

	// Notify only in safe mode or on really new events
	if (neednotify)
		NotifyEventHooks(g_hevEventAdded, contactNotifyID, (LPARAM)ofsNew);

	return (MEVENT)ofsNew;
}

BOOL CDb3Mmap::DeleteEvent(MEVENT hDbEvent)
{
	mir_cslockfull lck(m_csDbAccess);
	DBEvent dbe = *(DBEvent*)DBRead((DWORD)hDbEvent, nullptr);
	if (dbe.signature != DBEVENT_SIGNATURE)
		return 1;

	DBCachedContact *cc;
	if (dbe.contactID) {
		if ((cc = m_cache->GetCachedContact(dbe.contactID)) == nullptr)
			return 2;
		if (cc->IsSub())
			if ((cc = m_cache->GetCachedContact(cc->parentID)) == nullptr)
				return 3;
	}
	else cc = nullptr;

	DWORD ofsContact = (cc) ? cc->dwOfsContact : m_dbHeader.ofsUser;
	DBContact dbc = *(DBContact *)DBRead(ofsContact, nullptr);
	if (dbc.signature != DBCONTACT_SIGNATURE)
		return 1;

	lck.unlock();
	log1("delete event @ %08x", hContact);

	// call notifier while outside mutex
	NotifyEventHooks(g_hevEventDeleted, dbe.contactID, (LPARAM)hDbEvent);

	// get back in
	lck.lock();
	dbc = *(DBContact*)DBRead(ofsContact, nullptr);
	dbe = *(DBEvent*)DBRead((DWORD)hDbEvent, nullptr);

	// check if this was the first unread, if so, recalc the first unread
	if (dbc.ofsFirstUnread == (DWORD)hDbEvent) {
		for (DBEvent *dbeNext = &dbe;;) {
			if (dbeNext->ofsNext == 0) {
				dbc.ofsFirstUnread = 0;
				dbc.tsFirstUnread = 0;
				break;
			}
			DWORD ofsThis = dbeNext->ofsNext;
			dbeNext = (DBEvent*)DBRead(ofsThis, nullptr);
			if (!dbeNext->markedRead()) {
				dbc.ofsFirstUnread = ofsThis;
				dbc.tsFirstUnread = dbeNext->timestamp;
				break;
			}
		}
	}

	// get previous and next events in chain and change offsets
	if (dbe.ofsPrev == 0) {
		if (dbe.ofsNext == 0)
			dbc.ofsFirstEvent = dbc.ofsLastEvent = 0;
		else {
			DBEvent *dbeNext = (DBEvent*)DBRead(dbe.ofsNext, nullptr);
			dbeNext->ofsPrev = 0;
			DBWrite(dbe.ofsNext, dbeNext, sizeof(DBEvent));
			dbc.ofsFirstEvent = dbe.ofsNext;
		}
	}
	else {
		if (dbe.ofsNext == 0) {
			DBEvent *dbePrev = (DBEvent*)DBRead(dbe.ofsPrev, nullptr);
			dbePrev->ofsNext = 0;
			DBWrite(dbe.ofsPrev, dbePrev, sizeof(DBEvent));
			dbc.ofsLastEvent = dbe.ofsPrev;
		}
		else {
			DBEvent *dbePrev = (DBEvent*)DBRead(dbe.ofsPrev, nullptr);
			dbePrev->ofsNext = dbe.ofsNext;
			DBWrite(dbe.ofsPrev, dbePrev, sizeof(DBEvent));

			DBEvent *dbeNext = (DBEvent*)DBRead(dbe.ofsNext, nullptr);
			dbeNext->ofsPrev = dbe.ofsPrev;
			DBWrite(dbe.ofsNext, dbeNext, sizeof(DBEvent));
		}
	}

	// decrement event count
	dbc.eventCount--;
	DBWrite(ofsContact, &dbc, sizeof(DBContact));

	// delete event
	DeleteSpace((DWORD)hDbEvent, offsetof(DBEvent, blob) + dbe.cbBlob);

	// also update a sub
	if (cc && dbe.contactID != cc->contactID) {
		DBContact *pSub = (DBContact*)DBRead(GetContactOffset(dbe.contactID), nullptr);
		if (pSub->eventCount > 0)
			pSub->eventCount--;
	}

	DBFlush(0);
	return 0;
}

BOOL CDb3Mmap::EditEvent(MCONTACT, MEVENT, const DBEVENTINFO*)
{
	return 1;
}

LONG CDb3Mmap::GetBlobSize(MEVENT hDbEvent)
{
	mir_cslock lck(m_csDbAccess);
	DBEvent *dbe = AdaptEvent((DWORD)hDbEvent, 0);
	return (dbe->signature != DBEVENT_SIGNATURE) ? -1 : dbe->cbBlob;
}

BOOL CDb3Mmap::GetEvent(MEVENT hDbEvent, DBEVENTINFO *dbei)
{
	if (dbei == nullptr) return 1;
	if (dbei->cbBlob > 0 && dbei->pBlob == nullptr) {
		dbei->cbBlob = 0;
		return 1;
	}

	mir_cslock lck(m_csDbAccess);
	DBEvent *dbe = AdaptEvent((DWORD)hDbEvent, 0);
	if (dbe->signature != DBEVENT_SIGNATURE)
		return 1;

	dbei->szModule = GetModuleNameByOfs(dbe->ofsModuleName);
	dbei->timestamp = dbe->timestamp;
	dbei->flags = dbe->flags;
	dbei->eventType = dbe->wEventType;

	DWORD cbBlob = dbe->cbBlob;
	size_t bytesToCopy = cbBlob;
	if (dbei->cbBlob == -1)
		dbei->pBlob = (PBYTE)mir_calloc(cbBlob + 2);
	else if (dbei->cbBlob < cbBlob)
		bytesToCopy = dbei->cbBlob;

	dbei->cbBlob = dbe->cbBlob;
	if (bytesToCopy && dbei->pBlob) {
		BYTE *pSrc;
		if (m_dbHeader.version >= DB_095_1_VERSION)
			pSrc = DBRead(DWORD(hDbEvent) + offsetof(DBEvent, blob), nullptr);
		else
			pSrc = DBRead(DWORD(hDbEvent) + offsetof(DBEvent_094, blob), nullptr);
		if (dbe->flags & DBEF_ENCRYPTED) {
			dbei->flags &= ~DBEF_ENCRYPTED;
			size_t len;
			BYTE* pBlob = (BYTE*)m_crypto->decodeBuffer(pSrc, dbe->cbBlob, &len);
			if (pBlob == nullptr)
				return 1;

			memcpy(dbei->pBlob, pBlob, bytesToCopy);
			if (bytesToCopy > (int)len)
				memset(dbei->pBlob + len, 0, bytesToCopy - len);
			mir_free(pBlob);
		}
		else memcpy(dbei->pBlob, pSrc, bytesToCopy);
	}
	return 0;
}

BOOL CDb3Mmap::MarkEventRead(MCONTACT contactID, MEVENT hDbEvent)
{
	DBCachedContact *cc;
	if (contactID) {
		if ((cc = m_cache->GetCachedContact(contactID)) == nullptr)
			return -1;
		if (cc->IsSub())
			if ((cc = m_cache->GetCachedContact(cc->parentID)) == nullptr)
				return -1;
	}
	else cc = nullptr;

	mir_cslockfull lck(m_csDbAccess);
	DWORD ofsContact = (cc) ? cc->dwOfsContact : m_dbHeader.ofsUser;
	DBContact dbc = *(DBContact*)DBRead(ofsContact, nullptr);
	DBEvent *dbe = (DBEvent*)DBRead((DWORD)hDbEvent, nullptr);
	if (dbe->signature != DBEVENT_SIGNATURE || dbc.signature != DBCONTACT_SIGNATURE)
		return -1;

	if (dbe->markedRead())
		return dbe->flags;

	// log1("mark read @ %08x", hContact);
	dbe->flags |= DBEF_READ;
	DBWrite((DWORD)hDbEvent, dbe, sizeof(DBEvent));
	BOOL ret = dbe->flags;
	if (dbc.ofsFirstUnread == (DWORD)hDbEvent) {
		for (;;) {
			if (dbe->ofsNext == 0) {
				dbc.ofsFirstUnread = 0;
				dbc.tsFirstUnread = 0;
				break;
			}
			DWORD ofsThis = dbe->ofsNext;
			dbe = (DBEvent*)DBRead(ofsThis, nullptr);
			if (!dbe->markedRead()) {
				dbc.ofsFirstUnread = ofsThis;
				dbc.tsFirstUnread = dbe->timestamp;
				break;
			}
		}
	}
	DBWrite(ofsContact, &dbc, sizeof(DBContact));
	DBFlush(0);

	lck.unlock();
	NotifyEventHooks(g_hevMarkedRead, contactID, (LPARAM)hDbEvent);
	return ret;
}

MCONTACT CDb3Mmap::GetEventContact(MEVENT hDbEvent)
{
	mir_cslock lck(m_csDbAccess);
	DBEvent *dbe = AdaptEvent((DWORD)hDbEvent, INVALID_CONTACT_ID);
	return (dbe->signature != DBEVENT_SIGNATURE) ? INVALID_CONTACT_ID : dbe->contactID;
}

MEVENT CDb3Mmap::FindFirstEvent(MCONTACT contactID)
{
	DBCachedContact *cc;
	DWORD ofsContact = GetContactOffset(contactID, &cc);

	mir_cslock lck(m_csDbAccess);
	DBContact *dbc = (DBContact*)DBRead(ofsContact, nullptr);
	if (dbc->signature != DBCONTACT_SIGNATURE)
		return 0;
	if (!cc || !cc->IsSub())
		return MEVENT(dbc->ofsFirstEvent);

	if ((cc = m_cache->GetCachedContact(cc->parentID)) == nullptr)
		return 0;
	dbc = (DBContact*)DBRead(cc->dwOfsContact, nullptr);
	if (dbc->signature != DBCONTACT_SIGNATURE)
		return 0;

	for (DWORD dwOffset = dbc->ofsFirstEvent; dwOffset != 0;) {
		DBEvent *dbe = AdaptEvent(dwOffset, contactID);
		if (dbe->signature != DBEVENT_SIGNATURE)
			return 0;
		if (dbe->contactID == contactID)
			return MEVENT(dwOffset);
		dwOffset = dbe->ofsNext;
	}
	return 0;
}

MEVENT CDb3Mmap::FindFirstUnreadEvent(MCONTACT contactID)
{
	DBCachedContact *cc;
	DWORD ofsContact = GetContactOffset(contactID, &cc);

	mir_cslock lck(m_csDbAccess);
	DBContact *dbc = (DBContact*)DBRead(ofsContact, nullptr);
	if (dbc->signature != DBCONTACT_SIGNATURE)
		return 0;
	if (!cc || !cc->IsSub())
		return MEVENT(dbc->ofsFirstUnread);

	if ((cc = m_cache->GetCachedContact(cc->parentID)) == nullptr)
		return 0;
	dbc = (DBContact*)DBRead(cc->dwOfsContact, nullptr);
	if (dbc->signature != DBCONTACT_SIGNATURE)
		return 0;

	for (DWORD dwOffset = dbc->ofsFirstUnread; dwOffset != 0;) {
		DBEvent *dbe = AdaptEvent(dwOffset, contactID);
		if (dbe->signature != DBEVENT_SIGNATURE)
			return 0;
		if (dbe->contactID == contactID && !dbe->markedRead())
			return MEVENT(dwOffset);
		dwOffset = dbe->ofsNext;
	}
	return 0;
}

MEVENT CDb3Mmap::FindLastEvent(MCONTACT contactID)
{
	DBCachedContact *cc;
	DWORD ofsContact = GetContactOffset(contactID, &cc);

	mir_cslock lck(m_csDbAccess);
	DBContact *dbc = (DBContact*)DBRead(ofsContact, nullptr);
	if (dbc->signature != DBCONTACT_SIGNATURE)
		return 0;
	if (!cc || !cc->IsSub())
		return MEVENT(dbc->ofsLastEvent);

	if ((cc = m_cache->GetCachedContact(cc->parentID)) == nullptr)
		return 0;
	dbc = (DBContact*)DBRead(cc->dwOfsContact, nullptr);
	if (dbc->signature != DBCONTACT_SIGNATURE)
		return 0;

	for (DWORD dwOffset = dbc->ofsLastEvent; dwOffset != 0;) {
		DBEvent *dbe = AdaptEvent(dwOffset, contactID);
		if (dbe->signature != DBEVENT_SIGNATURE)
			return 0;
		if (dbe->contactID == contactID)
			return MEVENT(dwOffset);
		dwOffset = dbe->ofsPrev;
	}
	return 0;
}

MEVENT CDb3Mmap::FindNextEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	DBCachedContact *cc = (contactID) ? m_cache->GetCachedContact(contactID) : nullptr;

	mir_cslock lck(m_csDbAccess);
	DBEvent *dbe = AdaptEvent((DWORD)hDbEvent, contactID);
	if (dbe->signature != DBEVENT_SIGNATURE)
		return 0;
	if (!cc || !cc->IsSub())
		return MEVENT(dbe->ofsNext);

	for (DWORD dwOffset = dbe->ofsNext; dwOffset != 0;) {
		dbe = AdaptEvent(dwOffset, contactID);
		if (dbe->signature != DBEVENT_SIGNATURE)
			return 0;
		if (dbe->contactID == contactID)
			return MEVENT(dwOffset);
		dwOffset = dbe->ofsNext;
	}
	return 0;
}

MEVENT CDb3Mmap::FindPrevEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	DBCachedContact *cc = (contactID) ? m_cache->GetCachedContact(contactID) : nullptr;

	mir_cslock lck(m_csDbAccess);
	DBEvent *dbe = AdaptEvent((DWORD)hDbEvent, contactID);
	if (dbe->signature != DBEVENT_SIGNATURE)
		return 0;
	if (!cc || !cc->IsSub())
		return MEVENT(dbe->ofsPrev);

	for (DWORD dwOffset = dbe->ofsPrev; dwOffset != 0;) {
		dbe = AdaptEvent(dwOffset, contactID);
		if (dbe->signature != DBEVENT_SIGNATURE)
			return 0;
		if (dbe->contactID == contactID)
			return MEVENT(dwOffset);
		dwOffset = dbe->ofsPrev;
	}
	return 0;
}

DBEvent* CDb3Mmap::AdaptEvent(DWORD ofs, DWORD dwContactID)
{
	if (m_dbHeader.version >= DB_095_1_VERSION)
		return (DBEvent*)DBRead(ofs, nullptr);

	DBEvent_094 *pOldEvent = (DBEvent_094*)DBRead(ofs, nullptr);
	m_tmpEvent.signature = pOldEvent->signature;
	m_tmpEvent.contactID = dwContactID;
	memcpy(&m_tmpEvent.ofsPrev, &pOldEvent->ofsPrev, sizeof(DBEvent_094) - sizeof(DWORD));
	return &m_tmpEvent;
}

/////////////////////////////////////////////////////////////////////////////////////////
// low-level history cleaner

int CDb3Mmap::WipeContactHistory(DBContact *dbc)
{
	// drop subContact's history if any
	for (DWORD dwOffset = dbc->ofsFirstEvent; dwOffset != 0;) {
		DBEvent *pev = (DBEvent*)DBRead(dwOffset, nullptr);
		if (pev->signature != DBEVENT_SIGNATURE) // broken chain, don't touch it
			return 2;

		DWORD dwNext = pev->ofsNext;
		DeleteSpace(dwOffset, offsetof(DBEvent, blob) + pev->cbBlob);
		dwOffset = dwNext;
	}
	dbc->eventCount = 0; dbc->ofsFirstEvent = dbc->ofsLastEvent = dbc->ofsFirstUnread = dbc->tsFirstUnread = 0;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MEVENT CDb3Mmap::GetEventById(LPCSTR, LPCSTR)
{
	return 0;
}
