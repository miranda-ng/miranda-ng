/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012 Miranda NG project,
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

DBCachedContactValueList* CDb3Base::AddToCachedContactList(HANDLE hContact, int index)
{
	DBCachedContactValueList* VL = (DBCachedContactValueList*)HeapAlloc(m_hCacheHeap,HEAP_ZERO_MEMORY,sizeof(DBCachedContactValueList));
	VL->hContact = hContact;
	if (index == -1)
		m_lContacts.insert(VL);
	else
		m_lContacts.insert(VL,index);
	return VL;
}

#define proto_module  "Protocol"
#define proto_setting "p"

int CDb3Base::CheckProto(HANDLE hContact, const char *proto)
{
	char protobuf[MAX_PATH] = {0};
	DBVARIANT dbv;
	DBCONTACTGETSETTING sVal = {proto_module,proto_setting,&dbv};

 	dbv.type = DBVT_ASCIIZ;
	dbv.pszVal = protobuf;
	dbv.cchVal = sizeof(protobuf);

	if (GetContactSettingStatic(hContact, &sVal) != 0 || (dbv.type != DBVT_ASCIIZ))
		return 0;

	return !strcmp(protobuf,proto);
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
	DBCachedContactValueList VLtemp, *VL = NULL;
	VLtemp.hContact = hContact;

	mir_cslock lck(m_csDbAccess);
	while (VLtemp.hContact) {
		int index;
		if (( index = m_lContacts.getIndex(&VLtemp)) != -1) {
			VL = m_lContacts[index];
			if (VL->hNext != NULL) {
				if (!szProto || CheckProto(VL->hNext, szProto))
					return VL->hNext;

				VLtemp.hContact = VL->hNext;
				continue;
		}	}

		DBContact *dbc = (DBContact*)DBRead(VLtemp.hContact,sizeof(DBContact),NULL);
		if (dbc->signature != DBCONTACT_SIGNATURE)
			break;

		if ( VL == NULL )
			VL = AddToCachedContactList(VLtemp.hContact,index);

		VL->hNext = (HANDLE)dbc->ofsNext;
		if (VL->hNext != NULL && (!szProto || CheckProto(VL->hNext, szProto)))
			return VL->hNext;

		VLtemp.hContact = VL->hNext;
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

	DBCachedContactValueList VLtemp;
	VLtemp.hContact = hContact;
	int index;
	if ((index = m_lContacts.getIndex(&VLtemp)) != -1) {
		DBCachedContactValueList *VL = m_lContacts[index];
		DBCachedContactValue* V = VL->first;
		while ( V != NULL ) {
			DBCachedContactValue* V1 = V->next;
			FreeCachedVariant(&V->value);
			HeapFree( m_hCacheHeap, 0, V );
			V = V1;
		}
		HeapFree( m_hCacheHeap, 0, VL );

		if (VLtemp.hContact == m_hLastCachedContact)
			m_hLastCachedContact = NULL;
		m_lContacts.remove(index);
	}

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
		DBWrite(0,&m_dbHeader,sizeof(m_dbHeader));
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

		DBCachedContactValueList VLtemp;
		VLtemp.hContact = (HANDLE)ofsThis;
		if ((index = m_lContacts.getIndex(&VLtemp)) != -1) {
			DBCachedContactValueList *VL = m_lContacts[index];
			VL->hNext = (HANDLE)ofsNext;
		}
	}

	//delete contact
	DeleteSpace((DWORD)hContact, sizeof(DBContact));
	//decrement contact count
	m_dbHeader.contactCount--;
	DBWrite(0,&m_dbHeader,sizeof(m_dbHeader));
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
		DBWrite(ofsNew,&dbc,sizeof(DBContact));
		DBWrite(0,&m_dbHeader,sizeof(m_dbHeader));
		DBFlush(0);

		AddToCachedContactList((HANDLE)ofsNew, -1);
	}

	NotifyEventHooks(hContactAddedEvent,(WPARAM)ofsNew,0);
	return (HANDLE)ofsNew;
}

STDMETHODIMP_(BOOL) CDb3Base::IsDbContact(HANDLE hContact)
{
	mir_cslock lck(m_csDbAccess);

	DBCachedContactValueList VLtemp;
	VLtemp.hContact = hContact;
	int index = m_lContacts.getIndex(&VLtemp);
	if (index != -1)
		return TRUE;

	DBContact *dbc = (DBContact*)DBRead(hContact,sizeof(DBContact),NULL);
	if (dbc->signature == DBCONTACT_SIGNATURE) {
		AddToCachedContactList(hContact, index);
		return TRUE;
	}

	return FALSE;
}
