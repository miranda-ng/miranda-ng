/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
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

#include "..\..\core\commonheaders.h"

#if 0

extern HANDLE hCacheHeap;

/*

  the id cache has id/proto against hContact to lookup ID's fast to resolve to hContact,
  the protoBaseCache has hContact's sorted, so can lookup hContact->proto fast, these two caches
  share the same data, they're indexes, each entry might not have an "id" set.

  There is a small cache which maintains a protocol list

*/

/*

  The information we need to cache is not readily available, data has to be gathered at startup
  when a new contact is created/deleted, when a new proto registers and so on.

  The information we get at startup includes walking the contact chain and reading Protocol/p which
  will give us the protocol the contact is on, all this info is stored in contactEntry's within
  protoCache ONLY - contactCache is EMPTY at this point.

  We can not fetch the id of the contact because this information is only in SOME protocol plugins,
  this is a problem but we'll hook all proto registrations and ask each proto if it supports the
  returning this ID name, if not - it won't even use our id <-> contact look so no biggie!

*/

typedef struct {
	char * proto;		// within proto cache
	char * id;			// optional
	HANDLE hContact;
} contactEntry;

typedef struct {
	mir_cs csLock;
	SortedList contactCache;	// index for id/proto -> hContact
	SortedList protoCache;		// index for hContact -> proto/id
	SortedList protoNameCache;	// index of protocol names
} contactDir;

static contactDir condir;

// compare's id/proto and return's hContact's
int contactCacheCompare(void * a, void * b)
{
	contactEntry * x = (contactEntry *) a;
	contactEntry * y = (contactEntry *) b;
	int rc=0;
	// same protocol?
	rc = strcmp(x->proto, y->proto);
	if ( rc  ==  0 ) {
		// same id? id's might be missing
		if ( x->id && y->id ) rc = strcmp(x->id, y->id);
	}
	return rc;
}

// compares hContact's and returns associated data
int protoCacheCompare(void * a, void * b)
{
	contactEntry * x = (contactEntry *) a;
	contactEntry * y = (contactEntry *) b;
	if ( x->hContact < y->hContact ) return -1;
	if ( x->hContact > y->hContact ) return 1;
	return 0;
}

// keeps a list of protocol names
int protoNameCacheCompare(void * a, void * b)
{
	return strcmp( (char *)a, (char*)b );
}

// cache the protocol string so that its not allocated per contact but shared
char * contactDir_Proto_Add(contactDir * cd, char * proto)
{
	int index = 0 ;
	char * szCache = 0;
	mir_cslock lck(cd->csLock);
	if ( List_GetIndex(&cd->protoNameCache, proto, &index) ) szCache = cd->protoNameCache.items[index];
	else {
		szCache = HeapAlloc(hCacheHeap, HEAP_NO_SERIALIZE, strlen(proto)+1);
		strcpy(szCache, proto);
		List_Insert(&cd->protoNameCache, szCache, index);
	}
	return szCache;
}

// thread safe
char * contactDir_Proto_Get(contactDir * cd, HANDLE hContact)
{
	char * szCache = 0;
	int index = 0;
	contactEntry e;
	e.hContact=hContact;
	e.proto="";
	e.id="";
	mir_cslock lck(cd->csLock);
	if ( List_GetIndex(&cd->protoCache, &e, &index) ) {
		contactEntry * p = cd->protoCache.items[index];
		szCache = p->proto;
	}
	return szCache;
}

// thread tolerant, if updating id dont pass proto, if updating proto dont pass id
void contactDir_Contact_Add(contactDir * cd, HANDLE hContact, char * proto, char * id)
{
	// if a contact is gonna exist anywhere it's going to be in the ->protoCache which has a key of hContact
	// if id is not null then the contact should be indexed via the ->contactCache instead
	if ( id  ==  NULL ) {
		int index = 0;
		contactEntry e;
		e.hContact=hContact;
		e.proto = proto;
		e.id = "";
		mir_cslock lck(cd->csLock);
		if ( List_GetIndex(&cd->protoCache, &e, &index) ) {
			contactEntry * p = cd->protoCache.items[index];
			// this hContact is in the cache, protcol changing?
			p->proto = contactDir_Proto_Add(cd, proto); // just replace the old pointer
		} else {
			contactEntry * p = 0;
			// this hContact isn't in the cache, add it
			p = HeapAlloc(hCacheHeap, HEAP_NO_SERIALIZE, sizeof(contactEntry));
			p->proto = contactDir_Proto_Add(cd, proto);
			p->id = 0;
			p->hContact = hContact;
			// add it
			List_Insert(&cd->protoCache, p, index);
		}
	} else {
		// this contact HAS to be in ->protoCache since it was added during startup
		// need to find the contactEntry* that should already exist for it
	} //if
}

// only expected to be called at startup.
void contactDir_Proto_Walk(contactDir * cd)
{
	HANDLE hContact;
	char buf[128];
	DBCONTACTGETSETTING gsProto;
	DBVARIANT dbvProto;
	// setup the read structure
	gsProto.szModule="Protocol";
	gsProto.szSetting="p";
	gsProto.pValue = &dbvProto;
	// this might not work
	hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while ( hContact ) {
		// and how we'll get the reset
		dbvProto.type=DBVT_ASCIIZ;
		dbvProto.pszVal = (char *) &buf;
		dbvProto.cchVal = SIZEOF(buf);
		// figure out what hContact/Protocol/p is
		if ( CallService(MS_DB_CONTACT_GETSETTINGSTATIC,(WPARAM)hContact, (LPARAM)&gsProto) ==  0 ) {
			contactDir_Contact_Add(cd, hContact, buf, NULL);
		}
		// find next
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
}

// ctor/dtor

void contactDir_Init(contactDir * cd)
{
	cd->contactCache.increment=50;
	cd->contactCache.sortFunc=contactCacheCompare;
	cd->protoCache.increment=50;
	cd->protoCache.sortFunc=protoCacheCompare;
	cd->protoNameCache.increment=5;
	cd->protoNameCache.sortFunc=protoNameCacheCompare;
	// build a list of all hContact's and what proto's they are on
	contactDir_Proto_Walk(cd);
}

void contactDir_Deinit(contactDir * cd)
{
	List_Destroy(&cd->contactCache);
	List_Destroy(&cd->protoCache);
	List_Destroy(&cd->protoNameCache);
}

static int contactDirGetProto(WPARAM wParam, LPARAM lParam)
{
	return (int) contactDir_Proto_Get(&condir,(HANDLE)wParam);
}

#endif

void InitContactDir(void)
{
	return;
	//contactDir_Init(&condir);
	//CreateServiceFunction(MS_PROTODIR_PROTOFROMCONTACT, contactDirGetProto);
}

void UninitContactDir(void)
{
	return;
#if 0
	{
		int j;
		for ( j = 0; j< condir.protoCache.realCount; j++) {
			char buf[128];
			contactEntry * p = condir.protoCache.items[j];
			mir_snprintf(buf,SIZEOF(buf)," [%s] %s @ %x \n", p->proto, p->id ? p->id : "", p->hContact);
			OutputDebugString(buf);
		}
	}
	contactDir_Deinit(&condir);
#endif
}
