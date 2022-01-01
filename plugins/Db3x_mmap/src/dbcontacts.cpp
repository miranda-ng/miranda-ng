/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
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

STDMETHODIMP_(int) CDb3Mmap::GetContactCount(void)
{
	mir_cslock lck(m_csDbAccess);
	return m_dbHeader.contactCount;
}

STDMETHODIMP_(int) CDb3Mmap::GetContactSize(void)
{
	return sizeof(DBCachedContact);
}

STDMETHODIMP_(int) CDb3Mmap::DeleteContact(MCONTACT contactID)
{
	if (contactID == 0) // global contact cannot be removed
		return 1;

	mir_cslockfull lck(m_csDbAccess);
	uint32_t ofsContact = GetContactOffset(contactID);

	DBContact *dbc = (DBContact*)DBRead(ofsContact, nullptr);
	if (dbc->signature != DBCONTACT_SIGNATURE)
		return 1;

	if (ofsContact == m_dbHeader.ofsUser) {
		log0("FATAL: del of user chain attempted.");
		return 1;
	}

	lck.unlock();
	log0("del contact");

	// call notifier while outside mutex
	NotifyEventHooks(g_hevContactDeleted, contactID, 0);

	// get back in
	lck.lock();

	// delete settings chain
	uint32_t ofsThis = dbc->ofsFirstSettings;
	uint32_t ofsFirstEvent = dbc->ofsFirstEvent;
	while (ofsThis) {
		DBContactSettings *dbcs = (DBContactSettings*)DBRead(ofsThis, nullptr);
		uint32_t ofsNext = dbcs->ofsNext;
		DeleteSpace(ofsThis, offsetof(DBContactSettings, blob) + dbcs->cbBlob);
		ofsThis = ofsNext;
	}

	// delete event chain
	ofsThis = ofsFirstEvent;
	while (ofsThis) {
		DBEvent *dbe = (DBEvent*)DBRead(ofsThis, nullptr);
		uint32_t ofsNext = dbe->ofsNext;
		DeleteSpace(ofsThis, offsetof(DBEvent, blob) + dbe->cbBlob);
		ofsThis = ofsNext;
	}

	// find previous contact in chain and change ofsNext
	if (m_dbHeader.ofsFirstContact == ofsContact) {
		m_dbHeader.ofsFirstContact = dbc->ofsNext;
		DBWrite(0, &m_dbHeader, sizeof(m_dbHeader));
	}
	else {
		uint32_t ofsNext = dbc->ofsNext;
		ofsThis = m_dbHeader.ofsFirstContact;
		DBContact *dbcPrev = (DBContact*)DBRead(ofsThis, nullptr);
		while (dbcPrev->ofsNext != ofsContact) {
			if (dbcPrev->ofsNext == 0)
				DatabaseCorruption(nullptr);
			ofsThis = dbcPrev->ofsNext;
			dbcPrev = (DBContact*)DBRead(ofsThis, nullptr);
		}
		dbcPrev->ofsNext = ofsNext;
		DBWrite(ofsThis, dbcPrev, sizeof(DBContact));
	}

	// delete contact
	DeleteSpace(ofsContact, sizeof(DBContact));

	// decrement contact count
	m_dbHeader.contactCount--;
	DBWrite(0, &m_dbHeader, sizeof(m_dbHeader));
	DBFlush(0);

	// free cache item
	m_cache->FreeCachedContact(contactID);
	if (contactID == m_hLastCachedContact)
		m_hLastCachedContact = 0;
	return 0;
}

STDMETHODIMP_(MCONTACT) CDb3Mmap::AddContact()
{
	uint32_t ofsNew;
	log0("add contact");

	DBContact dbc = { 0 };
	dbc.signature = DBCONTACT_SIGNATURE;
	{
		mir_cslock lck(m_csDbAccess);
		ofsNew = CreateNewSpace(sizeof(DBContact));

		dbc.ofsNext = m_dbHeader.ofsFirstContact;
		dbc.dwContactID = m_dwMaxContactId++;
		m_dbHeader.ofsFirstContact = ofsNew;
		m_dbHeader.contactCount++;
		DBWrite(ofsNew, &dbc, sizeof(DBContact));
		DBWrite(0, &m_dbHeader, sizeof(m_dbHeader));
		DBFlush(0);
	}

	DBCachedContact *cc = m_cache->AddContactToCache(dbc.dwContactID);
	cc->dwOfsContact = ofsNew;

	NotifyEventHooks(g_hevContactAdded, dbc.dwContactID, 0);
	return dbc.dwContactID;
}

STDMETHODIMP_(BOOL) CDb3Mmap::IsDbContact(MCONTACT contactID)
{
	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	if (cc == nullptr)
		return FALSE;

	mir_cslock lck(m_csDbAccess);
	DBContact *dbc = (DBContact*)DBRead(cc->dwOfsContact, nullptr);
	if (dbc->signature == DBCONTACT_SIGNATURE) {
		m_cache->AddContactToCache(contactID);
		return TRUE;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// metacontacts support

static int SortEvent(const DBEvent *p1, const DBEvent *p2)
{
	return (LONG)p1->timestamp - (LONG)p2->timestamp;
}

BOOL CDb3Mmap::MetaMergeHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	mir_cslock lck(m_csDbAccess);
	DBContact *dbMeta = (DBContact*)DBRead(ccMeta->dwOfsContact, nullptr);
	DBContact *dbSub = (DBContact*)DBRead(ccSub->dwOfsContact, nullptr);
	if (dbMeta->signature != DBCONTACT_SIGNATURE || dbSub->signature != DBCONTACT_SIGNATURE)
		return 1;

	// special cases
	if (dbSub->ofsFirstEvent == 0) // hurrah, nothing to do
		return 0;

	LIST<DBEvent> arEvents(20000, SortEvent);
	BOOL ret = 0;
	__try {
		if (dbMeta->ofsFirstEvent == 0) { // simply chain history to a meta
			dbMeta->eventCount = dbSub->eventCount;
			dbMeta->ofsFirstEvent = dbSub->ofsFirstEvent;
			dbMeta->ofsLastEvent = dbSub->ofsLastEvent;
			dbMeta->ofsFirstUnread = dbSub->ofsFirstUnread;
			dbMeta->tsFirstUnread = dbSub->tsFirstUnread;
		}
		else {
			// there're events in both meta's & sub's event chains
			// relink sub's event chain to meta without changing events themselves
			for (uint32_t ofsMeta = dbMeta->ofsFirstEvent; ofsMeta != 0;) {
				DBEvent *pev = (DBEvent*)DBRead(ofsMeta, nullptr);
				if (pev->signature != DBEVENT_SIGNATURE) { // broken chain, don't touch it
					ret = 2;
					__leave;
				}

				arEvents.insert(pev);
				ofsMeta = pev->ofsNext;
			}

			for (uint32_t ofsSub = dbSub->ofsFirstEvent; ofsSub != 0;) {
				DBEvent *pev = (DBEvent*)DBRead(ofsSub, nullptr);
				if (pev->signature != DBEVENT_SIGNATURE) { // broken chain, don't touch it
					ret = 2;
					__leave;
				}

				arEvents.insert(pev);
				ofsSub = pev->ofsNext;
			}

			// all events are in memory, valid & sorted in the right order.
			// ready? steady? go!
			dbMeta->eventCount = arEvents.getCount();

			DBEvent *pFirst = arEvents[0];
			dbMeta->ofsFirstEvent = uint32_t((uint8_t*)pFirst - m_pDbCache);
			pFirst->ofsPrev = 0;
			dbMeta->ofsFirstUnread = pFirst->markedRead() ? 0 : dbMeta->ofsFirstEvent;

			DBEvent *pLast = arEvents[arEvents.getCount() - 1];
			dbMeta->ofsLastEvent = uint32_t((uint8_t*)pLast - m_pDbCache);
			pLast->ofsNext = 0;

			for (int i = 1; i < arEvents.getCount(); i++) {
				DBEvent *pPrev = arEvents[i - 1], *pNext = arEvents[i];
				pPrev->ofsNext = uint32_t((uint8_t*)pNext - m_pDbCache);
				pNext->ofsPrev = uint32_t((uint8_t*)pPrev - m_pDbCache);

				if (dbMeta->ofsFirstUnread == 0 && !pNext->markedRead())
					dbMeta->ofsFirstUnread = pPrev->ofsNext;
			}
		}

		// remove any traces of history from sub
		dbSub->ofsFirstEvent = dbSub->ofsLastEvent = dbSub->ofsFirstUnread = dbSub->tsFirstUnread = 0;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ret = 3;
	}

	FlushViewOfFile(m_pDbCache, 0);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL CDb3Mmap::MetaSplitHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	mir_cslock lck(m_csDbAccess);
	DBContact dbMeta = *(DBContact*)DBRead(ccMeta->dwOfsContact, nullptr);
	DBContact dbSub = *(DBContact*)DBRead(ccSub->dwOfsContact, nullptr);
	if (dbMeta.signature != DBCONTACT_SIGNATURE || dbSub.signature != DBCONTACT_SIGNATURE)
		return 1;

	if (dbMeta.ofsFirstEvent == 0) // nothing to do
		return 0;

	BOOL ret = 0;
	__try {
		if (ret = WipeContactHistory(&dbSub))
			__leave;

		uint32_t dwOffset = dbMeta.ofsFirstEvent;
		DBEvent *evMeta = nullptr, *evSub = nullptr;
		dbMeta.eventCount = 0; dbMeta.ofsFirstEvent = dbMeta.ofsLastEvent = dbMeta.ofsFirstUnread = dbMeta.tsFirstUnread = 0;

		while (dwOffset != 0) {
			DBEvent *evCurr = (DBEvent*)DBRead(dwOffset, nullptr);
			if (evCurr->signature != DBEVENT_SIGNATURE)
				break;

			uint32_t dwNext = evCurr->ofsNext; evCurr->ofsNext = 0;

			// extract it to sub's chain
			if (evCurr->contactID == ccSub->contactID) {
				dbSub.eventCount++;
				if (evSub != nullptr) {
					evSub->ofsNext = dwOffset;
					evCurr->ofsPrev = uint32_t((uint8_t*)evSub - m_pDbCache);
				}
				else {
					dbSub.ofsFirstEvent = dwOffset;
					evCurr->ofsPrev = 0;
				}
				if (dbSub.ofsFirstUnread == 0 && !evCurr->markedRead()) {
					dbSub.ofsFirstUnread = dwOffset;
					dbSub.tsFirstUnread = evCurr->timestamp;
				}
				dbSub.ofsLastEvent = dwOffset;
				evSub = evCurr;
			}
			else {
				dbMeta.eventCount++;
				if (evMeta != nullptr) {
					evMeta->ofsNext = dwOffset;
					evCurr->ofsPrev = uint32_t((uint8_t*)evMeta - m_pDbCache);
				}
				else {
					dbMeta.ofsFirstEvent = dwOffset;
					evCurr->ofsPrev = 0;
				}
				if (dbMeta.ofsFirstUnread == 0 && !evCurr->markedRead()) {
					dbMeta.ofsFirstUnread = dwOffset;
					dbMeta.tsFirstUnread = evCurr->timestamp;
				}
				dbMeta.ofsLastEvent = dwOffset;
				evMeta = evCurr;
			}

			dwOffset = dwNext;
		}

		DBWrite(ccSub->dwOfsContact, &dbSub, sizeof(DBContact));
		DBWrite(ccMeta->dwOfsContact, &dbMeta, sizeof(DBContact));
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ret = 3;
	}

	FlushViewOfFile(m_pDbCache, 0);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////
// initial cycle to fill the contacts' cache

struct COldMeta
{
	COldMeta(uint32_t _id, DBCachedContact *_cc) :
		hMetaID(_id), cc(_cc)
	{}

	uint32_t hMetaID;
	DBCachedContact *cc;
};

void CDb3Mmap::FillContacts()
{
	OBJLIST<COldMeta> arMetas(10, NumericKeySortT);

	for (uint32_t dwOffset = m_dbHeader.ofsFirstContact; dwOffset != 0;) {
		DBContact *p = (DBContact*)DBRead(dwOffset, nullptr);
		if (p->signature != DBCONTACT_SIGNATURE)
			break;

		uint32_t dwContactID;
		if (m_dbHeader.version >= DB_095_VERSION) {
			dwContactID = p->dwContactID;
			if (dwContactID >= m_dwMaxContactId)
				m_dwMaxContactId = dwContactID + 1;
		}
		else dwContactID = m_dwMaxContactId++;

		DBCachedContact *cc = m_cache->AddContactToCache(dwContactID);
		cc->dwOfsContact = dwOffset;
		CheckProto(cc, "");

		DBVARIANT dbv; dbv.type = DBVT_DWORD;
		cc->nSubs = (0 != GetContactSetting(dwContactID, META_PROTO, "NumContacts", &dbv)) ? -1 : dbv.dVal;
		if (cc->nSubs != -1) {
			cc->pSubs = (MCONTACT*)mir_alloc(cc->nSubs*sizeof(MCONTACT));
			for (int i = 0; i < cc->nSubs; i++) {
				char setting[100];
				mir_snprintf(setting, "Handle%d", i);
				cc->pSubs[i] = (0 != GetContactSetting(dwContactID, META_PROTO, setting, &dbv)) ? 0 : dbv.dVal;
			}
		}
		cc->nDefault = (0 != GetContactSetting(dwContactID, META_PROTO, "Default", &dbv)) ? -1 : dbv.dVal;
		cc->parentID = (0 != GetContactSetting(dwContactID, META_PROTO, "ParentMeta", &dbv)) ? 0 : dbv.dVal;

		// whether we need conversion or not
		if (!GetContactSetting(dwContactID, META_PROTO, "MetaID", &dbv))
			arMetas.insert(new COldMeta(dbv.dVal, cc));

		dwOffset = p->ofsNext;
	}

	// no need in conversion? quit then
	if (m_bReadOnly || arMetas.getCount() == 0)
		return;

	DBVARIANT dbv; dbv.type = DBVT_DWORD;
	for (MCONTACT hh = FindFirstContact(); hh; hh = FindNextContact(hh)) {
		if (GetContactSetting(hh, META_PROTO, "MetaLink", &dbv))
			continue;

		COldMeta *p = arMetas.find((COldMeta*)&dbv.dVal);
		if (p == nullptr)
			continue;

		if (GetContactSetting(hh, META_PROTO, "ContactNumber", &dbv))
			continue;

		DBCONTACTWRITESETTING dbws = { META_PROTO };
		dbws.value.type = DBVT_DWORD;

		DBCachedContact *ccMeta = p->cc;
		if (int(dbv.dVal) < ccMeta->nSubs) {
			ccMeta->pSubs[dbv.dVal] = hh;

			char setting[100];
			mir_snprintf(setting, "Handle%d", dbv.dVal);
			dbws.szSetting = setting;
			dbws.value.dVal = hh;
			WriteContactSetting(ccMeta->contactID, &dbws);
		}

		// store contact id instead of the old mc number
		dbws.szSetting = "ParentMeta";
		dbws.value.dVal = ccMeta->contactID;
		WriteContactSetting(hh, &dbws);

		// wipe out old data from subcontacts
		DeleteContactSetting(hh, META_PROTO, "ContactNumber");
		DeleteContactSetting(hh, META_PROTO, "MetaLink");
	}

	for (auto &p : arMetas) {
		DBCachedContact *ccMeta = p->cc;
		MCONTACT hContact = ccMeta->contactID;

		// we don't need it anymore
		if (!GetContactSetting(hContact, META_PROTO, "MetaID", &dbv)) {
			DeleteContactSetting(hContact, META_PROTO, "MetaID");
			WipeContactHistory((DBContact*)DBRead(ccMeta->dwOfsContact, nullptr));
		}

		for (int k = 0; k < ccMeta->nSubs; k++) {
			DBCachedContact *ccSub = m_cache->GetCachedContact(ccMeta->pSubs[k]);
			if (ccSub) {
				ccSub->parentID = hContact;
				MetaMergeHistory(ccMeta, ccSub);
			}
		}
	}
}

uint32_t CDb3Mmap::GetContactOffset(MCONTACT contactID, DBCachedContact **pcc)
{
	if (contactID == 0) {
		if (pcc) *pcc = nullptr;
		return m_dbHeader.ofsUser;
	}

	DBCachedContact *cc = m_cache->GetCachedContact(contactID);
	if (pcc) *pcc = cc;
	return (cc == nullptr) ? 0 : cc->dwOfsContact;
}
