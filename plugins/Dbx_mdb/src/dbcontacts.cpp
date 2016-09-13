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

int CDbxMdb::CheckProto(DBCachedContact *cc, const char *proto)
{
	if (cc->szProto == NULL) {
		char protobuf[MAX_PATH] = { 0 };
		DBVARIANT dbv;
		dbv.type = DBVT_ASCIIZ;
		dbv.pszVal = protobuf;
		dbv.cchVal = sizeof(protobuf);
		if (GetContactSettingStatic(cc->contactID, "Protocol", "p", &dbv) != 0 || (dbv.type != DBVT_ASCIIZ))
			return 0;

		cc->szProto = m_cache->GetCachedSetting(NULL, protobuf, 0, (int)strlen(protobuf));
	}

	return !strcmp(cc->szProto, proto);
}

STDMETHODIMP_(LONG) CDbxMdb::GetContactCount(void)
{
	return m_contactCount;
}

STDMETHODIMP_(LONG) CDbxMdb::GetContactSize(void)
{
	return sizeof(DBCachedContact);
}

STDMETHODIMP_(MCONTACT) CDbxMdb::FindFirstContact(const char *szProto)
{
	DBCachedContact *cc = m_cache->GetFirstContact();
	if (cc == NULL)
		return NULL;

	if (cc->contactID == 0)
		if ((cc = m_cache->GetNextContact(0)) == NULL)
			return NULL;

	if (!szProto || CheckProto(cc, szProto))
		return cc->contactID;

	return FindNextContact(cc->contactID, szProto);
}

STDMETHODIMP_(MCONTACT) CDbxMdb::FindNextContact(MCONTACT contactID, const char *szProto)
{
	while (contactID) {
		DBCachedContact *cc = m_cache->GetNextContact(contactID);
		if (cc == NULL)
			break;

		if (!szProto || CheckProto(cc, szProto))
			return cc->contactID;

		contactID = cc->contactID;
	}

	return NULL;
}

STDMETHODIMP_(LONG) CDbxMdb::DeleteContact(MCONTACT contactID)
{
	if (contactID == 0) // global contact cannot be removed
		return 1;

	NotifyEventHooks(hContactDeletedEvent, contactID, 0);
	
	
	{
		OBJLIST<EventItem> events(50);
		GatherContactHistory(contactID, events);
		while (events.getCount())
		{
			DeleteEvent(contactID, events[0].eventId);
			events.remove(0);
		}
	}
	{
		MDB_val key, data;
		DBSettingKey keyS = { contactID, 0 };

		txn_ptr txn(m_pMdbEnv);
		cursor_ptr cursor(txn, m_dbSettings);

		key.mv_size = sizeof(keyS); key.mv_data = &keyS;

		for (int res = mdb_cursor_get(cursor, &key, &data, MDB_SET_RANGE); res == MDB_SUCCESS; res = mdb_cursor_get(cursor, &key, &data, MDB_NEXT))
		{
			const DBSettingKey *pKey = (const DBSettingKey*)key.mv_data;
			if (pKey->hContact != contactID)
				break;
			mdb_cursor_del(cursor, 0);
		}

		txn.commit();
	}

	MDB_val key = { sizeof(MCONTACT), &contactID };
	for (;; Remap())
	{
		txn_ptr trnlck(m_pMdbEnv);
		MDB_CHECK(mdb_del(trnlck, m_dbContacts, &key, nullptr), 1);
		if (trnlck.commit() == MDB_SUCCESS)
			break;
	}

	InterlockedDecrement(&m_contactCount);

	return 0;
}

STDMETHODIMP_(MCONTACT) CDbxMdb::AddContact()
{
	MCONTACT dwContactId = InterlockedIncrement(&m_maxContactId);

	DBCachedContact *cc = m_cache->AddContactToCache(dwContactId);

	MDB_val key = { sizeof(MCONTACT), &dwContactId };
	MDB_val data = { sizeof(cc->dbc), &cc->dbc };

	for (;; Remap()) {
		txn_ptr trnlck(m_pMdbEnv);
		MDB_CHECK(mdb_put(trnlck, m_dbContacts, &key, &data, 0), 0);
		if (trnlck.commit() == MDB_SUCCESS)
			break;
	}

	InterlockedIncrement(&m_contactCount);
	NotifyEventHooks(hContactAddedEvent, dwContactId, 0);
	return dwContactId;
}

STDMETHODIMP_(BOOL) CDbxMdb::IsDbContact(MCONTACT contactID)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	return (cc != NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////
// metacontacts support

BOOL CDbxMdb::MetaDetouchSub(DBCachedContact *cc, int nSub)
{
	DbModule_Delete(cc->pSubs[nSub], META_PROTO);
	return 0;
}

BOOL CDbxMdb::MetaSetDefault(DBCachedContact *cc)
{
	return db_set_dw(cc->contactID, META_PROTO, "Default", cc->nDefault);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDbxMdb::GatherContactHistory(MCONTACT hContact, LIST<EventItem> &list)
{
	DBEventSortingKey keyVal = { 0, 0, hContact };
	MDB_val key = { sizeof(keyVal), &keyVal }, data;

	txn_ptr_ro trnlck(m_txn);
	cursor_ptr_ro cursor(m_curEventsSort);

	for (int res = mdb_cursor_get(cursor, &key, &data, MDB_SET_RANGE); res == MDB_SUCCESS; res = mdb_cursor_get(cursor, &key, &data, MDB_NEXT))
	{
		const DBEventSortingKey *pKey = (const DBEventSortingKey*)key.mv_data;
		if (pKey->hContact != hContact)
			return;

		list.insert(new EventItem(pKey->ts, pKey->hEvent));
	}
}

BOOL CDbxMdb::MetaMergeHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	LIST<EventItem> list(1000);
	GatherContactHistory(ccSub->contactID, list);

	for (int i = 0; i < list.getCount(); i++) {
		EventItem *EI = list[i];

		for (;; Remap()) 
		{
			txn_ptr trnlck(m_pMdbEnv);
			DBEventSortingKey insVal = { EI->eventId, EI->ts, ccMeta->contactID };
			MDB_val key = { sizeof(insVal), &insVal }, data = { 1, (void*)"" };
			mdb_put(trnlck, m_dbEventsSort, &key, &data, 0);
			if (trnlck.commit() == MDB_SUCCESS)
				break;
		}
		ccMeta->dbc.dwEventCount++;
		delete EI;
	}

	MDB_val keyc = { sizeof(MCONTACT), &ccMeta->contactID }, datac = { sizeof(ccMeta->dbc), &ccMeta->dbc };

	for (;; Remap())
	{
		txn_ptr trnlck(m_pMdbEnv);
		MDB_CHECK(mdb_put(trnlck, m_dbContacts, &keyc, &datac, 0), 1);
		if (trnlck.commit() == MDB_SUCCESS)
			break;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL CDbxMdb::MetaSplitHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	LIST<EventItem> list(1000);
	GatherContactHistory(ccSub->contactID, list);

	for (int i = 0; i < list.getCount(); i++) {
		EventItem *EI = list[i];

		for (;; Remap()) {
			txn_ptr trnlck(m_pMdbEnv);
			DBEventSortingKey insVal = { EI->eventId, EI->ts, ccMeta->contactID };
			MDB_val key = { sizeof(insVal), &insVal }, data = { 1, (void*)"" };
			mdb_del(trnlck, m_dbEventsSort, &key, &data);
			if (trnlck.commit() == MDB_SUCCESS)
				break;
		}
		ccMeta->dbc.dwEventCount--;
		delete EI;
	}

	MDB_val keyc = { sizeof(MCONTACT), &ccMeta->contactID }, datac = { sizeof(ccMeta->dbc), &ccMeta->dbc };

	for (;; Remap())
	{
		txn_ptr trnlck(m_pMdbEnv);
		MDB_CHECK(mdb_put(trnlck, m_dbContacts, &keyc, &datac, 0), 1);
		if (trnlck.commit() == MDB_SUCCESS)
			break;
	}
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

void CDbxMdb::FillContacts()
{
	LIST<DBCachedContact> arContacts(m_contactCount);

	txn_ptr_ro trnlck(m_txn);
	cursor_ptr_ro cursor(m_curContacts);

	MDB_val key, data;
	while (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == MDB_SUCCESS)
	{
		DBCachedContact *cc = m_cache->AddContactToCache(*(MCONTACT*)key.mv_data);
		cc->dbc = *(DBContact*)data.mv_data;

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
