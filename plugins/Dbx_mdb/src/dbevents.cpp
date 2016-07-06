/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-16 Miranda NG project (http://miranda-ng.org)
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
	dbe.contactID = contactID; // store native or subcontact's id
	dbe.iModuleId = GetModuleID(dbei->szModule);

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

	dbe.timestamp = dbei->timestamp;
	dbe.flags = dbei->flags;
	dbe.wEventType = dbei->eventType;
	dbe.cbBlob = dbei->cbBlob;
	BYTE *pBlob = dbei->pBlob;

	mir_ptr<BYTE> pCryptBlob;
	if (m_bEncrypted) {
		size_t len;
		BYTE *pResult = m_crypto->encodeBuffer(pBlob, dbe.cbBlob, &len);
		if (pResult != NULL) {
			pCryptBlob = pBlob = pResult;
			dbe.cbBlob = (uint16_t)len;
			dbe.flags |= DBEF_ENCRYPTED;
		}
	}


	MEVENT dwEventId = InterlockedIncrement(&m_dwMaxEventId);

	const auto Snapshot = [&]() { cc->Snapshot(); if (ccSub) ccSub->Snapshot(); };
	const auto Revert = [&]() { cc->Revert(); if (ccSub) ccSub->Revert(); };

	for (Snapshot();; Revert(), Remap()) {
		txn_ptr txn(m_pMdbEnv);

		MDB_val key = { sizeof(int), &dwEventId }, data = { sizeof(DBEvent) + dbe.cbBlob, NULL };
		MDB_CHECK(mdb_put(txn, m_dbEvents, &key, &data, MDB_RESERVE), 0);

		DBEvent *pNewEvent = (DBEvent*)data.mv_data;
		*pNewEvent = dbe;
		memcpy(pNewEvent + 1, pBlob, dbe.cbBlob);

		// add a sorting key
		DBEventSortingKey key2 = { contactID, dwEventId, dbe.timestamp };
		key.mv_size = sizeof(key2); key.mv_data = &key2;
		data.mv_size = 1; data.mv_data = (char*)("");
		MDB_CHECK(mdb_put(txn, m_dbEventsSort, &key, &data, 0), 0);

		cc->Advance(dwEventId, dbe);
		MDB_val keyc = { sizeof(MCONTACT), &contactID }, datac = { sizeof(DBContact), &cc->dbc };
		MDB_CHECK(mdb_put(txn, m_dbContacts, &keyc, &datac, 0), 0);

		// insert an event into a sub's history too
		if (ccSub != NULL) {
			key2.hContact = ccSub->contactID;
			MDB_CHECK(mdb_put(txn, m_dbEventsSort, &key, &data, 0), 0);

			ccSub->Advance(dwEventId, dbe);
			datac.mv_data = &ccSub->dbc;
			keyc.mv_data = &ccSub->contactID;
			MDB_CHECK(mdb_put(txn, m_dbContacts, &keyc, &datac, 0), 0);
		}

		if (txn.commit() == MDB_SUCCESS)
			break;
	}

	// Notify only in safe mode or on really new events
	if (m_safetyMode)
		NotifyEventHooks(hEventAddedEvent, contactNotifyID, dwEventId);

	return dwEventId;
}

STDMETHODIMP_(BOOL) CDbxMdb::DeleteEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID), *cc2 = nullptr;
	if (cc == NULL || cc->dbc.dwEventCount == 0)
		return 1;
	
	DBEvent dbe;
	{
		txn_ptr_ro txn(m_txn);
		MDB_val key = { sizeof(MEVENT), &hDbEvent }, data;
		if (mdb_get(txn, m_dbEvents, &key, &data) != MDB_SUCCESS)
			return 1;
		dbe = *(DBEvent*)data.mv_data;
	}

	if (contactID != dbe.contactID)
	{
		cc2 = m_cache->GetCachedContact(dbe.contactID);
	}

	const auto Snapshot = [&]() { cc->Snapshot(); if (cc2) cc2->Snapshot(); };
	const auto Revert = [&]() { cc->Revert(); if (cc2) cc2->Revert(); };
	
	for (Snapshot();; Revert(), Remap())
	{
		DBEventSortingKey key2 = { contactID, hDbEvent, dbe.timestamp };

		txn_ptr txn(m_pMdbEnv);
		MDB_val key = { sizeof(key2), &key2 }, data;

		MDB_CHECK(mdb_del(txn, m_dbEventsSort, &key, &data), 1)

		{
			key.mv_size = sizeof(MCONTACT); key.mv_data = &contactID;
			cc->dbc.dwEventCount--;
			if (cc->dbc.evFirstUnread == hDbEvent)
				FindNextUnread(txn, cc, key2);

			data.mv_size = sizeof(DBContact); data.mv_data = &cc->dbc;
			MDB_CHECK(mdb_put(txn, m_dbContacts, &key, &data, 0), 1);
		}

		if (cc2)
		{
			key2.hContact = dbe.contactID;
			MDB_CHECK(mdb_del(txn, m_dbEventsSort, &key, &data), 1);

			key.mv_size = sizeof(MCONTACT); key.mv_data = &contactID;
			cc2->dbc.dwEventCount--;
			if (cc2->dbc.evFirstUnread == hDbEvent)
				FindNextUnread(txn, cc2, key2);

			data.mv_size = sizeof(DBContact); data.mv_data = &cc2->dbc;
			MDB_CHECK(mdb_put(txn, m_dbContacts, &key, &data, 0), 1);

		}

		// remove a event
		key.mv_size = sizeof(MEVENT); key.mv_data = &hDbEvent;
		MDB_CHECK(mdb_del(txn, m_dbEvents, &key, &data), 1);

		if (txn.commit() == MDB_SUCCESS)
			break;
	}

	NotifyEventHooks(hEventDeletedEvent, contactID, hDbEvent);

	return 0;
}

STDMETHODIMP_(LONG) CDbxMdb::GetBlobSize(MEVENT hDbEvent)
{
	txn_ptr_ro txn(m_txn);

	MDB_val key = { sizeof(MEVENT), &hDbEvent }, data;
	if (mdb_get(txn, m_dbEvents, &key, &data) != MDB_SUCCESS)
		return -1;
	return ((const DBEvent*)data.mv_data)->cbBlob;
}

STDMETHODIMP_(BOOL) CDbxMdb::GetEvent(MEVENT hDbEvent, DBEVENTINFO *dbei)
{
	if (dbei == NULL || dbei->cbSize != sizeof(DBEVENTINFO)) return 1;
	if (dbei->cbBlob > 0 && dbei->pBlob == NULL) {
		dbei->cbBlob = 0;
		return 1;
	}

	txn_ptr_ro txn(m_txn);

	MDB_val key = { sizeof(MEVENT), &hDbEvent }, data;
	if (mdb_get(txn, m_dbEvents, &key, &data) != MDB_SUCCESS)
		return 1;

	const DBEvent *dbe = (const DBEvent*)data.mv_data;

	dbei->szModule = GetModuleName(dbe->iModuleId);
	dbei->timestamp = dbe->timestamp;
	dbei->flags = dbe->flags;
	dbei->eventType = dbe->wEventType;
	size_t bytesToCopy = min(dbei->cbBlob, dbe->cbBlob);
	dbei->cbBlob = dbe->cbBlob;
	if (bytesToCopy && dbei->pBlob) 
	{
		BYTE *pSrc = (BYTE*)data.mv_data + sizeof(DBEvent);
		if (dbe->flags & DBEF_ENCRYPTED) 
		{
			dbei->flags &= ~DBEF_ENCRYPTED;
			size_t len;
			BYTE* pBlob = (BYTE*)m_crypto->decodeBuffer(pSrc, dbe->cbBlob, &len);
			if (pBlob == NULL)
				return 1;

			memcpy(dbei->pBlob, pBlob, bytesToCopy);
			if (bytesToCopy > len)
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

	for (int res = mdb_cursor_get(cursor, &key, &data, MDB_SET); res == MDB_SUCCESS; res = mdb_cursor_get(cursor, &key, &data, MDB_NEXT))
	{
		const DBEvent *dbe = (const DBEvent*)data.mv_data;
		if (dbe->contactID != cc->contactID)
			break;
		if (!dbe->markedRead()) {
			cc->dbc.evFirstUnread = key2.hEvent;
			cc->dbc.tsFirstUnread = key2.ts;
			return;
		}
	}

	cc->dbc.evFirstUnread = cc->dbc.tsFirstUnread = 0;
}

STDMETHODIMP_(BOOL) CDbxMdb::MarkEventRead(MCONTACT contactID, MEVENT hDbEvent)
{
	if (hDbEvent == 0) return -1;

	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	if (cc == NULL)
		return -1;

	uint32_t wRetVal = -1;
	
	for (cc->Snapshot();; cc->Revert(), Remap())
	{
		txn_ptr txn(m_pMdbEnv);

		MDB_val key = { sizeof(MEVENT), &hDbEvent }, data;
		MDB_CHECK(mdb_get(txn, m_dbEvents, &key, &data), -1);

		const DBEvent *cdbe = (const DBEvent*)data.mv_data;

		if (cdbe->markedRead())
			return cdbe->flags;

		DBEventSortingKey keyVal = { contactID, hDbEvent, cdbe->timestamp };

		MDB_CHECK(mdb_put(txn, m_dbEvents, &key, &data, MDB_RESERVE), -1);

		DBEvent *pNewEvent = (DBEvent*)data.mv_data;
		*pNewEvent = *cdbe;

		wRetVal = (pNewEvent->flags |= DBEF_READ);

		FindNextUnread(txn, cc, keyVal);
		key.mv_size = sizeof(MCONTACT); key.mv_data = &contactID;
		data.mv_data = &cc->dbc; data.mv_size = sizeof(cc->dbc);
		MDB_CHECK(mdb_put(txn, m_dbContacts, &key, &data, 0), -1);

		if (txn.commit() == MDB_SUCCESS)
			break;
	}

	NotifyEventHooks(hEventMarkedRead, contactID, (LPARAM)hDbEvent);
	return wRetVal;
}

STDMETHODIMP_(MCONTACT) CDbxMdb::GetEventContact(MEVENT hDbEvent)
{
	if (hDbEvent == 0) 
		return INVALID_CONTACT_ID;

	txn_ptr_ro txn(m_txn);

	MDB_val key = { sizeof(MEVENT), &hDbEvent }, data;
	if (mdb_get(txn, m_dbEvents, &key, &data) != MDB_SUCCESS)
		return INVALID_CONTACT_ID;

	return ((const DBEvent*)data.mv_data)->contactID;
}

thread_local uint64_t t_tsLast = 0;
thread_local MEVENT t_evLast = 0;

STDMETHODIMP_(MEVENT) CDbxMdb::FindFirstEvent(MCONTACT contactID)
{
	DBEventSortingKey keyVal = { contactID, 0, 0 };
	MDB_val key = { sizeof(keyVal), &keyVal }, data;

	txn_ptr_ro txn(m_txn);

	cursor_ptr_ro cursor(m_curEventsSort);
	if (mdb_cursor_get(cursor, &key, &data, MDB_SET_RANGE) != MDB_SUCCESS)
		return t_evLast = 0;

	const DBEventSortingKey *pKey = (const DBEventSortingKey*)key.mv_data;
	t_tsLast = pKey->ts;
	return t_evLast = (pKey->hContact == contactID) ? pKey->hEvent : 0;
}

STDMETHODIMP_(MEVENT) CDbxMdb::FindFirstUnreadEvent(MCONTACT contactID)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	return (cc == NULL) ? 0 : cc->dbc.evFirstUnread;
}

STDMETHODIMP_(MEVENT) CDbxMdb::FindLastEvent(MCONTACT contactID)
{
	DBEventSortingKey keyVal = { contactID, 0xFFFFFFFF, 0xFFFFFFFFFFFFFFFF };
	MDB_val key = { sizeof(keyVal), &keyVal }, data;

	txn_ptr_ro txn(m_txn);
	cursor_ptr_ro cursor(m_curEventsSort);

	if (mdb_cursor_get(cursor, &key, &data, MDB_SET_RANGE) != MDB_SUCCESS)
	{
		if (mdb_cursor_get(cursor, &key, &data, MDB_LAST) != MDB_SUCCESS)
			return t_evLast = 0;
	}
	else
	{
		if (mdb_cursor_get(cursor, &key, &data, MDB_PREV) != MDB_SUCCESS)
			return t_evLast = 0;
	}

	const DBEventSortingKey *pKey = (const DBEventSortingKey*)key.mv_data;
	t_tsLast = pKey->ts;
	return t_evLast = (pKey->hContact == contactID) ? pKey->hEvent : 0;
}

STDMETHODIMP_(MEVENT) CDbxMdb::FindNextEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	if (hDbEvent == 0) 
		return t_evLast = 0;

	txn_ptr_ro txn(m_txn);

	if (t_evLast != hDbEvent) 
	{
		MDB_val key = { sizeof(MEVENT), &hDbEvent }, data;
		if (mdb_get(txn, m_dbEvents, &key, &data) != MDB_SUCCESS)
			return 0;
		t_tsLast = ((DBEvent*)data.mv_data)->timestamp;
	}

	DBEventSortingKey keyVal = { contactID, hDbEvent, t_tsLast };
	MDB_val key = { sizeof(keyVal), &keyVal }, data;

	cursor_ptr_ro cursor(m_curEventsSort);
	if (mdb_cursor_get(cursor, &key, &data, MDB_SET) != MDB_SUCCESS)
		return t_evLast = 0;

	if (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) != MDB_SUCCESS)
		return t_evLast = 0;

	const DBEventSortingKey *pKey = (const DBEventSortingKey*)key.mv_data;
	t_tsLast = pKey->ts;
	return t_evLast = (pKey->hContact == contactID) ? pKey->hEvent : 0;
}

STDMETHODIMP_(MEVENT) CDbxMdb::FindPrevEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	if (hDbEvent == 0) 
		return t_evLast = 0;

	MDB_val data;

	txn_ptr_ro txn(m_txn);

	if (t_evLast != hDbEvent) 
	{
		MDB_val key = { sizeof(MEVENT), &hDbEvent };
		if (mdb_get(txn, m_dbEvents, &key, &data) != MDB_SUCCESS)
			return 0;
		t_tsLast = ((DBEvent*)data.mv_data)->timestamp;
	}

	DBEventSortingKey keyVal = { contactID, hDbEvent, t_tsLast };
	MDB_val key = { sizeof(keyVal), &keyVal };

	cursor_ptr_ro cursor(m_curEventsSort);
	if (mdb_cursor_get(cursor, &key, &data, MDB_SET) != MDB_SUCCESS)
		return t_evLast = 0;

	if (mdb_cursor_get(cursor, &key, &data, MDB_PREV) != MDB_SUCCESS)
		return t_evLast = 0;

	const DBEventSortingKey *pKey = (const DBEventSortingKey*)key.mv_data;
	t_tsLast = pKey->ts;
	return t_evLast = (pKey->hContact == contactID) ? pKey->hEvent : 0;
}
