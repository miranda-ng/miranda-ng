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

INT_PTR Mydb_get(MCONTACT hContact, const char *szModule, const char *szSetting, DBVARIANT *dbv)
{
	memset(dbv, 0, sizeof(DBVARIANT));
	return db_get_s(hContact, szModule, szSetting, dbv, 0);
}

int Meta_EqualDBV(DBVARIANT *dbv, DBVARIANT *id) {
	DWORD res = 1;
	int i;
	if (dbv->type == id->type)
	{	// If the id parameter and the value returned by the db_get
		// are the same, this is the correct HANDLE, return it.
		switch(dbv->type)
		{
			case DBVT_DELETED:
				return 1;
			case DBVT_BYTE:
				if (dbv->bVal == id->bVal)
					return 1;
			case DBVT_WORD:
				if (dbv->wVal == id->wVal)
					return 1;
			case DBVT_DWORD:
				if (dbv->dVal == id->dVal)
					return 1;
				break;
			case DBVT_ASCIIZ:
			case DBVT_UTF8:
				if ( !strcmp(dbv->pszVal,id->pszVal))
					return 1;
			case DBVT_WCHAR:
				if ( !wcscmp(dbv->pwszVal,id->pwszVal))
					return 1;
			case DBVT_BLOB:
				if (dbv->cpbVal == id->cpbVal)
				{
					for (i=dbv->cpbVal;res && i<=0;i--)
						res = (dbv->pbVal[i] == id->pbVal[i]);
					if (res)
					{
						return 1;
					}
				}
				break;
		} // end switch
	} // end if (dbv.type == id.type)

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
MCONTACT Meta_GetHandle(const char *protocol, DBVARIANT *id)
{
	char *field;
	DBVARIANT dbv;
	DWORD i,res = 1;

	// Get the field identifying the contact in the database.
	if ( !ProtoServiceExists(protocol, PS_GETCAPS))
		return NULL;
	
	field = (char *)CallProtoService(protocol,PS_GETCAPS,PFLAG_UNIQUEIDSETTING,0);

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		// Scan the database and retrieve the field for each contact
		if ( !db_get(hContact, protocol, field, &dbv)) {
			if (dbv.type == id->type) {
				// If the id parameter and the value returned by the db_get
				// are the same, this is the correct HANDLE, return it.
				switch(dbv.type) {
				case DBVT_DELETED:
					break;
				case DBVT_BYTE:
					if (dbv.bVal == id->bVal)
						return hContact;
					break;
				case DBVT_WORD:
					if (dbv.wVal == id->wVal)
						return hContact;
					break;
				case DBVT_DWORD:
					if (dbv.dVal == id->dVal)
						return hContact;
					break;
				case DBVT_ASCIIZ:
				case DBVT_UTF8:
					if ( !strcmp(dbv.pszVal,id->pszVal)) {
						db_free(&dbv);
						return hContact;
					}
					db_free(&dbv);
					break;

				case DBVT_WCHAR:
					if ( !wcscmp(dbv.pwszVal,id->pwszVal)) {
						db_free(&dbv);
						return hContact;
					}
					db_free(&dbv);
					break;

				case DBVT_BLOB:
					if (dbv.cpbVal == id->cpbVal) {
						for (i=dbv.cpbVal;res && i<=0;i--)
							res = (dbv.pbVal[i] == id->pbVal[i]);
						if (res) {
							db_free(&dbv);
							return hContact;
						}
					}
					db_free(&dbv);
					break;
				} // end switch
			}
			else db_free(&dbv);
		}
	}
	return NULL;
}

/** Update the MetaContact login, depending on the protocol desired
*
* The login of the "MetaContacts" protocol will be copied from the login
* of the specified protocol.
*
* @param szProto :	The name of the protocol used to get the login that will be
*					affected to the "MetaContacts" protocol.
*
* @return			O on success, 1 otherwise.
*/
int Meta_SetNick(char *szProto)
{
	CONTACTINFO ci;
	ci.cbSize = sizeof(CONTACTINFO);
	ci.dwFlag = CNF_DISPLAY | CNF_TCHAR;
	ci.hContact = NULL;
	ci.szProto = szProto;
	if (CallService(MS_CONTACT_GETCONTACTINFO,0,(LPARAM)&ci))
		return 1;

	switch(ci.type) {
	case CNFT_BYTE:
		if ( db_set_b(NULL, META_PROTO, "Nick", ci.bVal))
			return 1;
		break;
	case CNFT_WORD:
		if ( db_set_w(NULL, META_PROTO, "Nick", ci.wVal))
			return 1;
		break;
	case CNFT_DWORD:
		if ( db_set_dw(NULL, META_PROTO, "Nick", ci.dVal))
			return 1;
		break;
	case CNFT_ASCIIZ:
		if ( db_set_ts(NULL, META_PROTO, "Nick", ci.pszVal))
			return 1;
		mir_free(ci.pszVal);
		break;
	default:
		if ( db_set_s(NULL, META_PROTO, "Nick",(char *)TranslateT("Sender")))
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
BOOL Meta_Assign(MCONTACT src, MCONTACT dest, BOOL set_as_default)
{
	DWORD metaID, num_contacts;
	char buffer[512], szId[40];
	WORD status;
	MCONTACT most_online;
		
	if ((metaID = db_get_dw(dest, META_PROTO, META_ID,(DWORD)-1))==-1) {
		MessageBox(0, TranslateT("Could not get MetaContact ID"), TranslateT("Assignment error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	if ((num_contacts = db_get_dw(dest, META_PROTO, "NumContacts",(DWORD)-1))==-1) {
		MessageBox(0, TranslateT("Could not retreive MetaContact contact count"), TranslateT("Assignment error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	char *szProto = GetContactProto(src);
	if (szProto == NULL) {
		MessageBox(0, TranslateT("Could not retreive contact protocol"), TranslateT("Assignment error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	// Get the login of the subcontact
	char *field = (char *)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
	DBVARIANT dbv;
	if ( db_get(src,szProto, field, &dbv)) {
		MessageBox(0, TranslateT("Could not get unique ID of contact"), TranslateT("Assignment error"), MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	// Check that is is 'on the list'
	if ( db_get_b(src, "CList", "NotOnList", 0) == 1) {
		MessageBox(0, TranslateT("Contact is 'Not on List' - please add the contact to your contact list before assigning."), TranslateT("Assignment error"), MB_OK | MB_ICONWARNING);
		db_free(&dbv);
		return FALSE;
	}

	num_contacts++;
	if (num_contacts >= MAX_CONTACTS) {
		MessageBox(0, TranslateT("MetaContact is full"), TranslateT("Assignment error"), MB_OK | MB_ICONWARNING);
		db_free(&dbv);
		return FALSE;
	}

	// write the contact's protocol
	strcpy(buffer, "Protocol");
	strcat(buffer, _itoa(num_contacts-1, szId, 10));

	if ( db_set_s(dest, META_PROTO, buffer, szProto)) {
		MessageBox(0, TranslateT("Could not write contact protocol to MetaContact"), TranslateT("Assignment error"), MB_OK | MB_ICONWARNING);
		db_free(&dbv);
		return FALSE;
	}

	// write the login
	strcpy(buffer, "Login");
	strcat(buffer, szId);

	if ( db_set(dest, META_PROTO, buffer, &dbv)) {
		MessageBox(0, TranslateT("Could not write unique ID of contact to MetaContact"), TranslateT("Assignment error"), MB_OK | MB_ICONWARNING);
		db_free(&dbv);
		return FALSE;
	}

	db_free(&dbv);

	// If we can get the nickname of the subcontact...
	if ( !db_get(src, szProto, "Nick", &dbv)) {
		// write the nickname
		strcpy(buffer, "Nick");
		strcat(buffer, szId);
		if (db_set(dest, META_PROTO, buffer, &dbv)) {
			MessageBox(0, TranslateT("Could not write nickname of contact to MetaContact"), TranslateT("Assignment error"), MB_OK | MB_ICONWARNING);
			return FALSE;
		}

		db_free(&dbv);
	}

	// write the display name
	strcpy(buffer, "CListName");
	strcat(buffer, szId);
	db_set_ts(dest, META_PROTO, buffer, pcli->pfnGetContactDisplayName(src, GCDNF_TCHAR));

	// Get the status
	if ( !szProto)
		status = ID_STATUS_OFFLINE;
	else
		status = db_get_w(src, szProto, "Status", ID_STATUS_OFFLINE);

	// write the status
	strcpy(buffer, "Status");
	strcat(buffer, szId);
	db_set_w(dest, META_PROTO, buffer, status);

	// write the handle
	strcpy(buffer, "Handle");
	strcat(buffer, szId);
	db_set_dw(dest, META_PROTO, buffer, (DWORD)src);

	// write status string
	strcpy(buffer, "StatusString");
	strcat(buffer, szId);

	TCHAR *szStatus = pcli->pfnGetStatusModeDescription(status, 0);
	db_set_ts(dest, META_PROTO, buffer, szStatus);

	// Write the link in the contact
	db_set_dw(src, META_PROTO, META_LINK, metaID);

	// Write the contact number in the contact
	db_set_dw(src, META_PROTO, "ContactNumber", num_contacts-1);

	// Write the handle in the contact
	db_set_dw(src, META_PROTO, "Handle", (DWORD)dest);

	// update count of contacts
	db_set_dw(dest, META_PROTO, "NumContacts", num_contacts);

	if (set_as_default) {
		db_set_dw(dest, META_PROTO, "Default", (WORD)(num_contacts - 1));
		NotifyEventHooks(hEventDefaultChanged, (WPARAM)dest, (LPARAM)src);
	}

	db_set_b(src, META_PROTO, "IsSubcontact", 1);
	// set nick to most online contact that can message
	most_online = Meta_GetMostOnline(dest);
	Meta_CopyContactNick(dest, most_online);

	// set status to that of most online contact
	Meta_FixStatus(dest);

	// if the new contact is the most online contact with avatar support, get avatar info
	most_online = Meta_GetMostOnlineSupporting(dest, PFLAGNUM_4, PF4_AVATARS);
	if (most_online == src) {
		PROTO_AVATAR_INFORMATIONT AI;
		AI.cbSize = sizeof(AI);
		AI.hContact = dest;
		AI.format = PA_FORMAT_UNKNOWN;
		_tcscpy(AI.filename, _T("X"));

		if ( CallProtoService(META_PROTO, PS_GETAVATARINFOT, 0, (LPARAM)&AI) == GAIR_SUCCESS)
	        db_set_ts(dest, "ContactPhoto", "File",AI.filename);
	}

	// Hide the contact
	Meta_SetGroup(src);

	// copy history
	if (options.copy_subcontact_history)
		copyHistory(src, dest);

	// Ignore status if the option is on
	if (options.suppress_status)
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
MCONTACT Meta_GetMostOnline(MCONTACT hMeta) {
	return Meta_GetMostOnlineSupporting(hMeta, PFLAGNUM_1, PF1_IM);
}
/** Get the 'most online' contact for a meta contact (according to above order) which supports the specified
* protocol service, and copies nick from that contact
*
* @param hMeta	\c HANDLE to a metacontact
*
* @return		\c HANDLE to a contact
*/

MCONTACT Meta_GetMostOnlineSupporting(MCONTACT hMeta, int pflagnum, unsigned long capability) {
	int most_online_status = ID_STATUS_OFFLINE;
	MCONTACT most_online_contact = NULL, hContact;
	int i, status, default_contact_number, num_contacts;
	unsigned long caps = 0;
	char *szProto, *most_online_proto;

	// you can't get more online than having the default contact ONLINE - so check that first
	if ((default_contact_number = db_get_dw(hMeta, META_PROTO, "Default",(DWORD)-1)) == (DWORD)-1)
	{
		// This is a simple contact - return NULL to signify error.
		// (this should normally not happen, since all meta contacts have a default contact)
		return NULL;
	}

	// if the default is beyond the end of the list (eek!) return null
	if (default_contact_number >= (num_contacts = db_get_dw(hMeta, META_PROTO, "NumContacts", 0)))
		return NULL;

	most_online_contact = Meta_GetContactHandle(hMeta, default_contact_number);
	szProto = GetContactProto(most_online_contact);
	caps = szProto ? CallProtoService(szProto, PS_GETCAPS, (WPARAM)pflagnum, 0) : 0;
	if (szProto && strcmp(szProto, "IRC") == 0) caps |= PF1_IM;
	// we are forced to do use default for sending - '-1' capability indicates no specific capability, but respect 'Force Default'
	if (szProto && db_get_b(hMeta, META_PROTO, "ForceDefault", 0) && capability != 0 && (capability == -1 || (caps & capability) == capability)) // capability is 0 when we're working out status
		return most_online_contact;

	// a subcontact is being temporarily 'forced' to do sending
	if ((most_online_contact = (MCONTACT)db_get_dw(hMeta, META_PROTO, "ForceSend", 0))) {
		caps = szProto ? CallProtoService(szProto, PS_GETCAPS, (WPARAM)pflagnum, 0) : 0;
		if (szProto && strcmp(szProto, "IRC") == 0) caps |= PF1_IM;
		if (szProto && (caps & capability) == capability && capability != 0) // capability is 0 when we're working out status
			return most_online_contact;
	}

	most_online_contact = Meta_GetContactHandle(hMeta, default_contact_number);
	szProto = GetContactProto(most_online_contact);
	if (szProto && CallProtoService(szProto, PS_GETSTATUS, 0, 0) >= ID_STATUS_ONLINE) {
		caps = szProto ? CallProtoService(szProto, PS_GETCAPS, (WPARAM)pflagnum, 0) : 0;
		if (szProto && strcmp(szProto, "IRC") == 0) caps |= PF1_IM;
		if (szProto && (capability == -1 || (caps & capability) == capability)) {
			most_online_status = db_get_w(most_online_contact, szProto, "Status", ID_STATUS_OFFLINE);

			// if our default is not offline, and option to use default is set - return default
			// and also if our default is online, return it
			if (most_online_status == ID_STATUS_ONLINE || (most_online_status != ID_STATUS_OFFLINE && options.always_use_default)) {
				return most_online_contact;
			}
		} else
			most_online_status = ID_STATUS_OFFLINE;
	} else
		most_online_status = ID_STATUS_OFFLINE;

	most_online_proto = szProto;
	// otherwise, check all the subcontacts for the one closest to the ONLINE state which supports the required capability
	for (i = 0; i < num_contacts; i++) {
		if (i == default_contact_number) // already checked that (i.e. initial value of most_online_contact and most_online_status are those of the default contact)
			continue;

		hContact = Meta_GetContactHandle(hMeta, i);
		szProto = GetContactProto(hContact);

		if ( !szProto || CallProtoService(szProto, PS_GETSTATUS, 0, 0) < ID_STATUS_ONLINE) // szProto offline or connecting
			continue;

		caps = szProto ? CallProtoService(szProto, PS_GETCAPS, (WPARAM)pflagnum, 0) : 0;
		if (szProto && strcmp(szProto, "IRC") == 0) caps |= PF1_IM;
		if (szProto && (capability == -1 || (caps & capability) == capability)) {

			status = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);

			if (status == ID_STATUS_ONLINE) {
				most_online_contact = hContact;
				most_online_proto = szProto;
				return most_online_contact;
			} else if (status <= ID_STATUS_OFFLINE) // status below ID_STATUS_OFFLINE is a connecting status
				continue;
			else {
				if (GetRealPriority(szProto, status) < GetRealPriority(most_online_proto, most_online_status)) {
					most_online_status = status;
					most_online_contact = hContact;
					most_online_proto = szProto;
				}
			}
		}
	}

	// no online contacts? if we're trying to message, use 'send offline' contact
	if (most_online_status == ID_STATUS_OFFLINE && capability == PF1_IM) {
		MCONTACT hOffline = Meta_GetContactHandle(hMeta, db_get_dw(hMeta, META_PROTO, "OfflineSend", (DWORD)-1));
		if (hOffline)
			most_online_contact = hOffline;
	}

	return most_online_contact;
}

int Meta_GetContactNumber(MCONTACT hContact) {
	return db_get_dw(hContact, META_PROTO, "ContactNumber", -1);
}

BOOL dbv_same(DBVARIANT *dbv1, DBVARIANT *dbv2) {
	if (dbv1->type != dbv2->type) return FALSE;

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

 void copy_settings_array(MCONTACT hMeta, char *module, const char *settings[], int num_settings) {
	int num_contacts = db_get_dw(hMeta, META_PROTO, "NumContacts", (DWORD)-1),
		default_contact = db_get_dw(hMeta, META_PROTO, "Default", (DWORD)-1),
		most_online = Meta_GetContactNumber(Meta_GetMostOnline(hMeta));

	MCONTACT hContact;
	int i, j;
	char *used_mod;
	
	DBVARIANT dbv1, dbv2;
	BOOL free, got_val, bDataWritten;

	BOOL use_default = FALSE;
	int source_contact = (use_default ? default_contact : most_online);

	if (source_contact < 0 || source_contact >= num_contacts) return;

	for (i = 0; i < num_settings; i++) {
		bDataWritten = FALSE;
		for (j = 0; j < num_contacts && !bDataWritten; j++) {
			// do source (most online) first
			if (j == 0) 
				hContact = Meta_GetContactHandle(hMeta, source_contact);
			else if (j <= source_contact)
				hContact = Meta_GetContactHandle(hMeta, j - 1);
			else
				hContact = Meta_GetContactHandle(hMeta, j);

			if (hContact) {
				if ( !module) {
					used_mod = GetContactProto(hContact);
					if ( !used_mod)
						continue; // next contact
				}
				else used_mod = module;

				if (j == 0 && strcmp(settings[i], "MirVer") == 0) //Always reset MirVer
					db_unset(hMeta, (module ? used_mod : META_PROTO), settings[i]);
					
				got_val = !Mydb_get(hContact, used_mod, settings[i], &dbv2);
				if (got_val) {
					free = !Mydb_get(hMeta, (module ? used_mod : META_PROTO), settings[i], &dbv1);
					
					if (strcmp(settings[i], "MirVer") == 0) {
						if (db_get_w(hContact, used_mod, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE) {
							if ( !free || (dbv1.pszVal == NULL || strcmp(dbv1.pszVal, "") == 0 || strlen(dbv1.pszVal) < 2)) {
								db_set(hMeta, (module ? used_mod : META_PROTO), settings[i], &dbv2);
								bDataWritten = TRUE; //only break if found something to copy
							}
						}
					}
					else {
						if ( !free || !dbv_same(&dbv1, &dbv2)) {
							db_set(hMeta, (module ? used_mod : META_PROTO), settings[i], &dbv2);
							if (dbv2.type == DBVT_ASCIIZ || dbv2.type == DBVT_UTF8) {
								if (dbv2.pszVal != NULL && strcmp(dbv2.pszVal, "") != 0)
									bDataWritten = TRUE; //only break if found something to copy
							} else if (dbv2.type == DBVT_WCHAR) {
								if (dbv2.pwszVal != 0 && wcscmp(dbv2.pwszVal, L"") != 0)
									bDataWritten = TRUE; //only break if found something to copy
							} else
								bDataWritten = TRUE; //only break if found something to copy
						}
						else
							bDataWritten = TRUE;

					}		
					db_free(&dbv2);
					if (free)db_free(&dbv1);
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
// szProto: 
// clist: "StatusMsg"

void CopyStatusData(MCONTACT hMeta)
{
	int num_contacts = db_get_dw(hMeta, META_PROTO, "NumContacts", (DWORD)-1),
	    most_online = Meta_GetContactNumber(Meta_GetMostOnline(hMeta));
	WORD status = db_get_w(hMeta, META_PROTO, "Status", ID_STATUS_OFFLINE);
	MCONTACT hContact;
	BOOL bDoneStatus = FALSE, bDoneXStatus = FALSE;

	for (int i = 0; i < num_contacts; i++) {
		if (i == 0) 
			hContact = Meta_GetContactHandle(hMeta, most_online);
		else if (i <= most_online)
			hContact = Meta_GetContactHandle(hMeta, i - 1);
		else
			hContact = Meta_GetContactHandle(hMeta, i);

		char *szProto = GetContactProto(hContact);

		if (szProto && db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE) == status) {
			DBVARIANT dbv;
			if ( !bDoneStatus && !Mydb_get(hContact, "CList", "StatusMsg", &dbv)) {
				db_set(hMeta, "CList", "StatusMsg", &dbv);
				db_free(&dbv);
				bDoneStatus = TRUE;
			}
			if ((!bDoneXStatus) && (!Mydb_get(hContact, szProto, "XStatusId", &dbv)) && dbv.type != DBVT_DELETED) {
				db_set_s(hMeta, META_PROTO, "XStatusProto", szProto);
				db_set(hMeta, META_PROTO, "XStatusId", &dbv);

				db_free(&dbv);
				if ( !Mydb_get(hContact, szProto, "XStatusMsg", &dbv)) {
					db_set(hMeta, META_PROTO, "XStatusMsg", &dbv);
					db_free(&dbv);
				}
				if ( !Mydb_get(hContact, szProto, "XStatusName", &dbv)) {
					db_set(hMeta, META_PROTO, "XStatusName", &dbv);
					db_free(&dbv);
				}
				bDoneXStatus = TRUE;
			}
		}

		if (bDoneStatus && bDoneXStatus)
			break;
	}

	if ( !bDoneStatus) db_unset(hMeta, "CList", "StatusMsg");
	if ( !bDoneXStatus) {
		db_unset(hMeta, META_PROTO, "XStatusId");
		db_unset(hMeta, META_PROTO, "XStatusMsg");
		db_unset(hMeta, META_PROTO, "XStatusName");
	}
}

void Meta_CopyData(MCONTACT hMeta)
{
	if (options.copydata) {
		CopyStatusData(hMeta);

		copy_settings_array(hMeta, 0, ProtoSettings, 25);
		copy_settings_array(hMeta, "mBirthday", UserInfoSettings, 3);
		copy_settings_array(hMeta, "ContactPhoto", ContactPhotoSettings, 5);

		if (options.copy_userinfo) 
			copy_settings_array(hMeta, "UserInfo", UserInfoSettings, 71);
	}
}


MCONTACT Meta_GetContactHandle(MCONTACT hMeta, int contact_number)
{
	char buffer[512], buffer2[512];
	int num_contacts = db_get_dw(hMeta, META_PROTO, "NumContacts", 0);

	if (contact_number >= num_contacts || contact_number < 0) return 0;
	
	strcpy(buffer, "Handle");
	strcat(buffer, _itoa(contact_number, buffer2, 10));
	return (MCONTACT)db_get_dw(hMeta, META_PROTO, buffer, 0);
}

int Meta_SetHandles(void)
{
	DWORD meta_id, num_contacts, contact_number;
	MCONTACT hContact2, hContact = db_find_first(), hNextContact;
	char buffer[512];
	TCHAR nick_buffer[128], buffer2[40];
	BOOL found;

	while ( hContact != NULL ) {
		if ((meta_id = db_get_dw(hContact, META_PROTO, META_LINK,(DWORD)-1))!=(DWORD)-1) {
			// is a subcontact

			// get nick for debug messages
			_tcscpy(nick_buffer, _T("meta_id: "));
			_tcscat(nick_buffer, _itot(meta_id, buffer2, 10));

			contact_number = db_get_dw(hContact, META_PROTO, "ContactNumber", (DWORD)-1);
			_tcscat(nick_buffer, _T(", contact num: "));
			_tcscat(nick_buffer, _itot(contact_number, buffer2, 10));

			if (contact_number < 0) {
				// problem!
				MessageBox(0, TranslateT("Subcontact contact number < 0 - deleting MetaContact"), nick_buffer, MB_OK | MB_ICONERROR);
				//CallService(MS_DB_CONTACT_DELETE, (WPARMA)hContact, 0);
				hNextContact = db_find_next(hContact);
				Meta_Delete(hContact, (LPARAM)1);
				hContact = hNextContact;
				continue;		
				//return 1;
			}

			// ensure the window open flag is not present
			db_unset(hContact, META_PROTO, "WindowOpen");

			// find metacontact
			found = FALSE;
			hContact2 = db_find_first();

			while ( hContact2 != NULL ) {
				if (db_get_dw(hContact2, META_PROTO, META_ID,(DWORD)-1) == meta_id) {
					found = TRUE;

					// set handle
					db_set_dw(hContact, META_PROTO, "Handle", (DWORD)hContact2);

					// increment contact counter (cleared in Load function)
					db_set_b(hContact2, META_PROTO, "ContactCountCheck", 
						(unsigned char)(db_get_b(hContact2, META_PROTO, "ContactCountCheck", 0) + 1));

					num_contacts = db_get_dw(hContact2, META_PROTO, "NumContacts", (DWORD)-1);
					if (contact_number >= 0 && contact_number < num_contacts) {
						// set metacontact's handle to us
						char szId[40];
						strcpy(buffer, "Handle");
						strcat(buffer, _itoa((int)contact_number, szId, 10));
						db_set_dw(hContact2, META_PROTO, buffer, (DWORD)hContact);
					}
					else {
						TCHAR buff[256];
						// problem - contact number is greater than meta's number of contacts
						mir_sntprintf(buff, SIZEOF(buff), TranslateT("Subcontact contact number (%d) > meta num contacts (%d) - deleting MetaContact"), contact_number, num_contacts);
						MessageBox(0, buff, nick_buffer, MB_OK | MB_ICONERROR);

						hNextContact = db_find_next(hContact);
						Meta_Delete(hContact, (LPARAM)1);
						hContact = hNextContact;
						continue;		
					}
				}

				hContact2 = db_find_next(hContact2);
			}

			if ( !found) {
				// problem - subcontact's meta not found
				MessageBox(0, TranslateT("Subcontact's MetaContact not found - deleting MetaContact data"), nick_buffer, MB_OK | MB_ICONERROR);

				// delete meta data
				db_unset(hContact, META_PROTO, "IsSubcontact");
				db_unset(hContact, META_PROTO, META_LINK);
				db_unset(hContact, META_PROTO, "Handle");
				db_unset(hContact, META_PROTO, "ContactNumber");
				Meta_RestoreGroup(hContact);
				db_unset(hContact, META_PROTO, "OldCListGroup");

				// stop ignoring, if we were
				if (options.suppress_status)
					CallService(MS_IGNORE_UNIGNORE, hContact, (WPARAM)IGNOREEVENT_USERONLINE);

			}
			else {
				if ( !db_get_b(hContact, META_PROTO, "IsSubcontact", 0))
					db_set_b(hContact, META_PROTO, "IsSubcontact", 1);
			}
		}
		else db_unset(hContact, META_PROTO, "Handle");

		if ((meta_id = db_get_dw(hContact, META_PROTO, META_ID,(DWORD)-1))!=(DWORD)-1) {
			// is a metacontact

			// get nick for debug messages
			_tcscpy(nick_buffer, _T("meta_id: "));
			_tcscat(nick_buffer, _itot(meta_id, buffer2, 10));

			// ensure the window open flag is not present
			db_unset(hContact, META_PROTO, "WindowOpen");

			// ensure default is reasonable
			contact_number = db_get_dw(hContact, META_PROTO, "Default", -1);
			num_contacts = db_get_dw(hContact, META_PROTO, "NumContacts", (DWORD)-1);

			if (num_contacts < 0) {
				// problem
				MessageBox(0, TranslateT("MetaContact number of contacts < 0 - deleting MetaContact"), nick_buffer, MB_OK | MB_ICONERROR);
				hNextContact = db_find_next(hContact);
				Meta_Delete(hContact, (LPARAM)1);
				hContact = hNextContact;
				continue;		
			}

			if (contact_number < 0 || contact_number >= num_contacts) {
				// problem
				MessageBox(0, TranslateT("MetaContact default contact number out of range - deleting MetaContact"), nick_buffer, MB_OK | MB_ICONERROR);
				hNextContact = db_find_next(hContact);
				Meta_Delete(hContact, (LPARAM)1);
				hContact = hNextContact;
				continue;		
			}
		}

		hContact = db_find_next(hContact);
	}

	// loop through one more time - check contact counts match
	hContact = db_find_first();
	while ( hContact != NULL ) {
		if ((meta_id = db_get_dw(hContact, META_PROTO, META_ID,(DWORD)-1))!=(DWORD)-1) {
			// get nick for debug messages

			num_contacts = db_get_b(hContact, META_PROTO, "ContactCountCheck", -2);
			if (num_contacts != (DWORD)db_get_dw(hContact, META_PROTO, "NumContacts", (DWORD)-1)) {
				hNextContact = db_find_next(hContact);

				_tcscpy(nick_buffer, TranslateT("Meta ID: "));
				_tcscat(nick_buffer, _itot(meta_id, buffer2, 10));
				MessageBox(0, TranslateT("MetaContact corrupted - the number of subcontacts is incorrect.\nDeleting MetaContact."), nick_buffer, MB_OK | MB_ICONERROR);
				
				Meta_Delete(hContact, (LPARAM)1);
				hContact = hNextContact;
				continue;		
			}
		}
		hContact = db_find_next(hContact);
	}

	return 0;
}

/** Hide all contacts linked to any meta contact, and set handle links
*
* Additionally, set all sub contacts and metacontacts to offline so that status notifications are always sent
*
* and ensure metafilter in place
*/

#define szMsg LPGEN("The 'MetaContacts Hidden Group' has been added to your contact list.\n\
This is most likely due to server-side contact information. To fix this, so that\n\
MetaContacts continues to function correctly, you should:\n\
   - disable MetaContacts using the 'Toggle MetaContacts Off' main menu item\n\
   - move all contacts out of this group\n\
   - synchronize your contacts with the server\n\
   - re-enable MetaContacts")

int Meta_HideLinkedContacts(void) {
	DBVARIANT dbv, dbv2;
	DWORD meta_id, num_contacts, contact_number;
	WORD status;
	char buffer[512], buffer2[512];
	char *szProto, *group_name;
	int hGroup = 1;

	MCONTACT hContact2, hContact = db_find_first();

	// ensure the hidden group does not exist (how this occurs i wonder but there have been reports!)
	// (sometimes protocol server side groups are to blame - msn and icq)
	if ( !meta_group_hack_disabled) do {
		group_name = (char *)CallService(MS_CLIST_GROUPGETNAME, (WPARAM)hGroup, 0);
		if (group_name && !strcmp(group_name, META_HIDDEN_GROUP)) {
			// disabled because it shows a message box
			//CallService(MS_CLIST_GROUPDELETE, (WPARAM)hGroup, 0);
			MessageBox(0, TranslateT(szMsg), TranslateT("MetaContacts Warning"), MB_ICONWARNING | MB_OK);
			break;
		}
		hGroup++;
	} while(group_name);
	
	while (hContact != NULL) {
		if ((meta_id = db_get_dw(hContact, META_PROTO, META_LINK,(DWORD)-1))!=(DWORD)-1) {
			// get contact number
			contact_number = db_get_dw(hContact, META_PROTO, "ContactNumber", (DWORD)-1);

			// prepare to update metacontact record of subcontat status
			szProto = GetContactProto(hContact);

			// save old group and move to invisible group (i.e. non-existent group)
			Meta_SetGroup(hContact);

			// find metacontact
			hContact2 = db_find_first();

			while ( hContact2 != NULL ) {
				if (db_get_dw(hContact2, META_PROTO, META_ID,(DWORD)-1) == meta_id) {
					num_contacts = db_get_dw(hContact2, META_PROTO, "NumContacts", (DWORD)-1);
					if (contact_number >= 0 && contact_number < num_contacts) {

						if ( !szProto)
							status = ID_STATUS_OFFLINE;
						else
							status = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
						
						// update metacontact's record of status for this contact
						strcpy(buffer, "Status");
						strcat(buffer, _itoa(contact_number, buffer2, 10));
						db_set_w(hContact2, META_PROTO, buffer, status);

						// update metacontact's record of nick for this contact
						if (szProto && !db_get(hContact, szProto, "Nick", &dbv)) {
							strcpy(buffer, "Nick");
							strcat(buffer, _itoa(contact_number, buffer2, 10));
							db_set(hContact2, META_PROTO, buffer, &dbv);
							
							strcpy(buffer, "CListName");
							strcat(buffer, _itoa(contact_number, buffer2, 10));
							if (db_get(hContact, "CList", "MyHandle", &dbv2)) {
								db_set(hContact2, META_PROTO, buffer, &dbv);
							} else {
								db_set(hContact2, META_PROTO, buffer, &dbv2);
								db_free(&dbv2);
							}

							db_free(&dbv);
						} else {
							if ( !db_get(hContact, "CList", "MyHandle", &dbv)) {
								strcpy(buffer, "CListName");
								strcat(buffer, _itoa(contact_number, buffer2, 10));
								db_set(hContact2, META_PROTO, buffer, &dbv);
								db_free(&dbv);
							}
						}
					}
				}

				hContact2 = db_find_next(hContact2);
			}

			if (options.suppress_status)
				CallService(MS_IGNORE_IGNORE, hContact, (WPARAM)IGNOREEVENT_USERONLINE);
		}

		hContact = db_find_next(hContact);
	}	

	// do metacontacts after handles set properly above
	hContact = db_find_first();
	while ( hContact != NULL ) {
		if (db_get_dw(hContact, META_PROTO, META_ID,(DWORD)-1)!=(DWORD)-1) {
			// is a meta contact
			MCONTACT hMostOnline = Meta_GetMostOnline(hContact); // set nick
			Meta_CopyContactNick(hContact, hMostOnline);

			Meta_FixStatus(hContact);

		}

		hContact = db_find_next(hContact);
	}	

	return 0;
}

/** Unhide all contacts linked to any meta contact
*
*/
int Meta_UnhideLinkedContacts(void)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		if (db_get_dw(hContact, META_PROTO, META_LINK,(DWORD)-1)!=(DWORD)-1) {
			// has a link - unhide it
			// restore old group
			Meta_RestoreGroup(hContact);			
		}
	}	

	return 0;
}

int Meta_HideMetaContacts(int hide)
{
	// set status suppression
	if (hide) Meta_SuppressStatus(FALSE);
	else Meta_SuppressStatus(options.suppress_status);

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		if (db_get_dw(hContact, META_PROTO, META_ID,(DWORD)-1)!=(DWORD)-1) {
			// is a meta contact
			if (hide)
				db_set_b(hContact, "CList", "Hidden", 1);
			else
				db_unset(hContact, "CList", "Hidden");
		}
		else if (db_get_dw(hContact, META_PROTO, META_LINK,(DWORD)-1)!=(DWORD)-1) {
			// when metacontacts are hidden, show subcontacts, and vice versa
			if (hide)
				Meta_RestoreGroup(hContact);
			else
				Meta_SetGroup(hContact);
		}
	}	

	return 0;
}

void Meta_RestoreGroup(MCONTACT hContact) {

	if (meta_group_hack_disabled) return; // clist has called api function to disable group hack - yay!

	// the existence of this service means that clist_meta_mw is active and will do the hiding for us
	if (ServiceExists(MS_CLUI_METASUPPORT)) return;

	// show it anyway - users are reporting contacts removed from meta remain 'hidden'
	// possible suspect - server side groups cause hidden group hack to fail, users hide contacts via clist->delete->hide option
	db_unset(hContact, META_PROTO, "Hidden");

	if (db_get_b(hContact, META_PROTO, "Hidden", 0) == 1)
	{
		// if we hid it, unhide it
		db_unset(hContact, META_PROTO, "Hidden");
		db_unset(hContact, "CList", "Hidden");
	} else {
		DBCONTACTWRITESETTING cws;

		if ( !db_get(hContact, META_PROTO, "OldCListGroup", &cws.value)) {

			if ((cws.value.type == DBVT_ASCIIZ || cws.value.type == DBVT_UTF8) && !strcmp(cws.value.pszVal, META_HIDDEN_GROUP)) {
				db_unset(hContact, "CList", "Group");
			} else {
				int hGroup = 1;
				char *name = 0;
				BOOL found = FALSE;
				do {
					name = (char *)CallService(MS_CLIST_GROUPGETNAME, (WPARAM)hGroup, 0);
					if (name && !strcmp(name, cws.value.pszVal)) {
						found = TRUE;
						break;
					}
					hGroup++;
				} while(name);

				if (found)
					db_set(hContact, "CList", "Group", &cws.value);
				else {
					// put back into metacontact's group
					DBVARIANT dbv;
					MCONTACT hMeta = (MCONTACT)db_get_dw(hContact, META_PROTO, "Handle", 0);
					if (hMeta && !Mydb_get(hMeta, "CList", "Group", &dbv)) {
						db_set(hContact, "CList", "Group", &dbv);
						db_free(&dbv);
					}
					else db_unset(hContact, "CList", "Group");
				}
			}
			db_free(&cws.value);
		}
		db_unset(hContact, META_PROTO, "OldCListGroup");

		if ( !db_get(hContact, "CList", "Group", &cws.value)) {
			if ((cws.value.type == DBVT_ASCIIZ || cws.value.type == DBVT_UTF8) && !strcmp(cws.value.pszVal, META_HIDDEN_GROUP)) {
				db_unset(hContact, "CList", "Group");
			}
			db_free(&cws.value);
		}
	}

	// show it anyway - users are reporting contacts removed from meta remain 'hidden'
	// possible suspect - server side groups cause hidden group hack to fail, users hide contacts via clist->delete->hide option
	db_unset(hContact, "CList", "Hidden");
}

void Meta_SetGroup(MCONTACT hContact) {
	char *szProto, *uid;

	if (meta_group_hack_disabled) return; // clist has called api function to disable group hack - yay!

	// the existence of this service means that clist_meta_mw is active and will do the hiding for us
	if (ServiceExists(MS_CLUI_METASUPPORT)) return;

	szProto = GetContactProto(hContact);
	if (szProto)
		uid   = (char *)CallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);

	if (szProto && uid && (INT_PTR)uid != CALLSERVICE_NOTFOUND && !strcmp(JABBER_UNIQUE_ID_SETTING, uid)) {
		// if it's a jabber contact, hide it, and record the fact that it was us who did
		db_set_b(hContact, META_PROTO, "Hidden", 1);
		db_set_b(hContact, "CList", "Hidden", 1);
	}
	else {
		DBVARIANT dbv;
		// save old group and move to invisible group (i.e. non-existent group)
		if ( !Mydb_get(hContact, "CList", "Group", &dbv)) {
			if ((dbv.type == DBVT_ASCIIZ || dbv.type == DBVT_UTF8) && !strcmp(dbv.pszVal, META_HIDDEN_GROUP))
				; // it's already in the group (shouldn't be - but maybe a crash)
			else
				db_set(hContact, META_PROTO, "OldCListGroup", &dbv);

			db_free(&dbv);
		}
		else db_unset(hContact, META_PROTO, "OldCListGroup");

		db_set_s(hContact, "CList", "Group", META_HIDDEN_GROUP);
	}
}

int Meta_SuppressStatus(BOOL suppress)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		if (db_get_dw(hContact, META_PROTO, META_LINK,(DWORD)-1)!=(DWORD)-1) {
			// is a subcontact
			if (suppress)
				CallService(MS_IGNORE_IGNORE, hContact, (WPARAM)IGNOREEVENT_USERONLINE);
			else
				CallService(MS_IGNORE_UNIGNORE, hContact, (WPARAM)IGNOREEVENT_USERONLINE);
		}
	}	

	return 0;
}

int Meta_CopyContactNick(MCONTACT hMeta, MCONTACT hContact) {
	DBVARIANT dbv, dbv_proto;
	char *szProto;

	if (options.lockHandle) {
		hContact = Meta_GetContactHandle(hMeta, 0);
	}

	if ( !hContact) return 1;

	//szProto = GetContactProto(hContact);
	// read szProto direct from db, since we do this on load and other szProto plugins may not be loaded yet
	if ( !db_get(hContact, "Protocol", "p", &dbv_proto)) {

		szProto = dbv_proto.pszVal;
		if (options.clist_contact_name == CNNT_NICK && szProto) {
			if ( !Mydb_get(hContact, szProto, "Nick", &dbv)) {
				db_set(hMeta, META_PROTO, "Nick", &dbv);
				db_free(&dbv);
				//CallService(MS_CLIST_INVALIDATEDISPLAYNAME, (WPARAM)hMeta, 0);
				//CallService(MS_CLUI_CONTACTRENAMED, (WPARAM)hMeta, 0);
				db_free(&dbv_proto);
				return 0;
			}
		} else if (options.clist_contact_name == CNNT_DISPLAYNAME) {
			TCHAR *name = pcli->pfnGetContactDisplayName(hContact, GCDNF_TCHAR);
			if (name && _tcscmp(name, TranslateT("(Unknown Contact)")) != 0) {
				db_set_ts(hMeta, META_PROTO, "Nick", name);
				db_free(&dbv_proto);
				return 0;
			}
		}
		db_free(&dbv_proto);
	}
	return 1;
}

int Meta_SetAllNicks()
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		if (db_get_dw(hContact, META_PROTO, META_ID,(DWORD)-1)!=(DWORD)-1) {
			MCONTACT most_online = Meta_GetMostOnline(hContact);
			Meta_CopyContactNick(hContact, most_online);
			Meta_FixStatus(hContact);
			Meta_CopyData(hContact);
		}
	
	}
	return 0;
}

int Meta_IsHiddenGroup(const char *group_name)
{
	if (group_name && !strcmp(group_name, META_HIDDEN_GROUP))
		return 1;

	return 0;
}

int Meta_SwapContacts(MCONTACT hMeta, DWORD contact_number1, DWORD contact_number2) {
	DBVARIANT dbv1, dbv2;

	MCONTACT hContact1 = Meta_GetContactHandle(hMeta, contact_number1), 
		hContact2 = Meta_GetContactHandle(hMeta, contact_number2);
	char buff1[512], buff12[512], buff2[512], buff22[512];
	BOOL ok1, ok2;

	// swap the protocol
	strcpy(buff1, "Protocol");
	strcat(buff1, _itoa(contact_number1, buff12, 10));
	strcpy(buff2, "Protocol");
	strcat(buff2, _itoa(contact_number2, buff22, 10));
	ok1 = !Mydb_get(hMeta, META_PROTO, buff1, &dbv1);
	ok2 = !Mydb_get(hMeta, META_PROTO, buff2, &dbv2);
	if (ok1) {
		db_set(hMeta, META_PROTO, buff2, &dbv1);
		db_free(&dbv1);
	}
	if (ok2) {
		db_set(hMeta, META_PROTO, buff1, &dbv2);
		db_free(&dbv2);
	}

	// swap the status
	strcpy(buff1, "Status");
	strcat(buff1, _itoa(contact_number1, buff12, 10));
	strcpy(buff2, "Status");
	strcat(buff2, _itoa(contact_number2, buff22, 10));
	ok1 = !Mydb_get(hMeta, META_PROTO, buff1, &dbv1);
	ok1 = !Mydb_get(hMeta, META_PROTO, buff2, &dbv2);
	if (ok1) {
		db_set(hMeta, META_PROTO, buff2, &dbv1);
		db_free(&dbv1);
	}
	if (ok2) {
		db_set(hMeta, META_PROTO, buff1, &dbv2);
		db_free(&dbv2);
	}

	// swap the status string
	strcpy(buff1, "StatusString");
	strcat(buff1, _itoa(contact_number1, buff12, 10));
	strcpy(buff2, "StatusString");
	strcat(buff2, _itoa(contact_number2, buff22, 10));
	ok1 = !Mydb_get(hMeta, META_PROTO, buff1, &dbv1);
	ok2 = !Mydb_get(hMeta, META_PROTO, buff2, &dbv2);
	if (ok1) {
		db_set(hMeta, META_PROTO, buff2, &dbv1);
		db_free(&dbv1);
	}
	if (ok2) {
		db_set(hMeta, META_PROTO, buff1, &dbv2);
		db_free(&dbv2);
	}

	// swap the login
	strcpy(buff1, "Login");
	strcat(buff1, _itoa(contact_number1, buff12, 10));
	strcpy(buff2, "Login");
	strcat(buff2, _itoa(contact_number2, buff22, 10));
	ok1 = !Mydb_get(hMeta, META_PROTO, buff1, &dbv1);
	ok2 = !Mydb_get(hMeta, META_PROTO, buff2, &dbv2);
	if (ok1) {
		db_unset(hMeta, META_PROTO, buff2);
		db_set(hMeta, META_PROTO, buff2, &dbv1);
		db_free(&dbv1);
	}
	if (ok2) {
		db_unset(hMeta, META_PROTO, buff1);
		db_set(hMeta, META_PROTO, buff1, &dbv2);
		db_free(&dbv2);
	}

	// swap the nick
	strcpy(buff1, "Nick");
	strcat(buff1, _itoa(contact_number1, buff12, 10));
	strcpy(buff2, "Nick");
	strcat(buff2, _itoa(contact_number2, buff22, 10));
	ok1 = !Mydb_get(hMeta, META_PROTO, buff1, &dbv1);
	ok2 = !Mydb_get(hMeta, META_PROTO, buff2, &dbv2);
	if (ok1) {
		db_set(hMeta, META_PROTO, buff2, &dbv1);
		db_free(&dbv1);
	} else {
		db_unset(hMeta, META_PROTO, buff2);
	}
	if (ok2) {
		db_set(hMeta, META_PROTO, buff1, &dbv2);
		db_free(&dbv2);
	} else {
		db_unset(hMeta, META_PROTO, buff1);
	}

	// swap the clist name
	strcpy(buff1, "CListName");
	strcat(buff1, _itoa(contact_number1, buff12, 10));
	strcpy(buff2, "CListName");
	strcat(buff2, _itoa(contact_number2, buff22, 10));
	ok1 = !Mydb_get(hMeta, META_PROTO, buff1, &dbv1);
	ok2 = !Mydb_get(hMeta, META_PROTO, buff2, &dbv2);
	if (ok1) {
		db_set(hMeta, META_PROTO, buff2, &dbv1);
		db_free(&dbv1);
	} else {
		db_unset(hMeta, META_PROTO, buff2);
	}
	if (ok2) {
		db_set(hMeta, META_PROTO, buff1, &dbv2);
		db_free(&dbv2);
	} else {
		db_unset(hMeta, META_PROTO, buff1);
	}

	// swap the handle
	strcpy(buff1, "Handle");
	strcat(buff1, _itoa(contact_number1, buff12, 10));
	strcpy(buff2, "Handle");
	strcat(buff2, _itoa(contact_number2, buff22, 10));
	ok1 = !Mydb_get(hMeta, META_PROTO, buff1, &dbv1);
	ok2 = !Mydb_get(hMeta, META_PROTO, buff2, &dbv2);
	if (ok1) {
		db_set(hMeta, META_PROTO, buff2, &dbv1);
		db_free(&dbv1);
	} else {
		db_unset(hMeta, META_PROTO, buff2);
	}
	if (ok2) {
		db_set(hMeta, META_PROTO, buff1, &dbv2);
		db_free(&dbv2);
	} else {
		db_unset(hMeta, META_PROTO, buff1);
	}

	// finally, inform the contacts of their change in position
	db_set_dw(hContact1, META_PROTO, "ContactNumber", (DWORD)contact_number2);
	db_set_dw(hContact2, META_PROTO, "ContactNumber", (DWORD)contact_number1);

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
void copyHistory(MCONTACT hContactFrom, MCONTACT hContactTo) 
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

	if ( !hContactFrom || !hContactTo) return; 

	//id = Meta_GetUniqueIdentifier(hContactFrom, &id_length);
	//if ( !id) return;

	progress_dialog = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_COPYPROGRESS), 0, DlgProcNull);
	ShowWindow(progress_dialog, SW_SHOW);

	prog = GetDlgItem(progress_dialog, IDC_PROG);

	//CallService(MS_DB_SETSAFETYMODE, (WPARAM)FALSE, 0);
	for (hDbEvent = db_event_first(hContactFrom); hDbEvent; hDbEvent = db_event_next(hDbEvent)) 
	{ 
		// get the event 
		ZeroMemory(&dbei, sizeof(dbei)); 
		dbei.cbSize = sizeof(dbei); 

		if ((dbei.cbBlob = db_event_getBlobSize(hDbEvent)) == -1)
			break;

		buffer = (BYTE *)mir_realloc(buffer, dbei.cbBlob);// + id_length);
		dbei.pBlob = buffer; 
		if ( db_event_get(hDbEvent, &dbei)) 
			break; 

		// i.e. optoins.days_history == 0;
		if (time_now == earliest_time) earliest_time = dbei.timestamp;

		if (dbei.timestamp < earliest_time)
			continue;

		if (dbei.eventType != EVENTTYPE_MESSAGE && dbei.eventType != EVENTTYPE_FILE && dbei.eventType != EVENTTYPE_URL)
			continue;

		if (time_now > earliest_time) { // just in case!
			SendMessage(prog, PBM_SETPOS, (WPARAM)(int)(100.0 * (dbei.timestamp - earliest_time) / (time_now - earliest_time)), 0);	
			UpdateWindow(progress_dialog);
		}

		dbei.szModule = META_PROTO;
		dbei.flags &= ~DBEF_FIRST;
		db_event_add(hContactTo, &dbei); 
	}		

	DestroyWindow(progress_dialog);
	if (buffer) mir_free(buffer);
	//mir_free(id);
}

void Meta_FixStatus(MCONTACT hMeta)
{
	MCONTACT most_online = Meta_GetMostOnlineSupporting(hMeta, PFLAGNUM_1, 0);
	if (most_online) {
		char *szProto = GetContactProto(most_online);
		if (szProto) {
			WORD status = (WORD)db_get_w(most_online, szProto, "Status", (WORD)ID_STATUS_OFFLINE);
			db_set_w(hMeta, META_PROTO, "Status", status);
		}
		else db_set_w(hMeta, META_PROTO, "Status", (WORD)ID_STATUS_OFFLINE);
	}
	else db_set_w(hMeta, META_PROTO, "Status", (WORD)ID_STATUS_OFFLINE);
}

INT_PTR Meta_IsEnabled() {
	return db_get_b(0, META_PROTO, "Enabled", 1) && (meta_group_hack_disabled || db_get_b(NULL, "CList", "UseGroups", 1));
}



