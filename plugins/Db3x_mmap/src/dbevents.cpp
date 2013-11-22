/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012-13 Miranda NG project,
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

static HANDLE hEventDeletedEvent,hEventAddedEvent,hEventFilterAddedEvent;

STDMETHODIMP_(LONG) CDb3Base::GetEventCount(HANDLE hContact)
{
	mir_cslock lck(m_csDbAccess);
	if (hContact == 0)
		hContact = (HANDLE)m_dbHeader.ofsUser;

	DBContact *dbc = (DBContact*)DBRead(hContact,sizeof(DBContact),NULL);
	return (dbc->signature != DBCONTACT_SIGNATURE) ? -1 : dbc->eventCount;
}

STDMETHODIMP_(HANDLE) CDb3Base::AddEvent(HANDLE hContact, DBEVENTINFO *dbei)
{
	if (dbei == NULL || dbei->cbSize != sizeof(DBEVENTINFO)) return 0;
	if (dbei->timestamp == 0) return 0;
	if (NotifyEventHooks(hEventFilterAddedEvent, (WPARAM)hContact, (LPARAM)dbei))
		return 0;

	bool neednotify;
	mir_cslockfull lck(m_csDbAccess);

	DWORD ofsContact = (hContact == 0) ? m_dbHeader.ofsUser : (DWORD)hContact;
	DBContact dbc = *(DBContact*)DBRead(ofsContact,sizeof(DBContact),NULL);
	if (dbc.signature != DBCONTACT_SIGNATURE)
	  	return 0;

	DWORD ofsNew = CreateNewSpace(offsetof(DBEvent,blob) + dbei->cbBlob);
	DWORD ofsModuleName = GetModuleNameOfs(dbei->szModule);

	DBEvent dbe;
	dbe.signature = DBEVENT_SIGNATURE;
	dbe.ofsModuleName = ofsModuleName;
	dbe.timestamp = dbei->timestamp;
	dbe.flags = dbei->flags;
	dbe.eventType = dbei->eventType;
	dbe.cbBlob = dbei->cbBlob;
	//find where to put it - sort by timestamp
	if (dbc.eventCount == 0) {
		dbe.ofsPrev = (DWORD)hContact;
		dbe.ofsNext = 0;
		dbe.flags |= DBEF_FIRST;
		dbc.ofsFirstEvent = dbc.ofsLastEvent = ofsNew;
	}
	else {
		DBEvent *dbeTest = (DBEvent*)DBRead(dbc.ofsFirstEvent,sizeof(DBEvent),NULL);
		// Should new event be placed before first event in chain?
		if (dbei->timestamp < dbeTest->timestamp) {
			dbe.ofsPrev = (DWORD)hContact;
			dbe.ofsNext = dbc.ofsFirstEvent;
			dbe.flags |= DBEF_FIRST;
			dbc.ofsFirstEvent = ofsNew;
			dbeTest = (DBEvent*)DBRead(dbe.ofsNext,sizeof(DBEvent),NULL);
			dbeTest->flags &= ~DBEF_FIRST;
			dbeTest->ofsPrev = ofsNew;
			DBWrite(dbe.ofsNext,dbeTest,sizeof(DBEvent));
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

	if (!(dbe.flags&(DBEF_READ|DBEF_SENT))) {
		if (dbe.timestamp<dbc.timestampFirstUnread || dbc.timestampFirstUnread == 0) {
			dbc.timestampFirstUnread = dbe.timestamp;
			dbc.ofsFirstUnreadEvent = ofsNew;
		}
		neednotify = TRUE;
	}
	else neednotify = m_safetyMode;

	DBWrite(ofsContact,&dbc,sizeof(DBContact));
	DBWrite(ofsNew,&dbe,offsetof(DBEvent,blob));
	DBWrite(ofsNew+offsetof(DBEvent,blob),dbei->pBlob,dbei->cbBlob); // encode
	DBFlush(0);
	lck.unlock();
	
	log1("add event @ %08x",ofsNew);

	// Notify only in safe mode or on really new events
	if (neednotify)
		NotifyEventHooks(hEventAddedEvent, (WPARAM)hContact, (LPARAM)ofsNew);

	return (HANDLE)ofsNew;
}

STDMETHODIMP_(BOOL) CDb3Base::DeleteEvent(HANDLE hContact, HANDLE hDbEvent)
{
	mir_cslockfull lck(m_csDbAccess);

	DWORD ofsContact = (hContact == 0) ? m_dbHeader.ofsUser : (DWORD)hContact;
	DBContact dbc = *(DBContact*)DBRead(ofsContact, sizeof(DBContact), NULL);
	DBEvent dbe = *(DBEvent*)DBRead(hDbEvent, sizeof(DBEvent), NULL);
	if (dbc.signature != DBCONTACT_SIGNATURE || dbe.signature != DBEVENT_SIGNATURE)
		return 1;

	lck.unlock();
	log1("delete event @ %08x", hContact);
	
	//call notifier while outside mutex
	NotifyEventHooks(hEventDeletedEvent,(WPARAM)hContact, (LPARAM)hDbEvent);

	//get back in
	lck.lock();
	dbc = *(DBContact*)DBRead(ofsContact,sizeof(DBContact),NULL);
	dbe = *(DBEvent*)DBRead(hDbEvent,sizeof(DBEvent),NULL);
	
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
			dbeNext = (DBEvent*)DBRead(ofsThis,sizeof(DBEvent),NULL);
			if ( !(dbeNext->flags & (DBEF_READ | DBEF_SENT))) {
				dbc.ofsFirstUnreadEvent = ofsThis;
				dbc.timestampFirstUnread = dbeNext->timestamp;
				break;
			}
		}
	}

	//get previous and next events in chain and change offsets
	if (dbe.flags&DBEF_FIRST) {
		if (dbe.ofsNext == 0)
			dbc.ofsFirstEvent = dbc.ofsLastEvent = 0;
		else {
			DBEvent *dbeNext = (DBEvent*)DBRead(dbe.ofsNext,sizeof(DBEvent),NULL);
			dbeNext->flags |= DBEF_FIRST;
			dbeNext->ofsPrev = dbe.ofsPrev;
			DBWrite(dbe.ofsNext,dbeNext,sizeof(DBEvent));
			dbc.ofsFirstEvent = dbe.ofsNext;
		}
	}
	else {
		if (dbe.ofsNext == 0) {
			DBEvent *dbePrev = (DBEvent*)DBRead(dbe.ofsPrev,sizeof(DBEvent),NULL);
			dbePrev->ofsNext = 0;
			DBWrite(dbe.ofsPrev,dbePrev,sizeof(DBEvent));
			dbc.ofsLastEvent = dbe.ofsPrev;
		}
		else {
			DBEvent *dbePrev = (DBEvent*)DBRead(dbe.ofsPrev,sizeof(DBEvent),NULL);
			dbePrev->ofsNext = dbe.ofsNext;
			DBWrite(dbe.ofsPrev,dbePrev,sizeof(DBEvent));
			
			DBEvent *dbeNext = (DBEvent*)DBRead(dbe.ofsNext,sizeof(DBEvent),NULL);
			dbeNext->ofsPrev = dbe.ofsPrev;
			DBWrite(dbe.ofsNext,dbeNext,sizeof(DBEvent));
		}
	}
	//delete event
	DeleteSpace((DWORD)hDbEvent, offsetof(DBEvent,blob)+dbe.cbBlob);
	//decrement event count
	dbc.eventCount--;
	DBWrite(ofsContact,&dbc,sizeof(DBContact));
	DBFlush(0);
	return 0;
}

STDMETHODIMP_(LONG) CDb3Base::GetBlobSize(HANDLE hDbEvent)
{
	mir_cslock lck(m_csDbAccess);
	DBEvent *dbe = (DBEvent*)DBRead(hDbEvent, sizeof(DBEvent), NULL);
	return (dbe->signature != DBEVENT_SIGNATURE) ? -1 : dbe->cbBlob;
}

STDMETHODIMP_(BOOL) CDb3Base::GetEvent(HANDLE hDbEvent, DBEVENTINFO *dbei)
{
	if (dbei == NULL || dbei->cbSize != sizeof(DBEVENTINFO)) return 1;
	if (dbei->cbBlob > 0 && dbei->pBlob == NULL) {
		dbei->cbBlob = 0;
		return 1;
	}

	mir_cslock lck(m_csDbAccess);
	DBEvent *dbe = (DBEvent*)DBRead(hDbEvent,sizeof(DBEvent),NULL);
	if (dbe->signature != DBEVENT_SIGNATURE)
	  	return 1;

	dbei->szModule = GetModuleNameByOfs(dbe->ofsModuleName);
	dbei->timestamp = dbe->timestamp;
	dbei->flags = dbe->flags;
	dbei->eventType = dbe->eventType;
	int bytesToCopy = (dbei->cbBlob < dbe->cbBlob) ? dbei->cbBlob : dbe->cbBlob;
	dbei->cbBlob = dbe->cbBlob;
	if (bytesToCopy && dbei->pBlob) {
		for (int i = 0;;i += MAXCACHEDREADSIZE) {
			if (bytesToCopy-i <= MAXCACHEDREADSIZE) {
				MoveMemory(dbei->pBlob + i, DBRead(DWORD(hDbEvent) + offsetof(DBEvent, blob) + i, bytesToCopy - i, NULL), bytesToCopy - i); // decode
				break;
			}
			MoveMemory(dbei->pBlob + i, DBRead(DWORD(hDbEvent) + offsetof(DBEvent, blob) + i, MAXCACHEDREADSIZE, NULL), MAXCACHEDREADSIZE); // decode
		}
	}
	return 0;
}

STDMETHODIMP_(BOOL) CDb3Base::MarkEventRead(HANDLE hContact, HANDLE hDbEvent)
{
	DBEvent *dbe;
	DBContact dbc;
	DWORD ofsThis;

	mir_cslock lck(m_csDbAccess);
	if (hContact == 0)
		hContact = (HANDLE)m_dbHeader.ofsUser;
	dbc = *(DBContact*)DBRead(hContact, sizeof(DBContact), NULL);
	dbe = (DBEvent*)DBRead(hDbEvent, sizeof(DBEvent), NULL);
	if (dbe->signature != DBEVENT_SIGNATURE || dbc.signature != DBCONTACT_SIGNATURE)
	  	return -1;

	if ((dbe->flags & DBEF_READ) || (dbe->flags & DBEF_SENT))
		return (INT_PTR)dbe->flags;

	//log1("mark read @ %08x", hContact);
	dbe->flags |= DBEF_READ;
	DBWrite((DWORD)hDbEvent,dbe,sizeof(DBEvent));
	BOOL ret = dbe->flags;
	if (dbc.ofsFirstUnreadEvent == (DWORD)hDbEvent) {
		for (;;) {
			if (dbe->ofsNext == 0) {
				dbc.ofsFirstUnreadEvent = 0;
				dbc.timestampFirstUnread = 0;
				break;
			}
			ofsThis = dbe->ofsNext;
			dbe = (DBEvent*)DBRead(ofsThis,sizeof(DBEvent),NULL);
			if ( !(dbe->flags & (DBEF_READ | DBEF_SENT))) {
				dbc.ofsFirstUnreadEvent = ofsThis;
				dbc.timestampFirstUnread = dbe->timestamp;
				break;
			}
		}
	}
	DBWrite((DWORD)hContact, &dbc, sizeof(DBContact));
	DBFlush(0);
	return ret;
}

STDMETHODIMP_(HANDLE) CDb3Base::GetEventContact(HANDLE hDbEvent)
{
	mir_cslock lck(m_csDbAccess);
	DBEvent *dbe = (DBEvent*)DBRead(hDbEvent,sizeof(DBEvent),NULL);
	if (dbe->signature != DBEVENT_SIGNATURE)
	  	return (HANDLE)-1;

	while (!(dbe->flags & DBEF_FIRST))
		dbe = (DBEvent*)DBRead(dbe->ofsPrev,sizeof(DBEvent),NULL);
	
	return (HANDLE)dbe->ofsPrev;
}

STDMETHODIMP_(HANDLE) CDb3Base::FindFirstEvent(HANDLE hContact)
{
	mir_cslock lck(m_csDbAccess);
	if (hContact == 0)
		hContact = (HANDLE)m_dbHeader.ofsUser;
	
	DBContact *dbc = (DBContact*)DBRead(hContact, sizeof(DBContact), NULL);
	return (dbc->signature != DBCONTACT_SIGNATURE) ? 0 : (HANDLE)dbc->ofsFirstEvent;
}

STDMETHODIMP_(HANDLE) CDb3Base::FindFirstUnreadEvent(HANDLE hContact)
{
	mir_cslock lck(m_csDbAccess);
	if (hContact == 0)
		hContact = (HANDLE)m_dbHeader.ofsUser;

	DBContact *dbc = (DBContact*)DBRead(hContact,sizeof(DBContact),NULL);
	return (dbc->signature != DBCONTACT_SIGNATURE) ? 0 : (HANDLE)dbc->ofsFirstUnreadEvent;
}

STDMETHODIMP_(HANDLE) CDb3Base::FindLastEvent(HANDLE hContact)
{
	mir_cslock lck(m_csDbAccess);
	if (hContact == 0)
		hContact = (HANDLE)m_dbHeader.ofsUser;

	DBContact *dbc = (DBContact*)DBRead(hContact,sizeof(DBContact),NULL);
	return (dbc->signature != DBCONTACT_SIGNATURE) ? 0 : (HANDLE)dbc->ofsLastEvent;
}

STDMETHODIMP_(HANDLE) CDb3Base::FindNextEvent(HANDLE hDbEvent)
{
	mir_cslock lck(m_csDbAccess);
	DBEvent *dbe = (DBEvent*)DBRead(hDbEvent,sizeof(DBEvent),NULL);
	return (dbe->signature != DBEVENT_SIGNATURE) ? 0 : (HANDLE)dbe->ofsNext;
}

STDMETHODIMP_(HANDLE) CDb3Base::FindPrevEvent(HANDLE hDbEvent)
{
	mir_cslock lck(m_csDbAccess);
	DBEvent *dbe = (DBEvent*)DBRead(hDbEvent,sizeof(DBEvent),NULL);
	if (dbe->signature != DBEVENT_SIGNATURE) return 0;
	return (dbe->flags & DBEF_FIRST) ? 0 : (HANDLE)dbe->ofsPrev;
}
