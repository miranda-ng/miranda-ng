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

HANDLE hContactDeletedEvent, hContactAddedEvent;

DBCachedContactValueList* AddToCachedContactList(HANDLE hContact, int index)
{
	DBCachedContactValueList* VL;
	VL = (DBCachedContactValueList*)HeapAlloc(hCacheHeap,HEAP_ZERO_MEMORY,sizeof(DBCachedContactValueList));
	VL->hContact = hContact;
	if (index == -1) List_GetIndex(&lContacts,VL,&index);
	List_Insert(&lContacts,VL,index);
	return VL;
}

#define proto_module  "Protocol"
#define proto_setting "p"

int CDdxMmap::CheckProto(HANDLE hContact, const char *proto)
{
	static char protobuf[MAX_PATH] = {0};
	static DBVARIANT dbv;
	static DBCONTACTGETSETTING sVal = {proto_module,proto_setting,&dbv};

 	dbv.type = DBVT_ASCIIZ;
	dbv.pszVal = protobuf;
	dbv.cchVal = sizeof(protobuf);

	if (GetContactSettingStatic(hContact, &sVal) != 0 || (dbv.type != DBVT_ASCIIZ))
		return 0;

	return !strcmp(protobuf,proto);
}

int InitContacts(void)
{
	hContactDeletedEvent = CreateHookableEvent(ME_DB_CONTACT_DELETED);
	hContactAddedEvent = CreateHookableEvent(ME_DB_CONTACT_ADDED);
	return 0;
}

STDMETHODIMP_(LONG) CDdxMmap::GetContactCount(void)
{
	mir_cslock lck(csDbAccess);
	return dbHeader.contactCount;
}

STDMETHODIMP_(HANDLE) CDdxMmap::FindFirstContact(const char *szProto)
{
	mir_cslock lck(csDbAccess);
	HANDLE ret = (HANDLE)dbHeader.ofsFirstContact;
	if (szProto && !CheckProto(ret, szProto))
		ret = FindNextContact(ret, szProto);
	return ret;
}

STDMETHODIMP_(HANDLE) CDdxMmap::FindNextContact(HANDLE hContact, const char *szProto)
{
	int index;
	DBContact *dbc;
	DBCachedContactValueList VLtemp, *VL = NULL;
	VLtemp.hContact = hContact;

	mir_cslock lck(csDbAccess);
	while(VLtemp.hContact) {
		if ( List_GetIndex(&lContacts,&VLtemp,&index)) {
			VL = ( DBCachedContactValueList* )lContacts.items[index];
			if (VL->hNext != NULL) {
				if (!szProto || CheckProto(VL->hNext, szProto))
					return VL->hNext;

				VLtemp.hContact = VL->hNext;
				continue;
		}	}

		dbc = (DBContact*)DBRead(VLtemp.hContact,sizeof(DBContact),NULL);
		if (dbc->signature!=DBCONTACT_SIGNATURE)
			break;
		else {
			if ( VL == NULL )
				VL = AddToCachedContactList(VLtemp.hContact,index);

			VL->hNext = (HANDLE)dbc->ofsNext;
			if (VL->hNext != NULL && (!szProto || CheckProto(VL->hNext, szProto)))
				return VL->hNext;

			VLtemp.hContact = VL->hNext;
	}	}

	return NULL;
}

STDMETHODIMP_(LONG) CDdxMmap::DeleteContact(HANDLE hContact)
{
	DWORD ofsThis,ofsNext,ofsFirstEvent;
	DBContactSettings *dbcs;
	DBEvent *dbe;
	int index;

	if (hContact == NULL)
		return 1;

	{
		mir_cslock lck(csDbAccess);
		DBContact *dbc = (DBContact*)DBRead(hContact, sizeof(DBContact), NULL);
		if (dbc->signature != DBCONTACT_SIGNATURE)
			return 1;

		if (hContact == (HANDLE)dbHeader.ofsUser) {
			log0("FATAL: del of user chain attempted.");
			return 1;
		}
		log0("del contact");
	}

	//call notifier while outside mutex
	NotifyEventHooks(hContactDeletedEvent, (WPARAM)hContact, 0);

	//get back in
	mir_cslock lck(csDbAccess);

	DBCachedContactValueList VLtemp;
	VLtemp.hContact = hContact;
	if ( List_GetIndex(&lContacts,&VLtemp,&index))
	{
		DBCachedContactValueList *VL = ( DBCachedContactValueList* )lContacts.items[index];
		DBCachedContactValue* V = VL->first;
		while ( V != NULL ) {
			DBCachedContactValue* V1 = V->next;
			FreeCachedVariant(&V->value);
			HeapFree( hCacheHeap, 0, V );
			V = V1;
		}
		HeapFree( hCacheHeap, 0, VL );

		if (VLtemp.hContact == hLastCachedContact)
			hLastCachedContact = NULL;
		List_Remove(&lContacts,index);
	}

	DBContact *dbc = (DBContact*)DBRead(hContact, sizeof(DBContact), NULL);
	//delete settings chain
	ofsThis = dbc->ofsFirstSettings;
	ofsFirstEvent = dbc->ofsFirstEvent;
	while(ofsThis) {
		dbcs = (struct DBContactSettings*)DBRead(ofsThis,sizeof(struct DBContactSettings),NULL);
		ofsNext = dbcs->ofsNext;
		DeleteSpace(ofsThis,offsetof(struct DBContactSettings,blob)+dbcs->cbBlob);
		ofsThis = ofsNext;
	}
	//delete event chain
	ofsThis = ofsFirstEvent;
	while(ofsThis) {
		dbe = (DBEvent*)DBRead(ofsThis,sizeof(DBEvent),NULL);
		ofsNext = dbe->ofsNext;
		DeleteSpace(ofsThis,offsetof(DBEvent,blob)+dbe->cbBlob);
		ofsThis = ofsNext;
	}
	//find previous contact in chain and change ofsNext
	dbc = (DBContact*)DBRead(hContact, sizeof(DBContact), NULL);
	if (dbHeader.ofsFirstContact == (DWORD)hContact) {
		dbHeader.ofsFirstContact = dbc->ofsNext;
		DBWrite(0,&dbHeader,sizeof(dbHeader));
	}
	else {
		ofsNext = dbc->ofsNext;
		ofsThis = dbHeader.ofsFirstContact;
		DBContact *dbcPrev = (DBContact*)DBRead(ofsThis,sizeof(DBContact),NULL);
		while(dbcPrev->ofsNext != (DWORD)hContact) {
			if (dbcPrev->ofsNext == 0) DatabaseCorruption(NULL);
			ofsThis = dbcPrev->ofsNext;
			dbcPrev = (DBContact*)DBRead(ofsThis,sizeof(DBContact),NULL);
		}
		dbcPrev->ofsNext = ofsNext;
		DBWrite(ofsThis,dbcPrev,sizeof(DBContact));

		DBCachedContactValueList VLtemp;
		VLtemp.hContact = (HANDLE)ofsThis;
		if ( List_GetIndex(&lContacts,&VLtemp,&index)) {
			DBCachedContactValueList *VL = ( DBCachedContactValueList* )lContacts.items[index];
			VL->hNext = ( HANDLE )ofsNext;
		}
	}

	//delete contact
	DeleteSpace((DWORD)hContact, sizeof(DBContact));
	//decrement contact count
	dbHeader.contactCount--;
	DBWrite(0,&dbHeader,sizeof(dbHeader));
	DBFlush(0);
	return 0;
}

STDMETHODIMP_(HANDLE) CDdxMmap::AddContact()
{
	DWORD ofsNew;
	log0("add contact");
   {
		mir_cslock lck(csDbAccess);
		ofsNew = CreateNewSpace(sizeof(DBContact));

		DBContact dbc = { 0 };
		dbc.signature = DBCONTACT_SIGNATURE;
		dbc.ofsNext = dbHeader.ofsFirstContact;
		dbHeader.ofsFirstContact = ofsNew;
		dbHeader.contactCount++;
		DBWrite(ofsNew,&dbc,sizeof(DBContact));
		DBWrite(0,&dbHeader,sizeof(dbHeader));
		DBFlush(0);

		AddToCachedContactList((HANDLE)ofsNew, -1);
	}

	NotifyEventHooks(hContactAddedEvent,(WPARAM)ofsNew,0);
	return (HANDLE)ofsNew;
}

STDMETHODIMP_(BOOL) CDdxMmap::IsDbContact(HANDLE hContact)
{
	DBContact *dbc;
	DWORD ofsContact = (DWORD)hContact;
	int ret;

	EnterCriticalSection(&csDbAccess);
	{
		int index;
		DBCachedContactValueList VLtemp;
		VLtemp.hContact = hContact;
		if ( List_GetIndex(&lContacts,&VLtemp,&index))
			ret = TRUE;
		else {
			dbc = (DBContact*)DBRead(ofsContact,sizeof(DBContact),NULL);
			ret = dbc->signature == DBCONTACT_SIGNATURE;
			if (ret)
				AddToCachedContactList(hContact, index);
	}	}

	LeaveCriticalSection(&csDbAccess);
	return ret;
}
