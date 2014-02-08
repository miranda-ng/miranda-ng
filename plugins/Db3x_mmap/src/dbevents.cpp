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

DWORD GetModuleNameOfs(const char *szName);
char *GetModuleNameByOfs(DWORD ofs);

static HANDLE hEventDeletedEvent, hEventAddedEvent, hEventFilterAddedEvent;

STDMETHODIMP_(LONG) CDb3Mmap::GetEventCount(MCONTACT contactID)
{
	mir_cslock lck(m_csDbAccess);
	DBContact *dbc = (DBContact*)DBRead(GetContactOffset(contactID), sizeof(DBContact), NULL);
	return (dbc->signature != DBCONTACT_SIGNATURE) ? -1 : dbc->eventCount;
}

STDMETHODIMP_(HANDLE) CDb3Mmap::AddEvent(MCONTACT contactID, DBEVENTINFO *dbei)
{
	if (dbei == NULL || dbei->cbSize != sizeof(DBEVENTINFO)) return 0;
	if (dbei->timestamp == 0) return 0;

	if (NotifyEventHooks(hEventFilterAddedEvent, contactID, (LPARAM)dbei))
		return 0;

	DBEvent dbe;
	dbe.signature = DBEVENT_SIGNATURE;
	dbe.timestamp = dbei->timestamp;
	dbe.flags = dbei->flags;
	dbe.eventType = dbei->eventType;
	dbe.cbBlob = dbei->cbBlob;
	BYTE *pBlob = dbei->pBlob;

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

	bool neednotify;
	mir_cslockfull lck(m_csDbAccess);

	DWORD ofsContact = GetContactOffset(contactID);
	DBContact dbc = *(DBContact*)DBRead(ofsContact, sizeof(DBContact), NULL);
	if (dbc.signature != DBCONTACT_SIGNATURE)
		return 0;

	DWORD ofsNew = CreateNewSpace(offsetof(DBEvent, blob) + dbe.cbBlob);

	dbe.ofsModuleName = GetModuleNameOfs(dbei->szModule);
	// find where to put it - sort by timestamp
	if (dbc.eventCount == 0) {
		dbe.ofsPrev = ofsContact;
		dbe.ofsNext = 0;
		dbe.flags |= DBEF_FIRST;
		dbc.ofsFirstEvent = dbc.ofsLastEvent = ofsNew;
	}
	else {
		DBEvent *dbeTest = (DBEvent*)DBRead(dbc.ofsFirstEvent, sizeof(DBEvent), NULL);
		// Should new event be placed before first event in chain?
		if (dbe.timestamp < dbeTest->timestamp) {
			dbe.ofsPrev = ofsContact;
			dbe.ofsNext = dbc.ofsFirstEvent;
			dbe.flags |= DBEF_FIRST;
			dbc.ofsFirstEvent = ofsNew;
			dbeTest = (DBEvent*)DBRead(dbe.ofsNext, sizeof(DBEvent), NULL);
			dbeTest->flags &= ~DBEF_FIRST;
			dbeTest->ofsPrev = ofsNew;
			DBWrite(dbe.ofsNext, dbeTest, sizeof(DBEvent));
		}
		else {
			// Loop through the chain, starting at the end
			DWORD ofsThis = dbc.ofsLastEvent;
			dbeTest = (DBEvent*)DBRead(ofsThis, sizeof(DBEvent), NULL);
			for (;;) {
				// If the new event's timesstamp is equal to or greater than the
				// current dbevent, it will be inserted after. If not, continue
				// with the previous dbevent in chain.
				if (dbe.timestamp >= dbeTest->timestamp) {
					dbe.ofsPrev = ofsThis;
					dbe.ofsNext = dbeTest->ofsNext;
					dbeTest->ofsNext = ofsNew;
					DBWrite(ofsThis, dbeTest, sizeof(DBEvent));
					if (dbe.ofsNext == 0)
						dbc.ofsLastEvent = ofsNew;
					else {
						dbeTest = (DBEvent*)DBRead(dbe.ofsNext, sizeof(DBEvent), NULL);
						dbeTest->ofsPrev = ofsNew;
						DBWrite(dbe.ofsNext, dbeTest, sizeof(DBEvent));
					}
					break;
				}
				ofsThis = dbeTest->ofsPrev;
				dbeTest = (DBEvent*)DBRead(ofsThis, sizeof(DBEvent), NULL);
			}
		}
	}
	dbc.eventCount++;

	if (!(dbe.flags & (DBEF_READ | DBEF_SENT))) {
		if (dbe.timestamp < dbc.timestampFirstUnread || dbc.timestampFirstUnread == 0) {
			dbc.timestampFirstUnread = dbe.timestamp;
			dbc.ofsFirstUnreadEvent = ofsNew;
		}
		neednotify = true;
	}
	else neednotify = m_safetyMode;

	DBWrite(ofsContact, &dbc, sizeof(DBContact));
	DBWrite(ofsNew, &dbe, offsetof(DBEvent, blob));
	DBWrite(ofsNew + offsetof(DBEvent, blob), pBlob, dbe.cbBlob);
	DBFlush(0);
	lck.unlock();

	log1("add event @ %08x", ofsNew);

	// Notify only in safe mode or on really new events
	if (neednotify)
		NotifyEventHooks(hEventAddedEvent, contactID, (LPARAM)ofsNew);

	return (HANDLE)ofsNew;
}

STDMETHODIMP_(BOOL) CDb3Mmap::DeleteEvent(MCONTACT contactID, HANDLE hDbEvent)
{
	mir_cslockfull lck(m_csDbAccess);
	DWORD ofsContact = GetContactOffset(contactID);
	DBContact dbc = *(DBContact*)DBRead(ofsContact, sizeof(DBContact), NULL);
	DBEvent dbe = *(DBEvent*)DBRead((DWORD)hDbEvent, sizeof(DBEvent), NULL);
	if (dbc.signature != DBCONTACT_SIGNATURE || dbe.signature != DBEVENT_SIGNATURE)
		return 1;

	lck.unlock();
	log1("delete event @ %08x", hContact);

	//call notifier while outside mutex
	NotifyEventHooks(hEventDeletedEvent, contactID, (LPARAM)hDbEvent);

	//get back in
	lck.lock();
	dbc = *(DBContact*)DBRead(ofsContact, sizeof(DBContact), NULL);
	dbe = *(DBEvent*)DBRead((DWORD)hDbEvent, sizeof(DBEvent), NULL);

	//check if this was the first unread, if so, recalc the first unread
	if (dbc.ofsFirstUnreadEvent == (DWORD)hDbEvent) {
		DBEvent *dbeNext = &dbe;
		for (;;) {
			if (dbeNext->ofsNext == 0) {
				dbc.ofsFirstUnreadEvent = 0;
				dbc.timestampFirstUnread = 0;
				break;
			}
			DWORD ofsThis = dbeNext->ofsNext;
			dbeNext = (DBEvent*)DBRead(ofsThis, sizeof(DBEvent), NULL);
			if (!(dbeNext->flags & (DBEF_READ | DBEF_SENT))) {
				dbc.ofsFirstUnreadEvent = ofsThis;
				dbc.timestampFirstUnread = dbeNext->timestamp;
				break;
			}
		}
	}

	//get previous and next events in chain and change offsets
	if (dbe.flags & DBEF_FIRST) {
		if (dbe.ofsNext == 0)
			dbc.ofsFirstEvent = dbc.ofsLastEvent = 0;
		else {
			DBEvent *dbeNext = (DBEvent*)DBRead(dbe.ofsNext, sizeof(DBEvent), NULL);
			dbeNext->flags |= DBEF_FIRST;
			dbeNext->ofsPrev = dbe.ofsPrev;
			DBWrite(dbe.ofsNext, dbeNext, sizeof(DBEvent));
			dbc.ofsFirstEvent = dbe.ofsNext;
		}
	}
	else {
		if (dbe.ofsNext == 0) {
			DBEvent *dbePrev = (DBEvent*)DBRead(dbe.ofsPrev, sizeof(DBEvent), NULL);
			dbePrev->ofsNext = 0;
			DBWrite(dbe.ofsPrev, dbePrev, sizeof(DBEvent));
			dbc.ofsLastEvent = dbe.ofsPrev;
		}
		else {
			DBEvent *dbePrev = (DBEvent*)DBRead(dbe.ofsPrev, sizeof(DBEvent), NULL);
			dbePrev->ofsNext = dbe.ofsNext;
			DBWrite(dbe.ofsPrev, dbePrev, sizeof(DBEvent));

			DBEvent *dbeNext = (DBEvent*)DBRead(dbe.ofsNext, sizeof(DBEvent), NULL);
			dbeNext->ofsPrev = dbe.ofsPrev;
			DBWrite(dbe.ofsNext, dbeNext, sizeof(DBEvent));
		}
	}
	//delete event
	DeleteSpace((DWORD)hDbEvent, offsetof(DBEvent, blob) + dbe.cbBlob);
	//decrement event count
	dbc.eventCount--;
	DBWrite(ofsContact, &dbc, sizeof(DBContact));
	DBFlush(0);
	return 0;
}

STDMETHODIMP_(LONG) CDb3Mmap::GetBlobSize(HANDLE hDbEvent)
{
	mir_cslock lck(m_csDbAccess);
	DBEvent *dbe = (DBEvent*)DBRead((DWORD)hDbEvent, sizeof(DBEvent), NULL);
	return (dbe->signature != DBEVENT_SIGNATURE) ? -1 : dbe->cbBlob;
}

STDMETHODIMP_(BOOL) CDb3Mmap::GetEvent(HANDLE hDbEvent, DBEVENTINFO *dbei)
{
	if (dbei == NULL || dbei->cbSize != sizeof(DBEVENTINFO)) return 1;
	if (dbei->cbBlob > 0 && dbei->pBlob == NULL) {
		dbei->cbBlob = 0;
		return 1;
	}

	mir_cslock lck(m_csDbAccess);
	DBEvent *dbe = (DBEvent*)DBRead((DWORD)hDbEvent, sizeof(DBEvent), NULL);
	if (dbe->signature != DBEVENT_SIGNATURE)
		return 1;

	dbei->szModule = GetModuleNameByOfs(dbe->ofsModuleName);
	dbei->timestamp = dbe->timestamp;
	dbei->flags = dbe->flags;
	dbei->eventType = dbe->eventType;
	int bytesToCopy = (dbei->cbBlob < dbe->cbBlob) ? dbei->cbBlob : dbe->cbBlob;
	dbei->cbBlob = dbe->cbBlob;
	if (bytesToCopy && dbei->pBlob) {
		BYTE *pSrc = DBRead(DWORD(hDbEvent) + offsetof(DBEvent, blob), bytesToCopy, NULL);
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
		else MoveMemory(dbei->pBlob, pSrc, bytesToCopy);
	}
	return 0;
}

STDMETHODIMP_(BOOL) CDb3Mmap::MarkEventRead(MCONTACT contactID, HANDLE hDbEvent)
{
	mir_cslock lck(m_csDbAccess);
	DWORD ofsContact = GetContactOffset(contactID);
	DBContact dbc = *(DBContact*)DBRead(ofsContact, sizeof(DBContact), NULL);
	DBEvent *dbe = (DBEvent*)DBRead((DWORD)hDbEvent, sizeof(DBEvent), NULL);
	if (dbe->signature != DBEVENT_SIGNATURE || dbc.signature != DBCONTACT_SIGNATURE)
		return -1;

	if ((dbe->flags & DBEF_READ) || (dbe->flags & DBEF_SENT))
		return (INT_PTR)dbe->flags;

	//log1("mark read @ %08x", hContact);
	dbe->flags |= DBEF_READ;
	DBWrite((DWORD)hDbEvent, dbe, sizeof(DBEvent));
	BOOL ret = dbe->flags;
	if (dbc.ofsFirstUnreadEvent == (DWORD)hDbEvent) {
		for (;;) {
			if (dbe->ofsNext == 0) {
				dbc.ofsFirstUnreadEvent = 0;
				dbc.timestampFirstUnread = 0;
				break;
			}
			DWORD ofsThis = dbe->ofsNext;
			dbe = (DBEvent*)DBRead(ofsThis, sizeof(DBEvent), NULL);
			if (!(dbe->flags & (DBEF_READ | DBEF_SENT))) {
				dbc.ofsFirstUnreadEvent = ofsThis;
				dbc.timestampFirstUnread = dbe->timestamp;
				break;
			}
		}
	}
	DBWrite(ofsContact, &dbc, sizeof(DBContact));
	DBFlush(0);
	return ret;
}

STDMETHODIMP_(HANDLE) CDb3Mmap::GetEventContact(HANDLE hDbEvent)
{
	mir_cslock lck(m_csDbAccess);
	DBEvent *dbe = (DBEvent*)DBRead((DWORD)hDbEvent, sizeof(DBEvent), NULL);
	if (dbe->signature != DBEVENT_SIGNATURE)
		return (HANDLE)-1;

	while (!(dbe->flags & DBEF_FIRST))
		dbe = (DBEvent*)DBRead(dbe->ofsPrev, sizeof(DBEvent), NULL);

	return (HANDLE)dbe->ofsPrev;
}

STDMETHODIMP_(HANDLE) CDb3Mmap::FindFirstEvent(MCONTACT contactID)
{
	mir_cslock lck(m_csDbAccess);
	DWORD ofsContact = GetContactOffset(contactID);
	DBContact *dbc = (DBContact*)DBRead(ofsContact, sizeof(DBContact), NULL);
	return (dbc->signature != DBCONTACT_SIGNATURE) ? 0 : (HANDLE)dbc->ofsFirstEvent;
}

STDMETHODIMP_(HANDLE) CDb3Mmap::FindFirstUnreadEvent(MCONTACT contactID)
{
	mir_cslock lck(m_csDbAccess);
	DWORD ofsContact = GetContactOffset(contactID);
	DBContact *dbc = (DBContact*)DBRead(ofsContact, sizeof(DBContact), NULL);
	return (dbc->signature != DBCONTACT_SIGNATURE) ? 0 : (HANDLE)dbc->ofsFirstUnreadEvent;
}

STDMETHODIMP_(HANDLE) CDb3Mmap::FindLastEvent(MCONTACT contactID)
{
	mir_cslock lck(m_csDbAccess);
	DWORD ofsContact = GetContactOffset(contactID);
	DBContact *dbc = (DBContact*)DBRead(ofsContact, sizeof(DBContact), NULL);
	return (dbc->signature != DBCONTACT_SIGNATURE) ? 0 : (HANDLE)dbc->ofsLastEvent;
}

STDMETHODIMP_(HANDLE) CDb3Mmap::FindNextEvent(HANDLE hDbEvent)
{
	mir_cslock lck(m_csDbAccess);
	DBEvent *dbe = (DBEvent*)DBRead((DWORD)hDbEvent, sizeof(DBEvent), NULL);
	return (dbe->signature != DBEVENT_SIGNATURE) ? 0 : (HANDLE)dbe->ofsNext;
}

STDMETHODIMP_(HANDLE) CDb3Mmap::FindPrevEvent(HANDLE hDbEvent)
{
	mir_cslock lck(m_csDbAccess);
	DBEvent *dbe = (DBEvent*)DBRead((DWORD)hDbEvent, sizeof(DBEvent), NULL);
	if (dbe->signature != DBEVENT_SIGNATURE) return 0;
	return (dbe->flags & DBEF_FIRST) ? 0 : (HANDLE)dbe->ofsPrev;
}
