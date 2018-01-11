/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-18 Miranda NG team (https://miranda-ng.org)
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

STDMETHODIMP_(LONG) CDbxMDBX::GetContactCount(void)
{
	return m_contactCount;
}

STDMETHODIMP_(LONG) CDbxMDBX::GetContactSize(void)
{
	return sizeof(DBCachedContact);
}

STDMETHODIMP_(LONG) CDbxMDBX::DeleteContact(MCONTACT contactID)
{
	if (contactID == 0) // global contact cannot be removed
		return 1;

	NotifyEventHooks(hContactDeletedEvent, contactID, 0);
	{
		OBJLIST<EventItem> events(50);
		GatherContactHistory(contactID, events);
		while (events.getCount()) {
			DeleteEvent(contactID, events[0].eventId);
			events.remove(0);
		}
	}
	{
		MDBX_val key, data;
		DBSettingKey keyS = { contactID, 0, 0 };

		txn_ptr txn(m_env);
		cursor_ptr cursor(txn, m_dbSettings);

		key.iov_len = sizeof(keyS); key.iov_base = &keyS;

		for (int res = mdbx_cursor_get(cursor, &key, &data, MDBX_SET_RANGE); res == MDBX_SUCCESS; res = mdbx_cursor_get(cursor, &key, &data, MDBX_NEXT)) {
			const DBSettingKey *pKey = (const DBSettingKey*)key.iov_base;
			if (pKey->hContact != contactID)
				break;
			mdbx_cursor_del(cursor, 0);
		}

		txn.commit();
	}

	MDBX_val key = { &contactID, sizeof(MCONTACT) };
	{
		txn_ptr trnlck(m_env);
		if (mdbx_del(trnlck, m_dbContacts, &key, nullptr) != MDBX_SUCCESS)
			return 1;
		if (trnlck.commit() != MDBX_SUCCESS)
			return 1;
	}

	InterlockedDecrement(&m_contactCount);
	return 0;
}

STDMETHODIMP_(MCONTACT) CDbxMDBX::AddContact()
{
	MCONTACT dwContactId = InterlockedIncrement(&m_maxContactId);

	DBCachedContact *cc = m_cache->AddContactToCache(dwContactId);
	{
		MDBX_val key = { &dwContactId, sizeof(MCONTACT) };
		MDBX_val data = { &cc->dbc, sizeof(cc->dbc) };

		txn_ptr trnlck(m_env);
		if (mdbx_put(trnlck, m_dbContacts, &key, &data, 0) != MDBX_SUCCESS)
			return 0;
		if (trnlck.commit() != MDBX_SUCCESS)
			return 0;
	}

	InterlockedIncrement(&m_contactCount);
	NotifyEventHooks(hContactAddedEvent, dwContactId, 0);
	return dwContactId;
}

STDMETHODIMP_(BOOL) CDbxMDBX::IsDbContact(MCONTACT contactID)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	return (cc != NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDbxMDBX::GatherContactHistory(MCONTACT hContact, LIST<EventItem> &list)
{
	DBEventSortingKey keyVal = { hContact, 0, 0 };
	MDBX_val key = { &keyVal, sizeof(keyVal) }, data;

	txn_ptr_ro trnlck(m_txn);
	cursor_ptr_ro cursor(m_curEventsSort);

	for (int res = mdbx_cursor_get(cursor, &key, &data, MDBX_SET_RANGE); res == MDBX_SUCCESS; res = mdbx_cursor_get(cursor, &key, &data, MDBX_NEXT)) {
		const DBEventSortingKey *pKey = (const DBEventSortingKey*)key.iov_base;
		if (pKey->hContact != hContact)
			return;

		list.insert(new EventItem(pKey->ts, pKey->hEvent));
	}
}

BOOL CDbxMDBX::MetaMergeHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	LIST<EventItem> list(1000);
	GatherContactHistory(ccSub->contactID, list);

	for (int i = 0; i < list.getCount(); i++) {
		EventItem *EI = list[i];
		{
			txn_ptr trnlck(m_env);
			DBEventSortingKey insVal = { ccMeta->contactID, EI->eventId, EI->ts };
			MDBX_val key = { &insVal, sizeof(insVal) }, data = { (void*)"", 1 };

			if (mdbx_put(trnlck, m_dbEventsSort, &key, &data, 0) != MDBX_SUCCESS)
				return 1;
			if (trnlck.commit() != MDBX_SUCCESS)
				return 1;
		}
		ccMeta->dbc.dwEventCount++;
		delete EI;
	}

	MDBX_val keyc = { &ccMeta->contactID, sizeof(MCONTACT) }, datac = { &ccMeta->dbc, sizeof(ccMeta->dbc) };
	txn_ptr trnlck(m_env);
	if (mdbx_put(trnlck, m_dbContacts, &keyc, &datac, 0) != MDBX_SUCCESS)
		return 1;
	if (trnlck.commit() != MDBX_SUCCESS)
		return 1;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL CDbxMDBX::MetaSplitHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	LIST<EventItem> list(1000);
	GatherContactHistory(ccSub->contactID, list);

	for (int i = 0; i < list.getCount(); i++) {
		EventItem *EI = list[i];
		{
			txn_ptr trnlck(m_env);
			DBEventSortingKey insVal = { ccMeta->contactID, EI->eventId, EI->ts };
			MDBX_val key = { &insVal, sizeof(insVal) }, data = { (void*)"", 1 };
			if (mdbx_del(trnlck, m_dbEventsSort, &key, &data) != MDBX_SUCCESS)
				return 1;
			if (trnlck.commit() != MDBX_SUCCESS)
				return 1;
		}
		ccMeta->dbc.dwEventCount--;
		delete EI;
	}

	txn_ptr trnlck(m_env);
	MDBX_val keyc = { &ccMeta->contactID, sizeof(MCONTACT) }, datac = { &ccMeta->dbc, sizeof(ccMeta->dbc) };
	if(mdbx_put(trnlck, m_dbContacts, &keyc, &datac, 0) != MDBX_SUCCESS)
		return 1;
	if (trnlck.commit() != MDBX_SUCCESS)
		return 1;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void DBCachedContact::Advance(MEVENT id, DBEvent &dbe)
{
	dbc.dwEventCount++;

	if (dbe.flags & (DBEF_READ | DBEF_SENT))
		return;

	if (dbe.timestamp < dbc.tsFirstUnread || dbc.tsFirstUnread == 0) {
		dbc.tsFirstUnread = dbe.timestamp;
		dbc.evFirstUnread = id;
	}
}

void DBCachedContact::Snapshot()
{
	tmp_dbc = dbc;
}

void DBCachedContact::Revert()
{
	dbc = tmp_dbc;
}

/////////////////////////////////////////////////////////////////////////////////////////
// initial cycle to fill the contacts' cache

void CDbxMDBX::FillContacts()
{
	LIST<DBCachedContact> arContacts(m_contactCount);

	txn_ptr_ro trnlck(m_txn);
	cursor_ptr_ro cursor(m_curContacts);

	MDBX_val key, data;
	while (mdbx_cursor_get(cursor, &key, &data, MDBX_NEXT) == MDBX_SUCCESS) {
		DBCachedContact *cc = m_cache->AddContactToCache(*(MCONTACT*)key.iov_base);
		cc->dbc = *(DBContact*)data.iov_base;

		CheckProto(cc, "");

		DBVARIANT dbv; dbv.type = DBVT_DWORD;
		cc->nSubs = (0 != GetContactSetting(cc->contactID, META_PROTO, "NumContacts", &dbv)) ? -1 : dbv.dVal;
		if (cc->nSubs != -1) {
			cc->pSubs = (MCONTACT*)mir_alloc(cc->nSubs * sizeof(MCONTACT));
			for (int k = 0; k < cc->nSubs; k++) {
				char setting[100];
				mir_snprintf(setting, _countof(setting), "Handle%d", k);
				cc->pSubs[k] = (0 != GetContactSetting(cc->contactID, META_PROTO, setting, &dbv)) ? NULL : dbv.dVal;
			}
		}
		cc->nDefault = (0 != GetContactSetting(cc->contactID, META_PROTO, "Default", &dbv)) ? -1 : dbv.dVal;
		cc->parentID = (0 != GetContactSetting(cc->contactID, META_PROTO, "ParentMeta", &dbv)) ? NULL : dbv.dVal;
	}
}
