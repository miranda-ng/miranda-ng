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

STDMETHODIMP_(MCONTACT) CDbxMdb::FindFirstContact(const char *szProto)
{
	mir_cslock lck(m_csDbAccess);
	DBCachedContact *cc = m_cache->GetFirstContact();
	if (cc == NULL)
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

	return 0;
}

STDMETHODIMP_(MCONTACT) CDbxMdb::AddContact()
{
	DBContact dbc = { 0 };
	dbc.signature = DBCONTACT_SIGNATURE;
	{
		mir_cslock lck(m_csDbAccess);
	}

	DBCachedContact *cc = m_cache->AddContactToCache(dbc.dwContactID);

	NotifyEventHooks(hContactAddedEvent, dbc.dwContactID, 0);
	return dbc.dwContactID;
}

STDMETHODIMP_(BOOL) CDbxMdb::IsDbContact(MCONTACT contactID)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	if (cc == NULL)
		return FALSE;

	mir_cslock lck(m_csDbAccess);
	DBContact *dbc = NULL;
	if (dbc->signature == DBCONTACT_SIGNATURE) {
		m_cache->AddContactToCache(contactID);
		return TRUE;
	}

	return FALSE;
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

static int SortEvent(const DBEvent *p1, const DBEvent *p2)
{
	return (LONG)p1->timestamp - (LONG)p2->timestamp;
}

BOOL CDbxMdb::MetaMergeHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	mir_cslock lck(m_csDbAccess);
	return -11;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL CDbxMdb::MetaSplitHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	mir_cslock lck(m_csDbAccess);
	return -11;
}

/////////////////////////////////////////////////////////////////////////////////////////
// initial cycle to fill the contacts' cache

struct COldMeta
{
	COldMeta(DWORD _id, DBCachedContact *_cc) :
		hMetaID(_id), cc(_cc)
	{}

	DWORD hMetaID;
	DBCachedContact *cc;
};

void CDbxMdb::FillContacts()
{
	OBJLIST<COldMeta> arMetas(10, NumericKeySortT);

	for (DWORD dwOffset = 0; dwOffset != 0;) {

		int dwContactID = 0;
		DBCachedContact *cc = m_cache->AddContactToCache(dwContactID);
		cc->dwDriverData = dwOffset;
		CheckProto(cc, "");

		DBVARIANT dbv; dbv.type = DBVT_DWORD;
		cc->nSubs = (0 != GetContactSetting(dwContactID, META_PROTO, "NumContacts", &dbv)) ? -1 : dbv.dVal;
		if (cc->nSubs != -1) {
			cc->pSubs = (MCONTACT*)mir_alloc(cc->nSubs*sizeof(MCONTACT));
			for (int i = 0; i < cc->nSubs; i++) {
				char setting[100];
				mir_snprintf(setting, SIZEOF(setting), "Handle%d", i);
				cc->pSubs[i] = (0 != GetContactSetting(dwContactID, META_PROTO, setting, &dbv)) ? NULL : dbv.dVal;
			}
		}
		cc->nDefault = (0 != GetContactSetting(dwContactID, META_PROTO, "Default", &dbv)) ? -1 : dbv.dVal;
		cc->parentID = (0 != GetContactSetting(dwContactID, META_PROTO, "ParentMeta", &dbv)) ? NULL : dbv.dVal;

		// whether we need conversion or not
		if (!GetContactSetting(dwContactID, META_PROTO, "MetaID", &dbv))
			arMetas.insert(new COldMeta(dbv.dVal, cc));
	}
}
