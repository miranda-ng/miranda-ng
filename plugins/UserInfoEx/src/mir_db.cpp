/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

namespace DB {

/**
* This namespace contains all functions used to access or modify contacts in the database.
**/

namespace Contact {
/**
 * This function trys to guess, when an ICQ contact was added to database.
 **/
uint32_t	WhenAdded(uint32_t dwUIN, LPCSTR)
{
	DBEVENTINFO	dbei = {};
	DB::ECPTR pCursor(DB::Events(0));
	while (MEVENT edbe = pCursor.FetchNext()) {
		// get eventtype and compare
		if (!DB::Event::GetInfo(edbe, &dbei) && dbei.eventType == EVENTTYPE_ADDED) {
			if (!DB::Event::GetInfoWithData(edbe, &dbei)) {
				// extract UIN and compare with given one
				uint32_t dwEvtUIN;
				memcpy(&dwEvtUIN, dbei.pBlob, sizeof(uint32_t));
				MIR_FREE(dbei.pBlob);
				if (dwEvtUIN == dwUIN)
					return dbei.timestamp;
			}
		}
	}
	return 0;
}

} /* Contact */

namespace Module {

/**
* Enum Proc for DBModule_IsEmpty
* @param	pszSetting	- the setting
* @param	lParam		- DBCONTACTENUMSETTINGS - (LPARAM)&dbces
* @retval	TRUE		- always true
**/

static int IsEmptyEnumProc(LPCSTR, void*)
{
	return 1;
}

/**
* This function tests, whether a module belongs to a metacontact protocol
* @param	pszModule	- the module to read the setting from (e.g. USERINFO)
* @retval	TRUE		- the module belongs to a metacontact protocol
* @retval	FALSE		- the module belongs to a other protocol
**/

bool IsMeta(LPCSTR pszModule)
{
	return !mir_strcmp(pszModule, META_PROTO);
}

/**
* This function tests, whether a module is a meta contact, and user wants to scan it for settings
* @param	pszModule	- the module to read the setting from (e.g. USERINFO)
* @retval	TRUE		- the module is empty
* @retval	FALSE		- the module contains settings
**/

bool IsMetaAndScan(LPCSTR pszModule)
{
	return 0 != g_plugin.getByte(SET_META_SCAN, TRUE) && IsMeta(pszModule);
}

} /* namespace Module */

namespace Setting {

/**
* This function calls MS_DB_CONTACT_GETSETTING_STR service to get database values. 
* @param	hContact	- handle to the contact
* @param	pszModule	- the module to read the setting from (e.g. USERINFO)
* @param	pszSetting	- the setting to read
* @param	destType	- desired string type (DBVT_ASCIIZ, DBVT_WCHAR, DBVT_UTF8)
*
* @retval	0 - success
* @retval	1 - error
**/

uint8_t Get(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting, DBVARIANT *dbv, const uint8_t destType)
{
	// read value without translation to specific type
	uint8_t result = db_get_s(hContact, pszModule, pszSetting, dbv, 0) != 0;

	// Is value read successfully and destination type set?
	if (!result && destType)
		result = DB::Variant::ConvertString(dbv, destType);
	return result;
}

/**
* This function reads a value from the database and returns it as an ansi encoded string.
* @param	hContact	- handle to the contact
* @param	pszModule	- the module to read the setting from (e.g. USERINFO)
* @param	pszSetting	- the setting to read
*
* @return	string value
**/

LPSTR	GetAString(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting)
{
	DBVARIANT dbv;
	if (GetAString(hContact, pszModule, pszSetting, &dbv) == 0){
		if (DB::Variant::dbv2String(&dbv, DBVT_WCHAR) == 0)
			return dbv.pszVal;

		db_free(&dbv);
	}
	return nullptr;
}

/**
* This function reads a value from the database and returns it as an unicode encoded string. 
* @param	hContact	- handle to the contact
* @param	pszModule	- the module to read the setting from (e.g. USERINFO)
* @param	pszSetting	- the setting to read
*
* @return	string value
**/

LPWSTR GetWString(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting)
{
	DBVARIANT dbv;
	if (GetWString(hContact, pszModule, pszSetting, &dbv) == 0) {
		if (DB::Variant::dbv2String(&dbv, DBVT_WCHAR) == 0)
			return dbv.pwszVal;

		db_free(&dbv);
	}
	return nullptr;
}

/**
* This function calls MS_DB_CONTACT_GETSETTING_STR service to get database values. 
* It searches in pszModule first and if the setting does not exist there it tries proto to retrieve it.
* @param	hContact	- handle to the contact
* @param	pszModule	- the module to read the setting from (e.g. USERINFO)
* @param	szProto		- the contact's protocol to read the setting from (e.g. ICQ)
* @param	szSetting	- the setting to read
* @param	destType	- desired string type (DBVT_ASCIIZ, DBVT_WCHAR, DBVT_UTF8)
*
* @retval	0 - success
* @retval	1 - error
**/

uint8_t GetEx(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszProto, LPCSTR pszSetting, DBVARIANT *dbv, const uint8_t destType)
{
	uint8_t result = !pszModule || Get(hContact, pszModule, pszSetting, dbv, destType);
	// try to read setting from the contact's protocol module 
	if (result && pszProto) {
		result = Get(hContact, pszProto, pszSetting, dbv, destType) != 0;
		// try to get setting from a metasubcontact
		if (result && DB::Module::IsMetaAndScan(pszProto)) {
			const INT_PTR def = db_mc_getDefaultNum(hContact);
			MCONTACT hSubContact;
			// try to get setting from the default subcontact first
			if (def > -1 && def < INT_MAX) {
				hSubContact = db_mc_getSub(hContact, def);
				if (hSubContact)
					result = DB::Setting::GetEx(hSubContact, pszModule, Proto_GetBaseAccountName(hSubContact), pszSetting, dbv, destType) != 0;
			}
			// scan all subcontacts for the setting
			if (result) {
				const INT_PTR cnt = db_mc_getSubCount(hContact);
				if (cnt < INT_MAX) {
					INT_PTR i;
					for (i = 0; result && i < cnt; i++) {
						if (i != def) {
							hSubContact = db_mc_getSub(hContact, i);
							if (hSubContact)
								result = DB::Setting::GetEx(hSubContact, pszModule, Proto_GetBaseAccountName(hSubContact), pszSetting, dbv, destType) != 0;
	}	}	}	}	}	}

	return result;
}

/**
* This function is used by the controls of the details dialog and calls MS_DB_CONTACT_GETSETTING_STR service
* to get database values. It searches in pszModule first and if the setting does not exist there it tries proto
* to retrieve it.
* @param	hContact		- handle to the contact
* @param	pszModule		- the module to read the setting from (e.g. USERINFO)
* @param	pszSubModule	- the module to read the setting from a meta subcontract (e.g. USERINFO)
* @param	pszProto		- the contact's protocol to read the setting from (e.g. ICQ)
* @param	pszSetting		- the setting to read
* @param	destType		- desired string type (DBVT_ASCIIZ, DBVT_WCHAR, DBVT_UTF8)
*
* @return	This function returns the uint16_t which contains the source of information.
**/

uint16_t GetCtrl(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSubModule, LPCSTR pszProto, LPCSTR pszSetting, DBVARIANT *dbv, const uint8_t destType)
{
	uint16_t wFlags = 0;

	// read setting from given module
	if (hContact && pszModule && *pszModule && !Get(hContact, pszModule, pszSetting, dbv, destType)) {
		wFlags |= CTRLF_HASCUSTOM;
		if (Exists(hContact, pszProto, pszSetting))
			wFlags |= CTRLF_HASPROTO;
	}
	// read setting from contact's basic protocol
	else if (pszProto && *pszProto) {
		// try to read the setting from the basic protocol
		if (!Get(hContact, pszProto, pszSetting, dbv, destType))
			wFlags |= CTRLF_HASPROTO;

		// try to read the setting from the sub contacts' modules
		else if (DB::Module::IsMetaAndScan(pszProto)) {
			const INT_PTR def = db_mc_getDefaultNum(hContact);
			MCONTACT hSubContact;
			// try to get setting from the default subcontact first
			if (def > -1 && def < INT_MAX) {
				hSubContact = db_mc_getSub(hContact, def);
				if (hSubContact) {
					wFlags = GetCtrl(hSubContact, pszSubModule, nullptr, Proto_GetBaseAccountName(hSubContact), pszSetting, dbv, destType);
					if (wFlags != 0) {
						wFlags &= ~CTRLF_HASCUSTOM;
						wFlags |= CTRLF_HASMETA;
					}
				}
			}
			// copy the missing settings from the other subcontacts
			if (wFlags == 0) {
				INT_PTR i;
				const INT_PTR cnt = db_mc_getSubCount(hContact);
				for (i = 0; i < cnt; i++) {
					if (i != def) {
						hSubContact = db_mc_getSub(hContact, i);
						if (hSubContact) {
							wFlags = GetCtrl(hSubContact, pszSubModule, nullptr, Proto_GetBaseAccountName(hSubContact), pszSetting, dbv, destType);
							if (wFlags != 0) {
								wFlags &= ~CTRLF_HASCUSTOM;
								wFlags |= CTRLF_HASMETA;
								break;
	}	}	}	}	}	}	}

	if (wFlags == 0)
		dbv->type = DBVT_DELETED;

	return wFlags;
}

/**
* This function checks for the existence of the given setting in the database
* @param	hContact		- handle to the contact
* @param	pszModule		- the module to read the setting from (e.g. USERINFO)
* @param	pszSetting		- the setting to check
*
* @retval	TRUE			- setting exists
* @retval	FALSE			- setting does not exist
**/

uint8_t Exists(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting)
{
	if (pszModule && pszSetting) {
		CHAR szDummy[1];
		return 0 == db_get_static(hContact, pszModule, pszSetting, szDummy, 1);
	}
	return FALSE;
}

/**
* This function deletes all reluctant settings of an setting array such as My-phoneXX.
* @param	hContact		- handle to the contact
* @param	pszModule		- the module to read the setting from (e.g. USERINFO)
* @param	pszFormat		- the format, telling what a array of settings is ment
* @param	iStart			- the first index of the setting to delete
*
* @return	nothing
**/

void DeleteArray(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszFormat, int iStart)
{
	char pszSetting[MAXSETTING];
	do {
		mir_snprintf(pszSetting, pszFormat, iStart++);
	}
		while (!db_unset(hContact, pszModule, pszSetting));
}

} /* namespace Setting */

namespace Variant {

/**
* This function converts a string value of the DBVARIANT to the destination type
* but keeps all other values as is.
* @param		dbv			- pointer to DBVARIANT structure which is to manipulate
* @param		destType	- one of (DBVT_ASCIIZ, DBVT_UTF8 or DBVT_WCHAR)
*
* @retval		0			- success
* @retval		1			- error
**/

uint8_t ConvertString(DBVARIANT* dbv, const uint8_t destType)
{
	if (dbv == nullptr)
		return 1;

	switch (dbv->type) {
	// source value is of type "ascii"
	case DBVT_ASCIIZ:
		switch (destType) {
		// destination type is "utf8"
		case DBVT_UTF8:
			{
				LPSTR tmpBuf = mir_utf8encode(dbv->pszVal);
				mir_free(dbv->pszVal);
				dbv->pszVal = tmpBuf;
			}
			dbv->type = (dbv->pszVal) ? destType : DBVT_DELETED;
			break;
		// destination type is "wchar"
		case DBVT_WCHAR:
			{
				LPWSTR tmpBuf = mir_a2u(dbv->pszVal);
				mir_free(dbv->pszVal);
				dbv->pwszVal = tmpBuf;
			}
			dbv->type = (dbv->pwszVal) ? destType : DBVT_DELETED;
		}
		break;
	// source value is of type "utf8"
	case DBVT_UTF8:
		switch (destType) {
		// destination type is "ascii"
		case DBVT_ASCIIZ:
			mir_utf8decode(dbv->pszVal, nullptr);
			dbv->type = (dbv->pszVal) ? destType : DBVT_DELETED;
			break;
		// destination type is "wchar"
		case DBVT_WCHAR:
			LPSTR savePtr = dbv->pszVal;
			dbv->pszVal = nullptr;
			mir_utf8decode(savePtr, &dbv->pwszVal);
			mir_free(savePtr);
			dbv->type = (dbv->pwszVal) ? destType : DBVT_DELETED;
		}
		break;
	// source value is of type "wchar"
	case DBVT_WCHAR:
		switch (destType) {
			// destination type is "ascii"
		case DBVT_ASCIIZ:
			{
				LPSTR tmpBuf = mir_u2a(dbv->pwszVal);
				mir_free(dbv->pwszVal);
				dbv->pszVal = tmpBuf;
			}
			dbv->type = (dbv->pszVal) ? destType : DBVT_DELETED;
			break;
			// destination type is "utf8"
		case DBVT_UTF8:
			{
				LPSTR tmpBuf = mir_utf8encodeW(dbv->pwszVal);
				mir_free(dbv->pwszVal);
				dbv->pszVal = tmpBuf;
			}
			dbv->type = (dbv->pszVal) ? destType : DBVT_DELETED;
		}
	}
	return dbv->type == DBVT_DELETED;
}

/**
* This function completely converts a DBVARIANT to the destination string type.
* It includes uint8_t, uint16_t, uint32_t and all string types
* @param		dbv			- pointer to DBVARIANT structure which is to manipulate
* @param		destType	- one of (DBVT_ASCIIZ, DBVT_UTF8 or DBVT_WCHAR)
*
* @retval	0 - success
* @retval	1 - error
**/

uint8_t	dbv2String(DBVARIANT* dbv, const uint8_t destType)
{
	if (dbv == nullptr)
		return 1;

	wchar_t wbuf[32];
	CHAR buf[32];
	switch (destType) {
	// destination type is "utf8" or "ascii"
	case DBVT_ASCIIZ:
	case DBVT_UTF8:
		switch (dbv->type) {
		// source value is of type "byte"
		case DBVT_BYTE:
			_ultoa(dbv->bVal, buf, 10);
			dbv->pszVal = mir_strdup(buf);
			dbv->type = (dbv->pszVal) ? destType : DBVT_DELETED;
			break;
		// source value is of type "word"
		case DBVT_WORD:
			_ultoa(dbv->wVal, buf, 10);
			dbv->pszVal = mir_strdup(buf);
			dbv->type = (dbv->pszVal) ? destType : DBVT_DELETED;
			break;
		// source value is of type "dword"
		case DBVT_DWORD:
			_ultoa(dbv->dVal, buf, 10);
			dbv->pszVal = mir_strdup(buf);
			dbv->type = (dbv->pszVal) ? destType : DBVT_DELETED;
			break;
		// source value is of any string type
		case DBVT_ASCIIZ:
		case DBVT_WCHAR:
		case DBVT_UTF8:
			return ConvertString(dbv, destType);
		}
		break;
	
	// destination type is "wchar"
	case DBVT_WCHAR:
		switch (dbv->type) {
		// source value is of type "byte"
		case DBVT_BYTE:
			_ultow(dbv->bVal, wbuf, 10);
			dbv->pwszVal = mir_wstrdup(wbuf);
			dbv->type = (dbv->pwszVal) ? destType : DBVT_DELETED;
			break;
		// source value is of type "word"
		case DBVT_WORD:
			_ultow(dbv->wVal, wbuf, 10);
			dbv->pwszVal = mir_wstrdup(wbuf);
			dbv->type = (dbv->pwszVal) ? destType : DBVT_DELETED;
			break;
		// source value is of type "dword"
		case DBVT_DWORD:
			_ultow(dbv->dVal, wbuf, 10);
			dbv->pwszVal = mir_wstrdup(wbuf);
			dbv->type = (dbv->pwszVal) ? destType : DBVT_DELETED;
			break;
		// source value is of any string type
		case DBVT_ASCIIZ:
		case DBVT_WCHAR:
		case DBVT_UTF8:
			return ConvertString(dbv, destType);
		}
	}
	return dbv->type != destType;
}

} /* namespace Variant */

namespace Event {

/**
* This function initializes the DBEVENTINFO structure and calls 
* db_event_get() to retrieve information about an event.
* @param	hEvent			- the handle of the event to get information for
* @param	dbei			- the pointer to a DBEVENTINFO structure, which retrieves all information.
*
* @retval	0				- success
* @retval	nonezero		- failure
**/

bool GetInfo(MEVENT hEvent, DBEVENTINFO *dbei)
{
	dbei->cbBlob = 0;
	dbei->pBlob  = nullptr;
	return db_event_get(hEvent, dbei) != 0;
}

/**
* This function initializes the DBEVENTINFO structure and calls 
* db_event_get() to retrieve information about an event.
* @param	hEvent			- the handle of the event to get information for
* @param	dbei			- the pointer to a DBEVENTINFO structure, which retrieves all information.
*
* @retval	0				- success
* @retval	1				- failure
**/

bool GetInfoWithData(MEVENT hEvent, DBEVENTINFO *dbei)
{
	if (!dbei->cbBlob) {
		INT_PTR size = db_event_getBlobSize(hEvent);
		dbei->cbBlob = (size != -1) ? (uint32_t)size : 0;
	}
	if (dbei->cbBlob) {
		dbei->pBlob = (uint8_t*) mir_alloc(dbei->cbBlob);
		if (dbei->pBlob == nullptr)
			dbei->cbBlob = 0;
	}
	else dbei->pBlob = nullptr;

	bool result = db_event_get(hEvent, dbei) != 0;
	if (result && dbei->pBlob) {
		mir_free(dbei->pBlob);
		dbei->pBlob = nullptr;
	}
	return result;
}

/**
* This function returns the timestamp for the given event.
* @param	hEvent			- the handle of the event to get the timestamp for
*
* @retval	0 if no timestamp is available
* @retval	timestamp
**/

uint32_t	TimeOf(MEVENT hEvent)
{
	DBEVENTINFO dbei;
	if (!GetInfo(hEvent, &dbei))
		return dbei.timestamp;

	return 0;
}

/**
* This function compares two DBEVENTINFO structures against each other.
* It compares the timestamp, eventType and module names.
* @param	d1				- pointer to the first DBEVENTINFO structure
* @param	d2				- pointer to the second DBEVENTINFO structure
* @param	Data			- default false, if true compare also blob data
.*
* @retval	TRUE			- The structures describe the same event.
* @retval	FALSE			- The two structure's events differ from each other.
**/

bool __forceinline IsEqual(const DBEVENTINFO *d1, const DBEVENTINFO *d2, bool Data)
{
	bool res = d1 && d2 && 
				(d1->timestamp == d2->timestamp) && 
				(d1->eventType == d2->eventType) &&
				(d1->cbBlob == d2->cbBlob) && 
				(!d1->szModule || !d2->szModule || !_stricmp(d1->szModule, d2->szModule));
	if (Data)
		return res && (!d1->pBlob || !d2->pBlob || !memcmp(d1->pBlob,d2->pBlob,d1->cbBlob));

	return res;
}

/**
* This functions searches for an existing event in the database, which matches
* the information provided by 'dbei'. In order to fasten up the search e.g. 
* while checking many events, this function stars searching from the last
* found event.
* @param	hContact			- the handle of the contact to search events for
* @param	hDbExistingEvent	- an existing database event to start the search from.
* @param	dbei				- the pointer to a DBEVENTINFO structure
*
* @retval	TRUE	- the event identified by its information in @c dbei exists.
* @retval	FALSE	- no event with the information of @c dbei exists.
*
**/

bool Exists(MCONTACT hContact, MEVENT& hDbExistingEvent, DBEVENTINFO *dbei)
{
	bool result = false;
	DBEVENTINFO	edbei;

	if (!hDbExistingEvent) {
		hDbExistingEvent = db_event_first(hContact);
		if (hDbExistingEvent) {
			if (!GetInfo(hDbExistingEvent, &edbei)) {
				if ((dbei->timestamp < edbei.timestamp))
					return false;

				if (IsEqual(dbei, &edbei, false)) {
					if (!GetInfoWithData(hDbExistingEvent, &edbei)) {
						if (IsEqual(dbei, &edbei, true)) {
							mir_free(edbei.pBlob);
							return true;
						}
						mir_free(edbei.pBlob);
					}
				}
			}
			MEVENT edbe = db_event_last(hContact);
			if (edbe == hDbExistingEvent)
				return FALSE;

			hDbExistingEvent = edbe;
		}
	}
	if (hDbExistingEvent) {
		MEVENT sdbe = hDbExistingEvent;
		for (MEVENT edbe = sdbe; edbe && !GetInfo(edbe, &edbei) && (dbei->timestamp <= edbei.timestamp); edbe = db_event_prev(hContact, edbe)) {
			hDbExistingEvent = edbe;
			//compare without data (faster)
			if ( result = IsEqual(dbei, &edbei, false)) {
				if (NULL == (result = !GetInfoWithData(edbe, &edbei)))
					continue;

				//compare with data
				result = IsEqual(dbei, &edbei, true);
				mir_free(edbei.pBlob);
				if (result)
					break;
			}
		} /*end for*/

		if (!result) {
			for (MEVENT edbe = db_event_next(hContact, sdbe); edbe && !GetInfo(edbe, &edbei) && (dbei->timestamp >= edbei.timestamp); edbe = db_event_next(hContact, edbe)) {
				hDbExistingEvent = edbe;
				//compare without data (faster)
				if (result = IsEqual(dbei, &edbei, false)) {
					if (NULL == (result = !GetInfoWithData(edbe, &edbei)))
						continue;

					//compare with data
					result = IsEqual(dbei, &edbei, true);
					mir_free(edbei.pBlob);
					if (result)
						break;
				}
			}
		}
	}
	return result;
}

} /* namespace Events */

int CEnumList::EnumProc(LPCSTR pszName, void *pParam)
{
	if (pszName)
		((CEnumList*)pParam)->Insert(pszName);
	return 0;
}

int CEnumList::EnumSettingsProc(LPCSTR pszName, void *param)
{
	return EnumProc(pszName, param);
}

int CEnumList::CompareProc(LPCSTR p1, LPCSTR p2)
{
	if (p1) {
		if (p2)
			return mir_strcmp(p1, p2);

		return 1;
	}
	return 0;
}

CEnumList::CEnumList()	: LIST<CHAR>(50, CEnumList::CompareProc)
{
}

CEnumList::~CEnumList() 
{ 
	for (auto &it : *this)
		mir_free(it);
}

LPSTR CEnumList::Insert(LPCSTR str)
{
	LPSTR p = mir_strdup(str);
	if (p && !insert(p)) {
		mir_free(p);
		p = nullptr;
	}
	return p;
}

INT_PTR CEnumList::EnumModules()
{
	return db_enum_modules(CEnumList::EnumProc, this);
}

/**
* @retval	-1	- no settings to enumerate
* @retval	 0	- success
**/

INT_PTR CEnumList::EnumSettings(MCONTACT hContact, LPCSTR pszModule)
{
	return db_enum_settings(hContact, &CEnumList::EnumSettingsProc, pszModule, this);
}

} /* namespace DB */