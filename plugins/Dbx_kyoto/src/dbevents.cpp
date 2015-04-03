/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

STDMETHODIMP_(LONG) CDbxKyoto::GetEventCount(MCONTACT contactID)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	return (cc == NULL) ? 0 : cc->dbc.dwEventCount;
}

STDMETHODIMP_(MEVENT) CDbxKyoto::AddEvent(MCONTACT contactID, DBEVENTINFO *dbei)
{
	if (dbei == NULL || dbei->cbSize != sizeof(DBEVENTINFO)) return 0;
	if (dbei->timestamp == 0) return 0;

	DBEvent dbe;
	dbe.dwSignature = DBEVENT_SIGNATURE;
	dbe.contactID = contactID; // store native or subcontact's id
	dbe.ofsModuleName = GetModuleNameOfs(dbei->szModule);
	dbe.timestamp = dbei->timestamp;
	dbe.flags = dbei->flags;
	dbe.wEventType = dbei->eventType;
	dbe.cbBlob = dbei->cbBlob;
	BYTE *pBlob = dbei->pBlob;

	MCONTACT contactNotifyID = contactID;
	DBCachedContact *cc, *ccSub = NULL;
	if ((cc = m_cache->GetCachedContact(contactID)) == NULL)
		return 0;

	if (cc->IsSub()) {
		ccSub = cc;
		if ((cc = m_cache->GetCachedContact(cc->parentID)) == NULL)
			return 0;

		// set default sub to the event's source
		if (!(dbei->flags & DBEF_SENT))
			db_mc_setDefault(cc->contactID, contactID, false);
		contactID = cc->contactID; // and add an event to a metahistory
		if (db_mc_isEnabled())
			contactNotifyID = contactID;
	}

	if (m_safetyMode)
		if (NotifyEventHooks(hEventFilterAddedEvent, contactNotifyID, (LPARAM)dbei))
			return NULL;

	mir_ptr<BYTE> pCryptBlob;
	if (m_bEncrypted) {
		size_t len;
		BYTE *pResult = m_crypto->encodeBuffer(pBlob, dbe.cbBlob, &len);
		if (pResult != NULL) {
			pCryptBlob = pBlob = pResult;
			dbe.cbBlob = (DWORD)len;
			dbe.flags |= DBEF_ENCRYPTED;
		}
	}

	DWORD dwEventId = ++m_dwMaxEventId;

	BYTE *pDest = (BYTE*)_alloca(sizeof(DBEvent) + dbe.cbBlob);
	memcpy(pDest, &dbe, sizeof(DBEvent));
	memcpy(pDest + sizeof(DBEvent), pBlob, dbe.cbBlob);
	m_dbEvents.set((LPCSTR)&dwEventId, sizeof(int), (LPCSTR)pDest, sizeof(DBEvent) + dbe.cbBlob);

	// add a sorting key
	DBEventSortingKey key2 = { contactID, dbe.timestamp, dwEventId };
	m_dbEventsSort.set((LPCSTR)&key2, sizeof(key2), "", 1);

	cc->Advance(dwEventId, dbe);
	m_dbContacts.set((LPCSTR)&contactID, sizeof(int), (LPCSTR)&cc->dbc, sizeof(DBContact));

	// insert an event into a sub's history too
	if (ccSub != NULL) {
		key2.dwContactId = ccSub->contactID;
		m_dbEventsSort.set((LPCSTR)&key2, sizeof(key2), "", 1);

		ccSub->Advance(dwEventId, dbe);
		m_dbContacts.set((LPCSTR)&ccSub->contactID, sizeof(int), (LPCSTR)&ccSub->dbc, sizeof(DBContact));
	}

	// Notify only in safe mode or on really new events
	if (m_safetyMode)
		NotifyEventHooks(hEventAddedEvent, contactNotifyID, dwEventId);

	return dwEventId;
}

STDMETHODIMP_(BOOL) CDbxKyoto::DeleteEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	if (hDbEvent == 0) return INVALID_CONTACT_ID;

	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	if (cc == NULL || cc->dbc.dwEventCount == 0)
		return 1;

	DBEvent dbe;
	if (-1 == m_dbEvents.get((LPCSTR)&hDbEvent, sizeof(MEVENT), (LPSTR)&dbe, sizeof(dbe)))
		return 1;

	DWORD dwSavedContact = dbe.contactID;
	DBEventSortingKey key2 = { contactID, dbe.timestamp, hDbEvent };
	m_dbEvents.remove((LPCSTR)&hDbEvent, sizeof(MEVENT));

	// remove a sorting key
	m_dbEventsSort.remove((LPCSTR)&key2, sizeof(key2));

	// remove a sub's history entry too
	if (contactID != dwSavedContact) {
		key2.dwContactId = dwSavedContact;
		m_dbEventsSort.remove((LPCSTR)&key2, sizeof(key2));
	}

	// update a contact
	cc->dbc.dwEventCount--;
	if (cc->dbc.dwFirstUnread == hDbEvent)
		FindNextUnread(cc, key2);

	// call notifier while outside mutex
	NotifyEventHooks(hEventDeletedEvent, contactID, hDbEvent);
	return 0;
}

STDMETHODIMP_(LONG) CDbxKyoto::GetBlobSize(MEVENT hDbEvent)
{
	DBEvent dbe;
	if (-1 == m_dbEvents.get((LPCSTR)&hDbEvent, sizeof(MEVENT), (LPSTR)&dbe, sizeof(dbe)))
		return -1;

	return (dbe.dwSignature == DBEVENT_SIGNATURE) ? dbe.cbBlob : 0;
}

STDMETHODIMP_(BOOL) CDbxKyoto::GetEvent(MEVENT hDbEvent, DBEVENTINFO *dbei)
{
	if (hDbEvent == 0 || dbei == NULL || dbei->cbSize != sizeof(DBEVENTINFO)) return 1;
	if (dbei->cbBlob > 0 && dbei->pBlob == NULL) {
		dbei->cbBlob = 0;
		return 1;
	}

	char rec[65536];
	if (-1 == m_dbEvents.get((LPCSTR)&hDbEvent, sizeof(MEVENT), rec, sizeof(rec)))
		return 1;

	DBEvent *dbe = (DBEvent*)rec;
	if (dbe->dwSignature != DBEVENT_SIGNATURE)
		return 1;

	dbei->szModule = GetModuleNameByOfs(dbe->ofsModuleName);
	dbei->timestamp = dbe->timestamp;
	dbei->flags = dbe->flags;
	dbei->eventType = dbe->wEventType;
	int bytesToCopy = (dbei->cbBlob < dbe->cbBlob) ? dbei->cbBlob : dbe->cbBlob;
	dbei->cbBlob = dbe->cbBlob;
	if (bytesToCopy && dbei->pBlob) {
		BYTE *pSrc = (BYTE*)rec + sizeof(DBEvent);
		if (dbe->flags & DBEF_ENCRYPTED) {
			dbei->flags &= ~DBEF_ENCRYPTED;
			size_t len;
			BYTE* pBlob = (BYTE*)m_crypto->decodeBuffer(pSrc, dbe->cbBlob, &len);
			if (pBlob == NULL)
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

void CDbxKyoto::FindNextUnread(DBCachedContact *cc, DBEventSortingKey &key2)
{
	key2.dwEventId++;

	cursor_ptr cursor(m_dbEventsSort);
	cursor->jump((LPCSTR)&key2, sizeof(key2));
	while(cursor->step()) {
		size_t size;
		const char *pRec;
		delete[] cursor->get(&size, &pRec, &size);
		DBEvent *dbe = (DBEvent*)pRec;
		if (!dbe->markedRead()) {
			cc->dbc.dwFirstUnread = key2.dwEventId;
			cc->dbc.tsFirstUnread = key2.ts;
			return;
		}
	}

	cc->dbc.dwFirstUnread = cc->dbc.tsFirstUnread = 0;
}

STDMETHODIMP_(BOOL) CDbxKyoto::MarkEventRead(MCONTACT contactID, MEVENT hDbEvent)
{
	if (hDbEvent == 0) return -1;

	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	if (cc == NULL)
		return -1;

	char rec[65536];
	int32_t recLen = m_dbEvents.get((LPCSTR)&hDbEvent, sizeof(MEVENT), rec, sizeof(rec));
	if (recLen == -1)
		return -1;

	DBEvent *dbe = (DBEvent*)rec;
	if (dbe->dwSignature != DBEVENT_SIGNATURE)
		return -1;

	if (dbe->markedRead())
		return dbe->flags;

	DBEventSortingKey key2 = { contactID, dbe->timestamp, hDbEvent };

	dbe->flags |= DBEF_READ;
	m_dbEvents.set((LPCSTR)&hDbEvent, sizeof(MEVENT), rec, recLen);

	FindNextUnread(cc, key2);
	m_dbContacts.set((LPCSTR)&contactID, sizeof(int), (LPCSTR)&cc->dbc, sizeof(cc->dbc));

	NotifyEventHooks(hEventMarkedRead, contactID, (LPARAM)hDbEvent);
	return dbe->flags;
}

STDMETHODIMP_(MCONTACT) CDbxKyoto::GetEventContact(MEVENT hDbEvent)
{
	if (hDbEvent == 0) return INVALID_CONTACT_ID;

	char rec[65536];
	if (-1 == m_dbEvents.get((LPCSTR)&hDbEvent, sizeof(MEVENT), rec, sizeof(rec)))
		return 1;

	DBEvent *dbe = (DBEvent*)rec;
	return (dbe->dwSignature == DBEVENT_SIGNATURE) ? dbe->contactID : INVALID_CONTACT_ID;
}

STDMETHODIMP_(MEVENT) CDbxKyoto::FindFirstEvent(MCONTACT contactID)
{
	DBEventSortingKey keyVal = { contactID, 0, 0 };
	cursor_ptr cursor(m_dbEventsSort);
	cursor->jump((LPCSTR)&keyVal, sizeof(keyVal));

	size_t size;
	DBEventSortingKey *pKey = (DBEventSortingKey*)cursor->get_key(&size);
	if (pKey == NULL)
		return m_evLast = 0;

	m_tsLast = pKey->ts;
	m_evLast = (pKey->dwContactId == contactID) ? pKey->dwEventId : 0;
	delete[] pKey;
	return m_evLast;
}

STDMETHODIMP_(MEVENT) CDbxKyoto::FindFirstUnreadEvent(MCONTACT contactID)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	return (cc == NULL) ? 0 : cc->dbc.dwFirstUnread;
}

STDMETHODIMP_(MEVENT) CDbxKyoto::FindLastEvent(MCONTACT contactID)
{
	DBEventSortingKey keyVal = { contactID, 0xFFFFFFFF, 0xFFFFFFFF };
	cursor_ptr cursor(m_dbEventsSort);
	cursor->jump_back((LPCSTR)&keyVal, sizeof(keyVal));

	size_t size;
	DBEventSortingKey *pKey = (DBEventSortingKey*)cursor->get_key(&size);
	if (pKey == NULL)
		return m_evLast = 0;

	m_tsLast = pKey->ts;
	m_evLast = (pKey->dwContactId == contactID) ? pKey->dwEventId : 0;
	delete[] pKey;
	return m_evLast;
}

STDMETHODIMP_(MEVENT) CDbxKyoto::FindNextEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	if (hDbEvent == 0) return m_evLast = 0;

	DWORD ts;

	if (m_evLast != hDbEvent) {
		DBEvent dbe;
		if (-1 == m_dbEvents.get((LPCSTR)&hDbEvent, sizeof(MEVENT), (LPSTR)&dbe, sizeof(dbe)))
			return 0;
		m_tsLast = ts = dbe.timestamp;
	}
	else ts = m_tsLast;

	DBEventSortingKey keyVal = { contactID, ts, hDbEvent+1 };
	cursor_ptr cursor(m_dbEventsSort);
	cursor->jump((LPCSTR)&keyVal, sizeof(keyVal));
	cursor->step();

	size_t size;
	DBEventSortingKey *pKey = (DBEventSortingKey*)cursor->get_key(&size);
	if (pKey == NULL)
		return m_evLast = 0;

	m_tsLast = pKey->ts;
	m_evLast = (pKey->dwContactId == contactID) ? pKey->dwEventId : 0;
	delete[] pKey;
	return m_evLast;
}

STDMETHODIMP_(MEVENT) CDbxKyoto::FindPrevEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	if (hDbEvent == 0) return m_evLast = 0;

	DWORD ts;

	if (m_evLast != hDbEvent) {
		DBEvent dbe;
		if (-1 == m_dbEvents.get((LPCSTR)&hDbEvent, sizeof(MEVENT), (LPSTR)&dbe, sizeof(dbe)))
			return 0;
		m_tsLast = ts = dbe.timestamp;
	}
	else ts = m_tsLast;

	DBEventSortingKey keyVal = { contactID, ts, hDbEvent-1 };
	cursor_ptr cursor(m_dbEventsSort);
	cursor->jump_back((LPCSTR)&keyVal, sizeof(keyVal));
	cursor->step_back();

	size_t size;
	DBEventSortingKey *pKey = (DBEventSortingKey*)cursor->get_key(&size);
	if (pKey == NULL)
		return m_evLast = 0;

	m_tsLast = pKey->ts;
	m_evLast = (pKey->dwContactId == contactID) ? pKey->dwEventId : 0;
	delete[] pKey;
	return m_evLast;
}

/////////////////////////////////////////////////////////////////////////////////////////
// low-level history cleaner

int CDbxKyoto::WipeContactHistory(DBContact*)
{
	// drop subContact's history if any
	return 0;
}
