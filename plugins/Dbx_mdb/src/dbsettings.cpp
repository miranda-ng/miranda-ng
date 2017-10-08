/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org)
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

BOOL CDbxMdb::IsSettingEncrypted(LPCSTR szModule, LPCSTR szSetting)
{
	if (!_strnicmp(szSetting, "password", 8))      return true;
	if (!strcmp(szSetting, "NLProxyAuthPassword")) return true;
	if (!strcmp(szSetting, "LNPassword"))          return true;
	if (!strcmp(szSetting, "FileProxyPassword"))   return true;
	if (!strcmp(szSetting, "TokenSecret"))         return true;

	if (!strcmp(szModule, "SecureIM")) {
		if (!strcmp(szSetting, "pgp"))              return true;
		if (!strcmp(szSetting, "pgpPrivKey"))       return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool ValidLookupName(LPCSTR szModule, LPCSTR szSetting)
{
	if (!strcmp(szModule, META_PROTO))
		return strcmp(szSetting, "IsSubcontact") && strcmp(szSetting, "ParentMetaID");

	if (!strcmp(szModule, "Ignore"))
		return false;

	return true;
}

int CDbxMdb::GetContactSettingWorker(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, int isStatic)
{
	if (szSetting == NULL || szModule == NULL)
		return 1;

	size_t settingNameLen = strlen(szSetting);
	size_t moduleNameLen = strlen(szModule);

LBL_Seek:
	char *szCachedSettingName = m_cache->GetCachedSetting(szModule, szSetting, moduleNameLen, settingNameLen);

	DBVARIANT *pCachedValue = m_cache->GetCachedValuePtr(contactID, szCachedSettingName, 0);
	if (pCachedValue != NULL) 
	{
		if (pCachedValue->type == DBVT_ASCIIZ || pCachedValue->type == DBVT_UTF8) 
		{
			int cbOrigLen = dbv->cchVal;
			char *cbOrigPtr = dbv->pszVal;
			memcpy(dbv, pCachedValue, sizeof(DBVARIANT));
			if (isStatic) 
			{
				int cbLen = 0;
				if (pCachedValue->pszVal != NULL)
					cbLen = (int)strlen(pCachedValue->pszVal);

				cbOrigLen--;
				dbv->pszVal = cbOrigPtr;
				if (cbLen < cbOrigLen)
					cbOrigLen = cbLen;
				memcpy(dbv->pszVal, pCachedValue->pszVal, cbOrigLen);
				dbv->pszVal[cbOrigLen] = 0;
				dbv->cchVal = cbLen;
			}
			else 
			{
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

	DBCachedContact *cc = (contactID) ? m_cache->GetCachedContact(contactID) : NULL;

	txn_ptr_ro trnlck(m_txn);

	DBSettingKey *keyVal = (DBSettingKey *)_alloca(sizeof(DBSettingKey) + settingNameLen + 1);
	keyVal->hContact = contactID;
	keyVal->dwModuleId = GetModuleID(szModule);
	memcpy(&keyVal->szSettingName, szSetting, settingNameLen + 1);


	MDBX_val key = { keyVal,  sizeof(DBSettingKey) + settingNameLen + 1 }, data;
	if (mdbx_get(trnlck, m_dbSettings, &key, &data) != MDBX_SUCCESS) 
	{
		// try to get the missing mc setting from the active sub
		if (cc && cc->IsMeta() && ValidLookupName(szModule, szSetting)) 
		{
			if (contactID = db_mc_getDefault(contactID))
			{
				if (szModule = GetContactProto(contactID)) 
				{
					moduleNameLen = strlen(szModule);
					goto LBL_Seek;
				}
			}
		}
		return 1;
	}

	const BYTE *pBlob = (const BYTE*)data.iov_base;
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
		if (isStatic) 
		{
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
		if (isStatic) 
		{
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
		if (m_crypto == NULL)
			return 1;

		varLen = *(WORD*)pBlob;
		pBlob += 2;
		
		size_t realLen;
		ptrA decoded(m_crypto->decodeString(pBlob, varLen, &realLen));
		if (decoded == NULL)
			return 1;

		varLen = (WORD)realLen;
		dbv->type = DBVT_UTF8;
		if (isStatic) 
		{
			dbv->cchVal--;
			if (varLen < dbv->cchVal)
				dbv->cchVal = varLen;
			memcpy(dbv->pszVal, decoded, dbv->cchVal);
			dbv->pszVal[dbv->cchVal] = 0;
			dbv->cchVal = varLen;
		}
		else 
		{
			dbv->pszVal = (char*)mir_alloc(1 + varLen);
			memcpy(dbv->pszVal, decoded, varLen);
			dbv->pszVal[varLen] = 0;
		}
		break;
	}

	/**** add to cache **********************/
	if (iType != DBVT_BLOB && iType != DBVT_ENCRYPTED) {
		pCachedValue = m_cache->GetCachedValuePtr(contactID, szCachedSettingName, 1);
		if (pCachedValue != NULL)
			m_cache->SetCachedVariant(dbv, pCachedValue);
	}

	return 0;
}

STDMETHODIMP_(BOOL) CDbxMdb::GetContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv)
{
	dbv->type = 0;
	if (GetContactSettingWorker(contactID, szModule, szSetting, dbv, 0))
		return 1;

	if (dbv->type == DBVT_UTF8) 
	{
		WCHAR *tmp = NULL;
		char *p = NEWSTR_ALLOCA(dbv->pszVal);
		if (mir_utf8decode(p, &tmp) != NULL) 
		{
			BOOL bUsed = FALSE;
			int  result = WideCharToMultiByte(m_codePage, WC_NO_BEST_FIT_CHARS, tmp, -1, NULL, 0, NULL, &bUsed);

			mir_free(dbv->pszVal);

			if (bUsed || result == 0) {
				dbv->type = DBVT_WCHAR;
				dbv->pwszVal = tmp;
			}
			else {
				dbv->type = DBVT_ASCIIZ;
				dbv->pszVal = (char *)mir_alloc(result);
				WideCharToMultiByte(m_codePage, WC_NO_BEST_FIT_CHARS, tmp, -1, dbv->pszVal, result, NULL, NULL);
				mir_free(tmp);
			}
		}
		else 
		{
			dbv->type = DBVT_ASCIIZ;
			mir_free(tmp);
		}
	}

	return 0;
}

STDMETHODIMP_(BOOL) CDbxMdb::GetContactSettingStr(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv)
{
	int iSaveType = dbv->type;

	if (GetContactSettingWorker(contactID, szModule, szSetting, dbv, 0))
		return 1;

	if (iSaveType == 0 || iSaveType == dbv->type)
		return 0;

	if (dbv->type != DBVT_ASCIIZ && dbv->type != DBVT_UTF8)
		return 1;

	if (iSaveType == DBVT_WCHAR) {
		if (dbv->type != DBVT_UTF8) {
			int len = MultiByteToWideChar(CP_ACP, 0, dbv->pszVal, -1, NULL, 0);
			wchar_t *wszResult = (wchar_t*)mir_alloc((len + 1)*sizeof(wchar_t));
			if (wszResult == NULL)
				return 1;

			MultiByteToWideChar(CP_ACP, 0, dbv->pszVal, -1, wszResult, len);
			wszResult[len] = 0;
			mir_free(dbv->pszVal);
			dbv->pwszVal = wszResult;
		}
		else {
			char* savePtr = NEWSTR_ALLOCA(dbv->pszVal);
			mir_free(dbv->pszVal);
			if (!mir_utf8decode(savePtr, &dbv->pwszVal))
				return 1;
		}
	}
	else if (iSaveType == DBVT_UTF8) {
		char* tmpBuf = mir_utf8encode(dbv->pszVal);
		if (tmpBuf == NULL)
			return 1;

		mir_free(dbv->pszVal);
		dbv->pszVal = tmpBuf;
	}
	else if (iSaveType == DBVT_ASCIIZ)
		mir_utf8decode(dbv->pszVal, NULL);

	dbv->type = iSaveType;
	return 0;
}

STDMETHODIMP_(BOOL) CDbxMdb::GetContactSettingStatic(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv)
{
	if (GetContactSettingWorker(contactID, szModule, szSetting, dbv, 1))
		return 1;

	if (dbv->type == DBVT_UTF8) {
		mir_utf8decode(dbv->pszVal, NULL);
		dbv->type = DBVT_ASCIIZ;
	}

	return 0;
}

STDMETHODIMP_(BOOL) CDbxMdb::FreeVariant(DBVARIANT *dbv)
{
	if (dbv == 0) return 1;

	switch (dbv->type) {
	case DBVT_ASCIIZ:
	case DBVT_UTF8:
	case DBVT_WCHAR:
		if (dbv->pszVal) mir_free(dbv->pszVal);
		dbv->pszVal = 0;
		break;
	case DBVT_BLOB:
		if (dbv->pbVal) mir_free(dbv->pbVal);
		dbv->pbVal = 0;
		break;
	}
	dbv->type = 0;
	return 0;
}

STDMETHODIMP_(BOOL) CDbxMdb::SetSettingResident(BOOL bIsResident, const char *pszSettingName)
{
	char *szSetting = m_cache->GetCachedSetting(NULL, pszSettingName, 0, (int)strlen(pszSettingName));
	szSetting[-1] = (char)bIsResident;

	int idx = m_lResidentSettings.getIndex(szSetting);
	if (idx == -1) {
		if (bIsResident)
			m_lResidentSettings.insert(szSetting);
	}
	else if (!bIsResident)
		m_lResidentSettings.remove(idx);

	return 0;
}

STDMETHODIMP_(BOOL) CDbxMdb::WriteContactSetting(MCONTACT contactID, DBCONTACTWRITESETTING *dbcws)
{
	if (dbcws == NULL || dbcws->szSetting == NULL || dbcws->szModule == NULL || m_bReadOnly)
		return 1;

	// the db format can't tolerate more than 255 bytes of space (incl. null) for settings+module name
	size_t settingNameLen = strlen(dbcws->szSetting);
	size_t moduleNameLen = strlen(dbcws->szModule);

	// used for notifications
	DBCONTACTWRITESETTING dbcwNotif = *dbcws;
	if (dbcwNotif.value.type == DBVT_WCHAR) 
	{
		if (dbcwNotif.value.pszVal != NULL) 
		{
			T2Utf val(dbcwNotif.value.pwszVal);
			if (val == NULL)
				return 1;

			dbcwNotif.value.pszVal = NEWSTR_ALLOCA(val);
			dbcwNotif.value.type = DBVT_UTF8;
		}
		else return 1;
	}

	if (dbcwNotif.szModule == NULL || dbcwNotif.szSetting == NULL)
		return 1;

	DBCONTACTWRITESETTING dbcwWork = dbcwNotif;

	mir_ptr<BYTE> pEncoded(NULL);
	bool bIsEncrypted = false;
	switch (dbcwWork.value.type) {
	case DBVT_BYTE: case DBVT_WORD: case DBVT_DWORD:
		break;

	case DBVT_ASCIIZ: case DBVT_UTF8:
		bIsEncrypted = m_bEncrypted || IsSettingEncrypted(dbcws->szModule, dbcws->szSetting);
	LBL_WriteString:
		if (dbcwWork.value.pszVal == NULL)
			return 1;
		dbcwWork.value.cchVal = (WORD)strlen(dbcwWork.value.pszVal);
		if (bIsEncrypted) 
		{
			size_t len;
			BYTE *pResult = m_crypto->encodeString(dbcwWork.value.pszVal, &len);
			if (pResult != NULL) 
			{
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
		if (dbcwWork.value.pbVal == NULL)
			return 1;
		break;
	default:
		return 1;
	}

	char *szCachedSettingName = m_cache->GetCachedSetting(dbcwWork.szModule, dbcwWork.szSetting, moduleNameLen, settingNameLen);

	// we don't cache blobs and passwords
	if (dbcwWork.value.type != DBVT_BLOB && dbcwWork.value.type != DBVT_ENCRYPTED && !bIsEncrypted) 
	{
		DBVARIANT *pCachedValue = m_cache->GetCachedValuePtr(contactID, szCachedSettingName, 1);
		if (pCachedValue != NULL) 
		{
			bool bIsIdentical = false;
			if (pCachedValue->type == dbcwWork.value.type) 
			{
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
		if (szCachedSettingName[-1] != 0) 
		{
			NotifyEventHooks(hSettingChangeEvent, contactID, (LPARAM)&dbcwWork);
			return 0;
		}
	}
	else m_cache->GetCachedValuePtr(contactID, szCachedSettingName, -1);

	DBSettingKey *keyVal = (DBSettingKey *)_alloca(sizeof(DBSettingKey) + settingNameLen + 1);
	keyVal->hContact = contactID;
	keyVal->dwModuleId = GetModuleID(dbcws->szModule);
	memcpy(&keyVal->szSettingName, dbcws->szSetting, settingNameLen + 1);


	MDBX_val key = { keyVal,  sizeof(DBSettingKey) + settingNameLen + 1 }, data;

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
	}

	for (;; Remap()) {
		txn_ptr trnlck(m_pMdbEnv);
		MDBX_CHECK(mdbx_put(trnlck, m_dbSettings, &key, &data, MDBX_RESERVE), 1);

		BYTE *pBlob = (BYTE*)data.iov_base;
		*pBlob++ = dbcwWork.value.type;
		switch (dbcwWork.value.type) {
		case DBVT_BYTE:  *pBlob = dbcwWork.value.bVal; break;
		case DBVT_WORD:  *(WORD*)pBlob = dbcwWork.value.wVal; break;
		case DBVT_DWORD: *(DWORD*)pBlob = dbcwWork.value.dVal; break;

		case DBVT_ASCIIZ:
		case DBVT_UTF8:
			data.iov_len = *(WORD*)pBlob = dbcwWork.value.cchVal;
			pBlob += 2;
			memcpy(pBlob, dbcwWork.value.pszVal, dbcwWork.value.cchVal);
			break;

		case DBVT_BLOB:
		case DBVT_ENCRYPTED:
			data.iov_len = *(WORD*)pBlob = dbcwWork.value.cpbVal;
			pBlob += 2;
			memcpy(pBlob, dbcwWork.value.pbVal, dbcwWork.value.cpbVal);
		}

		if (trnlck.commit() == MDBX_SUCCESS)
			break;
	}

	// notify
	NotifyEventHooks(hSettingChangeEvent, contactID, (LPARAM)&dbcwNotif);
	return 0;
}

STDMETHODIMP_(BOOL) CDbxMdb::DeleteContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting)
{
	if (!szModule || !szSetting)
		return 1;

	size_t settingNameLen = strlen(szSetting);
	size_t moduleNameLen = strlen(szModule);

	char *szCachedSettingName = m_cache->GetCachedSetting(szModule, szSetting, moduleNameLen, settingNameLen);

	if (szCachedSettingName[-1] == 0)  // it's not a resident variable
	{
		DBSettingKey *keyVal = (DBSettingKey*)_alloca(sizeof(DBSettingKey) + settingNameLen + 1);
		keyVal->hContact = contactID;
		keyVal->dwModuleId = GetModuleID(szModule);
		memcpy(&keyVal->szSettingName, szSetting, settingNameLen + 1);

		MDBX_val key = { keyVal,  sizeof(DBSettingKey) + settingNameLen + 1 };

		for (;; Remap()) 
		{
			txn_ptr trnlck(m_pMdbEnv);
			MDBX_CHECK(mdbx_del(trnlck, m_dbSettings, &key, nullptr), 1);
			if (trnlck.commit() == MDBX_SUCCESS)
				break;
		}
	}

	m_cache->GetCachedValuePtr(contactID, szCachedSettingName, -1);

	// notify
	DBCONTACTWRITESETTING dbcws = { 0 };
	dbcws.szModule = szModule;
	dbcws.szSetting = szSetting;
	dbcws.value.type = DBVT_DELETED;
	NotifyEventHooks(hSettingChangeEvent, contactID, (LPARAM)&dbcws);
	return 0;
}

STDMETHODIMP_(BOOL) CDbxMdb::EnumContactSettings(MCONTACT hContact, DBSETTINGENUMPROC pfnEnumProc, const char *szModule, const void *param)
{
	int result = -1;

	DBSettingKey keyVal = { hContact, GetModuleID(szModule) };
	txn_ptr_ro txn(m_txn);
	cursor_ptr_ro cursor(m_curSettings);

	MDBX_val key = { &keyVal, sizeof(keyVal) }, data;

	for (int res = mdbx_cursor_get(cursor, &key, &data, MDBX_SET_RANGE); res == MDBX_SUCCESS; res = mdbx_cursor_get(cursor, &key, &data, MDBX_NEXT))
	{
		const DBSettingKey *pKey = (const DBSettingKey*)key.iov_base;
		if (pKey->hContact != hContact || pKey->dwModuleId != keyVal.dwModuleId)
			break;
		result = pfnEnumProc(pKey->szSettingName, LPARAM(param));
	}

	return result;
}

STDMETHODIMP_(BOOL) CDbxMdb::EnumResidentSettings(DBMODULEENUMPROC pFunc, const void *pParam)
{
	for (int i = 0; i < m_lResidentSettings.getCount(); i++)
		if (int ret = pFunc(m_lResidentSettings[i], 0, (LPARAM)pParam)) 
			return ret;
	return 0;
}
