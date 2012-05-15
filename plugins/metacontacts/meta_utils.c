/*
MetaContacts Plugin for Miranda IM.

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

/** @file meta_utils.c 
*
* Diverses functions useful in different places.
*/

#include "metacontacts.h"

HANDLE invisiGroup;
POINT menuMousePoint;

INT_PTR MyDBWriteContactSetting(HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv) {
	DBCONTACTWRITESETTING dcws;
	dcws.szModule = szModule;
	dcws.szSetting = szSetting;
	dcws.value = *dbv;
	return CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&dcws);
}

INT_PTR MyDBGetContactSetting(HANDLE hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv) {
	static BOOL strsvc, strsvcset = FALSE;
	memset(dbv, 0, sizeof(DBVARIANT));

	if(!strsvcset) {strsvc = ServiceExists(MS_DB_CONTACT_GETSETTING_STR); strsvcset = TRUE;}
	
	// preserve unicode strings - this service should return other data types unchanged
	if(strsvc) return DBGetContactSettingW(hContact, szModule, szSetting, dbv);
	
	return DBGetContactSetting(hContact, szModule, szSetting, dbv);
}

int Meta_EqualDBV(DBVARIANT *dbv, DBVARIANT *id) {
	DWORD res = 1;
	int i;
	if(dbv->type == id->type)
	{	// If the id parameter and the value returned by the DBGetContactSetting
		// are the same, this is the correct HANDLE, return it.
		switch(dbv->type)
		{
			case DBVT_DELETED:
				return 1;
			case DBVT_BYTE:
				if(dbv->bVal == id->bVal)
					return 1;
			case DBVT_WORD:
				if(dbv->wVal == id->wVal)
					return 1;
			case DBVT_DWORD:
				if(dbv->dVal == id->dVal)
					return 1;
				break;
			case DBVT_ASCIIZ:
			case DBVT_UTF8:
				if(!strcmp(dbv->pszVal,id->pszVal))
					return 1;
			case DBVT_WCHAR:
				if(!wcscmp(dbv->pwszVal,id->pwszVal))
					return 1;
			case DBVT_BLOB:
				if(dbv->cpbVal == id->cpbVal)
				{
					for(i=dbv->cpbVal;res && i<=0;i--)
						res = (dbv->pbVal[i] == id->pbVal[i]);
					if(res)
					{
						return 1;
					}
				}
				break;
		} // end switch
	} // end if(dbv.type == id.type)

	return 0;
}

/** Retrieve a \c HANDLE from a protocol and an identifier
*
* @param protocol :	Name of the protocol
* @param id :		Unique field identifiying the contact searched
*
* @return			a \c HANDLE to the specified contact or \c NULL
					if no contact has been found.
*/
HANDLE Meta_GetHandle(const char *protocol, DBVARIANT *id)
{
	char *field;
	DBVARIANT dbv;
	HANDLE hUser;
	DWORD i,res = 1;
	// Get the field identifying the contact in the database.
	char str[MAXMODULELABELLENGTH];
	strcpy(str,protocol);
	strcat(str,PS_GETCAPS);
	if(!ServiceExists(str))
		return NULL;
	
	field = (char *)CallProtoService(protocol,PS_GETCAPS,PFLAG_UNIQUEIDSETTING,0);
	hUser = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	while(hUser)
	{	// Scan the database and retrieve the field for each contact
		if(!DBGetContactSetting(hUser,protocol,field,&dbv))
		{
			if(dbv.type == id->type)
			{	// If the id parameter and the value returned by the DBGetContactSetting
				// are the same, this is the correct HANDLE, return it.
				switch(dbv.type)
				{
					case DBVT_DELETED:
						break;
					case DBVT_BYTE:
						if(dbv.bVal == id->bVal)
							return hUser;
						break;
					case DBVT_WORD:
						if(dbv.wVal == id->wVal)
							return hUser;
						break;
					case DBVT_DWORD:
						if(dbv.dVal == id->dVal)
							return hUser;
						break;
					case DBVT_ASCIIZ:
					case DBVT_UTF8:
						if(!strcmp(dbv.pszVal,id->pszVal))
						{
							DBFreeVariant(&dbv);
							return hUser;
						}
						else
						{
							DBFreeVariant(&dbv);
							break;
						}
					case DBVT_WCHAR:
						if(!wcscmp(dbv.pwszVal,id->pwszVal))
						{
							DBFreeVariant(&dbv);
							return hUser;
						}
						else
						{
							DBFreeVariant(&dbv);
							break;
						}
					case DBVT_BLOB:
						if(dbv.cpbVal == id->cpbVal)
						{
							for(i=dbv.cpbVal;res && i<=0;i--)
								res = (dbv.pbVal[i] == id->pbVal[i]);
							if(res)
							{
								DBFreeVariant(&dbv);
								return hUser;
							}
						}
						DBFreeVariant(&dbv);
						break;
				} // end switch
			} else
				DBFreeVariant(&dbv);
		} // end if(!DBGetContactSetting(hUser,protocol,field,&dbv))
		// This contact wasn't the good one, go to the next.
		hUser = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hUser,0);
	} // end while
	return NULL;
}

/** Update the MetaContact login, depending on the protocol desired
*
* The login of the "MetaContacts" protocol will be copied from the login
* of the specified protocol.
*
* @param proto :	The name of the protocol used to get the login that will be
*					affected to the "MetaContacts" protocol.
*
* @return			O on success, 1 otherwise.
*/
int Meta_SetNick(char *proto)
{
	CONTACTINFO ci;
	ci.cbSize = sizeof(CONTACTINFO);
	ci.dwFlag = CNF_DISPLAY;
	ci.hContact = NULL;
	ci.szProto = proto;
	if(CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci))
		return 1;
	switch(ci.type)
	{
		case CNFT_BYTE:
			if(DBWriteContactSettingByte(NULL,META_PROTO,"Nick",ci.bVal))
				return 1;
			break;
		case CNFT_WORD:
			if(DBWriteContactSettingWord(NULL,META_PROTO,"Nick",ci.wVal))
				return 1;
			break;
		case CNFT_DWORD:
			if(DBWriteContactSettingDword(NULL,META_PROTO,"Nick",ci.dVal))
				return 1;
			break;
		case CNFT_ASCIIZ:
			if(DBWriteContactSettingString(NULL,META_PROTO,"Nick",ci.pszVal))
				return 1;
			mir_free(ci.pszVal);
			break;
		default:
			if(DBWriteContactSettingString(NULL,META_PROTO,"Nick",(char *)Translate("Sender")))
				return 1;
			break;
	}
	return 0;
}

/** Assign a contact (src) to a metacontact (dest)
*
* @param src			\c HANDLE to a contact that should be assigned
* @param dest			\c HANDLE to a metacontact that will host the contact
* @param set_as_default	\c bool flag to indicate whether the new contact becomes the default
*
* @return		TRUE on success, FALSE otherwise
*/
BOOL Meta_Assign(HANDLE src, HANDLE dest, BOOL set_as_default)
{
	DBCONTACTWRITESETTING cws;
	DWORD metaID;
	char *proto,
		*field;
	DWORD num_contacts;
	char buffer[512], buffer2[512];
	WORD status;
	HANDLE most_online;
		
	if((metaID=DBGetContactSettingDword(dest,META_PROTO,META_ID,(DWORD)-1))==-1) {
		MessageBox(0, Translate("Could not get MetaContact id"), Translate("Assignment Error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}
	if((num_contacts=DBGetContactSettingDword(dest,META_PROTO,"NumContacts",(DWORD)-1))==-1) {
		MessageBox(0, Translate("Could not retreive MetaContact contact count"), Translate("Assignment Error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}
	if(!(proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)src,0))) {
		MessageBox(0, Translate("Could not retreive contact protocol"), Translate("Assignment Error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	// Get the login of the subcontact
	field = (char *)CallProtoService(proto,PS_GETCAPS,PFLAG_UNIQUEIDSETTING,0);
	if(DBGetContactSetting(src,proto,field,&cws.value)) {
		MessageBox(0, Translate("Could not get unique id of contact"), Translate("Assignment Error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	// Check that is is 'on the list'
	if(DBGetContactSettingByte(src, "CList", "NotOnList", 0) == 1) {
		MessageBox(0, Translate("Contact is 'Not on List' - please add the contact to your contact list before assigning."), Translate("Assignment Error"), MB_OK | MB_ICONWARNING);
		DBFreeVariant(&cws.value);
		return FALSE;
	}

	num_contacts++;
	if(num_contacts >= MAX_CONTACTS) {
		MessageBox(0, Translate("MetaContact is full"), Translate("Assignment Error"), MB_OK | MB_ICONWARNING);
		DBFreeVariant(&cws.value);
		return FALSE;
	}

	// write the contact's protocol
	strcpy(buffer, "Protocol");
	strcat(buffer, _itoa((int)(num_contacts -1), buffer2, 10));

	if(DBWriteContactSettingString(dest, META_PROTO, buffer, proto)) {
		MessageBox(0, Translate("Could not write contact protocol to MetaContact"), Translate("Assignment Error"), MB_OK | MB_ICONWARNING);
		DBFreeVariant(&cws.value);
		return FALSE;
	}

	// write the login
	strcpy(buffer, "Login");
	strcat(buffer, _itoa((int)(num_contacts - 1), buffer2, 10));

	cws.szModule=META_PROTO;
	cws.szSetting=buffer;

	if(CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)dest,(LPARAM)&cws)) {
		MessageBox(0, Translate("Could not write unique id of contact to MetaContact"), Translate("Assignment Error"), MB_OK | MB_ICONWARNING);
		DBFreeVariant(&cws.value);
		return FALSE;
	}

	DBFreeVariant(&cws.value);

	// If we can get the nickname of the subcontact...
	if(!DBGetContactSetting(src,proto,"Nick",&cws.value)) {
		// write the nickname
		strcpy(buffer, "Nick");
		strcat(buffer, _itoa((int)(num_contacts - 1), buffer2, 10));

		cws.szModule=META_PROTO;
		cws.szSetting=buffer;

		if(CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)dest,(LPARAM)&cws)) {
			MessageBox(0, Translate("Could not write nickname of contact to MetaContact"), Translate("Assignment Error"), MB_OK | MB_ICONWARNING);
			return FALSE;
		}

		DBFreeVariant(&cws.value);
	}

	{
		// Get the displayname of the subcontact
		char *name = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)src, 0);
		wchar_t *wname = (wchar_t *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)src, GCDNF_UNICODE);

		// write the display name
		strcpy(buffer, "CListName");
		strcat(buffer, _itoa((int)(num_contacts - 1), buffer2, 10));

		if(wname && strncmp(name, (char *)wname, strlen(name)) != 0) {
			DBWriteContactSettingWString(dest, META_PROTO, buffer, wname);
		} else
			DBWriteContactSettingString(dest, META_PROTO, buffer, name);

		// Get the status
		if(!proto)
			status = ID_STATUS_OFFLINE;
		else
			status = DBGetContactSettingWord(src, proto, "Status", ID_STATUS_OFFLINE);
	}

	// write the status
	strcpy(buffer, "Status");
	strcat(buffer, _itoa((int)(num_contacts - 1), buffer2, 10));
	if(DBWriteContactSettingWord(dest, META_PROTO, buffer, status)) {
		MessageBox(0, Translate("Could not write contact status to MetaContact"), Translate("Assignment Error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	// write the handle
	strcpy(buffer, "Handle");
	strcat(buffer, _itoa((int)(num_contacts - 1), buffer2, 10));
	if(DBWriteContactSettingDword(dest, META_PROTO, buffer, (DWORD)src)) {
		MessageBox(0, Translate("Could not write contact handle to MetaContact"), Translate("Assignment Error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	// write status string
	strcpy(buffer, "StatusString");
	strcat(buffer, _itoa((int)(num_contacts - 1), buffer2, 10));
	Meta_GetStatusString(status, buffer2, 512);
	if(DBWriteContactSettingString(dest, META_PROTO, buffer, buffer2)) {
		MessageBox(0, Translate("Could not write contact status string to MetaContact"), Translate("Assignment Error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	// Add the MetaContactFilter module to the old contact to intercept messages and forward to the new
	// metacontact
	if( !CallService( MS_PROTO_ISPROTOONCONTACT, (WPARAM)src, ( LPARAM )META_FILTER ))
		CallService( MS_PROTO_ADDTOCONTACT, (WPARAM)src, ( LPARAM )META_FILTER );

	// Write the link in the contact
	if(DBWriteContactSettingDword(src,META_PROTO,META_LINK,metaID)) {
		MessageBox(0, Translate("Could not write MetaContact id to contact"), Translate("Assignment Error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	// Write the contact number in the contact
	if(DBWriteContactSettingDword(src,META_PROTO,"ContactNumber",(DWORD)(num_contacts - 1))) {
		MessageBox(0, Translate("Could not write MetaContact contact number to contact"), Translate("Assignment Error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	// Write the handle in the contact
	if(DBWriteContactSettingDword(src,META_PROTO,"Handle",(DWORD)dest)) {
		MessageBox(0, Translate("Could not write MetaContact contact number to contact"), Translate("Assignment Error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	// update count of contacts
	if(DBWriteContactSettingDword(dest,META_PROTO,"NumContacts",num_contacts)) {
		MessageBox(0, Translate("Could not write contact count to MetaContact"), Translate("Assignment Error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	if(set_as_default) {
		DBWriteContactSettingDword(dest, META_PROTO, "Default", (WORD)(num_contacts - 1));
		NotifyEventHooks(hEventDefaultChanged, (WPARAM)dest, (LPARAM)src);
	}

	DBWriteContactSettingByte(src, META_PROTO, "IsSubcontact", 1);
	// set nick to most online contact that can message
	most_online = Meta_GetMostOnline(dest);
	Meta_CopyContactNick(dest, most_online);

	// set status to that of most online contact
	Meta_FixStatus(dest);

	// if the new contact is the most online contact with avatar support, get avatar info
	most_online = Meta_GetMostOnlineSupporting(dest, PFLAGNUM_4, PF4_AVATARS);
	if(most_online == src) {
		PROTO_AVATAR_INFORMATION AI;

		AI.cbSize = sizeof(AI);
		AI.hContact = dest;
		AI.format = PA_FORMAT_UNKNOWN;
		strcpy(AI.filename, "X");

		if((int)CallProtoService(META_PROTO, PS_GETAVATARINFO, 0, (LPARAM)&AI) == GAIR_SUCCESS)
	        DBWriteContactSettingString(dest, "ContactPhoto", "File",AI.filename);
	}

	// Hide the contact
	Meta_SetGroup(src);

	// copy history
	if(options.copy_subcontact_history)
		copyHistory(src, dest);

	// Ignore status if the option is on
	if(options.suppress_status)
		CallService(MS_IGNORE_IGNORE, (WPARAM)src, (WPARAM)IGNOREEVENT_USERONLINE);

	// copy other data
	Meta_CopyData(dest);

	NotifyEventHooks(hSubcontactsChanged, (WPARAM)dest, 0);
	return TRUE;
}


/**
*	Convenience method - get most online contact supporting messaging
*
*/
HANDLE Meta_GetMostOnline(HANDLE hMeta) {
	return Meta_GetMostOnlineSupporting(hMeta, PFLAGNUM_1, PF1_IM);
}
/** Get the 'most online' contact for a meta contact (according to above order) which supports the specified
* protocol service, and copies nick from that contact
*
* @param hMeta	\c HANDLE to a metacontact
*
* @return		\c HANDLE to a contact
*/

HANDLE Meta_GetMostOnlineSupporting(HANDLE hMeta, int pflagnum, unsigned long capability) {
	int most_online_status = ID_STATUS_OFFLINE;
	HANDLE most_online_contact = NULL, hContact;
	int i, status, default_contact_number, num_contacts;
	unsigned long caps = 0;
	char *proto, *most_online_proto;

	// you can't get more online than having the default contact ONLINE - so check that first
	if((default_contact_number = DBGetContactSettingDword(hMeta,META_PROTO,"Default",(DWORD)-1)) == (DWORD)-1)
	{
		// This is a simple contact - return NULL to signify error.
		// (this should normally not happen, since all meta contacts have a default contact)
		return NULL;
	}

	// if the default is beyond the end of the list (eek!) return null
	if(default_contact_number >= (num_contacts = DBGetContactSettingDword(hMeta, META_PROTO, "NumContacts", 0)))
		return NULL;

	most_online_contact = Meta_GetContactHandle(hMeta, default_contact_number);
	proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)most_online_contact, 0);
	caps = proto ? CallProtoService(proto, PS_GETCAPS, (WPARAM)pflagnum, 0) : 0;
	if(proto && strcmp(proto, "IRC") == 0) caps |= PF1_IM;
	// we are forced to do use default for sending - '-1' capability indicates no specific capability, but respect 'Force Default'
	if(proto && DBGetContactSettingByte(hMeta, META_PROTO, "ForceDefault", 0) && capability != 0 && (capability == -1 || (caps & capability) == capability)) // capability is 0 when we're working out status
		return most_online_contact;

	// a subcontact is being temporarily 'forced' to do sending
	if((most_online_contact = (HANDLE)DBGetContactSettingDword(hMeta, META_PROTO, "ForceSend", 0))) {
		caps = proto ? CallProtoService(proto, PS_GETCAPS, (WPARAM)pflagnum, 0) : 0;
		if(proto && strcmp(proto, "IRC") == 0) caps |= PF1_IM;
		if(proto && (caps & capability) == capability && capability != 0) // capability is 0 when we're working out status
			return most_online_contact;
	}

	most_online_contact = Meta_GetContactHandle(hMeta, default_contact_number);
	proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)most_online_contact, 0);
	if(proto && CallProtoService(proto, PS_GETSTATUS, 0, 0) >= ID_STATUS_ONLINE) {
		caps = proto ? CallProtoService(proto, PS_GETCAPS, (WPARAM)pflagnum, 0) : 0;
		if(proto && strcmp(proto, "IRC") == 0) caps |= PF1_IM;
		if(proto && (capability == -1 || (caps & capability) == capability)) {
			most_online_status = DBGetContactSettingWord(most_online_contact, proto, "Status", ID_STATUS_OFFLINE);

			// if our default is not offline, and option to use default is set - return default
			// and also if our default is online, return it
			if(most_online_status == ID_STATUS_ONLINE || (most_online_status != ID_STATUS_OFFLINE && options.always_use_default)) {
				return most_online_contact;
			}
		} else
			most_online_status = ID_STATUS_OFFLINE;
	} else
		most_online_status = ID_STATUS_OFFLINE;

	most_online_proto = proto;
	// otherwise, check all the subcontacts for the one closest to the ONLINE state which supports the required capability
	for(i = 0; i < num_contacts; i++) {
		if(i == default_contact_number) // already checked that (i.e. initial value of most_online_contact and most_online_status are those of the default contact)
			continue;

		hContact = Meta_GetContactHandle(hMeta, i);
		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

		if(!proto || CallProtoService(proto, PS_GETSTATUS, 0, 0) < ID_STATUS_ONLINE) // proto offline or connecting
			continue;

		caps = proto ? CallProtoService(proto, PS_GETCAPS, (WPARAM)pflagnum, 0) : 0;
		if(proto && strcmp(proto, "IRC") == 0) caps |= PF1_IM;
		if(proto && (capability == -1 || (caps & capability) == capability)) {

			status = DBGetContactSettingWord(hContact, proto, "Status", ID_STATUS_OFFLINE);

			if(status == ID_STATUS_ONLINE) {
				most_online_contact = hContact;
				most_online_proto = proto;
				return most_online_contact;
			} else if(status <= ID_STATUS_OFFLINE) // status below ID_STATUS_OFFLINE is a connecting status
				continue;
			else {
				if(GetRealPriority(proto, status) < GetRealPriority(most_online_proto, most_online_status)) {
					most_online_status = status;
					most_online_contact = hContact;
					most_online_proto = proto;
				}
			}
		}
	}

	// no online contacts? if we're trying to message, use 'send offline' contact
	if(most_online_status == ID_STATUS_OFFLINE && capability == PF1_IM) {
		HANDLE hOffline = Meta_GetContactHandle(hMeta, DBGetContactSettingDword(hMeta, META_PROTO, "OfflineSend", (DWORD)-1));
		if(hOffline)
			most_online_contact = hOffline;
	}

	return most_online_contact;
}

int Meta_GetContactNumber(HANDLE hContact) {
	return DBGetContactSettingDword(hContact, META_PROTO, "ContactNumber", -1);
}

BOOL dbv_same(DBVARIANT *dbv1, DBVARIANT *dbv2) {
	if(dbv1->type != dbv2->type) return FALSE;

	switch(dbv1->type) {
	case DBVT_BYTE:
		return dbv1->bVal == dbv2->bVal;
	case DBVT_WORD:
		return dbv1->wVal == dbv2->wVal;
	case DBVT_DWORD:
		return dbv1->dVal == dbv2->dVal;
	case DBVT_ASCIIZ:
		return !strcmp(dbv1->pszVal, dbv2->pszVal);
	case DBVT_BLOB:
		return (dbv1->cpbVal == dbv2->cpbVal && !memcmp(dbv1->pbVal, dbv2->pbVal, dbv1->cpbVal));
		break;
	default:
		return FALSE;
	}
}

 void copy_settings_array(HANDLE hMeta, char *module, const char *settings[], int num_settings) {
	int num_contacts = DBGetContactSettingDword(hMeta, META_PROTO, "NumContacts", (DWORD)-1),
		default_contact = DBGetContactSettingDword(hMeta, META_PROTO, "Default", (DWORD)-1),
		most_online = Meta_GetContactNumber(Meta_GetMostOnline(hMeta));

	HANDLE hContact;
	int i, j;
	char *used_mod;
	
	DBVARIANT dbv1, dbv2;
	BOOL free, got_val, bDataWritten;

	BOOL use_default = FALSE;
	int source_contact = (use_default ? default_contact : most_online);

	if(source_contact < 0 || source_contact >= num_contacts) return;

	for(i = 0; i < num_settings; i++) {
		bDataWritten = FALSE;
		for(j = 0; j < num_contacts && !bDataWritten; j++) {
			// do source (most online) first
			if(j == 0) 
				hContact = Meta_GetContactHandle(hMeta, source_contact);
			else if(j <= source_contact)
				hContact = Meta_GetContactHandle(hMeta, j - 1);
			else
				hContact = Meta_GetContactHandle(hMeta, j);

			if(hContact) {
				if(!module) {
					used_mod = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
					if(!used_mod) continue; // next contact
				} else
					used_mod = module;

				if(j == 0 && strcmp(settings[i], "MirVer") == 0) { //Always reset MirVer
					DBDeleteContactSetting(hMeta, (module ? used_mod : META_PROTO), settings[i]);
				}
					
				got_val = !MyDBGetContactSetting(hContact, used_mod, settings[i], &dbv2);
				if(got_val) {
					free = !MyDBGetContactSetting(hMeta, (module ? used_mod : META_PROTO), settings[i], &dbv1);
					
					if(strcmp(settings[i], "MirVer") == 0) {
						if(DBGetContactSettingWord(hContact, used_mod, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE) {
							if(!free || (dbv1.pszVal == NULL || strcmp(dbv1.pszVal, "") == 0 || strlen(dbv1.pszVal) < 2)) {
								MyDBWriteContactSetting(hMeta, (module ? used_mod : META_PROTO), settings[i], &dbv2);
								bDataWritten = TRUE; //only break if found something to copy
							}
						}

					} else {
						if(!free || !dbv_same(&dbv1, &dbv2)) {
							MyDBWriteContactSetting(hMeta, (module ? used_mod : META_PROTO), settings[i], &dbv2);
							if(dbv2.type == DBVT_ASCIIZ || dbv2.type == DBVT_UTF8) {
								if(dbv2.pszVal != NULL && strcmp(dbv2.pszVal, "") != 0)
									bDataWritten = TRUE; //only break if found something to copy
							} else if(dbv2.type == DBVT_WCHAR) {
								if(dbv2.pwszVal != 0 && wcscmp(dbv2.pwszVal, L"") != 0)
									bDataWritten = TRUE; //only break if found something to copy
							} else
								bDataWritten = TRUE; //only break if found something to copy
						}
						else
							bDataWritten = TRUE;

					}		
					DBFreeVariant(&dbv2);
					if(free)DBFreeVariant(&dbv1);
				} 
			}
		}
	}
}

const char *ProtoSettings[25] = 
	{"BirthDay", "BirthMonth", "BirthYear", "Age", "Cell", "Cellular", "Homepage", "email", "Email", "e-mail",
	"FirstName", "MiddleName", "LastName", "Title", "Timezone", "Gender", "MirVer", "ApparentMode", "IdleTS", "LogonTS", "IP", "RealIP",
	"Auth", "ListeningTo", "Country"};
const char *UserInfoSettings[71] = 
	{"NickName", "FirstName", "MiddleName", "LastName", "Title", "Timezone", "Gender", "DOBd", "DOBm", "DOBy",
	"Mye-mail0", "Mye-mail1", "MyPhone0", "MyPhone1", "MyNotes", "PersonalWWW", 
	"HomePhone", "HomeFax", "HomeMobile", "HomeStreet", "HomeCity", "HomeState", "HomeZip", "HomeCountry",
	"WorkPhone", "WorkFax", "WorkMobile", "WorkStreet", "WorkCity", "WorkState", "WorkZip", "WorkCountry", "Company", "Department", "Position", 
	"Occupation", "Cellular", "Cell", "Phone", "Notes",

	 "e-mail", "e-mail0", "e-mail1", "Homepage", "MaritalStatus",
	"CompanyCellular", "CompanyCity", "CompanyState", "CompanyStreet", "CompanyCountry", "Companye-mail", 
	"CompanyHomepage", "CompanyDepartment", "CompanyOccupation", "CompanyPosition", "CompanyZip", 

	"OriginCity", "OriginState", "OriginStreet", "OriginCountry", "OriginZip", 
	"City", "State", "Street", "Country", "Zip", 

	"Language1", "Language2", "Language3", "Partner", "Gender"};
const char *ContactPhotoSettings[5] = 
	{"File","Backup","Format","ImageHash","RFile"};
const char *MBirthdaySettings[3] = 
	{ "BirthDay", "BirthMonth", "BirthYear"};

// special handling for status message
// copy from first subcontact with any of these values that has the same status as the most online contact
// proto: 
// clist: "StatusMsg"
void CopyStatusData(HANDLE hMeta) {
	int num_contacts = DBGetContactSettingDword(hMeta, META_PROTO, "NumContacts", (DWORD)-1),
		most_online = Meta_GetContactNumber(Meta_GetMostOnline(hMeta));
	WORD status = DBGetContactSettingWord(hMeta, META_PROTO, "Status", ID_STATUS_OFFLINE);
	HANDLE hContact;
	DBVARIANT dbv;
	char *proto;
	BOOL bDoneStatus = FALSE, bDoneXStatus = FALSE;
	int i;

	for(i = 0; i < num_contacts; i++) {
		if(i == 0) 
			hContact = Meta_GetContactHandle(hMeta, most_online);
		else if(i <= most_online)
			hContact = Meta_GetContactHandle(hMeta, i - 1);
		else
			hContact = Meta_GetContactHandle(hMeta, i);

		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

		if(proto && DBGetContactSettingWord(hContact, proto, "Status", ID_STATUS_OFFLINE) == status) {
			if(!bDoneStatus && !MyDBGetContactSetting(hContact, "CList", "StatusMsg", &dbv)) {
				MyDBWriteContactSetting(hMeta, "CList", "StatusMsg", &dbv);
				DBFreeVariant(&dbv);
				bDoneStatus = TRUE;
			}
			if((!bDoneXStatus) && (!MyDBGetContactSetting(hContact, proto, "XStatusId", &dbv)) && dbv.type != DBVT_DELETED) {
				DBWriteContactSettingString(hMeta, META_PROTO, "XStatusProto", proto);
				MyDBWriteContactSetting(hMeta, META_PROTO, "XStatusId", &dbv);

				DBFreeVariant(&dbv);
				if(!MyDBGetContactSetting(hContact, proto, "XStatusMsg", &dbv)) {
					MyDBWriteContactSetting(hMeta, META_PROTO, "XStatusMsg", &dbv);
					DBFreeVariant(&dbv);
				}
				if(!MyDBGetContactSetting(hContact, proto, "XStatusName", &dbv)) {
					MyDBWriteContactSetting(hMeta, META_PROTO, "XStatusName", &dbv);
					DBFreeVariant(&dbv);
				}
				bDoneXStatus = TRUE;
			}
		}

		if(bDoneStatus && bDoneXStatus) break;
	}

	if(!bDoneStatus) DBDeleteContactSetting(hMeta, "CList", "StatusMsg");
	if(!bDoneXStatus) {
		DBDeleteContactSetting(hMeta, META_PROTO, "XStatusId");
		DBDeleteContactSetting(hMeta, META_PROTO, "XStatusMsg");
		DBDeleteContactSetting(hMeta, META_PROTO, "XStatusName");
	}
}

void Meta_CopyData(HANDLE hMeta) {
	if(options.copydata) {
		CopyStatusData(hMeta);

		copy_settings_array(hMeta, 0, ProtoSettings, 25);
		copy_settings_array(hMeta, "mBirthday", UserInfoSettings, 3);
		copy_settings_array(hMeta, "ContactPhoto", ContactPhotoSettings, 5);

		if(options.copy_userinfo) 
			copy_settings_array(hMeta, "UserInfo", UserInfoSettings, 71);
	}
}


HANDLE Meta_GetContactHandle(HANDLE hMeta, int contact_number) {
	char buffer[512], buffer2[512];
	int num_contacts = DBGetContactSettingDword(hMeta, META_PROTO, "NumContacts", 0);

	if(contact_number >= num_contacts || contact_number < 0) return 0;
	
	strcpy(buffer, "Handle");
	strcat(buffer, _itoa(contact_number, buffer2, 10));
	return (HANDLE)DBGetContactSettingDword(hMeta, META_PROTO, buffer, 0);

}

int Meta_SetHandles(void) {
	DWORD meta_id, num_contacts, contact_number;
	HANDLE hContact2, hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 ), hNextContact;
	char buffer[512], buffer2[512];
	char nick_buffer[128];
	BOOL found;

	while ( hContact != NULL ) {
		if((meta_id = DBGetContactSettingDword(hContact,META_PROTO,META_LINK,(DWORD)-1))!=(DWORD)-1) {
			// is a subcontact

			// get nick for debug messages
			strcpy(nick_buffer, "meta_id: ");
			strcat(nick_buffer, _itoa(meta_id, buffer2, 10));

			contact_number = DBGetContactSettingDword(hContact, META_PROTO, "ContactNumber", (DWORD)-1);
			strcat(nick_buffer, ", contact num: ");
			strcat(nick_buffer, _itoa(contact_number, buffer2, 10));

			if(contact_number < 0) {
				// problem!
				MessageBox(0, "Subcontact contact number < 0 - deleting MetaContact", nick_buffer, MB_OK | MB_ICONERROR);
				//CallService(MS_DB_CONTACT_DELETE, (WPARMA)hContact, 0);
				hNextContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
				Meta_Delete((WPARAM)hContact, (LPARAM)1);
				hContact = hNextContact;
				continue;		
				//return 1;
			}

			// ensure the window open flag is not present
			DBDeleteContactSetting(hContact, META_PROTO, "WindowOpen");


			// find metacontact
			found = FALSE;
			hContact2 = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );

			while ( hContact2 != NULL ) {
				if(DBGetContactSettingDword(hContact2,META_PROTO,META_ID,(DWORD)-1) == meta_id) {
					found = TRUE;
					
					// set handle
					DBWriteContactSettingDword(hContact, META_PROTO, "Handle", (DWORD)hContact2);

					// increment contact counter (cleared in Load function)
					DBWriteContactSettingByte(hContact2, META_PROTO, "ContactCountCheck", 
						(unsigned char)(DBGetContactSettingByte(hContact2, META_PROTO, "ContactCountCheck", 0) + 1));

					num_contacts = DBGetContactSettingDword(hContact2, META_PROTO, "NumContacts", (DWORD)-1);
					if(contact_number >= 0 && contact_number < num_contacts) {
						// set metacontact's handle to us
						strcpy(buffer, "Handle");
						strcat(buffer, _itoa((int)contact_number, buffer2, 10));
						DBWriteContactSettingDword(hContact2, META_PROTO, buffer, (DWORD)hContact);
					} else {
						char buff[256];
						// problem - contact number is greater than meta's number of contacts
						sprintf(buff, "Subcontact contact number (%d) > meta num contacts (%d) - deleting MetaContact", (int)contact_number, (int)num_contacts);
						MessageBox(0, buff, nick_buffer, MB_OK | MB_ICONERROR);
						//CallService(MS_DB_CONTACT_DELETE, (WPARMA)hContact, 0);
						hNextContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
						Meta_Delete((WPARAM)hContact, (LPARAM)1);
						hContact = hNextContact;
						continue;		
						//return 1;
					}
				}

				hContact2 = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact2, 0 );
			}

			if(!found) {
				// problem - subcontact's meta not found
				MessageBox(0, "Subcontact's MetaContact not found - deleting MetaContact data", nick_buffer, MB_OK | MB_ICONERROR);

				// delete meta data
				DBDeleteContactSetting(hContact,META_PROTO,"IsSubcontact");
				DBDeleteContactSetting(hContact,META_PROTO,META_LINK);
				DBDeleteContactSetting(hContact,META_PROTO,"Handle");
				DBDeleteContactSetting(hContact,META_PROTO,"ContactNumber");
				Meta_RestoreGroup(hContact);
				DBDeleteContactSetting(hContact,META_PROTO,"OldCListGroup");
				
				CallService(MS_PROTO_REMOVEFROMCONTACT, (WPARAM)hContact, (LPARAM)META_FILTER);
				// stop ignoring, if we were
				if(options.suppress_status)
					CallService(MS_IGNORE_UNIGNORE, (WPARAM)hContact, (WPARAM)IGNOREEVENT_USERONLINE);
				
			} else {
				if(!DBGetContactSettingByte(hContact, META_PROTO, "IsSubcontact", 0))
					DBWriteContactSettingByte(hContact, META_PROTO, "IsSubcontact", 1);
			}

		} else
			DBDeleteContactSetting(hContact, META_PROTO, "Handle");

		if((meta_id = DBGetContactSettingDword(hContact,META_PROTO,META_ID,(DWORD)-1))!=(DWORD)-1) {
			// is a metacontact

			// get nick for debug messages
			strcpy(nick_buffer, "meta_id: ");
			strcat(nick_buffer, _itoa(meta_id, buffer2, 10));

			// ensure the window open flag is not present
			DBDeleteContactSetting(hContact, META_PROTO, "WindowOpen");

			// ensure default is reasonable
			contact_number = DBGetContactSettingDword(hContact, META_PROTO, "Default", -1);
			num_contacts = DBGetContactSettingDword(hContact, META_PROTO, "NumContacts", (DWORD)-1);

			if(num_contacts < 0) {
				// problem
				MessageBox(0, "MetaContact number of contacts < 0 - deleting MetaContact", nick_buffer, MB_OK | MB_ICONERROR);
				//CallService(MS_DB_CONTACT_DELETE, (WPARMA)hContact, 0);
				hNextContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
				Meta_Delete((WPARAM)hContact, (LPARAM)1);
				hContact = hNextContact;
				continue;		
				//return 1;
			}
			
			if(contact_number < 0 || contact_number >= num_contacts) {
				// problem
				MessageBox(0, "MetaContact default contact number out of range - deleting MetaContact", nick_buffer, MB_OK | MB_ICONERROR);
				hNextContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
				Meta_Delete((WPARAM)hContact, (LPARAM)1);
				hContact = hNextContact;
				//CallService(MS_DB_CONTACT_DELETE, (WPARMA)hContact, 0);
				continue;		
				//return 1;
			}
		}

		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}
	
	// loop through one more time - check contact counts match
	hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL ) {
		if((meta_id = DBGetContactSettingDword(hContact,META_PROTO,META_ID,(DWORD)-1))!=(DWORD)-1) {
			// get nick for debug messages
			strcpy(nick_buffer, Translate("MetaId: "));
			strcat(nick_buffer, _itoa(meta_id, buffer2, 10));

			num_contacts = DBGetContactSettingByte(hContact, META_PROTO, "ContactCountCheck", -2);
			if(num_contacts != (DWORD)DBGetContactSettingDword(hContact, META_PROTO, "NumContacts", (DWORD)-1)) {
				// mismatch
				//if(MessageBox(0, Translate("MetaContact corrupted - the number of subcontacts is incorrect.\nDelete MetaContact?"), nick_buffer, MB_YESNO | MB_ICONERROR) == IDYES) {
				//	Meta_Delete((WPARAM)hContact, (LPARAM)1); // second param prevents confirm dialog
				//} else
				//	return 1;
				hNextContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
				MessageBox(0, Translate("MetaContact corrupted - the number of subcontacts is incorrect.\nDeleting MetaContact."), nick_buffer, MB_OK | MB_ICONERROR);
				Meta_Delete((WPARAM)hContact, (LPARAM)1);
				hContact = hNextContact;
				continue;		
			}
		}
		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}

	return 0;
}

/** Hide all contacts linked to any meta contact, and set handle links
*
* Additionally, set all sub contacts and metacontacts to offline so that status notifications are always sent
*
* and ensure metafilter in place
*/
int Meta_HideLinkedContacts(void) {
	DBVARIANT dbv, dbv2;
	DWORD meta_id, num_contacts, contact_number;
	WORD status;
	char buffer[512], buffer2[512];
	char *proto, *group_name;
	int hGroup = 1;

	HANDLE hContact2, hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );

	// ensure the hidden group does not exist (how this occurs i wonder but there have been reports!)
	// (sometimes protocol server side groups are to blame - msn and icq)
	if(!meta_group_hack_disabled) do {
		group_name = (char *)CallService(MS_CLIST_GROUPGETNAME, (WPARAM)hGroup, 0);
		if(group_name && !strcmp(group_name, META_HIDDEN_GROUP)) {
			// disabled because it shows a message box
			//CallService(MS_CLIST_GROUPDELETE, (WPARAM)hGroup, 0);
			MessageBox(0, Translate("The 'MetaContacts Hidden Group' has been added to your contact list.\n"
							"This is most likely due to server-side contact information. To fix this, so that\n"
							"MetaContacts continues to function correctly, you should:\n"
							"   - disable MetaContacts using the 'Toggle MetaContacts Off' main menu item\n"
							"   - move all contacts out of this group\n"
							"   - synchronize your contacts with the server\n"
							"   - re-enable MetaContacts"), Translate("MetaContacts Warning"), MB_ICONWARNING | MB_OK);
			break;
		}
		hGroup++;
	} while(group_name);
	

	while ( hContact != NULL ) {
		if((meta_id = DBGetContactSettingDword(hContact,META_PROTO,META_LINK,(DWORD)-1))!=(DWORD)-1) {
			// is a subcontact

			// * ensure filter present
			if( !CallService( MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, ( LPARAM )META_FILTER ))
				CallService( MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, ( LPARAM )META_FILTER );


			// get contact number
			contact_number = DBGetContactSettingDword(hContact, META_PROTO, "ContactNumber", (DWORD)-1);

			// prepare to update metacontact record of subcontat status
			proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

			// save old group and move to invisible group (i.e. non-existent group)
			Meta_SetGroup(hContact);

			// find metacontact
			hContact2 = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );

			while ( hContact2 != NULL ) {
				if(DBGetContactSettingDword(hContact2,META_PROTO,META_ID,(DWORD)-1) == meta_id) {
					num_contacts = DBGetContactSettingDword(hContact2, META_PROTO, "NumContacts", (DWORD)-1);
					if(contact_number >= 0 && contact_number < num_contacts) {

						if(!proto)
							status = ID_STATUS_OFFLINE;
						else
							status = DBGetContactSettingWord(hContact, proto, "Status", ID_STATUS_OFFLINE);
						
						// update metacontact's record of status for this contact
						strcpy(buffer, "Status");
						strcat(buffer, _itoa(contact_number, buffer2, 10));
						DBWriteContactSettingWord(hContact2, META_PROTO, buffer, status);

						// update metacontact's record of nick for this contact
						if(proto && !DBGetContactSetting(hContact, proto, "Nick", &dbv)) {
							strcpy(buffer, "Nick");
							strcat(buffer, _itoa(contact_number, buffer2, 10));
							MyDBWriteContactSetting(hContact2, META_PROTO, buffer, &dbv);
							
							strcpy(buffer, "CListName");
							strcat(buffer, _itoa(contact_number, buffer2, 10));
							if(DBGetContactSetting(hContact, "CList", "MyHandle", &dbv2)) {
								MyDBWriteContactSetting(hContact2, META_PROTO, buffer, &dbv);
							} else {
								MyDBWriteContactSetting(hContact2, META_PROTO, buffer, &dbv2);
								DBFreeVariant(&dbv2);
							}

							DBFreeVariant(&dbv);
						} else {
							if(!DBGetContactSetting(hContact, "CList", "MyHandle", &dbv)) {
								strcpy(buffer, "CListName");
								strcat(buffer, _itoa(contact_number, buffer2, 10));
								MyDBWriteContactSetting(hContact2, META_PROTO, buffer, &dbv);
								DBFreeVariant(&dbv);
							}
						}
					}
				}

				hContact2 = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact2, 0 );
			}

			if(options.suppress_status)
				CallService(MS_IGNORE_IGNORE, (WPARAM)hContact, (WPARAM)IGNOREEVENT_USERONLINE);
		}

		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	

	// do metacontacts after handles set properly above
	hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL ) {
		if(DBGetContactSettingDword(hContact,META_PROTO,META_ID,(DWORD)-1)!=(DWORD)-1) {
			// is a meta contact
			HANDLE hMostOnline = Meta_GetMostOnline(hContact); // set nick
			Meta_CopyContactNick(hContact, hMostOnline);

			Meta_FixStatus(hContact);

		}

		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	

	CallService(MS_CLUI_SORTLIST, 0, 0);
	return 0;
}

/** Unhide all contacts linked to any meta contact
*
*/
int Meta_UnhideLinkedContacts(void) {
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );

	while ( hContact != NULL ) {
		if(DBGetContactSettingDword(hContact,META_PROTO,META_LINK,(DWORD)-1)!=(DWORD)-1) {
			// has a link - unhide it
			// restore old group
			Meta_RestoreGroup(hContact);			
		}

		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	

	if(!CallService(MS_SYSTEM_TERMINATED, 0, 0))
		CallService(MS_CLUI_SORTLIST, 0, 0);
	return 0;
}

int Meta_HideMetaContacts(int hide) {
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );

	// set status suppression
	if(hide) Meta_SuppressStatus(FALSE);
	else Meta_SuppressStatus(options.suppress_status);

	while ( hContact != NULL ) {
		if(DBGetContactSettingDword(hContact,META_PROTO,META_ID,(DWORD)-1)!=(DWORD)-1) {
			// is a meta contact

			if(hide)
				DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
			else
				DBDeleteContactSetting(hContact, "CList", "Hidden");

		} else if(DBGetContactSettingDword(hContact,META_PROTO,META_LINK,(DWORD)-1)!=(DWORD)-1) {
			// when metacontacts are hidden, show subcontacts, and vice versa
			if(hide) {
				Meta_RestoreGroup(hContact);
			} else {
				Meta_SetGroup(hContact);
			}
		}

		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	

	if(!CallService(MS_SYSTEM_TERMINATED, 0, 0))
		CallService(MS_CLUI_SORTLIST, 0, 0);
	return 0;
}

void Meta_RestoreGroup(HANDLE hContact) {

	if(meta_group_hack_disabled) return; // clist has called api function to disable group hack - yay!

	// the existence of this service means that clist_meta_mw is active and will do the hiding for us
	if(ServiceExists(MS_CLUI_METASUPPORT)) return;

	// show it anyway - users are reporting contacts removed from meta remain 'hidden'
	// possible suspect - server side groups cause hidden group hack to fail, users hide contacts via clist->delete->hide option
	DBDeleteContactSetting(hContact, META_PROTO, "Hidden");

	if(DBGetContactSettingByte(hContact, META_PROTO, "Hidden", 0) == 1)
	{
		// if we hid it, unhide it
		DBDeleteContactSetting(hContact, META_PROTO, "Hidden");
		DBDeleteContactSetting(hContact, "CList", "Hidden");
	} else {
		DBCONTACTWRITESETTING cws;

		if(!DBGetContactSetting(hContact, META_PROTO, "OldCListGroup", &cws.value)) {

			if((cws.value.type == DBVT_ASCIIZ || cws.value.type == DBVT_UTF8) && !strcmp(cws.value.pszVal, META_HIDDEN_GROUP)) {
				DBDeleteContactSetting(hContact, "CList", "Group");
			} else {
				int hGroup = 1;
				char *name = 0;
				BOOL found = FALSE;
				do {
					name = (char *)CallService(MS_CLIST_GROUPGETNAME, (WPARAM)hGroup, 0);
					if(name && !strcmp(name, cws.value.pszVal)) {
						found = TRUE;
						break;
					}
					hGroup++;
				} while(name);

				if(found) {
					cws.szModule = "CList";
					cws.szSetting = "Group";
					CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws);
				} else {
					//DBDeleteContactSetting(hContact, "CList", "Group");
					// put back into metacontact's group
					DBVARIANT dbv;
					HANDLE hMeta = (HANDLE)DBGetContactSettingDword(hContact, META_PROTO, "Handle", 0);
					if(hMeta && !MyDBGetContactSetting(hMeta, "CList", "Group", &dbv)) {
						MyDBWriteContactSetting(hContact, "CList", "Group", &dbv);
						DBFreeVariant(&dbv);
					} else
						DBDeleteContactSetting(hContact, "CList", "Group");
				}
			}
			DBFreeVariant(&cws.value);
		}
		DBDeleteContactSetting(hContact, META_PROTO, "OldCListGroup");

		if(!DBGetContactSetting(hContact, "CList", "Group", &cws.value)) {
			if((cws.value.type == DBVT_ASCIIZ || cws.value.type == DBVT_UTF8) && !strcmp(cws.value.pszVal, META_HIDDEN_GROUP)) {
				DBDeleteContactSetting(hContact, "CList", "Group");
			}
			DBFreeVariant(&cws.value);
		}
	}

	// show it anyway - users are reporting contacts removed from meta remain 'hidden'
	// possible suspect - server side groups cause hidden group hack to fail, users hide contacts via clist->delete->hide option
	DBDeleteContactSetting(hContact, "CList", "Hidden");
}

void Meta_SetGroup(HANDLE hContact) {
	char *proto, *uid;

	if(meta_group_hack_disabled) return; // clist has called api function to disable group hack - yay!

	// the existence of this service means that clist_meta_mw is active and will do the hiding for us
	if(ServiceExists(MS_CLUI_METASUPPORT)) return;

	proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if (proto)
		uid   = (char *)CallProtoService(proto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);

	if(proto && uid && (INT_PTR)uid != CALLSERVICE_NOTFOUND && !strcmp(JABBER_UNIQUE_ID_SETTING, uid)) {
		// if it's a jabber contact, hide it, and record the fact that it was us who did
		DBWriteContactSettingByte(hContact, META_PROTO, "Hidden", 1);
		DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
	} else {
		DBCONTACTWRITESETTING cws;
		// save old group and move to invisible group (i.e. non-existent group)
		if(!MyDBGetContactSetting(hContact, "CList", "Group", &cws.value)) {
			if((cws.value.type == DBVT_ASCIIZ || cws.value.type == DBVT_UTF8) && !strcmp(cws.value.pszVal, META_HIDDEN_GROUP)) {
				// it's already in the group (shouldn't be - but maybe a crash)
			} else {
				cws.szModule = META_PROTO;
				cws.szSetting = "OldCListGroup";
				CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)hContact,(LPARAM)&cws);
			}
			DBFreeVariant(&cws.value);
		} else
			DBDeleteContactSetting(hContact, META_PROTO, "OldCListGroup");

		DBWriteContactSettingString(hContact, "CList", "Group", META_HIDDEN_GROUP);
	}
}

void Meta_GetStatusString(int status, char *buf, size_t size) {
	switch(status) {
		case ID_STATUS_OFFLINE:
			strncpy(buf, Translate("Offline"), size);
			break;
		case ID_STATUS_ONLINE:
			strncpy(buf, Translate("Online"), size);
			break;
		case ID_STATUS_AWAY:
			strncpy(buf, Translate("Away"), size);
			break;
		case ID_STATUS_DND:
			strncpy(buf, Translate("DND"), size);
			break;
		case ID_STATUS_NA:
			strncpy(buf, Translate("N/A"), size);
			break;
		case ID_STATUS_OCCUPIED:
			strncpy(buf, Translate("Occupied"), size);
			break;
		case ID_STATUS_FREECHAT:
			strncpy(buf, Translate("Free to Chat"), size);
			break;
		case ID_STATUS_INVISIBLE:
			strncpy(buf, Translate("Invisible"), size);
			break;
		case ID_STATUS_ONTHEPHONE:
			strncpy(buf, Translate("On the Phone"), size);
			break;
		case ID_STATUS_OUTTOLUNCH:
			strncpy(buf, Translate("Out to Lunch"), size);
			break;
		case ID_STATUS_IDLE:
			strncpy(buf, Translate("IDLE"), size);
			break;
		default:
			strncpy(buf, Translate("Unknown"), size);
			break;
	}
}

int Meta_SuppressStatus(BOOL suppress) {
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );

	while ( hContact != NULL ) {
		if(DBGetContactSettingDword(hContact,META_PROTO,META_LINK,(DWORD)-1)!=(DWORD)-1) {
			// is a subcontact
			if(suppress)
				CallService(MS_IGNORE_IGNORE, (WPARAM)hContact, (WPARAM)IGNOREEVENT_USERONLINE);
			else
				CallService(MS_IGNORE_UNIGNORE, (WPARAM)hContact, (WPARAM)IGNOREEVENT_USERONLINE);
		}

		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	

	return 0;
}

int Meta_CopyContactNick(HANDLE hMeta, HANDLE hContact) {
	DBVARIANT dbv, dbv_proto;
	char *proto;

	if(options.lockHandle) {
		hContact = Meta_GetContactHandle(hMeta, 0);
	}

	if(!hContact) return 1;

	//proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	// read proto direct from db, since we do this on load and other proto plugins may not be loaded yet
	if(!DBGetContactSetting(hContact, "Protocol", "p", &dbv_proto)) {

		proto = dbv_proto.pszVal;
		if(options.clist_contact_name == CNNT_NICK && proto) {
			if(!MyDBGetContactSetting(hContact, proto, "Nick", &dbv)) {
				MyDBWriteContactSetting(hMeta, META_PROTO, "Nick", &dbv);
				DBFreeVariant(&dbv);
				//CallService(MS_CLIST_INVALIDATEDISPLAYNAME, (WPARAM)hMeta, 0);
				//CallService(MS_CLUI_CONTACTRENAMED, (WPARAM)hMeta, 0);
				DBFreeVariant(&dbv_proto);
				return 0;
			}
		} else if(options.clist_contact_name == CNNT_DISPLAYNAME) {
			char *name = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
			wchar_t *wname = (wchar_t *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, GCDNF_UNICODE);
	
			if(name && strcmp(name, Translate("(Unknown Contact)")) != 0) {
				if(wname && strncmp(name, (char *)wname, strlen(name)) != 0) {
					DBVARIANT dbv;
					dbv.pwszVal = wname;
					dbv.type = DBVT_WCHAR;

					MyDBWriteContactSetting(hMeta, META_PROTO, "Nick", &dbv);
				} else
					DBWriteContactSettingString(hMeta, META_PROTO, "Nick", name);
				//CallService(MS_CLIST_INVALIDATEDISPLAYNAME, (WPARAM)hMeta, 0);
				//CallService(MS_CLUI_CONTACTRENAMED, (WPARAM)hMeta, 0);
				DBFreeVariant(&dbv_proto);
				return 0;
			}
		}
		DBFreeVariant(&dbv_proto);
	}
	return 1;
}

int Meta_SetAllNicks() {
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 ), most_online;

	while ( hContact != NULL ) {
		if(DBGetContactSettingDword(hContact,META_PROTO,META_ID,(DWORD)-1)!=(DWORD)-1) {
			most_online = Meta_GetMostOnline(hContact);
			Meta_CopyContactNick(hContact, most_online);
			Meta_FixStatus(hContact);
			Meta_CopyData(hContact);
		}
		
		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}
	return 0;
}

int Meta_IsHiddenGroup(const char *group_name) {
	if(group_name && !strcmp(group_name, META_HIDDEN_GROUP)) return 1;

	return 0;
}

int Meta_SwapContacts(HANDLE hMeta, DWORD contact_number1, DWORD contact_number2) {
	DBVARIANT dbv1, dbv2;

	HANDLE hContact1 = Meta_GetContactHandle(hMeta, contact_number1), 
		hContact2 = Meta_GetContactHandle(hMeta, contact_number2);
	char buff1[512], buff12[512], buff2[512], buff22[512];
	BOOL ok1, ok2;

	// swap the protocol
	strcpy(buff1, "Protocol");
	strcat(buff1, _itoa(contact_number1, buff12, 10));
	strcpy(buff2, "Protocol");
	strcat(buff2, _itoa(contact_number2, buff22, 10));
	ok1 = !MyDBGetContactSetting(hMeta, META_PROTO, buff1, &dbv1);
	ok2 = !MyDBGetContactSetting(hMeta, META_PROTO, buff2, &dbv2);
	if(ok1) {
		MyDBWriteContactSetting(hMeta, META_PROTO, buff2, &dbv1);
		DBFreeVariant(&dbv1);
	}
	if(ok2) {
		MyDBWriteContactSetting(hMeta, META_PROTO, buff1, &dbv2);
		DBFreeVariant(&dbv2);
	}

	// swap the status
	strcpy(buff1, "Status");
	strcat(buff1, _itoa(contact_number1, buff12, 10));
	strcpy(buff2, "Status");
	strcat(buff2, _itoa(contact_number2, buff22, 10));
	ok1 = !MyDBGetContactSetting(hMeta, META_PROTO, buff1, &dbv1);
	ok1 = !MyDBGetContactSetting(hMeta, META_PROTO, buff2, &dbv2);
	if(ok1) {
		MyDBWriteContactSetting(hMeta, META_PROTO, buff2, &dbv1);
		DBFreeVariant(&dbv1);
	}
	if(ok2) {
		MyDBWriteContactSetting(hMeta, META_PROTO, buff1, &dbv2);
		DBFreeVariant(&dbv2);
	}

	// swap the status string
	strcpy(buff1, "StatusString");
	strcat(buff1, _itoa(contact_number1, buff12, 10));
	strcpy(buff2, "StatusString");
	strcat(buff2, _itoa(contact_number2, buff22, 10));
	ok1 = !MyDBGetContactSetting(hMeta, META_PROTO, buff1, &dbv1);
	ok2 = !MyDBGetContactSetting(hMeta, META_PROTO, buff2, &dbv2);
	if(ok1) {
		MyDBWriteContactSetting(hMeta, META_PROTO, buff2, &dbv1);
		DBFreeVariant(&dbv1);
	}
	if(ok2) {
		MyDBWriteContactSetting(hMeta, META_PROTO, buff1, &dbv2);
		DBFreeVariant(&dbv2);
	}

	// swap the login
	strcpy(buff1, "Login");
	strcat(buff1, _itoa(contact_number1, buff12, 10));
	strcpy(buff2, "Login");
	strcat(buff2, _itoa(contact_number2, buff22, 10));
	ok1 = !MyDBGetContactSetting(hMeta, META_PROTO, buff1, &dbv1);
	ok2 = !MyDBGetContactSetting(hMeta, META_PROTO, buff2, &dbv2);
	if(ok1) {
		DBDeleteContactSetting(hMeta, META_PROTO, buff2);
		MyDBWriteContactSetting(hMeta, META_PROTO, buff2, &dbv1);
		DBFreeVariant(&dbv1);
	}
	if(ok2) {
		DBDeleteContactSetting(hMeta, META_PROTO, buff1);
		MyDBWriteContactSetting(hMeta, META_PROTO, buff1, &dbv2);
		DBFreeVariant(&dbv2);
	}

	// swap the nick
	strcpy(buff1, "Nick");
	strcat(buff1, _itoa(contact_number1, buff12, 10));
	strcpy(buff2, "Nick");
	strcat(buff2, _itoa(contact_number2, buff22, 10));
	ok1 = !MyDBGetContactSetting(hMeta, META_PROTO, buff1, &dbv1);
	ok2 = !MyDBGetContactSetting(hMeta, META_PROTO, buff2, &dbv2);
	if(ok1) {
		MyDBWriteContactSetting(hMeta, META_PROTO, buff2, &dbv1);
		DBFreeVariant(&dbv1);
	} else {
		DBDeleteContactSetting(hMeta, META_PROTO, buff2);
	}
	if(ok2) {
		MyDBWriteContactSetting(hMeta, META_PROTO, buff1, &dbv2);
		DBFreeVariant(&dbv2);
	} else {
		DBDeleteContactSetting(hMeta, META_PROTO, buff1);
	}

	// swap the clist name
	strcpy(buff1, "CListName");
	strcat(buff1, _itoa(contact_number1, buff12, 10));
	strcpy(buff2, "CListName");
	strcat(buff2, _itoa(contact_number2, buff22, 10));
	ok1 = !MyDBGetContactSetting(hMeta, META_PROTO, buff1, &dbv1);
	ok2 = !MyDBGetContactSetting(hMeta, META_PROTO, buff2, &dbv2);
	if(ok1) {
		MyDBWriteContactSetting(hMeta, META_PROTO, buff2, &dbv1);
		DBFreeVariant(&dbv1);
	} else {
		DBDeleteContactSetting(hMeta, META_PROTO, buff2);
	}
	if(ok2) {
		MyDBWriteContactSetting(hMeta, META_PROTO, buff1, &dbv2);
		DBFreeVariant(&dbv2);
	} else {
		DBDeleteContactSetting(hMeta, META_PROTO, buff1);
	}

	// swap the handle
	strcpy(buff1, "Handle");
	strcat(buff1, _itoa(contact_number1, buff12, 10));
	strcpy(buff2, "Handle");
	strcat(buff2, _itoa(contact_number2, buff22, 10));
	ok1 = !MyDBGetContactSetting(hMeta, META_PROTO, buff1, &dbv1);
	ok2 = !MyDBGetContactSetting(hMeta, META_PROTO, buff2, &dbv2);
	if(ok1) {
		MyDBWriteContactSetting(hMeta, META_PROTO, buff2, &dbv1);
		DBFreeVariant(&dbv1);
	} else {
		DBDeleteContactSetting(hMeta, META_PROTO, buff2);
	}
	if(ok2) {
		MyDBWriteContactSetting(hMeta, META_PROTO, buff1, &dbv2);
		DBFreeVariant(&dbv2);
	} else {
		DBDeleteContactSetting(hMeta, META_PROTO, buff1);
	}

	// finally, inform the contacts of their change in position
	DBWriteContactSettingDword(hContact1, META_PROTO, "ContactNumber", (DWORD)contact_number2);
	DBWriteContactSettingDword(hContact2, META_PROTO, "ContactNumber", (DWORD)contact_number1);

	return 0;
}

INT_PTR CALLBACK DlgProcNull(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		{
			HWND prog = GetDlgItem(hwndDlg, IDC_PROG);

			TranslateDialogDefault( hwndDlg );
			
			SendMessage(prog, PBM_SETPOS, 0, 0);
			return TRUE;
		}
	}
	return FALSE;
}

// function to copy history from one contact to another - courtesy JdGordon (thx)
void copyHistory(HANDLE hContactFrom,HANDLE hContactTo) 
{ 
	HANDLE hDbEvent; 
	DBEVENTINFO dbei; 
	//char *id;
	//DWORD id_length;
	//DWORD oldBlobSize;
	DWORD time_now = time(0);
	DWORD earliest_time = time_now - options.days_history * 24 * 60 * 60;
	BYTE *buffer = 0;
	HWND progress_dialog, prog;

	if (!hContactFrom || !hContactTo) return; 

	//id = Meta_GetUniqueIdentifier(hContactFrom, &id_length);
	//if(!id) return;

	progress_dialog = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_COPYPROGRESS), 0, DlgProcNull);
	ShowWindow(progress_dialog, SW_SHOW);

	prog = GetDlgItem(progress_dialog, IDC_PROG);

	//CallService(MS_DB_SETSAFETYMODE, (WPARAM)FALSE, 0);
	for (hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDFIRST,(WPARAM)hContactFrom, 0); 
		hDbEvent; 
		hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDNEXT,(WPARAM)hDbEvent,0)) 
	{ 
		// get the event 
		ZeroMemory(&dbei, sizeof(dbei)); 
		dbei.cbSize = sizeof(dbei); 

		if((dbei.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hDbEvent, 0)) == -1)
			break;

		buffer = mir_realloc(buffer, dbei.cbBlob);// + id_length);
		dbei.pBlob = buffer; 

		if (CallService(MS_DB_EVENT_GET,(WPARAM)hDbEvent,(LPARAM)&dbei)) 
			break; 

		// i.e. optoins.days_history == 0;
		if(time_now == earliest_time) earliest_time = dbei.timestamp;

		if(dbei.timestamp < earliest_time)
			continue;

		if(dbei.eventType != EVENTTYPE_MESSAGE && dbei.eventType != EVENTTYPE_FILE && dbei.eventType != EVENTTYPE_URL)
			continue;

		if(time_now > earliest_time) { // just in case!
			SendMessage(prog, PBM_SETPOS, (WPARAM)(int)(100.0 * (dbei.timestamp - earliest_time) / (time_now - earliest_time)), 0);	
			UpdateWindow(progress_dialog);
		}

		dbei.szModule = META_PROTO;
		//oldBlobSize = dbei.cbBlob;
		//dbei.cbBlob = oldBlobSize + id_length;
		dbei.flags &= ~DBEF_FIRST;
		//memcpy(dbei.pBlob + oldBlobSize, id, id_length);
		CallService(MS_DB_EVENT_ADD, (WPARAM)hContactTo, (LPARAM)&dbei); 
	}		
	//CallService(MS_DB_SETSAFETYMODE, (WPARAM)TRUE, 0);

	DestroyWindow(progress_dialog);
	if(buffer) mir_free(buffer);
	//mir_free(id);
}

/*
void Meta_RemoveHistory(HANDLE hContactRemoveFrom, HANDLE hContactSource) {
	HANDLE hDbEvent; 
	DBEVENTINFO dbei = { 0 }; 
	char *id;
	DWORD id_length;
	BYTE *buffer = 0;
	DWORD time_now = time(0), earliest_time = time_now;
	HWND progress_dialog, prog;

	if (!hContactRemoveFrom || !hContactSource) return; 

	id = Meta_GetUniqueIdentifier(hContactSource, &id_length);
	if(!id) return;

	progress_dialog = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DELPROGRESS), 0, DlgProcNull);
	ShowWindow(progress_dialog, SW_SHOW);

	prog = GetDlgItem(progress_dialog, IDC_PROG);

	//CallService(MS_DB_SETSAFETYMODE, (WPARAM)FALSE, 0);
	hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDFIRST,(WPARAM)hContactRemoveFrom, 0); 
	while (hDbEvent) 
	{
		// get the event 
		ZeroMemory(&dbei, sizeof(dbei)); 
		dbei.cbSize = sizeof(dbei); 

		if((dbei.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hDbEvent, 0)) == -1) {
			break;
		}

		buffer = mir_realloc(buffer, dbei.cbBlob + id_length);
		dbei.pBlob = buffer;

		if (CallService(MS_DB_EVENT_GET,(WPARAM)hDbEvent,(LPARAM)&dbei)) break; 

		if(dbei.timestamp < earliest_time)
			earliest_time = dbei.timestamp;

		if(dbei.eventType != EVENTTYPE_MESSAGE && dbei.eventType != EVENTTYPE_FILE && dbei.eventType != EVENTTYPE_URL) {
			hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDNEXT,(WPARAM)hDbEvent,0); 
			continue;
		}

		if(dbei.cbBlob >= id_length && strncmp((char *)(dbei.pBlob + dbei.cbBlob - id_length), id, id_length - 1) == 0) {
			// event is tagged with source contact id - remove
			HANDLE hDbEventNext;

			if(time_now > earliest_time) { // just in case
				SendMessage(prog, PBM_SETPOS, (WPARAM)(int)(100.0 * (dbei.timestamp - earliest_time) / (time_now - earliest_time)), 0);	
				UpdateWindow(progress_dialog);
			}

			hDbEventNext = (HANDLE)CallService(MS_DB_EVENT_FINDNEXT,(WPARAM)hDbEvent,0); 
			CallService(MS_DB_EVENT_DELETE, (WPARAM)hContactRemoveFrom, (LPARAM)hDbEvent);
			hDbEvent = hDbEventNext;
			continue;
		}
		
		hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDNEXT,(WPARAM)hDbEvent,0); 
	}
	//CallService(MS_DB_SETSAFETYMODE, (WPARAM)TRUE, 0);

	DestroyWindow(progress_dialog);
	if(buffer) mir_free(buffer);
	mir_free(id);
}

char *Meta_GetUniqueIdentifier(HANDLE hContact, DWORD *pused) {
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	char *field;
	char *id;
	char buff[256];
	DWORD used;
	DBVARIANT dbv;

	id = (char *)mir_alloc(256);

	if(!proto) return 0;
	
	strncpy(id, proto, 127);
	strcat(id, "*"); // seperate proto & proto id with asterisk to make one long comparable string

	used = strlen(id);

	field = (char *)CallProtoService(proto,PS_GETCAPS,PFLAG_UNIQUEIDSETTING,0);
	if(!field) return 0;

	DBGetContactSetting(hContact,proto,field,&dbv);
	switch(dbv.type)
	{
		case DBVT_ASCIIZ:
			sprintf(buff,"%s",dbv.pszVal);
			break;
		case DBVT_BYTE:
			sprintf(buff,"%d",dbv.bVal);
			break;
		case DBVT_WORD:
			sprintf(buff,"%d",dbv.wVal);
			break;
		case DBVT_DWORD:
			sprintf(buff,"%d",dbv.dVal);
			break;
		default:
			sprintf(buff,"bugger this");
	}
	DBFreeVariant(&dbv);

	strncpy(id + used, buff, 256 - used);
	if(used) {
		*pused = (DWORD)(used + strlen(buff) + 1);
		if(*pused > 256) *pused = 256;
	}

	return id;
}
*/
void Meta_FixStatus(HANDLE hMeta) {
	HANDLE most_online = Meta_GetMostOnlineSupporting(hMeta, PFLAGNUM_1, 0);
	if(most_online) {
		char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)most_online, 0);
		if(proto) {
			WORD status = (WORD)DBGetContactSettingWord(most_online, proto, "Status", (WORD)ID_STATUS_OFFLINE);
			DBWriteContactSettingWord(hMeta, META_PROTO, "Status", status);
		} else 
			DBWriteContactSettingWord(hMeta, META_PROTO, "Status", (WORD)ID_STATUS_OFFLINE);
	} else
		DBWriteContactSettingWord(hMeta, META_PROTO, "Status", (WORD)ID_STATUS_OFFLINE);

	// trigger setting changed in mw_clist to reset extra icons
	if(ServiceExists(MS_CLIST_EXTRA_SET_ICON)) {
		DBWriteContactSettingString(hMeta, "DummyModule", "e-mail", "blah");
		//DBDeleteContactSetting(hMeta, "DummyModule", "e-mail"); // causes 'empty settings group' error in dbtool
	}

}

INT_PTR Meta_IsEnabled() {
	return DBGetContactSettingByte(0, META_PROTO, "Enabled", 1) && (meta_group_hack_disabled || DBGetContactSettingByte(NULL, "CList", "UseGroups", 1));
}



