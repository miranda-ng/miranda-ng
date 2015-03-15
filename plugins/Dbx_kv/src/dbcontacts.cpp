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

int CDbxKV::CheckProto(DBCachedContact *cc, const char *proto)
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

STDMETHODIMP_(LONG) CDbxKV::GetContactCount(void)
{
	mir_cslock lck(m_csDbAccess);
	return m_contactCount;
}

STDMETHODIMP_(LONG) CDbxKV::GetContactSize(void)
{
	return sizeof(DBCachedContact);
}

STDMETHODIMP_(MCONTACT) CDbxKV::FindFirstContact(const char *szProto)
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

STDMETHODIMP_(MCONTACT) CDbxKV::FindNextContact(MCONTACT contactID, const char *szProto)
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

STDMETHODIMP_(LONG) CDbxKV::DeleteContact(MCONTACT contactID)
{
	if (contactID == 0) // global contact cannot be removed
		return 1;

	// call notifier while outside mutex
	NotifyEventHooks(hContactDeletedEvent, contactID, 0);

	// delete 
	mir_cslock lck(m_csDbAccess);

	ham_key_t key = { sizeof(MCONTACT), &contactID };
	ham_db_erase(m_dbContacts, NULL, &key, 0);
	return 0;
}

STDMETHODIMP_(MCONTACT) CDbxKV::AddContact()
{
	DWORD dwContactId;
	{
		mir_cslock lck(m_csDbAccess);
		dwContactId = m_dwMaxContactId++;

		DBCachedContact *cc = m_cache->AddContactToCache(dwContactId);
		cc->dbc.dwSignature = DBCONTACT_SIGNATURE;

		ham_key_t key = { sizeof(MCONTACT), &dwContactId };
		ham_record_t data = { sizeof(cc->dbc), &cc->dbc };
		ham_db_insert(m_dbContacts, NULL, &key, &data, HAM_OVERWRITE);
	}

	NotifyEventHooks(hContactAddedEvent, dwContactId, 0);
	return dwContactId;
}

STDMETHODIMP_(BOOL) CDbxKV::IsDbContact(MCONTACT contactID)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	return (cc != NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////
// metacontacts support

BOOL CDbxKV::MetaDetouchSub(DBCachedContact *cc, int nSub)
{
	CallService(MS_DB_MODULE_DELETE, cc->pSubs[nSub], (LPARAM)META_PROTO);
	return 0;
}

BOOL CDbxKV::MetaSetDefault(DBCachedContact *cc)
{
	return db_set_dw(cc->contactID, META_PROTO, "Default", cc->nDefault);
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL CDbxKV::MetaMergeHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	DBEventSortingKey keyVal = { ccSub->contactID, 0, 0 }, insVal = { ccMeta->contactID, 0, 0 };
	ham_key_t key = { sizeof(keyVal), &keyVal }, key2 = { sizeof(insVal), &insVal };
	ham_record_t data;

	cursor_ptr cursor(m_dbEventsSort);
	if (ham_cursor_find(cursor, &key, &data, HAM_FIND_GT_MATCH) != HAM_SUCCESS)
		return 0;
	 
	do {
		DBEventSortingKey *pKey = (DBEventSortingKey*)key.data;
		if (pKey->dwContactId != ccSub->contactID)
			break;

		insVal.ts = pKey->ts;
		insVal.dwEventId = pKey->dwEventId;
		ham_db_insert(m_dbEventsSort, NULL, &key2, &data, HAM_OVERWRITE);

		ccMeta->dbc.dwEventCount++;
	} while (ham_cursor_move(cursor, &key, &data, HAM_CURSOR_NEXT) == HAM_SUCCESS);

	// now update the number of events in a metacontact
	ham_key_t keyc = { sizeof(int), &ccMeta->contactID };
	ham_record_t datac = { sizeof(ccMeta->dbc), &ccMeta->dbc };
	ham_db_insert(m_dbContacts, NULL, &keyc, &datac, HAM_OVERWRITE);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL CDbxKV::MetaSplitHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	DBEventSortingKey keyVal = { ccSub->contactID, 0, 0 }, delVal = { ccMeta->contactID, 0, 0 };
	ham_key_t key = { sizeof(keyVal), &keyVal }, key2 = { sizeof(delVal), &delVal };
	ham_record_t data;

	cursor_ptr cursor(m_dbEventsSort);
	if (ham_cursor_find(cursor, &key, &data, HAM_FIND_GT_MATCH) != HAM_SUCCESS)
		return 0;

	do {
		DBEventSortingKey *pKey = (DBEventSortingKey*)key.data;
		if (pKey->dwContactId != ccSub->contactID)
			break;

		delVal.ts = pKey->ts;
		delVal.dwEventId = pKey->dwEventId;
		ham_db_erase(m_dbEventsSort, NULL, &key2, 0);

		ccMeta->dbc.dwEventCount--;
	} while (ham_cursor_move(cursor, &key, &data, HAM_CURSOR_NEXT) == HAM_SUCCESS);

	// now update the number of events in a metacontact
	ham_key_t keyc = { sizeof(int), &ccMeta->contactID };
	ham_record_t datac = { sizeof(ccMeta->dbc), &ccMeta->dbc };
	ham_db_insert(m_dbContacts, NULL, &keyc, &datac, HAM_OVERWRITE);
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

void CDbxKV::FillContacts()
{
	m_contactCount = 0;

	ham_key_t key = { 0 };
	ham_record_t rec = { 0 };
	cursor_ptr cursor(m_dbContacts);
	if (ham_cursor_move(cursor, &key, &rec, HAM_CURSOR_FIRST) != HAM_SUCCESS) // empty table?
		return;

	do {
		DBContact *dbc = (DBContact*)rec.data;
		if (dbc->dwSignature != DBCONTACT_SIGNATURE)
			DatabaseCorruption(NULL);

		DBCachedContact *cc = m_cache->AddContactToCache(*(DWORD*)key.data);
		cc->dbc.dwSignature = DBCONTACT_SIGNATURE;
		cc->dbc.dwEventCount = dbc->dwEventCount;
		cc->dbc.dwFirstUnread = dbc->dwFirstUnread;
		cc->dbc.tsFirstUnread = dbc->tsFirstUnread;

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
	} while (ham_cursor_move(cursor, &key, &rec, HAM_CURSOR_NEXT) == HAM_SUCCESS);
}
