/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

#define VLT(n) ((n == DBVT_UTF8 || n == DBVT_ENCRYPTED)?DBVT_ASCIIZ:n)

BOOL CDbxKV::IsSettingEncrypted(LPCSTR szModule, LPCSTR szSetting)
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

int CDbxKV::GetContactSettingWorker(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, int isStatic)
{
	if (szSetting == NULL || szModule == NULL)
		return 1;

	// the db format can't tolerate more than 255 bytes of space (incl. null) for settings+module name
	int settingNameLen = (int)strlen(szSetting);
	int moduleNameLen = (int)strlen(szModule);
	if (settingNameLen > 0xFE) {
#ifdef _DEBUG
		OutputDebugStringA("GetContactSettingWorker() got a > 255 setting name length. \n");
#endif
		return 1;
	}
	if (moduleNameLen > 0xFE) {
#ifdef _DEBUG
		OutputDebugStringA("GetContactSettingWorker() got a > 255 module name length. \n");
#endif
		return 1;
	}

	mir_cslock lck(m_csDbAccess);

LBL_Seek:
	char *szCachedSettingName = m_cache->GetCachedSetting(szModule, szSetting, moduleNameLen, settingNameLen);

	DBVARIANT *pCachedValue = m_cache->GetCachedValuePtr(contactID, szCachedSettingName, 0);
	if (pCachedValue != NULL) {
		if (pCachedValue->type == DBVT_ASCIIZ || pCachedValue->type == DBVT_UTF8) {
			int cbOrigLen = dbv->cchVal;
			char *cbOrigPtr = dbv->pszVal;
			memcpy(dbv, pCachedValue, sizeof(DBVARIANT));
			if (isStatic) {
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

	DBCachedContact *cc = (contactID) ? m_cache->GetCachedContact(contactID) : NULL;

	DBSettingSortingKey keySearch;
	keySearch.dwContactID = contactID;
	keySearch.dwOfsModule = GetModuleNameOfs(szModule);
	strncpy_s(keySearch.szSettingName, szSetting, _TRUNCATE);

	ham_key_t key = { 2 * sizeof(DWORD) + settingNameLen, &keySearch };
	ham_record_t rec = { 0 };
	if (ham_db_find(m_dbSettings, NULL, &key, &rec, 0)) {
		// try to get the missing mc setting from the active sub
		if (cc && cc->IsMeta() && ValidLookupName(szModule, szSetting)) {
			if (contactID = db_mc_getDefault(contactID)) {
				if (szModule = GetContactProto(contactID)) {
					moduleNameLen = (int)strlen(szModule);
					goto LBL_Seek;
				}
			}
		}
		return 1;
	}

	BYTE *pBlob = (BYTE*)rec.data;
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
			memmove(dbv->pszVal, pBlob, dbv->cchVal); // decode
			dbv->pszVal[dbv->cchVal] = 0;
			dbv->cchVal = varLen;
		}
		else {
			dbv->pszVal = (char*)mir_alloc(1 + varLen);
			memmove(dbv->pszVal, pBlob, varLen);
			dbv->pszVal[varLen] = 0;
		}
		break;

	case DBVT_BLOB:
		varLen = *(WORD*)pBlob;
		pBlob += 2;
		if (isStatic) {
			if (varLen < dbv->cpbVal)
				dbv->cpbVal = varLen;
			memmove(dbv->pbVal, pBlob, dbv->cpbVal);
		}
		else {
			dbv->pbVal = (BYTE *)mir_alloc(varLen);
			memmove(dbv->pbVal, pBlob, varLen);
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
		break;
	}

	/**** add to cache **********************/
	if (iType != DBVT_BLOB && iType != DBVT_ENCRYPTED) {
		DBVARIANT *pCachedValue = m_cache->GetCachedValuePtr(contactID, szCachedSettingName, 1);
		if (pCachedValue != NULL)
			m_cache->SetCachedVariant(dbv, pCachedValue);
	}

	return 0;
}

STDMETHODIMP_(BOOL) CDbxKV::GetContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv)
{
	dbv->type = 0;
	if (GetContactSettingWorker(contactID, szModule, szSetting, dbv, 0))
		return 1;

	if (dbv->type == DBVT_UTF8) {
		WCHAR *tmp = NULL;
		char *p = NEWSTR_ALLOCA(dbv->pszVal);
		if (mir_utf8decode(p, &tmp) != NULL) {
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
		else {
			dbv->type = DBVT_ASCIIZ;
			mir_free(tmp);
		}
	}

	return 0;
}

STDMETHODIMP_(BOOL) CDbxKV::GetContactSettingStr(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv)
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

STDMETHODIMP_(BOOL) CDbxKV::GetContactSettingStatic(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv)
{
	if (GetContactSettingWorker(contactID, szModule, szSetting, dbv, 1))
		return 1;

	if (dbv->type == DBVT_UTF8) {
		mir_utf8decode(dbv->pszVal, NULL);
		dbv->type = DBVT_ASCIIZ;
	}

	return 0;
}

STDMETHODIMP_(BOOL) CDbxKV::FreeVariant(DBVARIANT *dbv)
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

STDMETHODIMP_(BOOL) CDbxKV::SetSettingResident(BOOL bIsResident, const char *pszSettingName)
{
	char *szSetting = m_cache->GetCachedSetting(NULL, pszSettingName, 0, (int)strlen(pszSettingName));
	szSetting[-1] = (char)bIsResident;

	mir_cslock lck(m_csDbAccess);
	int idx = m_lResidentSettings.getIndex(szSetting);
	if (idx == -1) {
		if (bIsResident)
			m_lResidentSettings.insert(szSetting);
	}
	else if (!bIsResident)
		m_lResidentSettings.remove(idx);

	return 0;
}

STDMETHODIMP_(BOOL) CDbxKV::WriteContactSetting(MCONTACT contactID, DBCONTACTWRITESETTING *dbcws)
{
	if (dbcws == NULL || dbcws->szSetting == NULL || dbcws->szModule == NULL || m_bReadOnly)
		return 1;

	// the db format can't tolerate more than 255 bytes of space (incl. null) for settings+module name
	int settingNameLen = (int)strlen(dbcws->szSetting);
	int moduleNameLen = (int)strlen(dbcws->szModule);
	if (settingNameLen > 0xFE) {
#ifdef _DEBUG
		OutputDebugStringA("WriteContactSetting() got a > 255 setting name length. \n");
#endif
		return 1;
	}
	if (moduleNameLen > 0xFE) {
#ifdef _DEBUG
		OutputDebugStringA("WriteContactSetting() got a > 255 module name length. \n");
#endif
		return 1;
	}

	// used for notifications
	DBCONTACTWRITESETTING dbcwNotif = *dbcws;
	if (dbcwNotif.value.type == DBVT_WCHAR) {
		if (dbcwNotif.value.pszVal != NULL) {
			char* val = mir_utf8encodeW(dbcwNotif.value.pwszVal);
			if (val == NULL)
				return 1;

			dbcwNotif.value.pszVal = (char*)alloca(strlen(val) + 1);
			strcpy(dbcwNotif.value.pszVal, val);
			mir_free(val);
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
		if (bIsEncrypted) {
			size_t len;
			BYTE *pResult = m_crypto->encodeString(dbcwWork.value.pszVal, &len);
			if (pResult != NULL) {
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

	mir_cslockfull lck(m_csDbAccess);

	char *szCachedSettingName = m_cache->GetCachedSetting(dbcwWork.szModule, dbcwWork.szSetting, moduleNameLen, settingNameLen);

	// we don't cache blobs and passwords
	if (dbcwWork.value.type != DBVT_BLOB && dbcwWork.value.type != DBVT_ENCRYPTED && !bIsEncrypted) {
		DBVARIANT *pCachedValue = m_cache->GetCachedValuePtr(contactID, szCachedSettingName, 1);
		if (pCachedValue != NULL) {
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
			NotifyEventHooks(hSettingChangeEvent, contactID, (LPARAM)&dbcwWork);
			return 0;
		}
	}
	else m_cache->GetCachedValuePtr(contactID, szCachedSettingName, -1);

	DBSettingSortingKey keySearch;
	keySearch.dwContactID = contactID;
	keySearch.dwOfsModule = GetModuleNameOfs(dbcws->szModule);
	strncpy_s(keySearch.szSettingName, dbcws->szSetting, _TRUNCATE);

	ham_key_t key = { 2 * sizeof(DWORD) + settingNameLen, &keySearch };
	ham_record_t rec = { 0 };

	switch (dbcwWork.value.type) {
	case DBVT_BYTE:  rec.size = 2; break;
	case DBVT_WORD:  rec.size = 3; break;
	case DBVT_DWORD: rec.size = 5; break;
	
	case DBVT_ASCIIZ: 
	case DBVT_UTF8:
		rec.size = 3 + dbcwWork.value.cchVal; break;

	case DBVT_BLOB:
	case DBVT_ENCRYPTED:
		rec.size = 3 + dbcwWork.value.cpbVal; break;

	default:
		return 1;
	}

	rec.data = _alloca(rec.size);
	BYTE *pBlob = (BYTE*)rec.data;
	*pBlob++ = dbcwWork.value.type;
	switch (dbcwWork.value.type) {
	case DBVT_BYTE:  *pBlob = dbcwWork.value.bVal; break;
	case DBVT_WORD:  *(WORD*)pBlob = dbcwWork.value.wVal; break;
	case DBVT_DWORD: *(DWORD*)pBlob = dbcwWork.value.dVal; break;

	case DBVT_ASCIIZ:
	case DBVT_UTF8:
		*(WORD*)pBlob = dbcwWork.value.cchVal;
		memcpy(pBlob+2, dbcwWork.value.pszVal, dbcwWork.value.cchVal);
		break;

	case DBVT_BLOB:
	case DBVT_ENCRYPTED:
		*(WORD*)pBlob = dbcwWork.value.cpbVal;
		memcpy(pBlob+2, dbcwWork.value.pbVal, dbcwWork.value.cpbVal);
	}
	ham_db_insert(m_dbSettings, NULL, &key, &rec, HAM_OVERWRITE);
	lck.unlock();

	// notify
	NotifyEventHooks(hSettingChangeEvent, contactID, (LPARAM)&dbcwNotif);
	return 0;
}

STDMETHODIMP_(BOOL) CDbxKV::DeleteContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting)
{
	if (!szModule || !szSetting)
		return 1;

	// the db format can't tolerate more than 255 bytes of space (incl. null) for settings+module name
	int settingNameLen = (int)strlen(szSetting);
	int moduleNameLen = (int)strlen(szModule);
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
			DBSettingSortingKey keySearch;
			keySearch.dwContactID = contactID;
			keySearch.dwOfsModule = GetModuleNameOfs(szModule);
			strncpy_s(keySearch.szSettingName, szSetting, _TRUNCATE);

			ham_key_t key = { 2 * sizeof(DWORD) + settingNameLen, &keySearch };
			if (ham_db_erase(m_dbSettings, NULL, &key, 0) != HAM_SUCCESS)
				return 1;
		}

		m_cache->GetCachedValuePtr(saveContact, szCachedSettingName, -1);
	}

	// notify
	DBCONTACTWRITESETTING dbcws = { 0 };
	dbcws.szModule = szModule;
	dbcws.szSetting = szSetting;
	dbcws.value.type = DBVT_DELETED;
	NotifyEventHooks(hSettingChangeEvent, saveContact, (LPARAM)&dbcws);
	return 0;
}

STDMETHODIMP_(BOOL) CDbxKV::EnumContactSettings(MCONTACT contactID, DBCONTACTENUMSETTINGS* dbces)
{
	if (!dbces->szModule)
		return -1;

	mir_cslock lck(m_csDbAccess);

	DBSettingSortingKey keySearch;
	keySearch.dwContactID = contactID;
	keySearch.dwOfsModule = GetModuleNameOfs(dbces->szModule);
	memset(keySearch.szSettingName, 0, SIZEOF(keySearch.szSettingName));

	int result = -1;

	ham_record_t rec = { 0 };
	ham_key_t key = { sizeof(keySearch), &keySearch };

	cursor_ptr cursor(m_dbSettings);
	if (ham_cursor_find(cursor, &key, &rec, HAM_FIND_GEQ_MATCH) == HAM_SUCCESS) {
		do {
			DBSettingSortingKey *pKey = (DBSettingSortingKey*)key.data;
			if (pKey->dwContactID != contactID || pKey->dwOfsModule != keySearch.dwOfsModule)
				break;

			char szSetting[256];
			strncpy_s(szSetting, pKey->szSettingName, key.size - sizeof(DWORD) * 2);
			result = (dbces->pfnEnumProc)(szSetting, dbces->lParam);
		} while (ham_cursor_move(cursor, &key, &rec, HAM_CURSOR_NEXT) == HAM_SUCCESS);
	}

	return result;
}

STDMETHODIMP_(BOOL) CDbxKV::EnumResidentSettings(DBMODULEENUMPROC pFunc, void *pParam)
{
	for (int i = 0; i < m_lResidentSettings.getCount(); i++) {
		int ret = pFunc(m_lResidentSettings[i], 0, (LPARAM)pParam);
		if (ret) return ret;
	}
	return 0;
}
