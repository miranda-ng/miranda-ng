/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012-14 Miranda NG project,
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

int CDb3Base::CheckProto(HANDLE hContact, const char *proto)
{
	DBCachedContact *cc = m_cache->GetCachedContact(hContact);
	if (cc == NULL)
		cc = m_cache->AddContactToCache(hContact);

	if (cc->szProto == NULL) {
		char protobuf[MAX_PATH] = {0};
		DBVARIANT dbv;
 		dbv.type = DBVT_ASCIIZ;
		dbv.pszVal = protobuf;
		dbv.cchVal = sizeof(protobuf);
		if (GetContactSettingStatic(hContact, "Protocol", "p", &dbv) != 0 || (dbv.type != DBVT_ASCIIZ))
			return 0;

		cc->szProto = m_cache->GetCachedSetting(NULL, protobuf, 0, (int)strlen(protobuf));
	}

	return !strcmp(cc->szProto, proto);
}

STDMETHODIMP_(LONG) CDb3Base::GetContactCount(void)
{
	mir_cslock lck(m_csDbAccess);
	return m_dbHeader.contactCount;
}

STDMETHODIMP_(HANDLE) CDb3Base::FindFirstContact(const char *szProto)
{
	mir_cslock lck(m_csDbAccess);
	HANDLE ret = (HANDLE)m_dbHeader.ofsFirstContact;
	if (szProto && !CheckProto(ret, szProto))
		ret = FindNextContact(ret, szProto);
	return ret;
}

STDMETHODIMP_(HANDLE) CDb3Base::FindNextContact(HANDLE hContact, const char *szProto)
{
	mir_cslock lck(m_csDbAccess);
	while (hContact) {
		DBCachedContact *VL = m_cache->GetCachedContact(hContact);
		if (VL != NULL) {
			if (VL->hNext != NULL) {
				if (!szProto || CheckProto(VL->hNext, szProto))
					return VL->hNext;

				hContact = VL->hNext;
				continue;
		}	}

		DBContact *dbc = (DBContact*)DBRead(hContact, sizeof(DBContact), NULL);
		if (dbc->signature != DBCONTACT_SIGNATURE)
			break;

		if (VL == NULL)
			VL = m_cache->AddContactToCache(hContact);

		VL->hNext = (HANDLE)dbc->ofsNext;
		if (VL->hNext != NULL && (!szProto || CheckProto(VL->hNext, szProto)))
			return VL->hNext;

		hContact = VL->hNext;
	}

	return NULL;
}

STDMETHODIMP_(LONG) CDb3Base::DeleteContact(HANDLE hContact)
{
	if (hContact == NULL)
		return 1;

	mir_cslockfull lck(m_csDbAccess);
	DBContact *dbc = (DBContact*)DBRead(hContact, sizeof(DBContact), NULL);
	if (dbc->signature != DBCONTACT_SIGNATURE)
		return 1;

	if (hContact == (HANDLE)m_dbHeader.ofsUser) {
		log0("FATAL: del of user chain attempted.");
		return 1;
	}

	lck.unlock();
	log0("del contact");

	// call notifier while outside mutex
	NotifyEventHooks(hContactDeletedEvent, (WPARAM)hContact, 0);

	// get back in
	lck.lock();

	m_cache->FreeCachedContact(hContact);
	if (hContact == m_hLastCachedContact)
		m_hLastCachedContact = NULL;

	dbc = (DBContact*)DBRead(hContact, sizeof(DBContact), NULL);
	//delete settings chain
	DWORD ofsThis = dbc->ofsFirstSettings;
	DWORD ofsFirstEvent = dbc->ofsFirstEvent;
	while (ofsThis) {
		DBContactSettings *dbcs = (DBContactSettings*)DBRead(ofsThis,sizeof(DBContactSettings),NULL);
		DWORD ofsNext = dbcs->ofsNext;
		DeleteSpace(ofsThis,offsetof(DBContactSettings,blob)+dbcs->cbBlob);
		ofsThis = ofsNext;
	}
	//delete event chain
	ofsThis = ofsFirstEvent;
	while (ofsThis) {
		DBEvent *dbe = (DBEvent*)DBRead(ofsThis,sizeof(DBEvent),NULL);
		DWORD ofsNext = dbe->ofsNext;
		DeleteSpace(ofsThis,offsetof(DBEvent,blob)+dbe->cbBlob);
		ofsThis = ofsNext;
	}
	//find previous contact in chain and change ofsNext
	dbc = (DBContact*)DBRead(hContact, sizeof(DBContact), NULL);
	if (m_dbHeader.ofsFirstContact == (DWORD)hContact) {
		m_dbHeader.ofsFirstContact = dbc->ofsNext;
		DBWrite(0, &m_dbHeader, sizeof(m_dbHeader));
	}
	else {
		DWORD ofsNext = dbc->ofsNext;
		ofsThis = m_dbHeader.ofsFirstContact;
		DBContact *dbcPrev = (DBContact*)DBRead(ofsThis,sizeof(DBContact),NULL);
		while (dbcPrev->ofsNext != (DWORD)hContact) {
			if (dbcPrev->ofsNext == 0) DatabaseCorruption(NULL);
			ofsThis = dbcPrev->ofsNext;
			dbcPrev = (DBContact*)DBRead(ofsThis,sizeof(DBContact),NULL);
		}
		dbcPrev->ofsNext = ofsNext;
		DBWrite(ofsThis,dbcPrev,sizeof(DBContact));
	}

	//delete contact
	DeleteSpace((DWORD)hContact, sizeof(DBContact));
	//decrement contact count
	m_dbHeader.contactCount--;
	DBWrite(0, &m_dbHeader, sizeof(m_dbHeader));
	DBFlush(0);
	return 0;
}

STDMETHODIMP_(HANDLE) CDb3Base::AddContact()
{
	DWORD ofsNew;
	log0("add contact");
   {
		mir_cslock lck(m_csDbAccess);
		ofsNew = CreateNewSpace(sizeof(DBContact));

		DBContact dbc = { 0 };
		dbc.signature = DBCONTACT_SIGNATURE;
		dbc.ofsNext = m_dbHeader.ofsFirstContact;
		m_dbHeader.ofsFirstContact = ofsNew;
		m_dbHeader.contactCount++;
		DBWrite(ofsNew, &dbc, sizeof(DBContact));
		DBWrite(0, &m_dbHeader, sizeof(m_dbHeader));
		DBFlush(0);
	}
	m_cache->AddContactToCache((HANDLE)ofsNew);

	NotifyEventHooks(hContactAddedEvent,(WPARAM)ofsNew,0);
	return (HANDLE)ofsNew;
}

STDMETHODIMP_(BOOL) CDb3Base::IsDbContact(HANDLE hContact)
{
	if (m_cache->GetCachedContact(hContact))
		return TRUE;

	mir_cslock lck(m_csDbAccess);
	DBContact *dbc = (DBContact*)DBRead(hContact,sizeof(DBContact),NULL);
	if (dbc->signature == DBCONTACT_SIGNATURE) {
		m_cache->AddContactToCache(hContact);
		return TRUE;
	}

	return FALSE;
}
