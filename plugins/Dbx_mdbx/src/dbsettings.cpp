/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-20 Miranda NG team (https://miranda-ng.org)
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

void CDbxMDBX::FillSettings()
{
	cursor_ptr pCursor(StartTran(), m_dbSettings);

	DBSettingKey keyVal = {};
	MDBX_val key = { &keyVal, sizeof(keyVal) }, data;
	for (int res = mdbx_cursor_get(pCursor, &key, &data, MDBX_SET_RANGE); res == MDBX_SUCCESS; res = mdbx_cursor_get(pCursor, &key, &data, MDBX_NEXT)) {
		const DBSettingKey *pKey = (const DBSettingKey*)key.iov_base;

		auto *szModule = GetModuleName(pKey->dwModuleId);
		if (szModule == nullptr)
			continue;

		const BYTE *pBlob = (const BYTE*)data.iov_base;
		if (*pBlob == DBVT_DELETED)
			continue;

		size_t settingNameLen = strlen(pKey->szSettingName);
		size_t moduleNameLen = strlen(szModule);

		char *szCachedSettingName = m_cache->GetCachedSetting(szModule, pKey->szSettingName, moduleNameLen, settingNameLen);

		DBVARIANT *dbv = m_cache->GetCachedValuePtr(pKey->hContact, szCachedSettingName, 1);
		if (dbv == nullptr) // garbage! a setting for removed/non-existent contact
			continue;

		WORD varLen;

		BYTE iType = dbv->type = pBlob[0]; pBlob++;
		switch (iType) {
		case DBVT_BYTE:  dbv->bVal = *pBlob; break;
		case DBVT_WORD:  dbv->wVal = *(WORD*)pBlob; break;
		case DBVT_DWORD: dbv->dVal = *(DWORD*)pBlob; break;

		case DBVT_UTF8:
		case DBVT_ASCIIZ:
			varLen = *(WORD*)pBlob;
			pBlob += 2;
			dbv->pszVal = (char*)mir_alloc(1 + varLen);
			memcpy(dbv->pszVal, pBlob, varLen);
			dbv->pszVal[varLen] = 0;
			break;

		case DBVT_BLOB:
		case DBVT_ENCRYPTED:
			varLen = *(WORD*)pBlob;
			pBlob += 2;
			dbv->pbVal = (BYTE *)mir_alloc(varLen);
			memcpy(dbv->pbVal, pBlob, varLen);
			dbv->cpbVal = varLen;
			break;
		}
	}

	for (DBCachedContact *cc = m_cache->GetFirstContact(); cc; cc = m_cache->GetNextContact(cc->contactID)) {
		CheckProto(cc, "");

		DBVARIANT dbv; dbv.type = DBVT_DWORD;
		cc->nSubs = (0 != GetContactSetting(cc->contactID, META_PROTO, "NumContacts", &dbv)) ? -1 : dbv.dVal;
		if (cc->nSubs != -1) {
			cc->pSubs = (MCONTACT*)mir_alloc(cc->nSubs * sizeof(MCONTACT));
			for (int k = 0; k < cc->nSubs; k++) {
				char setting[100];
				mir_snprintf(setting, _countof(setting), "Handle%d", k);
				cc->pSubs[k] = (0 != GetContactSetting(cc->contactID, META_PROTO, setting, &dbv)) ? 0 : dbv.dVal;
			}
		}
		cc->nDefault = (0 != GetContactSetting(cc->contactID, META_PROTO, "Default", &dbv)) ? -1 : dbv.dVal;
		cc->parentID = (0 != GetContactSetting(cc->contactID, META_PROTO, "ParentMeta", &dbv)) ? 0 : dbv.dVal;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

#define VLT(n) ((n == DBVT_UTF8 || n == DBVT_ENCRYPTED)?DBVT_ASCIIZ:n)

static bool ValidLookupName(LPCSTR szModule, LPCSTR szSetting)
{
	if (!strcmp(szModule, META_PROTO))
		return strcmp(szSetting, "IsSubcontact") && strcmp(szSetting, "ParentMetaID");

	return false;
}

int CDbxMDBX::GetContactSettingWorker(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, int isStatic)
{
	if (szSetting == nullptr || szModule == nullptr)
		return 1;

	DBVARIANT *pCachedValue;
	size_t settingNameLen = strlen(szSetting);
	size_t moduleNameLen = strlen(szModule);
	{
		mir_cslock lck(m_csDbAccess);

LBL_Seek:
		char *szCachedSettingName = m_cache->GetCachedSetting(szModule, szSetting, moduleNameLen, settingNameLen);

		pCachedValue = m_cache->GetCachedValuePtr(contactID, szCachedSettingName, 0);
		if (pCachedValue == nullptr) {
			// if nothing was faound, try to lookup the same setting from meta's default contact
			if (contactID) {
				DBCachedContact *cc = m_cache->GetCachedContact(contactID);
				if (cc && cc->IsMeta() && ValidLookupName(szModule, szSetting)) {
					if (contactID = db_mc_getDefault(contactID)) {
						szModule = Proto_GetBaseAccountName(contactID);
						moduleNameLen = strlen(szModule);
						goto LBL_Seek;
					}
				}
			}

			// otherwise fail
			return 1;
		}
	}

	switch(pCachedValue->type) {
	case DBVT_ASCIIZ:
	case DBVT_UTF8:
		dbv->type = pCachedValue->type;
		if (isStatic) {
			int cbLen = (int)mir_strlen(pCachedValue->pszVal);
			int cbOrigLen = dbv->cchVal;
			cbOrigLen--;
			if (cbLen < cbOrigLen)
				cbOrigLen = cbLen;
			memcpy(dbv->pszVal, pCachedValue->pszVal, cbOrigLen);
			dbv->pszVal[cbOrigLen] = 0;
			dbv->cchVal = cbLen;
		}
		else {
			dbv->pszVal = (char *)mir_alloc(strlen(pCachedValue->pszVal) + 1);
			strcpy(dbv->pszVal, pCachedValue->pszVal);
			dbv->cchVal = pCachedValue->cchVal;
		}
		break;

	case DBVT_BLOB:
		dbv->type = DBVT_BLOB;
		if (isStatic) {
			if (pCachedValue->cpbVal < dbv->cpbVal)
				dbv->cpbVal = pCachedValue->cpbVal;
			memcpy(dbv->pbVal, pCachedValue->pbVal, dbv->cpbVal);
		}
		else {
			dbv->pbVal = (BYTE *)mir_alloc(pCachedValue->cpbVal);
			memcpy(dbv->pbVal, pCachedValue->pbVal, pCachedValue->cpbVal);
		}
		dbv->cpbVal = pCachedValue->cpbVal;
		break;

	case DBVT_ENCRYPTED:
		if (m_crypto != nullptr) {
			size_t realLen;
			ptrA decoded(m_crypto->decodeString(pCachedValue->pbVal, pCachedValue->cpbVal, &realLen));
			if (decoded == nullptr)
				return 1;

			dbv->type = DBVT_UTF8;
			if (isStatic) {
				dbv->cchVal--;
				if (realLen < dbv->cchVal)
					dbv->cchVal = WORD(realLen);
				memcpy(dbv->pszVal, decoded, dbv->cchVal);
				dbv->pszVal[dbv->cchVal] = 0;
				dbv->cchVal = WORD(realLen);
			}
			else {
				dbv->pszVal = (char *)mir_alloc(1 + realLen);
				memcpy(dbv->pszVal, decoded, realLen);
				dbv->pszVal[realLen] = 0;
			}
			break;
		}
		return 1;

	default:
		memcpy(dbv, pCachedValue, sizeof(DBVARIANT));
	}

	return 0;
}

BOOL CDbxMDBX::WriteContactSetting(MCONTACT contactID, DBCONTACTWRITESETTING *dbcws)
{
	if (dbcws == nullptr || dbcws->szSetting == nullptr || dbcws->szModule == nullptr || m_bReadOnly)
		return 1;

	// the db format can't tolerate more than 255 bytes of space (incl. null) for settings+module name
	size_t settingNameLen = strlen(dbcws->szSetting);
	size_t moduleNameLen = strlen(dbcws->szModule);

	// used for notifications
	DBCONTACTWRITESETTING dbcwNotif = *dbcws;
	if (dbcwNotif.value.type == DBVT_WCHAR) {
		if (dbcwNotif.value.pszVal != nullptr) {
			T2Utf val(dbcwNotif.value.pwszVal);
			if (!val)
				return 1;

			dbcwNotif.value.pszVal = NEWSTR_ALLOCA(val);
			dbcwNotif.value.type = DBVT_UTF8;
		}
		else return 1;
	}

	if (dbcwNotif.szModule == nullptr || dbcwNotif.szSetting == nullptr)
		return 1;

	DBCONTACTWRITESETTING dbcwWork = dbcwNotif;

	mir_ptr<BYTE> pEncoded(nullptr);
	bool bIsEncrypted = false;
	switch (dbcwWork.value.type) {
	case DBVT_BYTE: case DBVT_WORD: case DBVT_DWORD:
		break;

	case DBVT_ASCIIZ:
	case DBVT_UTF8:
		bIsEncrypted = m_bEncrypted || IsSettingEncrypted(dbcws->szModule, dbcws->szSetting);
		if (dbcwWork.value.pszVal == nullptr)
			return 1;

		dbcwWork.value.cchVal = (WORD)strlen(dbcwWork.value.pszVal);
		if (bIsEncrypted) {
			size_t len;
			BYTE *pResult = m_crypto->encodeString(dbcwWork.value.pszVal, &len);
			if (pResult != nullptr) {
				pEncoded = dbcwWork.value.pbVal = pResult;
				dbcwWork.value.cpbVal = (WORD)len;
				dbcwWork.value.type = DBVT_ENCRYPTED;
			}
		}
		break;

	case DBVT_BLOB:
	case DBVT_ENCRYPTED:
		if (dbcwWork.value.pbVal == nullptr)
			return 1;
		break;

	default:
		return 1;
	}

	mir_cslockfull lck(m_csDbAccess);
	char *szCachedSettingName = m_cache->GetCachedSetting(dbcwWork.szModule, dbcwWork.szSetting, moduleNameLen, settingNameLen);

	DBVARIANT *pCachedValue = m_cache->GetCachedValuePtr(contactID, szCachedSettingName, 1);
	if (pCachedValue != nullptr) {
		bool bIsIdentical = false;
		if (pCachedValue->type == dbcwWork.value.type) {
			switch (dbcwWork.value.type) {
			case DBVT_BYTE:   bIsIdentical = pCachedValue->bVal == dbcwWork.value.bVal;  break;
			case DBVT_WORD:   bIsIdentical = pCachedValue->wVal == dbcwWork.value.wVal;  break;
			case DBVT_DWORD:  bIsIdentical = pCachedValue->dVal == dbcwWork.value.dVal;  break;
			case DBVT_UTF8:
			case DBVT_ASCIIZ: bIsIdentical = strcmp(pCachedValue->pszVal, dbcwWork.value.pszVal) == 0; break;
			case DBVT_BLOB:
			case DBVT_ENCRYPTED:
				if (pCachedValue->cpbVal == dbcwWork.value.cchVal)
					bIsIdentical = memcmp(pCachedValue->pbVal, dbcwWork.value.pbVal, dbcwWork.value.cchVal);
				break;
			}
			if (bIsIdentical)
				return 0;
		}
		m_cache->SetCachedVariant(&dbcwWork.value, pCachedValue);
	}

	// for resident settings we simply emulate change hook and return
	if (szCachedSettingName[-1] != 0) {
		lck.unlock();
		NotifyEventHooks(g_hevSettingChanged, contactID, (LPARAM)&dbcwNotif);
		return 0;
	}

	// write down a setting to database
	DBSettingKey *keyVal = (DBSettingKey *)_alloca(sizeof(DBSettingKey) + settingNameLen);
	keyVal->hContact = contactID;
	keyVal->dwModuleId = GetModuleID(dbcws->szModule);
	memcpy(&keyVal->szSettingName, dbcws->szSetting, settingNameLen + 1);

	MDBX_val key = { keyVal,  sizeof(DBSettingKey) + settingNameLen }, data;

	switch (dbcwWork.value.type) {
	case DBVT_BYTE:  data.iov_len = 2; break;
	case DBVT_WORD:  data.iov_len = 3; break;
	case DBVT_DWORD: data.iov_len = 5; break;

	case DBVT_ASCIIZ:
	case DBVT_UTF8:
		data.iov_len = 3 + dbcwWork.value.cchVal; break;

	case DBVT_BLOB:
	case DBVT_ENCRYPTED:
		data.iov_len = 3 + dbcwWork.value.cpbVal; break;

	default:
		return 1;
	}

	data.iov_base = _alloca(data.iov_len);

	BYTE *pBlob = (BYTE*)data.iov_base;
	*pBlob++ = dbcwWork.value.type;
	switch (dbcwWork.value.type) {
	case DBVT_BYTE:  *pBlob = dbcwWork.value.bVal; break;
	case DBVT_WORD:  *(WORD*)pBlob = dbcwWork.value.wVal; break;
	case DBVT_DWORD: *(DWORD*)pBlob = dbcwWork.value.dVal; break;

	case DBVT_ASCIIZ:
	case DBVT_UTF8:
		*(WORD*)pBlob = dbcwWork.value.cchVal;
		pBlob += 2;
		memcpy(pBlob, dbcwWork.value.pszVal, dbcwWork.value.cchVal);
		break;

	case DBVT_BLOB:
	case DBVT_ENCRYPTED:
		*(WORD*)pBlob = dbcwWork.value.cpbVal;
		pBlob += 2;
		memcpy(pBlob, dbcwWork.value.pbVal, dbcwWork.value.cpbVal);
	}

	{
		txn_ptr trnlck(this);
		if (mdbx_put(trnlck, m_dbSettings, &key, &data, MDBX_UPSERT) != MDBX_SUCCESS)
			return 1;

		if (trnlck.Commit() != MDBX_SUCCESS)
			return 1;
	}

	// notify
	lck.unlock();

	DBFlush();
	NotifyEventHooks(g_hevSettingChanged, contactID, (LPARAM)&dbcwNotif);
	return 0;
}

BOOL CDbxMDBX::DeleteContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting)
{
	if (!szModule || !szSetting)
		return 1;

	size_t settingNameLen = strlen(szSetting);
	size_t moduleNameLen = strlen(szModule);
	{
		mir_cslock lck(m_csDbAccess);
		char *szCachedSettingName = m_cache->GetCachedSetting(szModule, szSetting, moduleNameLen, settingNameLen);

		if (szCachedSettingName[-1] == 0)  // it's not a resident variable
		{
			DBSettingKey *keyVal = (DBSettingKey*)_alloca(sizeof(DBSettingKey) + settingNameLen);
			keyVal->hContact = contactID;
			keyVal->dwModuleId = GetModuleID(szModule);
			memcpy(&keyVal->szSettingName, szSetting, settingNameLen + 1);

			txn_ptr trnlck(this);
			MDBX_val key = { keyVal,  sizeof(DBSettingKey) + settingNameLen };
			if (mdbx_del(trnlck, m_dbSettings, &key, nullptr) != MDBX_SUCCESS)
				return 1;
			if (trnlck.Commit() != MDBX_SUCCESS)
				return 1;
		}

		m_cache->GetCachedValuePtr(contactID, szCachedSettingName, -1);
	}

	DBFlush();

	// notify
	DBCONTACTWRITESETTING dbcws = { 0 };
	dbcws.szModule = szModule;
	dbcws.szSetting = szSetting;
	dbcws.value.type = DBVT_DELETED;
	NotifyEventHooks(g_hevSettingChanged, contactID, (LPARAM)&dbcws);
	return 0;
}

BOOL CDbxMDBX::EnumContactSettings(MCONTACT hContact, DBSETTINGENUMPROC pfnEnumProc, const char *szModule, void *param)
{
	LIST<char> arKeys(100);

	DBSettingKey keyVal = { hContact, GetModuleID(szModule), 0 };
	MDBX_val key = { &keyVal, sizeof(keyVal) }, data;
	{
		cursor_ptr pCursor(StartTran(), m_dbSettings);

		for (int res = mdbx_cursor_get(pCursor, &key, &data, MDBX_SET_RANGE); res == MDBX_SUCCESS; res = mdbx_cursor_get(pCursor, &key, &data, MDBX_NEXT)) {
			const DBSettingKey *pKey = (const DBSettingKey*)key.iov_base;
			if (pKey->hContact != hContact || pKey->dwModuleId != keyVal.dwModuleId)
				break;

			arKeys.insert((char*)pKey->szSettingName);
		}
	}

	int result = -1;
	for (auto &it : arKeys)
		result = pfnEnumProc(it, param);

	return result;
}
