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

#define VLT(n) ((n == DBVT_UTF8 || n == DBVT_ENCRYPTED)?DBVT_ASCIIZ:n)

static bool ValidLookupName(LPCSTR szModule, LPCSTR szSetting)
{
	if (!strcmp(szModule, META_PROTO))
		return strcmp(szSetting, "IsSubcontact") && strcmp(szSetting, "ParentMetaID");

	if (!strcmp(szModule, "Ignore"))
		return false;

	return true;
}

int CDbxMDBX::GetContactSettingWorker(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, int isStatic)
{
	if (szSetting == nullptr || szModule == nullptr)
		return 1;

	size_t settingNameLen = strlen(szSetting);
	size_t moduleNameLen = strlen(szModule);

	mir_cslock lck(m_csDbAccess);

LBL_Seek:
	char *szCachedSettingName = m_cache->GetCachedSetting(szModule, szSetting, moduleNameLen, settingNameLen);

	DBVARIANT *pCachedValue = m_cache->GetCachedValuePtr(contactID, szCachedSettingName, 0);
	if (pCachedValue != nullptr) {
		if (pCachedValue->type == DBVT_ASCIIZ || pCachedValue->type == DBVT_UTF8) {
			int cbOrigLen = dbv->cchVal;
			char *cbOrigPtr = dbv->pszVal;
			memcpy(dbv, pCachedValue, sizeof(DBVARIANT));
			if (isStatic) {
				int cbLen = 0;
				if (pCachedValue->pszVal != nullptr)
					cbLen = (int)strlen(pCachedValue->pszVal);

				cbOrigLen--;
				dbv->pszVal = cbOrigPtr;
				if (cbLen < cbOrigLen)
					cbOrigLen = cbLen;
				memcpy(dbv->pszVal, pCachedValue->pszVal, cbOrigLen);
				dbv->pszVal[cbOrigLen] = 0;
				dbv->cchVal = cbLen;
			}
			else {
				dbv->pszVal = (char*)mir_alloc(strlen(pCachedValue->pszVal) + 1);
				strcpy(dbv->pszVal, pCachedValue->pszVal);
			}
		}
		else memcpy(dbv, pCachedValue, sizeof(DBVARIANT));

		return (pCachedValue->type == DBVT_DELETED) ? 1 : 0;
	}

	// never look db for the resident variable
	if (szCachedSettingName[-1] != 0)
		return 1;

	DBCachedContact *cc = (contactID) ? m_cache->GetCachedContact(contactID) : nullptr;

	DBSettingKey *keyVal = (DBSettingKey *)_alloca(sizeof(DBSettingKey) + settingNameLen);
	keyVal->hContact = contactID;
	keyVal->dwModuleId = GetModuleID(szModule);
	memcpy(&keyVal->szSettingName, szSetting, settingNameLen + 1);

	int res;
	const BYTE *pBlob;
	{
		txn_ptr_ro trnlck(m_txn_ro);
		MDBX_val key = { keyVal,  sizeof(DBSettingKey) + settingNameLen }, data;
		res = mdbx_get(trnlck, m_dbSettings, &key, &data);
		pBlob = (const BYTE*)data.iov_base;
	}

	if (res != MDBX_SUCCESS) {
		// try to get the missing mc setting from the active sub
		if (cc && cc->IsMeta() && ValidLookupName(szModule, szSetting)) {
			if (contactID = db_mc_getDefault(contactID)) {
				if (szModule = Proto_GetBaseAccountName(contactID)) {
					moduleNameLen = strlen(szModule);
					goto LBL_Seek;
				}
			}
		}
		return 1;
	}

	if (isStatic && (pBlob[0] & DBVTF_VARIABLELENGTH) && VLT(dbv->type) != VLT(pBlob[0]))
		return 1;

	int varLen;
	BYTE iType = dbv->type = pBlob[0]; pBlob++;
	switch (iType) {
	case DBVT_DELETED: /* this setting is deleted */
		dbv->type = DBVT_DELETED;
		return 2;

	case DBVT_BYTE:  dbv->bVal = *pBlob; break;
	case DBVT_WORD:  dbv->wVal = *(WORD*)pBlob; break;
	case DBVT_DWORD: dbv->dVal = *(DWORD*)pBlob; break;

	case DBVT_UTF8:
	case DBVT_ASCIIZ:
		varLen = *(WORD*)pBlob;
		pBlob += 2;
		if (isStatic) {
			dbv->cchVal--;
			if (varLen < dbv->cchVal)
				dbv->cchVal = varLen;
			memcpy(dbv->pszVal, pBlob, dbv->cchVal); // decode
			dbv->pszVal[dbv->cchVal] = 0;
			dbv->cchVal = varLen;
		}
		else {
			dbv->pszVal = (char*)mir_alloc(1 + varLen);
			memcpy(dbv->pszVal, pBlob, varLen);
			dbv->pszVal[varLen] = 0;
		}
		break;

	case DBVT_BLOB:
		varLen = *(WORD*)pBlob;
		pBlob += 2;
		if (isStatic) {
			if (varLen < dbv->cpbVal)
				dbv->cpbVal = varLen;
			memcpy(dbv->pbVal, pBlob, dbv->cpbVal);
		}
		else {
			dbv->pbVal = (BYTE *)mir_alloc(varLen);
			memcpy(dbv->pbVal, pBlob, varLen);
		}
		dbv->cpbVal = varLen;
		break;

	case DBVT_ENCRYPTED:
		if (m_crypto == nullptr)
			return 1;

		varLen = *(WORD*)pBlob;
		pBlob += 2;

		size_t realLen;
		ptrA decoded(m_crypto->decodeString(pBlob, varLen, &realLen));
		if (decoded == nullptr)
			return 1;

		varLen = (WORD)realLen;
		dbv->type = DBVT_UTF8;
		if (isStatic) {
			dbv->cchVal--;
			if (varLen < dbv->cchVal)
				dbv->cchVal = varLen;
			memcpy(dbv->pszVal, decoded, dbv->cchVal);
			dbv->pszVal[dbv->cchVal] = 0;
			dbv->cchVal = varLen;
		}
		else {
			dbv->pszVal = (char*)mir_alloc(1 + varLen);
			memcpy(dbv->pszVal, decoded, varLen);
			dbv->pszVal[varLen] = 0;
		}
		break;
	}

	/**** add to cache **********************/
	if (iType != DBVT_BLOB && iType != DBVT_ENCRYPTED) {
		pCachedValue = m_cache->GetCachedValuePtr(contactID, szCachedSettingName, 1);
		if (pCachedValue != nullptr)
			m_cache->SetCachedVariant(dbv, pCachedValue);
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

	case DBVT_ASCIIZ: case DBVT_UTF8:
		bIsEncrypted = m_bEncrypted || IsSettingEncrypted(dbcws->szModule, dbcws->szSetting);
LBL_WriteString:
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

	case DBVT_UNENCRYPTED:
		dbcwNotif.value.type = dbcwWork.value.type = DBVT_UTF8;
		goto LBL_WriteString;

	case DBVT_BLOB: case DBVT_ENCRYPTED:
		if (dbcwWork.value.pbVal == nullptr)
			return 1;
		break;
	default:
		return 1;
	}

	mir_cslockfull lck(m_csDbAccess);
	char *szCachedSettingName = m_cache->GetCachedSetting(dbcwWork.szModule, dbcwWork.szSetting, moduleNameLen, settingNameLen);

	// we don't cache blobs and passwords
	if (dbcwWork.value.type != DBVT_BLOB && dbcwWork.value.type != DBVT_ENCRYPTED && !bIsEncrypted) {
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
				}
				if (bIsIdentical)
					return 0;
			}
			m_cache->SetCachedVariant(&dbcwWork.value, pCachedValue);
		}
		if (szCachedSettingName[-1] != 0) {
			lck.unlock();
			NotifyEventHooks(g_hevSettingChanged, contactID, (LPARAM)&dbcwWork);
			return 0;
		}
	}
	else m_cache->GetCachedValuePtr(contactID, szCachedSettingName, -1);

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
	{
		DBSettingKey keyVal = { hContact, GetModuleID(szModule), 0 };
		MDBX_val key = { &keyVal, sizeof(keyVal) }, data;

		txn_ptr_ro txn(m_txn_ro);
		cursor_ptr pCursor(m_txn_ro, m_dbSettings);

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
