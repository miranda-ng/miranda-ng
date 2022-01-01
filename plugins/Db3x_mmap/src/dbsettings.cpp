/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
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

#include "stdafx.h"

#pragma warning(disable: 4701)

#define VLT(n) ((n == DBVT_UTF8 || n == DBVT_ENCRYPTED) ? DBVT_ASCIIZ : n)

static bool ValidLookupName(LPCSTR szModule, LPCSTR szSetting)
{
	if (!mir_strcmp(szModule, META_PROTO))
		return mir_strcmp(szSetting, "IsSubcontact") && mir_strcmp(szSetting, "ParentMetaID");

	if (!mir_strcmp(szModule, "Ignore"))
		return false;

	return true;
}

int CDb3Mmap::GetContactSettingWorker(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, int isStatic)
{
	if (szSetting == nullptr || szModule == nullptr)
		return 1;

	// the db format can't tolerate more than 255 bytes of space (incl. null) for settings+module name
	int settingNameLen = (int)mir_strlen(szSetting);
	int moduleNameLen = (int)mir_strlen(szModule);
	if (settingNameLen > 0xFE || moduleNameLen > 0xFE)
		return 1;

	mir_cslock lck(m_csDbAccess);

LBL_Seek:
	char *szCachedSettingName = m_cache->GetCachedSetting(szModule, szSetting, moduleNameLen, settingNameLen);
	log3("get [%08p] %s (%p)", hContact, szCachedSettingName, szCachedSettingName);

	DBVARIANT *pCachedValue = m_cache->GetCachedValuePtr(contactID, szCachedSettingName, 0);
	if (pCachedValue != nullptr) {
		if (pCachedValue->type == DBVT_ASCIIZ || pCachedValue->type == DBVT_UTF8) {
			int cbOrigLen = dbv->cchVal;
			char *cbOrigPtr = dbv->pszVal;
			memcpy(dbv, pCachedValue, sizeof(DBVARIANT));
			if (isStatic) {
				int cbLen = 0;
				if (pCachedValue->pszVal != nullptr)
					cbLen = (int)mir_strlen(pCachedValue->pszVal);

				cbOrigLen--;
				dbv->pszVal = cbOrigPtr;
				if (cbLen < cbOrigLen)
					cbOrigLen = cbLen;
				memcpy(dbv->pszVal, pCachedValue->pszVal, cbOrigLen);
				dbv->pszVal[cbOrigLen] = 0;
				dbv->cchVal = cbLen;
			}
			else {
				dbv->pszVal = (char*)mir_alloc(mir_strlen(pCachedValue->pszVal) + 1);
				mir_strcpy(dbv->pszVal, pCachedValue->pszVal);
			}
		}
		else memcpy(dbv, pCachedValue, sizeof(DBVARIANT));

		log2("get cached %s (%p)", printVariant(dbv), pCachedValue);
		return (pCachedValue->type == DBVT_DELETED) ? 1 : 0;
	}

	// never look db for the resident variable
	if (szCachedSettingName[-1] != 0)
		return 1;

	DBCachedContact *cc;
	uint32_t ofsContact = GetContactOffset(contactID, &cc);

	uint32_t ofsModuleName = GetModuleNameOfs(szModule);

	DBContact dbc = *(DBContact*)DBRead(ofsContact, nullptr);
	if (dbc.signature != DBCONTACT_SIGNATURE)
		return 1;

	uint32_t ofsSettingsGroup = GetSettingsGroupOfsByModuleNameOfs(&dbc, ofsModuleName);
	if (ofsSettingsGroup) {
		int bytesRemaining;
		unsigned varLen;
		uint32_t ofsBlobPtr = ofsSettingsGroup + offsetof(DBContactSettings, blob);
		uint8_t *pBlob = DBRead(ofsBlobPtr, &bytesRemaining);
		while (pBlob[0]) {
			NeedBytes(1 + settingNameLen);
			if (pBlob[0] == settingNameLen && !memcmp(pBlob + 1, szSetting, settingNameLen)) {
				MoveAlong(1 + settingNameLen);
				NeedBytes(5);
				if (isStatic && (pBlob[0] & DBVTF_VARIABLELENGTH) && VLT(dbv->type) != VLT(pBlob[0]))
					return 1;

				uint8_t iType = dbv->type = pBlob[0];
				switch (iType) {
				case DBVT_DELETED: /* this setting is deleted */
					dbv->type = DBVT_DELETED;
					return 2;

				case DBVT_BYTE:  dbv->bVal = pBlob[1]; break;
				case DBVT_WORD:  memmove(&(dbv->wVal), (PWORD)(pBlob + 1), 2); break;
				case DBVT_DWORD: memmove(&(dbv->dVal), (PDWORD)(pBlob + 1), 4); break;

				case DBVT_UTF8:
				case DBVT_ASCIIZ:
					varLen = *(PWORD)(pBlob + 1);
					NeedBytes(int(3 + varLen));
					if (isStatic) {
						dbv->cchVal--;
						if (varLen < dbv->cchVal)
							dbv->cchVal = varLen;
						memmove(dbv->pszVal, pBlob + 3, dbv->cchVal); // decode
						dbv->pszVal[dbv->cchVal] = 0;
						dbv->cchVal = varLen;
					}
					else {
						dbv->pszVal = (char*)mir_alloc(1 + varLen);
						memmove(dbv->pszVal, pBlob + 3, varLen);
						dbv->pszVal[varLen] = 0;
					}
					break;

				case DBVT_BLOB:
					varLen = *(PWORD)(pBlob + 1);
					NeedBytes(int(3 + varLen));
					if (isStatic) {
						if (varLen < dbv->cpbVal)
							dbv->cpbVal = varLen;
						memmove(dbv->pbVal, pBlob + 3, dbv->cpbVal);
					}
					else {
						dbv->pbVal = (uint8_t *)mir_alloc(varLen);
						memmove(dbv->pbVal, pBlob + 3, varLen);
					}
					dbv->cpbVal = varLen;
					break;

				case DBVT_ENCRYPTED:
					if (m_crypto == nullptr)
						return 1;
					else {
						varLen = *(PWORD)(pBlob + 1);
						NeedBytes(int(3 + varLen));
						size_t realLen;
						ptrA decoded(m_crypto->decodeString(pBlob + 3, varLen, &realLen));
						if (decoded == nullptr)
							return 1;

						varLen = (uint16_t)realLen;
						dbv->type = DBVT_UTF8;
						if (isStatic) {
							dbv->cchVal--;
							if (varLen < dbv->cchVal)
								dbv->cchVal = varLen;
							memmove(dbv->pszVal, decoded, dbv->cchVal);
							dbv->pszVal[dbv->cchVal] = 0;
							dbv->cchVal = varLen;
						}
						else {
							dbv->pszVal = (char*)mir_alloc(1 + varLen);
							memmove(dbv->pszVal, decoded, varLen);
							dbv->pszVal[varLen] = 0;
						}
					}
					break;
				}

				/**** add to cache **********************/
				if (iType != DBVT_BLOB && iType != DBVT_ENCRYPTED) {
					pCachedValue = m_cache->GetCachedValuePtr(contactID, szCachedSettingName, 1);
					if (pCachedValue != nullptr) {
						m_cache->SetCachedVariant(dbv, pCachedValue);
						log3("set cached [%08p] %s (%p)", hContact, szCachedSettingName, pCachedValue);
					}
				}

				return 0;
			}
			NeedBytes(1);
			MoveAlong(pBlob[0] + 1);
			NeedBytes(3);
			MoveAlong(1 + GetSettingValueLength(pBlob));
			NeedBytes(1);
		}
	}

	// try to get the missing mc setting from the active sub
	if (cc && cc->IsMeta() && ValidLookupName(szModule, szSetting)) {
		if (contactID = db_mc_getDefault(contactID)) {
			if (szModule = Proto_GetBaseAccountName(contactID)) {
				moduleNameLen = (int)mir_strlen(szModule);
				goto LBL_Seek;
			}
		}
	}

	logg();
	return 1;
}

STDMETHODIMP_(BOOL) CDb3Mmap::WriteContactSettingWorker(MCONTACT contactID, DBCONTACTWRITESETTING &dbcws)
{
	log1(" write database as %s", printVariant(&dbcws.value));

	uint32_t settingNameLen = (uint32_t)mir_strlen(dbcws.szSetting);
	uint32_t ofsBlobPtr, ofsContact = GetContactOffset(contactID);
	if (ofsContact == 0) {
		_ASSERT(false); // contact doesn't exist?
		return 2;
	}

	uint32_t ofsModuleName = GetModuleNameOfs(dbcws.szModule);
	DBContact dbc = *(DBContact*)DBRead(ofsContact, nullptr);
	if (dbc.signature != DBCONTACT_SIGNATURE)
		return 1;

	// make sure the module group exists
	uint8_t *pBlob;
	int bytesRequired, bytesRemaining;
	DBContactSettings dbcs;
	uint32_t ofsSettingsGroup = GetSettingsGroupOfsByModuleNameOfs(&dbc, ofsModuleName);
	if (ofsSettingsGroup == 0) {  //module group didn't exist - make it
		switch (dbcws.value.type) {
		case DBVT_ASCIIZ: case DBVT_UTF8:
			bytesRequired = dbcws.value.cchVal + 2;
			break;
		case DBVT_BLOB: case DBVT_ENCRYPTED:
			bytesRequired = dbcws.value.cpbVal + 2;
			break;
		default:
			bytesRequired = dbcws.value.type;
		}
		bytesRequired += 2 + settingNameLen;
		bytesRequired += (DB_SETTINGS_RESIZE_GRANULARITY - (bytesRequired % DB_SETTINGS_RESIZE_GRANULARITY)) % DB_SETTINGS_RESIZE_GRANULARITY;
		ofsSettingsGroup = CreateNewSpace(bytesRequired + offsetof(DBContactSettings, blob));
		dbcs.signature = DBCONTACTSETTINGS_SIGNATURE;
		dbcs.ofsNext = dbc.ofsFirstSettings;
		dbcs.ofsModuleName = ofsModuleName;
		dbcs.cbBlob = bytesRequired;
		dbcs.blob[0] = 0;
		dbc.ofsFirstSettings = ofsSettingsGroup;
		DBWrite(ofsContact, &dbc, sizeof(DBContact));
		DBWrite(ofsSettingsGroup, &dbcs, sizeof(DBContactSettings));
		ofsBlobPtr = ofsSettingsGroup + offsetof(DBContactSettings, blob);
		pBlob = (uint8_t*)DBRead(ofsBlobPtr, &bytesRemaining);
	}
	else {
		dbcs = *(DBContactSettings*)DBRead(ofsSettingsGroup, &bytesRemaining);

		// find if the setting exists
		ofsBlobPtr = ofsSettingsGroup + offsetof(DBContactSettings, blob);
		pBlob = (uint8_t*)DBRead(ofsBlobPtr, &bytesRemaining);
		while (pBlob[0]) {
			NeedBytes(settingNameLen + 1);
			if (pBlob[0] == settingNameLen && !memcmp(pBlob + 1, dbcws.szSetting, settingNameLen))
				break;
			NeedBytes(1);
			MoveAlong(pBlob[0] + 1);
			NeedBytes(3);
			MoveAlong(1 + GetSettingValueLength(pBlob));
			NeedBytes(1);
		}

		// setting already existed, and up to end of name is in cache
		if (pBlob[0]) {
			MoveAlong(1 + settingNameLen);
			// if different type or variable length and length is different
			NeedBytes(3);
			if (pBlob[0] != dbcws.value.type ||
				((pBlob[0] == DBVT_ASCIIZ || pBlob[0] == DBVT_UTF8) && *(PWORD)(pBlob + 1) != dbcws.value.cchVal) ||
				((pBlob[0] == DBVT_BLOB || pBlob[0] == DBVT_ENCRYPTED) && *(PWORD)(pBlob + 1) != dbcws.value.cpbVal))
			{
				// bin it
				NeedBytes(3);
				int nameLen = 1 + settingNameLen;
				int valLen = 1 + GetSettingValueLength(pBlob);
				uint32_t ofsSettingToCut = ofsBlobPtr - nameLen;
				MoveAlong(valLen);
				NeedBytes(1);
				while (pBlob[0]) {
					MoveAlong(pBlob[0] + 1);
					NeedBytes(3);
					MoveAlong(1 + GetSettingValueLength(pBlob));
					NeedBytes(1);
				}
				DBMoveChunk(ofsSettingToCut, ofsSettingToCut + nameLen + valLen, ofsBlobPtr + 1 - ofsSettingToCut);
				ofsBlobPtr -= nameLen + valLen;
				pBlob = (uint8_t*)DBRead(ofsBlobPtr, &bytesRemaining);
			}
			else {
				// replace existing setting at pBlob
				MoveAlong(1);	// skip data type
				switch (dbcws.value.type) {
				case DBVT_BYTE:  DBWrite(ofsBlobPtr, &dbcws.value.bVal, 1); break;
				case DBVT_WORD:  DBWrite(ofsBlobPtr, &dbcws.value.wVal, 2); break;
				case DBVT_DWORD: DBWrite(ofsBlobPtr, &dbcws.value.dVal, 4); break;
				case DBVT_BLOB:
					DBWrite(ofsBlobPtr + 2, dbcws.value.pbVal, dbcws.value.cpbVal);
					break;
				case DBVT_ENCRYPTED:
					DBWrite(ofsBlobPtr + 2, dbcws.value.pbVal, dbcws.value.cpbVal);
					break;
				case DBVT_UTF8:
				case DBVT_ASCIIZ:
					DBWrite(ofsBlobPtr + 2, dbcws.value.pszVal, dbcws.value.cchVal);
					break;
				}
				// quit
				DBFlush(1);
				return 0;
			}
		}
	}

	// cannot do a simple replace, add setting to end of list
	// pBlob already points to end of list
	// see if it fits
	switch (dbcws.value.type) {
	case DBVT_ASCIIZ: case DBVT_UTF8:
		bytesRequired = dbcws.value.cchVal + 2;
		break;
	case DBVT_BLOB: case DBVT_ENCRYPTED:
		bytesRequired = dbcws.value.cpbVal + 2;
		break;
	default:
		bytesRequired = dbcws.value.type;
	}

	bytesRequired += 2 + settingNameLen;
	bytesRequired += ofsBlobPtr + 1 - (ofsSettingsGroup + offsetof(DBContactSettings, blob));

	if ((uint32_t)bytesRequired > dbcs.cbBlob) {
		// doesn't fit: move entire group
		DBContactSettings *dbcsPrev;
		uint32_t ofsDbcsPrev, ofsNew;

		InvalidateSettingsGroupOfsCacheEntry(ofsSettingsGroup);
		bytesRequired += (DB_SETTINGS_RESIZE_GRANULARITY - (bytesRequired % DB_SETTINGS_RESIZE_GRANULARITY)) % DB_SETTINGS_RESIZE_GRANULARITY;
		// find previous group to change its offset
		ofsDbcsPrev = dbc.ofsFirstSettings;
		if (ofsDbcsPrev == ofsSettingsGroup) ofsDbcsPrev = 0;
		else {
			dbcsPrev = (DBContactSettings*)DBRead(ofsDbcsPrev, nullptr);
			while (dbcsPrev->ofsNext != ofsSettingsGroup) {
				if (dbcsPrev->ofsNext == 0) DatabaseCorruption(nullptr);
				ofsDbcsPrev = dbcsPrev->ofsNext;
				dbcsPrev = (DBContactSettings*)DBRead(ofsDbcsPrev, nullptr);
			}
		}

		// create the new one
		ofsNew = ReallocSpace(ofsSettingsGroup, dbcs.cbBlob + offsetof(DBContactSettings, blob), bytesRequired + offsetof(DBContactSettings, blob));

		dbcs.cbBlob = bytesRequired;

		DBWrite(ofsNew, &dbcs, offsetof(DBContactSettings, blob));
		if (ofsDbcsPrev == 0) {
			dbc.ofsFirstSettings = ofsNew;
			DBWrite(ofsContact, &dbc, sizeof(DBContact));
		}
		else {
			dbcsPrev = (DBContactSettings*)DBRead(ofsDbcsPrev, nullptr);
			dbcsPrev->ofsNext = ofsNew;
			DBWrite(ofsDbcsPrev, dbcsPrev, offsetof(DBContactSettings, blob));
		}
		ofsBlobPtr += ofsNew - ofsSettingsGroup;
		ofsSettingsGroup = ofsNew;
		pBlob = (uint8_t*)DBRead(ofsBlobPtr, &bytesRemaining);
	}

	// we now have a place to put it and enough space: make it
	DBWrite(ofsBlobPtr, &settingNameLen, 1);
	DBWrite(ofsBlobPtr + 1, (PVOID)dbcws.szSetting, settingNameLen);
	MoveAlong(1 + settingNameLen);
	DBWrite(ofsBlobPtr, &dbcws.value.type, 1);
	MoveAlong(1);
	switch (dbcws.value.type) {
	case DBVT_BYTE: DBWrite(ofsBlobPtr, &dbcws.value.bVal, 1); MoveAlong(1); break;
	case DBVT_WORD: DBWrite(ofsBlobPtr, &dbcws.value.wVal, 2); MoveAlong(2); break;
	case DBVT_DWORD: DBWrite(ofsBlobPtr, &dbcws.value.dVal, 4); MoveAlong(4); break;

	case DBVT_BLOB:
		DBWrite(ofsBlobPtr, &dbcws.value.cpbVal, 2);
		DBWrite(ofsBlobPtr + 2, dbcws.value.pbVal, dbcws.value.cpbVal);
		MoveAlong(2 + dbcws.value.cpbVal);
		break;

	case DBVT_ENCRYPTED:
		DBWrite(ofsBlobPtr, &dbcws.value.cpbVal, 2);
		DBWrite(ofsBlobPtr + 2, dbcws.value.pbVal, dbcws.value.cpbVal);
		MoveAlong(2 + dbcws.value.cpbVal);
		break;

	case DBVT_UTF8: case DBVT_ASCIIZ:
		DBWrite(ofsBlobPtr, &dbcws.value.cchVal, 2);
		DBWrite(ofsBlobPtr + 2, dbcws.value.pszVal, dbcws.value.cchVal);
		MoveAlong(2 + dbcws.value.cchVal);
		break;
	}

	uint8_t zero = 0;
	DBWrite(ofsBlobPtr, &zero, 1);

	// quit
	DBFlush(1);
	return 0;
}

STDMETHODIMP_(BOOL) CDb3Mmap::DeleteContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting)
{
	if (!szModule || !szSetting)
		return 1;

	// the db format can't tolerate more than 255 bytes of space (incl. null) for settings+module name
	int settingNameLen = (int)mir_strlen(szSetting);
	int moduleNameLen = (int)mir_strlen(szModule);
	if (settingNameLen > 0xFE) {
#ifdef _DEBUG
		OutputDebugStringA("DeleteContactSetting() got a > 255 setting name length. \n");
#endif
		return 1;
	}
	if (moduleNameLen > 0xFE) {
#ifdef _DEBUG
		OutputDebugStringA("DeleteContactSetting() got a > 255 module name length. \n");
#endif
		return 1;
	}

	MCONTACT saveContact = contactID;
	{
		mir_cslock lck(m_csDbAccess);
		char *szCachedSettingName = m_cache->GetCachedSetting(szModule, szSetting, moduleNameLen, settingNameLen);
		if (szCachedSettingName[-1] == 0) { // it's not a resident variable
			uint32_t ofsModuleName = GetModuleNameOfs(szModule);
			uint32_t ofsContact = GetContactOffset(contactID);
			DBContact *dbc = (DBContact*)DBRead(ofsContact, nullptr);
			if (dbc->signature != DBCONTACT_SIGNATURE)
				return 1;

			// make sure the module group exists
			uint32_t ofsSettingsGroup = GetSettingsGroupOfsByModuleNameOfs(dbc, ofsModuleName);
			if (ofsSettingsGroup == 0)
				return 1;

			// find if the setting exists
			uint32_t ofsBlobPtr = ofsSettingsGroup + offsetof(DBContactSettings, blob);
			int bytesRemaining;
			uint8_t *pBlob = (uint8_t*)DBRead(ofsBlobPtr, &bytesRemaining);
			while (pBlob[0]) {
				NeedBytes(settingNameLen + 1);
				if (pBlob[0] == settingNameLen && !memcmp(pBlob + 1, szSetting, settingNameLen))
					break;
				NeedBytes(1);
				MoveAlong(pBlob[0] + 1);
				NeedBytes(3);
				MoveAlong(1 + GetSettingValueLength(pBlob));
				NeedBytes(1);
			}
			if (!pBlob[0]) //setting didn't exist
				return 1;

			// bin it
			MoveAlong(1 + settingNameLen);
			NeedBytes(3);
			int nameLen = 1 + settingNameLen;
			int valLen = 1 + GetSettingValueLength(pBlob);
			uint32_t ofsSettingToCut = ofsBlobPtr - nameLen;
			MoveAlong(valLen);
			NeedBytes(1);
			while (pBlob[0]) {
				MoveAlong(pBlob[0] + 1);
				NeedBytes(3);
				MoveAlong(1 + GetSettingValueLength(pBlob));
				NeedBytes(1);
			}
			DBMoveChunk(ofsSettingToCut, ofsSettingToCut + nameLen + valLen, ofsBlobPtr + 1 - ofsSettingToCut);
			DBFlush(1);

			// remove a value from cache anyway
			m_cache->GetCachedValuePtr(saveContact, szCachedSettingName, -1);
		}
		else { // resident variable
			// if a value doesn't exist, simply return error
			if (m_cache->GetCachedValuePtr(saveContact, szCachedSettingName, -1) == nullptr)
				return 1;
		}
	}

	// notify
	DBCONTACTWRITESETTING dbcws = {};
	dbcws.szModule = szModule;
	dbcws.szSetting = szSetting;
	dbcws.value.type = DBVT_DELETED;
	NotifyEventHooks(g_hevSettingChanged, saveContact, (LPARAM)&dbcws);
	return 0;
}

STDMETHODIMP_(BOOL) CDb3Mmap::EnumContactSettings(MCONTACT contactID, DBSETTINGENUMPROC pfnEnumProc, const char *szModule, void *param)
{
	if (!szModule)
		return -1;

	mir_cslock lck(m_csDbAccess);

	uint32_t ofsContact = GetContactOffset(contactID);
	if (ofsContact == 0)
		return -1;

	DBContact *dbc = (DBContact*)DBRead(ofsContact, nullptr);
	if (dbc->signature != DBCONTACT_SIGNATURE)
		return -1;

	uint32_t ofsModuleName = GetModuleNameOfs(szModule);
	uint32_t ofsSettings = GetSettingsGroupOfsByModuleNameOfs(dbc, ofsModuleName);
	if (!ofsSettings)
		return -1;

	uint32_t ofsBlobPtr = ofsSettings + offsetof(DBContactSettings, blob);
	int bytesRemaining;
	uint8_t *pBlob = (uint8_t*)DBRead(ofsBlobPtr, &bytesRemaining);
	if (pBlob[0] == 0)
		return -1;

	int result = 0;
	while (pBlob[0]) {
		NeedBytes(1);
		NeedBytes(1 + pBlob[0]);
		char szSetting[256];
		memcpy(szSetting, pBlob + 1, pBlob[0]); szSetting[pBlob[0]] = 0;
		result = pfnEnumProc(szSetting, param);
		MoveAlong(1 + pBlob[0]);
		NeedBytes(3);
		MoveAlong(1 + GetSettingValueLength(pBlob));
		NeedBytes(1);
	}
	return result;
}
