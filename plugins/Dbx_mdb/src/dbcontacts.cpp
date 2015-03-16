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
	mir_cslock lck(m_csDbAccess);
	return m_contactCount;
}

STDMETHODIMP_(LONG) CDbxMdb::GetContactSize(void)
{
	return sizeof(DBCachedContact);
}

STDMETHODIMP_(MCONTACT) CDbxMdb::FindFirstContact(const char *szProto)
{
	mir_cslock lck(m_csDbAccess);
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
	mir_cslock lck(m_csDbAccess);
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

	// call notifier while outside mutex
	NotifyEventHooks(hContactDeletedEvent, contactID, 0);

	// delete 
	mir_cslock lck(m_csDbAccess);

	MDB_val key = { sizeof(MCONTACT), &contactID };

	for (;; Remap()) {
		txn_ptr trnlck(m_pMdbEnv);
		MDB_CHECK(mdb_del(trnlck, m_dbContacts, &key, NULL), 0);
		if (trnlck.commit())
			break;
	}
	return 0;
}

STDMETHODIMP_(MCONTACT) CDbxMdb::AddContact()
{
	DWORD dwContactId;
	{
		mir_cslock lck(m_csDbAccess);
		dwContactId = m_dwMaxContactId++;

		DBCachedContact *cc = m_cache->AddContactToCache(dwContactId);
		cc->dbc.dwSignature = DBCONTACT_SIGNATURE;

		MDB_val key = { sizeof(MCONTACT), &dwContactId };
		MDB_val data = { sizeof(cc->dbc), &cc->dbc };

		for (;; Remap()) {
			txn_ptr trnlck(m_pMdbEnv);
			MDB_CHECK(mdb_put(trnlck, m_dbContacts, &key, &data, 0), 0);
			if (trnlck.commit())
				break;
		}
	}

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
	CallService(MS_DB_MODULE_DELETE, cc->pSubs[nSub], (LPARAM)META_PROTO);
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

	txn_ptr trnlck(m_pMdbEnv, true);
	cursor_ptr cursor(trnlck, m_dbEventsSort);
	mdb_cursor_get(cursor, &key, &data, MDB_SET);
	while (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == MDB_SUCCESS) {
		DBEventSortingKey *pKey = (DBEventSortingKey*)key.mv_data;
		if (pKey->dwContactId != hContact)
			return;

		list.insert(new EventItem(pKey->ts, pKey->dwEventId));
	}
}

BOOL CDbxMdb::MetaMergeHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	LIST<EventItem> list(10000);
	GatherContactHistory(ccSub->contactID, list);

	for (int i = 0; i < list.getCount(); i++) {
		EventItem *EI = list[i];

		for (;; Remap()) {
			txn_ptr trnlck(m_pMdbEnv);
			DBEventSortingKey insVal = { EI->eventId, EI->ts, ccMeta->contactID };
			MDB_val key = { sizeof(insVal), &insVal }, data = { 1, "" };
			mdb_put(trnlck, m_dbEventsSort, &key, &data, 0);
			if (trnlck.commit())
				break;
		}
		ccMeta->dbc.dwEventCount++;
		delete EI;
	}

	MDB_val keyc = { sizeof(int), &ccMeta->contactID }, datac = { sizeof(ccMeta->dbc), &ccMeta->dbc };
	txn_ptr trnlck(m_pMdbEnv);
	mdb_put(trnlck, m_dbContacts, &keyc, &datac, 0);
	trnlck.commit();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL CDbxMdb::MetaSplitHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	LIST<EventItem> list(10000);
	GatherContactHistory(ccSub->contactID, list);

	for (int i = 0; i < list.getCount(); i++) {
		EventItem *EI = list[i];

		for (;; Remap()) {
			txn_ptr trnlck(m_pMdbEnv);
			DBEventSortingKey insVal = { EI->eventId, EI->ts, ccMeta->contactID };
			MDB_val key = { sizeof(insVal), &insVal }, data = { 1, "" };
			mdb_del(trnlck, m_dbEventsSort, &key, &data);
			if (trnlck.commit())
				break;
		}
		ccMeta->dbc.dwEventCount--;
		delete EI;
	}

	MDB_val keyc = { sizeof(int), &ccMeta->contactID }, datac = { sizeof(ccMeta->dbc), &ccMeta->dbc };
	txn_ptr trnlck(m_pMdbEnv);
	mdb_put(trnlck, m_dbContacts, &keyc, &datac, 0);
	trnlck.commit();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void DBCachedContact::Advance(DWORD id, DBEvent &dbe)
{
	dbc.dwEventCount++;

	if (dbe.flags & (DBEF_READ | DBEF_SENT))
		return;

	if (dbe.timestamp < dbc.tsFirstUnread || dbc.tsFirstUnread == 0) {
		dbc.tsFirstUnread = dbe.timestamp;
		dbc.dwFirstUnread = id;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// initial cycle to fill the contacts' cache

void CDbxMdb::FillContacts()
{
	LIST<DBCachedContact> arContacts(10);

	txn_ptr trnlck(m_pMdbEnv);
	mdb_open(trnlck, "contacts", MDB_INTEGERKEY, &m_dbContacts);
	{
		cursor_ptr cursor(trnlck, m_dbContacts);

		MDB_val key, data;
		while (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == 0) {
			DBContact *dbc = (DBContact*)data.mv_data;
			if (dbc->dwSignature != DBCONTACT_SIGNATURE)
				DatabaseCorruption(NULL);

			DBCachedContact *cc = m_cache->AddContactToCache(*(DWORD*)key.mv_data);
			cc->dbc.dwSignature = DBCONTACT_SIGNATURE;
			cc->dbc.dwEventCount = dbc->dwEventCount;
			cc->dbc.dwFirstUnread = dbc->dwFirstUnread;
			cc->dbc.tsFirstUnread = dbc->tsFirstUnread;
			arContacts.insert(cc);
		}
	}

	m_contactCount = 0;
	for (int i = 0; i < arContacts.getCount(); i++) {
		DBCachedContact *cc = arContacts[i];
		CheckProto(cc, "");

		m_dwMaxContactId = cc->contactID+1;
		m_contactCount++;

		DBVARIANT dbv; dbv.type = DBVT_DWORD;
		cc->nSubs = (0 != GetContactSetting(cc->contactID, META_PROTO, "NumContacts", &dbv)) ? -1 : dbv.dVal;
		if (cc->nSubs != -1) {
			cc->pSubs = (MCONTACT*)mir_alloc(cc->nSubs*sizeof(MCONTACT));
			for (int i = 0; i < cc->nSubs; i++) {
				char setting[100];
				mir_snprintf(setting, SIZEOF(setting), "Handle%d", i);
				cc->pSubs[i] = (0 != GetContactSetting(cc->contactID, META_PROTO, setting, &dbv)) ? NULL : dbv.dVal;
			}
		}
		cc->nDefault = (0 != GetContactSetting(cc->contactID, META_PROTO, "Default", &dbv)) ? -1 : dbv.dVal;
		cc->parentID = (0 != GetContactSetting(cc->contactID, META_PROTO, "ParentMeta", &dbv)) ? NULL : dbv.dVal;
	}
}
