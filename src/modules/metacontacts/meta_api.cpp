/*
former MetaContacts Plugin for Miranda IM.

Copyright © 2014 Miranda NG Team
Copyright © 2004-07 Scott Ellis
Copyright © 2004 Universite Louis PASTEUR, STRASBOURG.

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

#include "metacontacts.h"

// gets the handle for the default contact
// wParam=(MCONTACT)hMetaContact
// lParam=0
// returns a handle to the default contact, or null on failure

static INT_PTR MetaAPI_GetDefault(WPARAM hMetaContact, LPARAM)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	if (cc == NULL)
		return 1;

	if (cc->nDefault != -1)
		return Meta_GetContactHandle(cc, cc->nDefault);

	return 0;
}

// gets the contact number for the default contact
// wParam=(MCONTACT)hMetaContact
// lParam=0
// returns a DWORD contact number, or -1 on failure

static INT_PTR MetaAPI_GetDefaultNum(WPARAM hMetaContact, LPARAM)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	return (cc == NULL) ? -1 : cc->nDefault;
}

// gets the handle for the 'most online' contact
// wParam=(MCONTACT)hMetaContact
// lParam=0
// returns a handle to the 'most online' contact

static INT_PTR MetaAPI_GetMostOnline(WPARAM hMetaContact, LPARAM)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	if (cc == NULL)
		return 1;

	return Meta_GetMostOnline(cc);
}

// gets the number of subcontacts for a metacontact
// wParam=(MCONTACT)hMetaContact
// lParam=0
// returns a DWORD representing the number of subcontacts for the given metacontact

static INT_PTR MetaAPI_GetNumContacts(WPARAM hMetaContact, LPARAM)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	return (cc == NULL) ? -1 : cc->nSubs;
}

// gets the handle of a subcontact, using the subcontact's number
// wParam=(MCONTACT)hMetaContact
// lParam=(DWORD)contact number
// returns a handle to the specified subcontact

static INT_PTR MetaAPI_GetContact(WPARAM hMetaContact, LPARAM lParam)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	return (cc == NULL) ? 0 : Meta_GetContactHandle(cc, lParam);
}

// sets the default contact, using the subcontact's contact number
// wParam=(MCONTACT)hMetaContact
// lParam=(DWORD)contact number
// returns 0 on success

static INT_PTR MetaAPI_SetDefaultContactNum(WPARAM hMetaContact, LPARAM lParam)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	if (cc == NULL)
		return 1;
	if ((int)lParam >= cc->nSubs || (int)lParam < 0)
		return 1;

	cc->nDefault = lParam;
	currDb->MetaSetDefault(cc);

	NotifyEventHooks(hEventDefaultChanged, hMetaContact, Meta_GetContactHandle(cc, (int)lParam));
	return 0;
}

// sets the default contact, using the subcontact's handle
// wParam=(MCONTACT)hMetaContact
// lParam=(MCONTACT)hSubcontact
// returns 0 on success

static INT_PTR MetaAPI_SetDefaultContact(WPARAM hMetaContact, LPARAM lParam)
{
	DBCachedContact *cc = CheckMeta(lParam);
	if (cc == NULL)
		return 1;

	int contact_number = Meta_GetContactNumber(cc, lParam);
	if (contact_number == -1) 
		return 1;

	cc->nDefault = contact_number;
	currDb->MetaSetDefault(cc);
	
	NotifyEventHooks(hEventDefaultChanged, hMetaContact, lParam);
	return 0;
}

// forces the metacontact to send using a specific subcontact, using the subcontact's contact number
// wParam=(MCONTACT)hMetaContact
// lParam=(DWORD)contact number
// returns 0 on success

static INT_PTR MetaAPI_ForceSendContactNum(WPARAM hMetaContact, LPARAM lParam)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	if (cc == NULL)
		return 1;

	MCONTACT hContact = Meta_GetContactHandle(cc, (int)lParam);
	if (!hContact || db_get_b(cc->contactID, META_PROTO, "ForceDefault", 0)) 
		return 1;

	db_set_dw(cc->contactID, META_PROTO, "ForceSend", hContact);

	NotifyEventHooks(hEventForceSend, hMetaContact, hContact);
	return 0;
}

// forces the metacontact to send using a specific subcontact, using the subcontact's handle
// wParam=(MCONTACT)hMetaContact
// lParam=(MCONTACT)hSubcontact
// returns 0 on success (will fail if 'force default' is in effect)

static INT_PTR MetaAPI_ForceSendContact(WPARAM hMetaContact, LPARAM lParam)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	if (cc == NULL)
		return 1;

	if (Meta_GetContactNumber(cc, lParam) == -1)
		return 1;

	if (!db_get_b(cc->contactID, META_PROTO, "ForceDefault", 0)) 
		return 1;

	db_set_dw(cc->contactID, META_PROTO, "ForceSend", lParam);

	NotifyEventHooks(hEventForceSend, hMetaContact, lParam);
	return 0;
}

// 'unforces' the metacontact to send using a specific subcontact
// wParam=(MCONTACT)hMetaContact
// lParam=0
// returns 0 on success (will fail if 'force default' is in effect)

INT_PTR MetaAPI_UnforceSendContact(WPARAM hMetaContact, LPARAM lParam)
{
	if (db_get_b(hMetaContact, META_PROTO, "ForceDefault", 0))
		return 1;

	db_set_dw(hMetaContact, META_PROTO, "ForceSend", 0);
		
	NotifyEventHooks(hEventUnforceSend, hMetaContact, lParam);
	return 0;
}

//'forces' or 'unforces' (i.e. toggles) the metacontact to send using it's default contact
// overrides 'force send' above, and will even force use of offline contacts
// will send ME_MC_FORCESEND event
//
// wParam=(MCONTACT)hMetaContact
// lParam=0
// returns 1(true) or 0(false) representing new state of 'force default'

INT_PTR MetaAPI_ForceDefault(WPARAM hMetaContact, LPARAM lParam)
{
	// forward to menu function
	Meta_ForceDefault(hMetaContact, lParam);
	return db_get_b(hMetaContact, META_PROTO, "ForceDefault", 0);
}
	
// method to get state of 'force' for a metacontact
// wParam=(MCONTACT)hMetaContact
// lParam= (DWORD)&contact_number or NULL
// if lparam supplied, the contact_number of the contatct 'in force' will be copied to the address it points to,
// or if none is in force, the value INVALID_CONTACT_ID will be copied
// (v0.8.0.8+ returns 1 if 'force default' is true with *lParam == default contact number, else returns 0 with *lParam as above)

INT_PTR MetaAPI_GetForceState(WPARAM hMetaContact, LPARAM lParam)
{
	DBCachedContact *cc = CheckMeta(hMetaContact);
	if (cc == NULL)
		return 0;

	if (db_get_b(hMetaContact, META_PROTO, "ForceDefault", 0)) {
		if (lParam) *(DWORD *)lParam = cc->nDefault;
		return 1;
	}

	MCONTACT hContact = (MCONTACT)db_get_dw(hMetaContact, META_PROTO, "ForceSend", 0);
	if (!hContact) {
		if (lParam) *(DWORD *)lParam = -1;
	}
	else {
		if (lParam) *(DWORD *)lParam = (DWORD)Meta_GetContactNumber(cc, hContact);
	}

	return 0;
}

// wParam=(HANDLE)hContact
// lParam=0
// convert a given contact into a metacontact

static INT_PTR MetaAPI_ConvertToMeta(WPARAM wParam, LPARAM lParam)
{
	return Meta_Convert(wParam, lParam);
}

// wParam=(HANDLE)hContact
// lParam=(HANDLE)hMeta
// add an existing contact to a metacontact

static INT_PTR MetaAPI_AddToMeta(WPARAM wParam, LPARAM lParam)
{
	return Meta_Assign(wParam, lParam, FALSE);
}

// wParam=0
// lParam=(HANDLE)hContact
// remove a contact from a metacontact

static INT_PTR MetaAPI_RemoveFromMeta(WPARAM wParam, LPARAM lParam)
{
	// notice we switch args - to keep the API function consistent with the others
	return Meta_Delete(lParam, wParam);
}

void CreateApiServices()
{
	CreateServiceFunction(MS_MC_GETDEFAULTCONTACT, MetaAPI_GetDefault);
	CreateServiceFunction(MS_MC_GETDEFAULTCONTACTNUM, MetaAPI_GetDefaultNum);
	CreateServiceFunction(MS_MC_GETMOSTONLINECONTACT, MetaAPI_GetMostOnline);
	CreateServiceFunction(MS_MC_GETNUMCONTACTS, MetaAPI_GetNumContacts);
	CreateServiceFunction(MS_MC_GETSUBCONTACT, MetaAPI_GetContact);
	CreateServiceFunction(MS_MC_SETDEFAULTCONTACTNUM, MetaAPI_SetDefaultContactNum);
	CreateServiceFunction(MS_MC_SETDEFAULTCONTACT, MetaAPI_SetDefaultContact);
	CreateServiceFunction(MS_MC_FORCESENDCONTACTNUM, MetaAPI_ForceSendContactNum);
	CreateServiceFunction(MS_MC_FORCESENDCONTACT, MetaAPI_ForceSendContact);
	CreateServiceFunction(MS_MC_UNFORCESENDCONTACT, MetaAPI_UnforceSendContact);
	CreateServiceFunction(MS_MC_GETFORCESTATE, MetaAPI_GetForceState);

	CreateServiceFunction(MS_MC_CONVERTTOMETA, MetaAPI_ConvertToMeta);
	CreateServiceFunction(MS_MC_ADDTOMETA, MetaAPI_AddToMeta);
	CreateServiceFunction(MS_MC_REMOVEFROMMETA, MetaAPI_RemoveFromMeta);
}
