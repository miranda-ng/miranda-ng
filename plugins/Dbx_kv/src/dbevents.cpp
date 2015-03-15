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

STDMETHODIMP_(LONG) CDbxKV::GetEventCount(MCONTACT contactID)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	return (cc == NULL) ? 0 : cc->dbc.dwEventCount;
}

STDMETHODIMP_(MEVENT) CDbxKV::AddEvent(MCONTACT contactID, DBEVENTINFO *dbei)
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
		// set default sub to the event's source
		if (!(dbei->flags & DBEF_SENT))
			db_mc_setDefault(cc->parentID, contactID, false);
		contactID = cc->parentID; // and add an event to a metahistory
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

	ham_key_t key = { sizeof(int), &dwEventId };
	ham_record_t rec = { sizeof(DBEvent) + dbe.cbBlob, pDest };
	ham_db_insert(m_dbEvents, NULL, &key, &rec, HAM_OVERWRITE);

	// add a sorting key
	DBEventSortingKey key2 = { contactID, dbe.timestamp, dwEventId };
	key.size = sizeof(key2); key.data = &key2;
	rec.size = 1; rec.data = "";
	ham_db_insert(m_dbEventsSort, NULL, &key, &rec, HAM_OVERWRITE);

	cc->Advance(dwEventId, dbe);
	ham_key_t keyc = { sizeof(int), &contactID };
	ham_record_t datac = { sizeof(DBContact), &cc->dbc };
	ham_db_insert(m_dbContacts, NULL, &keyc, &datac, HAM_OVERWRITE);

	// insert an event into a sub's history too
	if (ccSub != NULL) {
		key2.dwContactId = ccSub->contactID;
		ham_db_insert(m_dbEventsSort, NULL, &key, &rec, HAM_OVERWRITE);

		ccSub->Advance(dwEventId, dbe);
		datac.data = &ccSub->dbc;
		keyc.data = &ccSub->contactID;
		ham_db_insert(m_dbContacts, NULL, &keyc, &datac, HAM_OVERWRITE);
	}

	// Notify only in safe mode or on really new events
	if (m_safetyMode)
		NotifyEventHooks(hEventAddedEvent, contactNotifyID, dwEventId);

	return dwEventId;
}

STDMETHODIMP_(BOOL) CDbxKV::DeleteEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	if (cc == NULL || cc->dbc.dwEventCount == 0)
		return 1;

	ham_key_t key = { sizeof(MEVENT), &hDbEvent };
	ham_record_t rec = { 0 };
	if (ham_db_find(m_dbEvents, NULL, &key, &rec, HAM_FIND_EXACT_MATCH) != HAM_SUCCESS)
		return 1;

	DBEvent *dbe = (DBEvent*)rec.data;
	DWORD dwSavedContact = dbe->contactID;
	DBEventSortingKey key2 = { contactID, dbe->timestamp, hDbEvent };
	ham_db_erase(m_dbEvents, NULL, &key, 0);

	// remove a sorting key
	key.size = sizeof(key2); key.data = &key2;
	ham_db_erase(m_dbEventsSort, NULL, &key, 0);

	// remove a sub's history entry too
	if (contactID != dwSavedContact) {
		key2.dwContactId = dwSavedContact;
		ham_db_erase(m_dbEventsSort, NULL, &key, 0);
	}

	// update a contact
	key.size = sizeof(int); key.data = &contactID;
	cc->dbc.dwEventCount--;
	if (cc->dbc.dwFirstUnread == hDbEvent)
		FindNextUnread(cc, key2);

	// call notifier while outside mutex
	NotifyEventHooks(hEventDeletedEvent, contactID, hDbEvent);
	return 0;
}

STDMETHODIMP_(LONG) CDbxKV::GetBlobSize(MEVENT hDbEvent)
{
	ham_key_t key = { sizeof(MEVENT), &hDbEvent };
	ham_record_t rec = { 0 };
	if (ham_db_find(m_dbEvents, NULL, &key, &rec, HAM_FIND_EXACT_MATCH) != HAM_SUCCESS)
		return -1;

	DBEvent *dbe = (DBEvent*)rec.data;
	return (dbe->dwSignature == DBEVENT_SIGNATURE) ? dbe->cbBlob : 0;
}

STDMETHODIMP_(BOOL) CDbxKV::GetEvent(MEVENT hDbEvent, DBEVENTINFO *dbei)
{
	if (dbei == NULL || dbei->cbSize != sizeof(DBEVENTINFO)) return 1;
	if (dbei->cbBlob > 0 && dbei->pBlob == NULL) {
		dbei->cbBlob = 0;
		return 1;
	}

	ham_record_t rec = { 0 };
	ham_key_t key = { sizeof(MEVENT), &hDbEvent };
	if (ham_db_find(m_dbEvents, NULL, &key, &rec, HAM_FIND_EXACT_MATCH) != HAM_SUCCESS)
		return 1;

	DBEvent *dbe = (DBEvent*)rec.data;
	if (dbe->dwSignature != DBEVENT_SIGNATURE)
		return 1;

	dbei->szModule = GetModuleNameByOfs(dbe->ofsModuleName);
	dbei->timestamp = dbe->timestamp;
	dbei->flags = dbe->flags;
	dbei->eventType = dbe->wEventType;
	int bytesToCopy = (dbei->cbBlob < dbe->cbBlob) ? dbei->cbBlob : dbe->cbBlob;
	dbei->cbBlob = dbe->cbBlob;
	if (bytesToCopy && dbei->pBlob) {
		BYTE *pSrc = (BYTE*)rec.data + sizeof(DBEvent);
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

void CDbxKV::FindNextUnread(DBCachedContact *cc, DBEventSortingKey &key2)
{
	ham_record_t rec = { 0 };
	ham_key_t key = { sizeof(key2), &key2 };
	key2.dwEventId++;

	cursor_ptr cursor(m_dbEventsSort);
	if (ham_cursor_find(cursor, &key, &rec, HAM_FIND_GEQ_MATCH) != HAM_SUCCESS)
		return;

	do {
		DBEvent *dbe = (DBEvent*)rec.data;
		if (!dbe->markedRead()) {
			cc->dbc.dwFirstUnread = key2.dwEventId;
			cc->dbc.tsFirstUnread = key2.ts;
			return;
		}
	} while (ham_cursor_move(cursor, &key, &rec, HAM_CURSOR_NEXT) == 0);

	cc->dbc.dwFirstUnread = cc->dbc.tsFirstUnread = 0;
}

STDMETHODIMP_(BOOL) CDbxKV::MarkEventRead(MCONTACT contactID, MEVENT hDbEvent)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	if (cc == NULL)
		return -1;

	ham_record_t rec = { 0 };
	ham_key_t key = { sizeof(MEVENT), &hDbEvent };
	if (ham_db_find(m_dbEvents, NULL, &key, &rec, HAM_FIND_EXACT_MATCH) != HAM_SUCCESS)
		return -1;

	DBEvent *dbe = (DBEvent*)rec.data;
	if (dbe->dwSignature != DBEVENT_SIGNATURE)
		return -1;

	if (dbe->markedRead())
		return dbe->flags;

	DBEventSortingKey key2 = { contactID, dbe->timestamp, hDbEvent };

	dbe->flags |= DBEF_READ;
	ham_db_insert(m_dbEvents, NULL, &key, &rec, HAM_OVERWRITE);

	FindNextUnread(cc, key2);
	key.data = &contactID;
	rec.data = &cc->dbc; rec.size = sizeof(cc->dbc);
	ham_db_insert(m_dbContacts, NULL, &key, &rec, HAM_OVERWRITE);

	NotifyEventHooks(hEventMarkedRead, contactID, (LPARAM)hDbEvent);
	return dbe->flags;
}

STDMETHODIMP_(MCONTACT) CDbxKV::GetEventContact(MEVENT hDbEvent)
{
	ham_record_t rec = { 0 };
	ham_key_t key = { sizeof(MEVENT), &hDbEvent };
	if (ham_db_find(m_dbEvents, NULL, &key, &rec, HAM_FIND_EXACT_MATCH) != HAM_SUCCESS)
		return 0;

	DBEvent *dbe = (DBEvent*)rec.data;
	return (dbe->dwSignature == DBEVENT_SIGNATURE) ? dbe->contactID : INVALID_CONTACT_ID;
}

STDMETHODIMP_(MEVENT) CDbxKV::FindFirstEvent(MCONTACT contactID)
{
	DBEventSortingKey keyVal = { contactID, 0, 0 };
	ham_key_t key = { sizeof(keyVal), &keyVal };
	ham_record_t rec = { 0 };

	if (ham_db_find(m_dbEventsSort, NULL, &key, &rec, HAM_FIND_GT_MATCH) != HAM_SUCCESS)
		return m_evLast = 0;

	DBEventSortingKey *pKey = (DBEventSortingKey*)key.data;
	m_tsLast = pKey->ts;
	return m_evLast = (pKey->dwContactId == contactID) ? pKey->dwEventId : 0;
}

STDMETHODIMP_(MEVENT) CDbxKV::FindFirstUnreadEvent(MCONTACT contactID)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	return (cc == NULL) ? 0 : cc->dbc.dwFirstUnread;
}

STDMETHODIMP_(MEVENT) CDbxKV::FindLastEvent(MCONTACT contactID)
{
	DBEventSortingKey keyVal = { contactID, 0xFFFFFFFF, 0xFFFFFFFF };
	ham_key_t key = { sizeof(keyVal), &keyVal };
	ham_record_t rec = { 0 };

	if (ham_db_find(m_dbEventsSort, NULL, &key, &rec, HAM_FIND_LT_MATCH) != HAM_SUCCESS)
		return m_evLast = 0;
	
	DBEventSortingKey *pKey = (DBEventSortingKey*)key.data;
	m_tsLast = pKey->ts;
	return m_evLast = (pKey->dwContactId == contactID) ? pKey->dwEventId : 0;
}

STDMETHODIMP_(MEVENT) CDbxKV::FindNextEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	DWORD ts;
	ham_record_t rec = { 0 };

	if (m_evLast != hDbEvent) {
		ham_key_t key = { sizeof(MEVENT), &hDbEvent };
		if (ham_db_find(m_dbEvents, NULL, &key, &rec, HAM_FIND_EXACT_MATCH) != HAM_SUCCESS)
			return 0;
		m_tsLast = ts = ((DBEvent*)rec.data)->timestamp;
	}
	else ts = m_tsLast;

	DBEventSortingKey keyVal = { contactID, ts, hDbEvent };
	ham_key_t key = { sizeof(keyVal), &keyVal };
	if (ham_db_find(m_dbEventsSort, NULL, &key, &rec, HAM_FIND_GT_MATCH) != HAM_SUCCESS)
		return m_evLast = 0;

	DBEventSortingKey *pKey = (DBEventSortingKey*)key.data;
	m_tsLast = pKey->ts;
	return m_evLast = (pKey->dwContactId == contactID) ? pKey->dwEventId : 0;
}

STDMETHODIMP_(MEVENT) CDbxKV::FindPrevEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	DWORD ts;
	ham_record_t rec = { 0 };

	if (m_evLast != hDbEvent) {
		ham_key_t key = { sizeof(MEVENT), &hDbEvent };
		if (ham_db_find(m_dbEvents, NULL, &key, &rec, HAM_FIND_EXACT_MATCH) != HAM_SUCCESS)
			return 0;
		m_tsLast = ts = ((DBEvent*)rec.data)->timestamp;
	}
	else ts = m_tsLast;

	DBEventSortingKey keyVal = { contactID, ts, hDbEvent };
	ham_key_t key = { sizeof(keyVal), &keyVal };
	if (ham_db_find(m_dbEventsSort, NULL, &key, &rec, HAM_FIND_LT_MATCH) != HAM_SUCCESS)
		return m_evLast = 0;

	DBEventSortingKey *pKey = (DBEventSortingKey*)key.data;
	m_tsLast = pKey->ts;
	return m_evLast = (pKey->dwContactId == contactID) ? pKey->dwEventId : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// low-level history cleaner

int CDbxKV::WipeContactHistory(DBContact*)
{
	// drop subContact's history if any
	return 0;
}
