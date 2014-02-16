/*
MetaContacts Plugin for Miranda IM.

Copyright © 2004 Universite Louis PASTEUR, STRASBOURG.
Copyright © 2004 Scott Ellis (www.scottellis.com.au mail@scottellis.com.au)

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

/** @file meta_api.c 
*
* API functions needed to handle MetaContacts.
* Centralizes functions called by other plugins.
*/

#include "metacontacts.h"

//get the handle for a contact's parent metacontact
//wParam=(HANDLE)hSubContact
//lParam=0
//returns a handle to the parent metacontact, or null if this contact is not a subcontact
INT_PTR MetaAPI_GetMeta(WPARAM wParam, LPARAM lParam) {
	return (INT_PTR)(HANDLE)db_get_dw(wParam, META_PROTO, "Handle", 0);
}

//gets the handle for the default contact
//wParam=(HANDLE)hMetaContact
//lParam=0
//returns a handle to the default contact, or null on failure
INT_PTR MetaAPI_GetDefault(WPARAM wParam, LPARAM lParam) {
	DWORD default_contact_number = db_get_dw(wParam, META_PROTO, "Default", -1);
	if (default_contact_number != -1) {
		return (INT_PTR)Meta_GetContactHandle(wParam, default_contact_number);
	}
	return 0;
}

//gets the contact number for the default contact
//wParam=(HANDLE)hMetaContact
//lParam=0
//returns a DWORD contact number, or -1 on failure
INT_PTR MetaAPI_GetDefaultNum(WPARAM wParam, LPARAM lParam) {
	return db_get_dw(wParam, META_PROTO, "Default", -1);
}

//gets the handle for the 'most online' contact
//wParam=(HANDLE)hMetaContact
//lParam=0
//returns a handle to the 'most online' contact
INT_PTR MetaAPI_GetMostOnline(WPARAM wParam, LPARAM lParam) {
	return (INT_PTR)Meta_GetMostOnline(wParam);
}

//gets the number of subcontacts for a metacontact
//wParam=(HANDLE)hMetaContact
//lParam=0
//returns a DWORD representing the number of subcontacts for the given metacontact
INT_PTR MetaAPI_GetNumContacts(WPARAM wParam, LPARAM lParam) {
	DWORD num_contacts = db_get_dw(wParam, META_PROTO, "NumContacts", -1);
	return num_contacts;
}

//gets the handle of a subcontact, using the subcontact's number
//wParam=(HANDLE)hMetaContact
//lParam=(DWORD)contact number
//returns a handle to the specified subcontact
INT_PTR MetaAPI_GetContact(WPARAM wParam, LPARAM lParam) {
	return (INT_PTR)Meta_GetContactHandle(wParam, (DWORD)lParam);
}

//sets the default contact, using the subcontact's contact number
//wParam=(HANDLE)hMetaContact
//lParam=(DWORD)contact number
//returns 0 on success
INT_PTR MetaAPI_SetDefaultContactNum(WPARAM wParam, LPARAM lParam) {
	DWORD num_contacts = db_get_dw(wParam, META_PROTO, "NumContacts", -1);
	if (num_contacts == -1)
		return 1;
	if ((DWORD)lParam >= num_contacts || (DWORD)lParam < 0)
		return 1;
	if (db_set_dw(wParam, META_PROTO, "Default", (DWORD)lParam))
		return 1;

	NotifyEventHooks(hEventDefaultChanged, wParam, (LPARAM)Meta_GetContactHandle(wParam, (int)lParam));
	return 0;
}

//sets the default contact, using the subcontact's handle
//wParam=(HANDLE)hMetaContact
//lParam=(HANDLE)hSubcontact
//returns 0 on success
INT_PTR MetaAPI_SetDefaultContact(WPARAM wParam, LPARAM lParam) {
	MCONTACT hMeta = (MCONTACT)db_get_dw(lParam, META_PROTO, "Handle", 0);
	DWORD contact_number = Meta_GetContactNumber(lParam);
	if (contact_number == -1 || !hMeta || hMeta != wParam) 
		return 1;
	if (db_set_dw(hMeta, META_PROTO, "Default", contact_number))
		return 1;
	
	NotifyEventHooks(hEventDefaultChanged, wParam, lParam);
	return 0;
}

//forces the metacontact to send using a specific subcontact, using the subcontact's contact number
//wParam=(HANDLE)hMetaContact
//lParam=(DWORD)contact number
//returns 0 on success
INT_PTR MetaAPI_ForceSendContactNum(WPARAM wParam, LPARAM lParam) {
	MCONTACT hContact = Meta_GetContactHandle(wParam, (int)lParam);
	MCONTACT hMeta = (MCONTACT)db_get_dw(hContact, META_PROTO, "Handle", 0);
	if (!hContact || !hMeta || hMeta != wParam || db_get_b(hMeta, META_PROTO, "ForceDefault", 0)) 
		return 1;

	db_set_dw(hMeta, META_PROTO, "ForceSend", (DWORD)hContact);

	NotifyEventHooks(hEventForceSend, wParam, (LPARAM)hContact);
	return 0;
}

//forces the metacontact to send using a specific subcontact, using the subcontact's handle
//wParam=(HANDLE)hMetaContact
//lParam=(HANDLE)hSubcontact
//returns 0 on success (will fail if 'force default' is in effect)
INT_PTR MetaAPI_ForceSendContact(WPARAM wParam, LPARAM lParam) {
	MCONTACT hContact = lParam;
	MCONTACT hMeta = (MCONTACT)db_get_dw(hContact, META_PROTO, "Handle", 0);
	if (!hContact || !hMeta || hMeta != wParam || db_get_b(hMeta, META_PROTO, "ForceDefault", 0)) 
		return 1;

	db_set_dw(hMeta, META_PROTO, "ForceSend", (DWORD)hContact);

	NotifyEventHooks(hEventForceSend, wParam, lParam);
	return 0;
}

//'unforces' the metacontact to send using a specific subcontact
//wParam=(HANDLE)hMetaContact
//lParam=0
//returns 0 on success (will fail if 'force default' is in effect)
INT_PTR MetaAPI_UnforceSendContact(WPARAM wParam, LPARAM lParam) {
	if (db_get_b(wParam, META_PROTO, "ForceDefault", 0))
		return 1;

	db_set_dw(wParam, META_PROTO, "ForceSend", 0);
		
	NotifyEventHooks(hEventUnforceSend, wParam, lParam);
	return 0;
}


//'forces' or 'unforces' (i.e. toggles) the metacontact to send using it's default contact
// overrides 'force send' above, and will even force use of offline contacts
// will send ME_MC_FORCESEND event
//wParam=(HANDLE)hMetaContact
//lParam=0
//returns 1(true) or 0(false) representing new state of 'force default'
INT_PTR MetaAPI_ForceDefault(WPARAM wParam, LPARAM lParam) {
	// forward to menu function
	Meta_ForceDefault(wParam, lParam);
	return db_get_b(wParam, META_PROTO, "ForceDefault", 0);
}
	
// method to get state of 'force' for a metacontact
// wParam=(HANDLE)hMetaContact
// lParam= (DWORD)&contact_number or NULL
// if lparam supplied, the contact_number of the contatct 'in force' will be copied to the address it points to,
// or if none is in force, the value (DWORD)-1 will be copied
// (v0.8.0.8+ returns 1 if 'force default' is true with *lParam == default contact number, else returns 0 with *lParam as above)
INT_PTR MetaAPI_GetForceState(WPARAM wParam, LPARAM lParam) {
	MCONTACT hMeta = wParam;
	if ( !hMeta) return 0;
	
	if (db_get_b(hMeta, META_PROTO, "ForceDefault", 0)) {
		if (lParam) *(DWORD *)lParam = db_get_dw(wParam, META_PROTO, "Default", -1);
		return 1;
	}

	MCONTACT hContact = (MCONTACT)db_get_dw(hMeta, META_PROTO, "ForceSend", 0);
	if ( !hContact) {
		if (lParam) *(DWORD *)lParam = -1;
	} else {
		if (lParam) *(DWORD *)lParam = (DWORD)Meta_GetContactNumber(hContact);
	}

	return 0;
}

// method to get protocol name - used to be sure you're dealing with a "real" metacontacts plugin :)
// wParam=lParam=0
INT_PTR MetaAPI_GetProtoName(WPARAM wParam, LPARAM lParam) {
	return (INT_PTR)META_PROTO;
}

// added 0.9.5.0 (22/3/05)
// wParam=(HANDLE)hContact
// lParam=0
// convert a given contact into a metacontact
INT_PTR MetaAPI_ConvertToMeta(WPARAM wParam, LPARAM lParam) {
	return Meta_Convert(wParam, lParam);
}

// added 0.9.5.0 (22/3/05)
// wParam=(HANDLE)hContact
// lParam=(HANDLE)hMeta
// add an existing contact to a metacontact
INT_PTR MetaAPI_AddToMeta(WPARAM wParam, LPARAM lParam) {
	return Meta_Assign(wParam, lParam, FALSE);
}

// added 0.9.5.0 (22/3/05)
// wParam=0
// lParam=(HANDLE)hContact
// remove a contact from a metacontact
INT_PTR MetaAPI_RemoveFromMeta(WPARAM wParam, LPARAM lParam) {
	// notice we switch args - to keep the API function consistent with the others
	return Meta_Delete((WPARAM)lParam, (LPARAM)wParam);
}

// added 0.9.13.2 (6/10/05)
// wParam=(BOOL)disable
// lParam=0
// enable/disable the 'hidden group hack' - for clists that support subcontact hiding using 'IsSubcontact' setting
// should be called once in your 'onmodulesloaded' event handler

BOOL meta_group_hack_disabled = FALSE;		// this global flag is used in utils 'SetGroup' function

INT_PTR MetaAPI_DisableHiddenGroup(WPARAM wParam, LPARAM lParam) {
	meta_group_hack_disabled = (BOOL)wParam;
	return 0;
}
