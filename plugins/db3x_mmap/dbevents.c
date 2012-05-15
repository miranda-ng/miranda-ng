/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
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

extern BOOL safetyMode;

DWORD GetModuleNameOfs(const char *szName);
char *GetModuleNameByOfs(DWORD ofs);

static INT_PTR GetEventCount(WPARAM wParam,LPARAM lParam);
static INT_PTR AddEvent(WPARAM wParam,LPARAM lParam);
static INT_PTR DeleteEvent(WPARAM wParam,LPARAM lParam);
static INT_PTR GetBlobSize(WPARAM wParam,LPARAM lParam);
static INT_PTR GetEvent(WPARAM wParam,LPARAM lParam);
static INT_PTR MarkEventRead(WPARAM wParam,LPARAM lParam);
static INT_PTR GetEventContact(WPARAM wParam,LPARAM lParam);
static INT_PTR FindFirstEvent(WPARAM wParam,LPARAM lParam);
static INT_PTR FindFirstUnreadEvent(WPARAM wParam,LPARAM lParam);
static INT_PTR FindLastEvent(WPARAM wParam,LPARAM lParam);
static INT_PTR FindNextEvent(WPARAM wParam,LPARAM lParam);
static INT_PTR FindPrevEvent(WPARAM wParam,LPARAM lParam);

static HANDLE hEventDeletedEvent,hEventAddedEvent,hEventFilterAddedEvent;

int InitEvents(void)
{
	CreateServiceFunction(MS_DB_EVENT_GETCOUNT,GetEventCount);
	CreateServiceFunction(MS_DB_EVENT_ADD,AddEvent);
	CreateServiceFunction(MS_DB_EVENT_DELETE,DeleteEvent);
	CreateServiceFunction(MS_DB_EVENT_GETBLOBSIZE,GetBlobSize);
	CreateServiceFunction(MS_DB_EVENT_GET,GetEvent);
	CreateServiceFunction(MS_DB_EVENT_MARKREAD,MarkEventRead);
	CreateServiceFunction(MS_DB_EVENT_GETCONTACT,GetEventContact);
	CreateServiceFunction(MS_DB_EVENT_FINDFIRST,FindFirstEvent);
	CreateServiceFunction(MS_DB_EVENT_FINDFIRSTUNREAD,FindFirstUnreadEvent);
	CreateServiceFunction(MS_DB_EVENT_FINDLAST,FindLastEvent);
	CreateServiceFunction(MS_DB_EVENT_FINDNEXT,FindNextEvent);
	CreateServiceFunction(MS_DB_EVENT_FINDPREV,FindPrevEvent);
	hEventDeletedEvent=CreateHookableEvent(ME_DB_EVENT_DELETED);
	hEventAddedEvent=CreateHookableEvent(ME_DB_EVENT_ADDED);
	hEventFilterAddedEvent=CreateHookableEvent(ME_DB_EVENT_FILTER_ADD);
	return 0;
}

void UninitEvents(void)
{
}

static INT_PTR GetEventCount(WPARAM wParam,LPARAM lParam)
{
	INT_PTR ret;
	struct DBContact *dbc;

	EnterCriticalSection(&csDbAccess);
	if(wParam==0) wParam=dbHeader.ofsUser;
	dbc=(struct DBContact*)DBRead(wParam,sizeof(struct DBContact),NULL);
	if(dbc->signature!=DBCONTACT_SIGNATURE) ret=-1;
	else ret=dbc->eventCount;
	LeaveCriticalSection(&csDbAccess);
	return ret;
}

static INT_PTR AddEvent(WPARAM wParam,LPARAM lParam)
{
	DBEVENTINFO *dbei=(DBEVENTINFO*)lParam;
	struct DBContact dbc;
	struct DBEvent dbe,*dbeTest;
	DWORD ofsNew,ofsModuleName,ofsContact,ofsThis;
	BOOL neednotify;

	if(dbei==NULL||dbei->cbSize!=sizeof(DBEVENTINFO)) return 0;
	if(dbei->timestamp==0) return 0;
	if (NotifyEventHooks(hEventFilterAddedEvent,wParam,lParam)) {
		return 0;
	}
	EnterCriticalSection(&csDbAccess);
	if(wParam==0) ofsContact=dbHeader.ofsUser;
	else ofsContact=(DWORD)wParam;
	dbc=*(struct DBContact*)DBRead(ofsContact,sizeof(struct DBContact),NULL);
	if(dbc.signature!=DBCONTACT_SIGNATURE) {
		LeaveCriticalSection(&csDbAccess);
	  	return 0;
	}
	ofsNew=CreateNewSpace(offsetof(struct DBEvent,blob)+dbei->cbBlob);
	ofsModuleName=GetModuleNameOfs(dbei->szModule);

	dbe.signature=DBEVENT_SIGNATURE;
	dbe.ofsModuleName=ofsModuleName;
	dbe.timestamp=dbei->timestamp;
	dbe.flags=dbei->flags;
	dbe.eventType=dbei->eventType;
	dbe.cbBlob=dbei->cbBlob;
	//find where to put it - sort by timestamp
	if(dbc.eventCount==0) {
		dbe.ofsPrev=wParam;
		dbe.ofsNext=0;
		dbe.flags|=DBEF_FIRST;
		dbc.ofsFirstEvent=dbc.ofsLastEvent=ofsNew;
	}
	else {
		dbeTest=(struct DBEvent*)DBRead(dbc.ofsFirstEvent,sizeof(struct DBEvent),NULL);
		// Should new event be placed before first event in chain?
		if (dbei->timestamp < dbeTest->timestamp) {
			dbe.ofsPrev=wParam;
			dbe.ofsNext=dbc.ofsFirstEvent;
			dbe.flags|=DBEF_FIRST;
			dbc.ofsFirstEvent=ofsNew;
			dbeTest=(struct DBEvent*)DBRead(dbe.ofsNext,sizeof(struct DBEvent),NULL);
			dbeTest->flags&=~DBEF_FIRST;
			dbeTest->ofsPrev=ofsNew;
			DBWrite(dbe.ofsNext,dbeTest,sizeof(struct DBEvent));
		}
		else {
			// Loop through the chain, starting at the end
			ofsThis = dbc.ofsLastEvent;
			dbeTest = (struct DBEvent*)DBRead(ofsThis, sizeof(struct DBEvent), NULL);
			for(;;) {
				// If the new event's timesstamp is equal to or greater than the
				// current dbevent, it will be inserted after. If not, continue
				// with the previous dbevent in chain.
				if (dbe.timestamp >= dbeTest->timestamp) {
					dbe.ofsPrev = ofsThis;
					dbe.ofsNext = dbeTest->ofsNext;
					dbeTest->ofsNext = ofsNew;
					DBWrite(ofsThis, dbeTest, sizeof(struct DBEvent));
					if (dbe.ofsNext == 0)
						dbc.ofsLastEvent = ofsNew;
					else {
						dbeTest = (struct DBEvent*)DBRead(dbe.ofsNext, sizeof(struct DBEvent), NULL);
						dbeTest->ofsPrev = ofsNew;
						DBWrite(dbe.ofsNext, dbeTest, sizeof(struct DBEvent));
					}
					break;
				}
				ofsThis = dbeTest->ofsPrev;
				dbeTest = (struct DBEvent*)DBRead(ofsThis, sizeof(struct DBEvent), NULL);
			}
		}
	}
	dbc.eventCount++;
	if(!(dbe.flags&(DBEF_READ|DBEF_SENT))) {
		if(dbe.timestamp<dbc.timestampFirstUnread || dbc.timestampFirstUnread==0) {
			dbc.timestampFirstUnread=dbe.timestamp;
			dbc.ofsFirstUnreadEvent=ofsNew;
		}
		neednotify = TRUE;
	}
	else neednotify = safetyMode;

	DBWrite(ofsContact,&dbc,sizeof(struct DBContact));
	DBWrite(ofsNew,&dbe,offsetof(struct DBEvent,blob));
	DBWrite(ofsNew+offsetof(struct DBEvent,blob),dbei->pBlob,dbei->cbBlob);
	DBFlush(0);

	LeaveCriticalSection(&csDbAccess);
	log1("add event @ %08x",ofsNew);

	// Notify only in safe mode or on really new events
	if (neednotify)
		NotifyEventHooks(hEventAddedEvent,wParam,(LPARAM)ofsNew);

	return (INT_PTR)ofsNew;
}

static INT_PTR DeleteEvent(WPARAM wParam,LPARAM lParam)
{
	struct DBContact dbc;
	DWORD ofsContact,ofsThis;
	struct DBEvent dbe,*dbeNext,*dbePrev;

	EnterCriticalSection(&csDbAccess);
	if(wParam==0) ofsContact=dbHeader.ofsUser;
	else ofsContact=wParam;
	dbc=*(struct DBContact*)DBRead(ofsContact,sizeof(struct DBContact),NULL);
	dbe=*(struct DBEvent*)DBRead(lParam,sizeof(struct DBEvent),NULL);
	if(dbc.signature!=DBCONTACT_SIGNATURE || dbe.signature!=DBEVENT_SIGNATURE) {
		LeaveCriticalSection(&csDbAccess);
		return 1;
	}
	log1("delete event @ %08x",wParam);
	LeaveCriticalSection(&csDbAccess);
	//call notifier while outside mutex
	NotifyEventHooks(hEventDeletedEvent,wParam,lParam);
	//get back in
	EnterCriticalSection(&csDbAccess);
	dbc=*(struct DBContact*)DBRead(ofsContact,sizeof(struct DBContact),NULL);
	dbe=*(struct DBEvent*)DBRead(lParam,sizeof(struct DBEvent),NULL);
	//check if this was the first unread, if so, recalc the first unread
	if(dbc.ofsFirstUnreadEvent==(DWORD)lParam) {
		dbeNext=&dbe;
		for(;;) {
			if(dbeNext->ofsNext==0) {
				dbc.ofsFirstUnreadEvent=0;
				dbc.timestampFirstUnread=0;
				break;
			}
			ofsThis=dbeNext->ofsNext;
			dbeNext=(struct DBEvent*)DBRead(ofsThis,sizeof(struct DBEvent),NULL);
			if(!(dbeNext->flags&(DBEF_READ|DBEF_SENT))) {
				dbc.ofsFirstUnreadEvent=ofsThis;
				dbc.timestampFirstUnread=dbeNext->timestamp;
				break;
			}
		}
	}
	//get previous and next events in chain and change offsets
	if(dbe.flags&DBEF_FIRST) {
		if(dbe.ofsNext==0) {
			dbc.ofsFirstEvent=dbc.ofsLastEvent=0;
		}
		else {
			dbeNext=(struct DBEvent*)DBRead(dbe.ofsNext,sizeof(struct DBEvent),NULL);
			dbeNext->flags|=DBEF_FIRST;
			dbeNext->ofsPrev=dbe.ofsPrev;
			DBWrite(dbe.ofsNext,dbeNext,sizeof(struct DBEvent));
			dbc.ofsFirstEvent=dbe.ofsNext;
		}
	}
	else {
		if(dbe.ofsNext==0) {
			dbePrev=(struct DBEvent*)DBRead(dbe.ofsPrev,sizeof(struct DBEvent),NULL);
			dbePrev->ofsNext=0;
			DBWrite(dbe.ofsPrev,dbePrev,sizeof(struct DBEvent));
			dbc.ofsLastEvent=dbe.ofsPrev;
		}
		else {
			dbePrev=(struct DBEvent*)DBRead(dbe.ofsPrev,sizeof(struct DBEvent),NULL);
			dbePrev->ofsNext=dbe.ofsNext;
			DBWrite(dbe.ofsPrev,dbePrev,sizeof(struct DBEvent));
			dbeNext=(struct DBEvent*)DBRead(dbe.ofsNext,sizeof(struct DBEvent),NULL);
			dbeNext->ofsPrev=dbe.ofsPrev;
			DBWrite(dbe.ofsNext,dbeNext,sizeof(struct DBEvent));
		}
	}
	//delete event
	DeleteSpace(lParam,offsetof(struct DBEvent,blob)+dbe.cbBlob);
	//decrement event count
	dbc.eventCount--;
	DBWrite(ofsContact,&dbc,sizeof(struct DBContact));
	DBFlush(0);
	//quit
	LeaveCriticalSection(&csDbAccess);
	return 0;
}

static INT_PTR GetBlobSize(WPARAM wParam,LPARAM lParam)
{
	INT_PTR ret;
	struct DBEvent *dbe;

	EnterCriticalSection(&csDbAccess);
	dbe=(struct DBEvent*)DBRead(wParam,sizeof(struct DBEvent),NULL);
	if(dbe->signature!=DBEVENT_SIGNATURE) ret=-1;
	else ret=dbe->cbBlob;
	LeaveCriticalSection(&csDbAccess);
	return ret;
}

static INT_PTR GetEvent(WPARAM wParam,LPARAM lParam)
{
	struct DBEvent *dbe;
	DBEVENTINFO *dbei=(DBEVENTINFO*)lParam;
	int bytesToCopy,i;

	if(dbei==NULL||dbei->cbSize!=sizeof(DBEVENTINFO)) return 1;
	if(dbei->cbBlob>0 && dbei->pBlob==NULL) {
		dbei->cbBlob = 0;
		return 1;
	}
	EnterCriticalSection(&csDbAccess);
	dbe=(struct DBEvent*)DBRead(wParam,sizeof(struct DBEvent),NULL);
	if(dbe->signature!=DBEVENT_SIGNATURE) {
		LeaveCriticalSection(&csDbAccess);
	  	return 1;
	}
	dbei->szModule=GetModuleNameByOfs(dbe->ofsModuleName);
	dbei->timestamp=dbe->timestamp;
	dbei->flags=dbe->flags;
	dbei->eventType=dbe->eventType;
	if(dbei->cbBlob<dbe->cbBlob) bytesToCopy=dbei->cbBlob;
	else bytesToCopy=dbe->cbBlob;
	dbei->cbBlob=dbe->cbBlob;
	if (bytesToCopy && dbei->pBlob)
	{
		for(i=0;;i+=MAXCACHEDREADSIZE) {
			if(bytesToCopy-i<=MAXCACHEDREADSIZE) {
				CopyMemory(dbei->pBlob+i,DBRead(wParam+offsetof(struct DBEvent,blob)+i,bytesToCopy-i,NULL),bytesToCopy-i);
				break;
			}
			CopyMemory(dbei->pBlob+i,DBRead(wParam+offsetof(struct DBEvent,blob)+i,MAXCACHEDREADSIZE,NULL),MAXCACHEDREADSIZE);
		}
	}
	LeaveCriticalSection(&csDbAccess);
	return 0;
}

static INT_PTR MarkEventRead(WPARAM wParam,LPARAM lParam)
{
	INT_PTR ret;
	struct DBEvent *dbe;
	struct DBContact dbc;
	DWORD ofsThis;

	EnterCriticalSection(&csDbAccess);
	if(wParam==0) wParam=dbHeader.ofsUser;
	dbc=*(struct DBContact*)DBRead(wParam,sizeof(struct DBContact),NULL);
	dbe=(struct DBEvent*)DBRead(lParam,sizeof(struct DBEvent),NULL);
	if(dbe->signature!=DBEVENT_SIGNATURE || dbc.signature!=DBCONTACT_SIGNATURE) {
		LeaveCriticalSection(&csDbAccess);
	  	return -1;
	}
	if(dbe->flags&DBEF_READ || dbe->flags&DBEF_SENT) {
		ret=(INT_PTR)dbe->flags;
		LeaveCriticalSection(&csDbAccess);
		return ret;
	}
	log1("mark read @ %08x",wParam);
	dbe->flags|=DBEF_READ;
	DBWrite(lParam,dbe,sizeof(struct DBEvent));
	ret=(int)dbe->flags;
	if(dbc.ofsFirstUnreadEvent==(DWORD)lParam) {
		for(;;) {
			if(dbe->ofsNext==0) {
				dbc.ofsFirstUnreadEvent=0;
				dbc.timestampFirstUnread=0;
				break;
			}
			ofsThis=dbe->ofsNext;
			dbe=(struct DBEvent*)DBRead(ofsThis,sizeof(struct DBEvent),NULL);
			if(!(dbe->flags&(DBEF_READ|DBEF_SENT))) {
				dbc.ofsFirstUnreadEvent=ofsThis;
				dbc.timestampFirstUnread=dbe->timestamp;
				break;
			}
		}
	}
	DBWrite(wParam,&dbc,sizeof(struct DBContact));
	DBFlush(0);
	LeaveCriticalSection(&csDbAccess);
	return ret;
}

static INT_PTR GetEventContact(WPARAM wParam,LPARAM lParam)
{
	int ret;
	struct DBEvent *dbe;

	EnterCriticalSection(&csDbAccess);
	dbe=(struct DBEvent*)DBRead(wParam,sizeof(struct DBEvent),NULL);
	if(dbe->signature!=DBEVENT_SIGNATURE) {
		LeaveCriticalSection(&csDbAccess);
	  	return -1;
	}
	while(!(dbe->flags&DBEF_FIRST))
		dbe=(struct DBEvent*)DBRead(dbe->ofsPrev,sizeof(struct DBEvent),NULL);
	ret=(INT_PTR)dbe->ofsPrev;
	LeaveCriticalSection(&csDbAccess);
	return ret;
}

static INT_PTR FindFirstEvent(WPARAM wParam,LPARAM lParam)
{
	INT_PTR ret;
	struct DBContact *dbc;

	EnterCriticalSection(&csDbAccess);
	if(wParam==0) wParam=dbHeader.ofsUser;
	dbc=(struct DBContact*)DBRead(wParam,sizeof(struct DBContact),NULL);
	if(dbc->signature!=DBCONTACT_SIGNATURE) ret=0;
	else ret=(INT_PTR)dbc->ofsFirstEvent;
	LeaveCriticalSection(&csDbAccess);
	return ret;
}

static INT_PTR FindFirstUnreadEvent(WPARAM wParam,LPARAM lParam)
{
	INT_PTR ret;
	struct DBContact *dbc;

	EnterCriticalSection(&csDbAccess);
	if(wParam==0) wParam=dbHeader.ofsUser;
	dbc=(struct DBContact*)DBRead(wParam,sizeof(struct DBContact),NULL);
	if(dbc->signature!=DBCONTACT_SIGNATURE) ret=0;
	else ret=(INT_PTR)dbc->ofsFirstUnreadEvent;
	LeaveCriticalSection(&csDbAccess);
	return ret;
}

static INT_PTR FindLastEvent(WPARAM wParam,LPARAM lParam)
{
	INT_PTR ret;
	struct DBContact *dbc;

	EnterCriticalSection(&csDbAccess);
	if(wParam==0) wParam=dbHeader.ofsUser;
	dbc=(struct DBContact*)DBRead(wParam,sizeof(struct DBContact),NULL);
	if(dbc->signature!=DBCONTACT_SIGNATURE) ret=0;
	else ret=(INT_PTR)dbc->ofsLastEvent;
	LeaveCriticalSection(&csDbAccess);
	return ret;
}

static INT_PTR FindNextEvent(WPARAM wParam,LPARAM lParam)
{
	INT_PTR ret;
	struct DBEvent *dbe;

	EnterCriticalSection(&csDbAccess);
	dbe=(struct DBEvent*)DBRead(wParam,sizeof(struct DBEvent),NULL);
	if(dbe->signature!=DBEVENT_SIGNATURE) ret=0;
	else ret=(INT_PTR)dbe->ofsNext;
	LeaveCriticalSection(&csDbAccess);
	return ret;
}

static INT_PTR FindPrevEvent(WPARAM wParam,LPARAM lParam)
{
	INT_PTR ret;
	struct DBEvent *dbe;

	EnterCriticalSection(&csDbAccess);
	dbe=(struct DBEvent*)DBRead(wParam,sizeof(struct DBEvent),NULL);
	if(dbe->signature!=DBEVENT_SIGNATURE) ret=0;
	else if(dbe->flags&DBEF_FIRST) ret=0;
	else ret=(INT_PTR)dbe->ofsPrev;
	LeaveCriticalSection(&csDbAccess);
	return ret;
}
