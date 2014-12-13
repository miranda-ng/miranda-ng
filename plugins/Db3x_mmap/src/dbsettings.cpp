/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012-14 Miranda NG project,
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

DWORD GetModuleNameOfs(const char *szName);
DBCachedContact* AddToCachedContactList(MCONTACT contactID, int index);

#define VLT(n) ((n == DBVT_UTF8 || n == DBVT_ENCRYPTED)?DBVT_ASCIIZ:n)

BOOL CDb3Mmap::IsSettingEncrypted(LPCSTR szModule, LPCSTR szSetting)
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

int CDb3Mmap::GetContactSettingWorker(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, int isStatic)
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
	log3("get [%08p] %s (%p)", hContact, szCachedSettingName, szCachedSettingName);

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

		log2("get cached %s (%p)", printVariant(dbv), pCachedValue);
		return (pCachedValue->type == DBVT_DELETED) ? 1 : 0;
	}

	// never look db for the resident variable
	if (szCachedSettingName[-1] != 0)
		return 1;

	DBCachedContact *cc;
	DWORD ofsContact = GetContactOffset(contactID, &cc);

	DWORD ofsModuleName = GetModuleNameOfs(szModule);

	DBContact dbc = *(DBContact*)DBRead(ofsContact,sizeof(DBContact),NULL);
	if (dbc.signature != DBCONTACT_SIGNATURE)
		return 1;

	DWORD ofsSettingsGroup = GetSettingsGroupOfsByModuleNameOfs(&dbc, ofsContact, ofsModuleName);
	if (ofsSettingsGroup) {
		int bytesRemaining;
		unsigned varLen;
		DWORD ofsBlobPtr = ofsSettingsGroup + offsetof(DBContactSettings, blob);
		PBYTE pBlob = DBRead(ofsBlobPtr, sizeof(DBContactSettings), &bytesRemaining);
		while (pBlob[0]) {
			NeedBytes(1+settingNameLen);
			if (pBlob[0] == settingNameLen && !memcmp(pBlob+1,szSetting,settingNameLen)) {
				MoveAlong(1 + settingNameLen);
				NeedBytes(5);
				if (isStatic && (pBlob[0] & DBVTF_VARIABLELENGTH) && VLT(dbv->type) != VLT(pBlob[0]))
					return 1;

				BYTE iType = dbv->type = pBlob[0];
				switch (iType) {
				case DBVT_DELETED: /* this setting is deleted */
					dbv->type = DBVT_DELETED;
					return 2;

				case DBVT_BYTE:  dbv->bVal = pBlob[1]; break;
				case DBVT_WORD:  MoveMemory(&(dbv->wVal), (PWORD)(pBlob + 1), 2); break;
				case DBVT_DWORD: MoveMemory(&(dbv->dVal), (PDWORD)(pBlob + 1), 4); break;
				
				case DBVT_UTF8:
				case DBVT_ASCIIZ:
					varLen = *(PWORD)(pBlob + 1);
					NeedBytes(int(3 + varLen));
					if (isStatic) {
						dbv->cchVal--;
						if (varLen < dbv->cchVal)
							dbv->cchVal = varLen;
						MoveMemory(dbv->pszVal, pBlob + 3, dbv->cchVal); // decode
						dbv->pszVal[dbv->cchVal] = 0;
						dbv->cchVal = varLen;
					}
					else {
						dbv->pszVal = (char*)mir_alloc(1 + varLen);
						MoveMemory(dbv->pszVal, pBlob + 3, varLen);
						dbv->pszVal[varLen] = 0;
					}
					break;
				
				case DBVT_BLOB:
					varLen = *(PWORD)(pBlob + 1);
					NeedBytes(int(3 + varLen));
					if (isStatic) {
						if (varLen < dbv->cpbVal)
							dbv->cpbVal = varLen;
						MoveMemory(dbv->pbVal, pBlob + 3, dbv->cpbVal);
					}
					else {
						dbv->pbVal = (BYTE *)mir_alloc(varLen);
						MoveMemory(dbv->pbVal, pBlob + 3, varLen);
					}
					dbv->cpbVal = varLen;
					break;

				case DBVT_ENCRYPTED:
					if (m_crypto == NULL)
						return 1;
					else {
						varLen = *(PWORD)(pBlob + 1);
						NeedBytes(int(3 + varLen));
						size_t realLen;
						ptrA decoded(m_crypto->decodeString(pBlob + 3, varLen, &realLen));
						if (decoded == NULL)
							return 1;

						varLen = (WORD)realLen;
						dbv->type = DBVT_UTF8;
						if (isStatic) {
							dbv->cchVal--;
							if (varLen < dbv->cchVal)
								dbv->cchVal = varLen;
							MoveMemory(dbv->pszVal, decoded, dbv->cchVal);
							dbv->pszVal[dbv->cchVal] = 0;
							dbv->cchVal = varLen;
						}
						else {
							dbv->pszVal = (char*)mir_alloc(1 + varLen);
							MoveMemory(dbv->pszVal, decoded, varLen);
							dbv->pszVal[varLen] = 0;
						}
					}
					break;
				}

				/**** add to cache **********************/
				if (iType != DBVT_BLOB && iType != DBVT_ENCRYPTED) {
					DBVARIANT *pCachedValue = m_cache->GetCachedValuePtr(contactID, szCachedSettingName, 1);
					if (pCachedValue != NULL) {
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
			if (szModule = GetContactProto(contactID)) {
				moduleNameLen = (int)strlen(szModule);
				goto LBL_Seek;
			}
		}
	}

	logg();
	return 1;
}

STDMETHODIMP_(BOOL) CDb3Mmap::GetContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv)
{
	dbv->type = 0;
	if (GetContactSettingWorker(contactID, szModule, szSetting, dbv, 0))
		return 1;

	if (dbv->type == DBVT_UTF8 ) {
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

STDMETHODIMP_(BOOL) CDb3Mmap::GetContactSettingStr(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv)
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
			wchar_t* wszResult = (wchar_t*)mir_alloc((len + 1)*sizeof(wchar_t));
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

STDMETHODIMP_(BOOL) CDb3Mmap::GetContactSettingStatic(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv)
{
	if (GetContactSettingWorker(contactID, szModule, szSetting, dbv, 1))
		return 1;

	if (dbv->type == DBVT_UTF8) {
		mir_utf8decode(dbv->pszVal, NULL);
		dbv->type = DBVT_ASCIIZ;
	}

	return 0;
}

STDMETHODIMP_(BOOL) CDb3Mmap::FreeVariant(DBVARIANT *dbv)
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

STDMETHODIMP_(BOOL) CDb3Mmap::SetSettingResident(BOOL bIsResident, const char *pszSettingName)
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

STDMETHODIMP_(BOOL) CDb3Mmap::WriteContactSetting(MCONTACT contactID, DBCONTACTWRITESETTING *dbcws)
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

	DWORD ofsBlobPtr, ofsContact = GetContactOffset(contactID);
	if (ofsContact == 0) {
		_ASSERT(false); // contact doesn't exist?
		return 2;
	}

	char *szCachedSettingName = m_cache->GetCachedSetting(dbcwWork.szModule, dbcwWork.szSetting, moduleNameLen, settingNameLen);
	log3("set [%08p] %s (%p)", hContact, szCachedSettingName, szCachedSettingName);

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
			log2(" set resident as %s (%p)", printVariant(&dbcwWork.value), pCachedValue);
			NotifyEventHooks(hSettingChangeEvent, contactID, (LPARAM)&dbcwWork);
			return 0;
		}
	}
	else m_cache->GetCachedValuePtr(contactID, szCachedSettingName, -1);

	log1(" write database as %s", printVariant(&dbcwWork.value));

	DWORD ofsModuleName = GetModuleNameOfs(dbcwWork.szModule);
	DBContact dbc = *(DBContact*)DBRead(ofsContact, sizeof(DBContact), NULL);
	if (dbc.signature != DBCONTACT_SIGNATURE)
		return 1;

	// make sure the module group exists
	PBYTE pBlob;
	int bytesRequired, bytesRemaining;
	DBContactSettings dbcs;
	DWORD ofsSettingsGroup = GetSettingsGroupOfsByModuleNameOfs(&dbc, ofsContact, ofsModuleName);
	if (ofsSettingsGroup == 0) {  //module group didn't exist - make it
		switch (dbcwWork.value.type) {
		case DBVT_ASCIIZ: case DBVT_UTF8:
			bytesRequired = dbcwWork.value.cchVal + 2;
			break;
		case DBVT_BLOB: case DBVT_ENCRYPTED:
			bytesRequired = dbcwWork.value.cpbVal + 2;
			break;
		default:
			bytesRequired = dbcwWork.value.type;
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
		pBlob = (PBYTE)DBRead(ofsBlobPtr, 1, &bytesRemaining);
	}
	else {
		dbcs = *(DBContactSettings*)DBRead(ofsSettingsGroup, sizeof(DBContactSettings), &bytesRemaining);
		
		// find if the setting exists
		ofsBlobPtr = ofsSettingsGroup + offsetof(DBContactSettings, blob);
		pBlob = (PBYTE)DBRead(ofsBlobPtr, 1, &bytesRemaining);
		while (pBlob[0]) {
			NeedBytes(settingNameLen + 1);
			if (pBlob[0] == settingNameLen && !memcmp(pBlob + 1, dbcwWork.szSetting, settingNameLen))
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
			if (pBlob[0] != dbcwWork.value.type || 
				 ((pBlob[0] == DBVT_ASCIIZ || pBlob[0] == DBVT_UTF8) && *(PWORD)(pBlob + 1) != dbcwWork.value.cchVal) ||
				 ((pBlob[0] == DBVT_BLOB || pBlob[0] == DBVT_ENCRYPTED) && *(PWORD)(pBlob + 1) != dbcwWork.value.cpbVal))
			{
				// bin it
				NeedBytes(3);
				int nameLen = 1 + settingNameLen;
				int valLen = 1 + GetSettingValueLength(pBlob);
				DWORD ofsSettingToCut = ofsBlobPtr - nameLen;
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
				pBlob = (PBYTE)DBRead(ofsBlobPtr, 1, &bytesRemaining);
			}
			else {
				// replace existing setting at pBlob
				MoveAlong(1);	// skip data type
				switch (dbcwWork.value.type) {
				case DBVT_BYTE:  DBWrite(ofsBlobPtr, &dbcwWork.value.bVal, 1); break;
				case DBVT_WORD:  DBWrite(ofsBlobPtr, &dbcwWork.value.wVal, 2); break;
				case DBVT_DWORD: DBWrite(ofsBlobPtr, &dbcwWork.value.dVal, 4); break;
				case DBVT_BLOB:
					DBWrite(ofsBlobPtr + 2, dbcwWork.value.pbVal, dbcwWork.value.cpbVal);
					break;
				case DBVT_ENCRYPTED:
					DBWrite(ofsBlobPtr + 2, dbcwWork.value.pbVal, dbcwWork.value.cpbVal);
					break;
				case DBVT_UTF8:
				case DBVT_ASCIIZ:
					DBWrite(ofsBlobPtr + 2, dbcwWork.value.pszVal, dbcwWork.value.cchVal);
					break;
				}
				// quit
				DBFlush(1);
				lck.unlock();
				// notify
				NotifyEventHooks(hSettingChangeEvent, contactID, (LPARAM)&dbcwNotif);
				return 0;
			}
		}
	}
	
	// cannot do a simple replace, add setting to end of list
	// pBlob already points to end of list
	// see if it fits
	switch (dbcwWork.value.type) {
	case DBVT_ASCIIZ: case DBVT_UTF8:
		bytesRequired = dbcwWork.value.cchVal + 2;
		break;
	case DBVT_BLOB: case DBVT_ENCRYPTED:
		bytesRequired = dbcwWork.value.cpbVal + 2;
		break;
	default:
		bytesRequired = dbcwWork.value.type;
	}

	bytesRequired += 2 + settingNameLen;
	bytesRequired += ofsBlobPtr + 1 - (ofsSettingsGroup + offsetof(DBContactSettings, blob));

	if ((DWORD)bytesRequired > dbcs.cbBlob) {
		// doesn't fit: move entire group
		DBContactSettings *dbcsPrev;
		DWORD ofsDbcsPrev, ofsNew;

		InvalidateSettingsGroupOfsCacheEntry(ofsSettingsGroup);
		bytesRequired += (DB_SETTINGS_RESIZE_GRANULARITY - (bytesRequired % DB_SETTINGS_RESIZE_GRANULARITY)) % DB_SETTINGS_RESIZE_GRANULARITY;
		// find previous group to change its offset
		ofsDbcsPrev = dbc.ofsFirstSettings;
		if (ofsDbcsPrev == ofsSettingsGroup) ofsDbcsPrev = 0;
		else {
			dbcsPrev = (DBContactSettings*)DBRead(ofsDbcsPrev, sizeof(DBContactSettings), NULL);
			while (dbcsPrev->ofsNext != ofsSettingsGroup) {
				if (dbcsPrev->ofsNext == 0) DatabaseCorruption(NULL);
				ofsDbcsPrev = dbcsPrev->ofsNext;
				dbcsPrev = (DBContactSettings*)DBRead(ofsDbcsPrev, sizeof(DBContactSettings), NULL);
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
			dbcsPrev = (DBContactSettings*)DBRead(ofsDbcsPrev, sizeof(DBContactSettings), NULL);
			dbcsPrev->ofsNext = ofsNew;
			DBWrite(ofsDbcsPrev, dbcsPrev, offsetof(DBContactSettings, blob));
		}
		ofsBlobPtr += ofsNew - ofsSettingsGroup;
		ofsSettingsGroup = ofsNew;
		pBlob = (PBYTE)DBRead(ofsBlobPtr, 1, &bytesRemaining);
	}
	
	// we now have a place to put it and enough space: make it
	DBWrite(ofsBlobPtr, &settingNameLen, 1);
	DBWrite(ofsBlobPtr + 1, (PVOID)dbcwWork.szSetting, settingNameLen);
	MoveAlong(1 + settingNameLen);
	DBWrite(ofsBlobPtr, &dbcwWork.value.type, 1);
	MoveAlong(1);
	switch (dbcwWork.value.type) {
	case DBVT_BYTE: DBWrite(ofsBlobPtr, &dbcwWork.value.bVal, 1); MoveAlong(1); break;
	case DBVT_WORD: DBWrite(ofsBlobPtr, &dbcwWork.value.wVal, 2); MoveAlong(2); break;
	case DBVT_DWORD: DBWrite(ofsBlobPtr, &dbcwWork.value.dVal, 4); MoveAlong(4); break;

	case DBVT_BLOB:
		DBWrite(ofsBlobPtr, &dbcwWork.value.cpbVal, 2);
		DBWrite(ofsBlobPtr + 2, dbcwWork.value.pbVal, dbcwWork.value.cpbVal);
		MoveAlong(2 + dbcwWork.value.cpbVal);
		break;

	case DBVT_ENCRYPTED:
		DBWrite(ofsBlobPtr, &dbcwWork.value.cpbVal, 2);
		DBWrite(ofsBlobPtr + 2, dbcwWork.value.pbVal, dbcwWork.value.cpbVal);
		MoveAlong(2 + dbcwWork.value.cpbVal);
		break;

	case DBVT_UTF8: case DBVT_ASCIIZ:
		DBWrite(ofsBlobPtr, &dbcwWork.value.cchVal, 2);
		DBWrite(ofsBlobPtr + 2, dbcwWork.value.pszVal, dbcwWork.value.cchVal);
		MoveAlong(2 + dbcwWork.value.cchVal);
		break;
	}

	BYTE zero = 0;
	DBWrite(ofsBlobPtr, &zero, 1);

	// quit
	DBFlush(1);
	lck.unlock();

	// notify
	NotifyEventHooks(hSettingChangeEvent, contactID, (LPARAM)&dbcwNotif);
	return 0;
}

STDMETHODIMP_(BOOL) CDb3Mmap::DeleteContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting)
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
	char *szCachedSettingName = m_cache->GetCachedSetting(szModule, szSetting, moduleNameLen, settingNameLen);
	if (szCachedSettingName[-1] == 0) { // it's not a resident variable
		mir_cslock lck(m_csDbAccess);
		DWORD ofsModuleName = GetModuleNameOfs(szModule);
		DWORD ofsContact = GetContactOffset(contactID);
		DBContact *dbc = (DBContact*)DBRead(ofsContact, sizeof(DBContact), NULL);
		if (dbc->signature != DBCONTACT_SIGNATURE)
			return 1;

		// make sure the module group exists
		DWORD ofsSettingsGroup = GetSettingsGroupOfsByModuleNameOfs(dbc, ofsContact, ofsModuleName);
		if (ofsSettingsGroup == 0)
			return 1;

		// find if the setting exists
		DWORD ofsBlobPtr = ofsSettingsGroup + offsetof(DBContactSettings, blob);
		int bytesRemaining;
		PBYTE pBlob = (PBYTE)DBRead(ofsBlobPtr, 1, &bytesRemaining);
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
		DWORD ofsSettingToCut = ofsBlobPtr - nameLen;
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
	}

	m_cache->GetCachedValuePtr(saveContact, szCachedSettingName, -1);

	// notify
	DBCONTACTWRITESETTING dbcws = { 0 };
	dbcws.szModule = szModule;
	dbcws.szSetting = szSetting;
	dbcws.value.type = DBVT_DELETED;
	NotifyEventHooks(hSettingChangeEvent, saveContact, (LPARAM)&dbcws);
	return 0;
}

STDMETHODIMP_(BOOL) CDb3Mmap::EnumContactSettings(MCONTACT contactID, DBCONTACTENUMSETTINGS* dbces)
{
	if (!dbces->szModule)
		return -1;

	mir_cslock lck(m_csDbAccess);

	DWORD ofsContact = GetContactOffset(contactID);
	if (ofsContact == 0)
		return -1;

	DBContact *dbc = (DBContact*)DBRead(ofsContact, sizeof(DBContact), NULL);
	if (dbc->signature != DBCONTACT_SIGNATURE)
		return -1;

	DWORD ofsModuleName = GetModuleNameOfs(dbces->szModule);
	dbces->ofsSettings = GetSettingsGroupOfsByModuleNameOfs(dbc, ofsContact, ofsModuleName);
	if (!dbces->ofsSettings)
		return -1;

	DWORD ofsBlobPtr = dbces->ofsSettings + offsetof(DBContactSettings, blob);
	int bytesRemaining;
	PBYTE pBlob = (PBYTE)DBRead(ofsBlobPtr, 1, &bytesRemaining);
	if (pBlob[0] == 0)
		return -1;

	int result = 0;
	while (pBlob[0]) {
		NeedBytes(1);
		NeedBytes(1 + pBlob[0]);
		char szSetting[256];
		memcpy(szSetting, pBlob + 1, pBlob[0]); szSetting[pBlob[0]] = 0;
		result = (dbces->pfnEnumProc)(szSetting, dbces->lParam);
		MoveAlong(1 + pBlob[0]);
		NeedBytes(3);
		MoveAlong(1 + GetSettingValueLength(pBlob));
		NeedBytes(1);
	}
	return result;
}

STDMETHODIMP_(BOOL) CDb3Mmap::EnumResidentSettings(DBMODULEENUMPROC pFunc, void *pParam)
{
	for (int i = 0; i < m_lResidentSettings.getCount(); i++) {
		int ret = pFunc(m_lResidentSettings[i], 0, (LPARAM)pParam);
		if (ret) return ret;
	}
	return 0;
}
