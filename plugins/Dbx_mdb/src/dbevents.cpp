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

STDMETHODIMP_(LONG) CDbxMdb::GetEventCount(MCONTACT contactID)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	return (cc == NULL) ? 0 : cc->dbc.dwEventCount;
}

STDMETHODIMP_(MEVENT) CDbxMdb::AddEvent(MCONTACT contactID, DBEVENTINFO *dbei)
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

	mir_cslockfull lck(m_csDbAccess);
	DWORD dwEventId = ++m_dwMaxEventId;

	for (;; Remap()) {
		txn_ptr txn(m_pMdbEnv);

		MDB_val key = { sizeof(int), &dwEventId }, data = { sizeof(DBEvent) + dbe.cbBlob, NULL };
		MDB_CHECK(mdb_put(txn, m_dbEvents, &key, &data, MDB_RESERVE), 0);

		BYTE *pDest = (BYTE*)data.mv_data;
		memcpy(pDest, &dbe, sizeof(DBEvent));
		memcpy(pDest + sizeof(DBEvent), pBlob, dbe.cbBlob);

		// add a sorting key
		DBEventSortingKey key2 = { dwEventId, dbe.timestamp, contactID };
		key.mv_size = sizeof(key2); key.mv_data = &key2;
		data.mv_size = 1; data.mv_data = "";
		MDB_CHECK(mdb_put(txn, m_dbEventsSort, &key, &data, 0), 0);

		cc->Advance(dwEventId, dbe);
		MDB_val keyc = { sizeof(int), &contactID }, datac = { sizeof(DBContact), &cc->dbc };
		MDB_CHECK(mdb_put(txn, m_dbContacts, &keyc, &datac, 0), 0);

		// insert an event into a sub's history too
		if (ccSub != NULL) {
			key2.dwContactId = ccSub->contactID;
			MDB_CHECK(mdb_put(txn, m_dbEventsSort, &key, &data, 0), 0);

			ccSub->Advance(dwEventId, dbe);
			datac.mv_data = &ccSub->dbc;
			keyc.mv_data = &ccSub->contactID;
			MDB_CHECK(mdb_put(txn, m_dbContacts, &keyc, &datac, 0), 0);
		}

		if (txn.commit())
			break;
	}

	lck.unlock();

	// Notify only in safe mode or on really new events
	if (m_safetyMode)
		NotifyEventHooks(hEventAddedEvent, contactNotifyID, dwEventId);

	return dwEventId;
}

STDMETHODIMP_(BOOL) CDbxMdb::DeleteEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	if (cc == NULL || cc->dbc.dwEventCount == 0)
		return 1;

	mir_cslockfull lck(m_csDbAccess);
	txn_ptr txn(m_pMdbEnv);

	for (;; Remap()) {
		MDB_val key = { sizeof(MEVENT), &hDbEvent }, data;
		if (mdb_get(txn, m_dbEvents, &key, &data) != MDB_SUCCESS)
			return 1;

		DBEvent *dbe = (DBEvent*)data.mv_data;
		DWORD dwSavedContact = dbe->contactID;
		DBEventSortingKey key2 = { hDbEvent, dbe->timestamp, contactID };
		mdb_del(txn, m_dbEvents, &key, &data);

		// remove a sorting key
		key.mv_size = sizeof(key2); key.mv_data = &key2;
		mdb_del(txn, m_dbEventsSort, &key, &data);

		// remove a sub's history entry too
		if (contactID != dwSavedContact) {
			key2.dwContactId = dwSavedContact;
			mdb_del(txn, m_dbEventsSort, &key, &data);
		}

		// update a contact
		key.mv_size = sizeof(int); key.mv_data = &contactID;
		cc->dbc.dwEventCount--;
		if (cc->dbc.dwFirstUnread == hDbEvent)
			FindNextUnread(txn, cc, key2);

		if (txn.commit())
			break;
	}
	lck.unlock();

	// call notifier while outside mutex
	NotifyEventHooks(hEventDeletedEvent, contactID, hDbEvent);

	// get back in
	lck.lock();
	return 0;
}

STDMETHODIMP_(LONG) CDbxMdb::GetBlobSize(MEVENT hDbEvent)
{
	mir_cslock lck(m_csDbAccess);
	txn_ptr txn(m_pMdbEnv, true);

	MDB_val key = { sizeof(MEVENT), &hDbEvent }, data;
	if (mdb_get(txn, m_dbEvents, &key, &data) != MDB_SUCCESS)
		return -1;

	DBEvent *dbe = (DBEvent*)data.mv_data;
	return (dbe->dwSignature == DBEVENT_SIGNATURE) ? dbe->cbBlob : 0;
}

STDMETHODIMP_(BOOL) CDbxMdb::GetEvent(MEVENT hDbEvent, DBEVENTINFO *dbei)
{
	if (dbei == NULL || dbei->cbSize != sizeof(DBEVENTINFO)) return 1;
	if (dbei->cbBlob > 0 && dbei->pBlob == NULL) {
		dbei->cbBlob = 0;
		return 1;
	}

	mir_cslock lck(m_csDbAccess);
	txn_ptr txn(m_pMdbEnv, true);

	MDB_val key = { sizeof(MEVENT), &hDbEvent }, data;
	if (mdb_get(txn, m_dbEvents, &key, &data) != MDB_SUCCESS)
		return 1;

	DBEvent *dbe = (DBEvent*)data.mv_data;
	if (dbe->dwSignature != DBEVENT_SIGNATURE)
		return 1;

	dbei->szModule = GetModuleNameByOfs(dbe->ofsModuleName);
	dbei->timestamp = dbe->timestamp;
	dbei->flags = dbe->flags;
	dbei->eventType = dbe->wEventType;
	int bytesToCopy = (dbei->cbBlob < dbe->cbBlob) ? dbei->cbBlob : dbe->cbBlob;
	dbei->cbBlob = dbe->cbBlob;
	if (bytesToCopy && dbei->pBlob) {
		BYTE *pSrc = (BYTE*)data.mv_data + sizeof(DBEvent);
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

void CDbxMdb::FindNextUnread(const txn_ptr &txn, DBCachedContact *cc, DBEventSortingKey &key2)
{
	cursor_ptr cursor(txn, m_dbEventsSort);

	MDB_val key = { sizeof(key2), &key2 }, data;
	key2.dwEventId++;
	mdb_cursor_get(cursor, &key, &data, MDB_SET_KEY);
	while (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == 0) {
		DBEvent *dbe = (DBEvent*)data.mv_data;
		if (!dbe->markedRead()) {
			cc->dbc.dwFirstUnread = key2.dwEventId;
			cc->dbc.tsFirstUnread = key2.ts;
			return;
		}
	}

	cc->dbc.dwFirstUnread = cc->dbc.tsFirstUnread = 0;
}

STDMETHODIMP_(BOOL) CDbxMdb::MarkEventRead(MCONTACT contactID, MEVENT hDbEvent)
{
	if (hDbEvent == 0) return -1;

	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	if (cc == NULL)
		return -1;

	mir_cslockfull lck(m_csDbAccess);
	txn_ptr txn(m_pMdbEnv);

	MDB_val key = { sizeof(MEVENT), &hDbEvent }, data;
	if (mdb_get(txn, m_dbEvents, &key, &data) != MDB_SUCCESS)
		return 0;

	DBEvent *dbe = (DBEvent*)data.mv_data;
	if (dbe->dwSignature != DBEVENT_SIGNATURE)
		return -1;

	if (dbe->markedRead())
		return dbe->flags;

	DBEventSortingKey key2 = { hDbEvent, dbe->timestamp, contactID };

	dbe->flags |= DBEF_READ;
	mdb_put(txn, m_dbEvents, &key, &data, 0);

	FindNextUnread(txn, cc, key2);
	key.mv_data = &contactID;
	data.mv_data = &cc->dbc; data.mv_size = sizeof(cc->dbc);
	mdb_put(txn, m_dbContacts, &key, &data, 0);

	txn.commit();

	lck.unlock();
	NotifyEventHooks(hEventMarkedRead, contactID, (LPARAM)hDbEvent);
	return dbe->flags;
}

STDMETHODIMP_(MCONTACT) CDbxMdb::GetEventContact(MEVENT hDbEvent)
{
	if (hDbEvent == 0) return INVALID_CONTACT_ID;

	mir_cslock lck(m_csDbAccess);
	txn_ptr txn(m_pMdbEnv, true);

	MDB_val key = { sizeof(MEVENT), &hDbEvent }, data;
	if (mdb_get(txn, m_dbEvents, &key, &data) != MDB_SUCCESS)
		return INVALID_CONTACT_ID;

	DBEvent *dbe = (DBEvent*)data.mv_data;
	return (dbe->dwSignature == DBEVENT_SIGNATURE) ? dbe->contactID : INVALID_CONTACT_ID;
}

STDMETHODIMP_(MEVENT) CDbxMdb::FindFirstEvent(MCONTACT contactID)
{
	DBEventSortingKey keyVal = { 0, 0, contactID };
	MDB_val key = { sizeof(keyVal), &keyVal }, data;

	mir_cslock lck(m_csDbAccess);
	txn_ptr txn(m_pMdbEnv, true);

	cursor_ptr cursor(txn, m_dbEventsSort);
	mdb_cursor_get(cursor, &key, &data, MDB_SET);
	if (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) != MDB_SUCCESS)
		return m_evLast = 0;

	DBEventSortingKey *pKey = (DBEventSortingKey*)key.mv_data;
	m_tsLast = pKey->ts;
	return m_evLast = (pKey->dwContactId == contactID) ? pKey->dwEventId : 0;
}

STDMETHODIMP_(MEVENT) CDbxMdb::FindFirstUnreadEvent(MCONTACT contactID)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	return (cc == NULL) ? 0 : cc->dbc.dwFirstUnread;
}

STDMETHODIMP_(MEVENT) CDbxMdb::FindLastEvent(MCONTACT contactID)
{
	DBEventSortingKey keyVal = { 0xFFFFFFFF, 0xFFFFFFFF, contactID };
	MDB_val key = { sizeof(keyVal), &keyVal }, data;

	mir_cslock lck(m_csDbAccess);
	txn_ptr txn(m_pMdbEnv, true);

	cursor_ptr cursor(txn, m_dbEventsSort);
	mdb_cursor_get(cursor, &key, &data, MDB_SET);
	if (mdb_cursor_get(cursor, &key, &data, MDB_PREV) != MDB_SUCCESS)
		return m_evLast = 0;

	DBEventSortingKey *pKey = (DBEventSortingKey*)key.mv_data;
	m_tsLast = pKey->ts;
	return m_evLast = (pKey->dwContactId == contactID) ? pKey->dwEventId : 0;
}

STDMETHODIMP_(MEVENT) CDbxMdb::FindNextEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	if (hDbEvent == 0) return m_evLast = 0;

	MDB_val data;
	DWORD ts;

	mir_cslock lck(m_csDbAccess);
	txn_ptr txn(m_pMdbEnv, true);

	if (m_evLast != hDbEvent) {
		MDB_val key = { sizeof(MEVENT), &hDbEvent };
		if (mdb_get(txn, m_dbEvents, &key, &data) != MDB_SUCCESS)
			return 0;
		m_tsLast = ts = ((DBEvent*)data.mv_data)->timestamp;
	}
	else ts = m_tsLast;

	DBEventSortingKey keyVal = { hDbEvent, ts, contactID };
	MDB_val key = { sizeof(keyVal), &keyVal };

	cursor_ptr cursor(txn, m_dbEventsSort);
	if (mdb_cursor_get(cursor, &key, &data, MDB_SET) != MDB_SUCCESS)
		return m_evLast = 0;

	if (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) != MDB_SUCCESS)
		return m_evLast = 0;

	DBEventSortingKey *pKey = (DBEventSortingKey*)key.mv_data;
	m_tsLast = pKey->ts;
	return m_evLast = (pKey->dwContactId == contactID) ? pKey->dwEventId : 0;
}

STDMETHODIMP_(MEVENT) CDbxMdb::FindPrevEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	if (hDbEvent == 0) return m_evLast = 0;

	MDB_val data;
	DWORD ts;

	mir_cslock lck(m_csDbAccess);
	txn_ptr txn(m_pMdbEnv, true);

	if (m_evLast != hDbEvent) {
		MDB_val key = { sizeof(MEVENT), &hDbEvent };
		if (mdb_get(txn, m_dbEvents, &key, &data) != MDB_SUCCESS)
			return 0;
		m_tsLast = ts = ((DBEvent*)data.mv_data)->timestamp;
	}
	else ts = m_tsLast;

	DBEventSortingKey keyVal = { hDbEvent, ts, contactID };
	MDB_val key = { sizeof(keyVal), &keyVal };

	cursor_ptr cursor(txn, m_dbEventsSort);
	if (mdb_cursor_get(cursor, &key, &data, MDB_SET) != MDB_SUCCESS)
		return m_evLast = 0;

	if (mdb_cursor_get(cursor, &key, &data, MDB_PREV) != MDB_SUCCESS)
		return m_evLast = 0;

	DBEventSortingKey *pKey = (DBEventSortingKey*)key.mv_data;
	m_tsLast = pKey->ts;
	return m_evLast = (pKey->dwContactId == contactID) ? pKey->dwEventId : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// low-level history cleaner

int CDbxMdb::WipeContactHistory(DBContact*)
{
	// drop subContact's history if any
	return 0;
}
