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

static HANDLE hEventDeletedEvent, hEventAddedEvent, hEventFilterAddedEvent;

STDMETHODIMP_(LONG) CDbxMdb::GetEventCount(MCONTACT contactID)
{
	mir_cslock lck(m_csDbAccess);
	return -1;
}

STDMETHODIMP_(MEVENT) CDbxMdb::AddEvent(MCONTACT contactID, DBEVENTINFO *dbei)
{
	if (dbei == NULL || dbei->cbSize != sizeof(DBEVENTINFO)) return 0;
	if (dbei->timestamp == 0) return 0;

	DBEvent dbe;
	dbe.signature = DBEVENT_SIGNATURE;
	dbe.contactID = contactID; // store native or subcontact's id
	dbe.timestamp = dbei->timestamp;
	dbe.flags = dbei->flags;
	dbe.wEventType = dbei->eventType;
	dbe.cbBlob = dbei->cbBlob;
	BYTE *pBlob = dbei->pBlob;

	MCONTACT contactNotifyID = contactID;
	DBCachedContact *ccSub = NULL;
	if (contactID != 0) {
		DBCachedContact *cc = m_cache->GetCachedContact(contactID);
		if (cc == NULL)
			return NULL;

		if (cc->IsSub()) {
			ccSub = cc;
			// set default sub to the event's source
			if (!(dbei->flags & DBEF_SENT))
				db_mc_setDefault(cc->parentID, contactID, false);
			contactID = cc->parentID; // and add an event to a metahistory
			if (db_mc_isEnabled())
				contactNotifyID = contactID;
		}
	}

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

	lck.unlock();

	// Notify only in safe mode or on really new events
	NotifyEventHooks(hEventAddedEvent, contactNotifyID, (LPARAM)-1);

	return (MEVENT)0;
}

STDMETHODIMP_(BOOL) CDbxMdb::DeleteEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	DBCachedContact *cc;
	if (contactID) {
		if ((cc = m_cache->GetCachedContact(contactID)) == NULL)
			return 2;
		if (cc->IsSub())
			if ((cc = m_cache->GetCachedContact(cc->parentID)) == NULL)
				return 3;
	}
	else cc = NULL;

	mir_cslockfull lck(m_csDbAccess);
	lck.unlock();

	// call notifier while outside mutex
	NotifyEventHooks(hEventDeletedEvent, contactID, (LPARAM)hDbEvent);

	// get back in
	lck.lock();
	return 0;
}

STDMETHODIMP_(LONG) CDbxMdb::GetBlobSize(MEVENT hDbEvent)
{
	mir_cslock lck(m_csDbAccess);
	return -1;
}

STDMETHODIMP_(BOOL) CDbxMdb::GetEvent(MEVENT hDbEvent, DBEVENTINFO *dbei)
{
	if (dbei == NULL || dbei->cbSize != sizeof(DBEVENTINFO)) return 1;
	if (dbei->cbBlob > 0 && dbei->pBlob == NULL) {
		dbei->cbBlob = 0;
		return 1;
	}

	mir_cslock lck(m_csDbAccess);
	return 0;
}

STDMETHODIMP_(BOOL) CDbxMdb::MarkEventRead(MCONTACT contactID, MEVENT hDbEvent)
{
	DBCachedContact *cc;
	if (contactID) {
		if ((cc = m_cache->GetCachedContact(contactID)) == NULL)
			return -1;
		if (cc->IsSub())
			if ((cc = m_cache->GetCachedContact(cc->parentID)) == NULL)
				return -1;
	}
	else cc = NULL;

	mir_cslockfull lck(m_csDbAccess);

	lck.unlock();
	NotifyEventHooks(hEventMarkedRead, contactID, (LPARAM)hDbEvent);
	return -11;
}

STDMETHODIMP_(MCONTACT) CDbxMdb::GetEventContact(MEVENT hDbEvent)
{
	mir_cslock lck(m_csDbAccess);
	return INVALID_CONTACT_ID;
}

STDMETHODIMP_(MEVENT) CDbxMdb::FindFirstEvent(MCONTACT contactID)
{
	mir_cslock lck(m_csDbAccess);
	return NULL;
}

STDMETHODIMP_(MEVENT) CDbxMdb::FindFirstUnreadEvent(MCONTACT contactID)
{
	mir_cslock lck(m_csDbAccess);
	return NULL;
}

STDMETHODIMP_(MEVENT) CDbxMdb::FindLastEvent(MCONTACT contactID)
{
	mir_cslock lck(m_csDbAccess);
	return NULL;
}

STDMETHODIMP_(MEVENT) CDbxMdb::FindNextEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	DBCachedContact *cc = (contactID) ? m_cache->GetCachedContact(contactID) : NULL;

	mir_cslock lck(m_csDbAccess);
	return NULL;
}

STDMETHODIMP_(MEVENT) CDbxMdb::FindPrevEvent(MCONTACT contactID, MEVENT hDbEvent)
{
	DBCachedContact *cc = (contactID) ? m_cache->GetCachedContact(contactID) : NULL;

	mir_cslock lck(m_csDbAccess);
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// low-level history cleaner

int CDbxMdb::WipeContactHistory(DBContact *dbc)
{
	// drop subContact's history if any
	return 0;
}
