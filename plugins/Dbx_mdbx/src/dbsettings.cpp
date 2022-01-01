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

		const uint8_t *pBlob = (const uint8_t*)data.iov_base;
		if (*pBlob == DBVT_DELETED)
			continue;

		size_t settingNameLen = strlen(pKey->szSettingName);
		size_t moduleNameLen = strlen(szModule);

		char *szCachedSettingName = m_cache->GetCachedSetting(szModule, pKey->szSettingName, moduleNameLen, settingNameLen);

		DBVARIANT *dbv = m_cache->GetCachedValuePtr(pKey->hContact, szCachedSettingName, 1);
		if (dbv == nullptr) // garbage! a setting for removed/non-existent contact
			continue;

		uint16_t varLen;

		uint8_t iType = dbv->type = pBlob[0]; pBlob++;
		switch (iType) {
		case DBVT_BYTE:  dbv->bVal = *pBlob; break;
		case DBVT_WORD:  dbv->wVal = *(uint16_t*)pBlob; break;
		case DBVT_DWORD: dbv->dVal = *(uint32_t*)pBlob; break;

		case DBVT_UTF8:
		case DBVT_ASCIIZ:
			varLen = *(uint16_t*)pBlob;
			pBlob += 2;
			dbv->pszVal = (char*)mir_alloc(1 + varLen);
			memcpy(dbv->pszVal, pBlob, varLen);
			dbv->pszVal[varLen] = 0;
			break;

		case DBVT_BLOB:
		case DBVT_ENCRYPTED:
			varLen = *(uint16_t*)pBlob;
			pBlob += 2;
			dbv->pbVal = (uint8_t *)mir_alloc(varLen);
			memcpy(dbv->pbVal, pBlob, varLen);
			dbv->cpbVal = varLen;
			break;
		}
	}

	FillContactSettings();
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL CDbxMDBX::WriteContactSettingWorker(MCONTACT contactID, DBCONTACTWRITESETTING &dbcws)
{
	size_t settingNameLen = mir_strlen(dbcws.szSetting);

	// write down a setting to database
	DBSettingKey *keyVal = (DBSettingKey *)_alloca(sizeof(DBSettingKey) + settingNameLen);
	keyVal->hContact = contactID;
	keyVal->dwModuleId = GetModuleID(dbcws.szModule);
	memcpy(&keyVal->szSettingName, dbcws.szSetting, settingNameLen + 1);

	MDBX_val key = { keyVal,  sizeof(DBSettingKey) + settingNameLen }, data;

	switch (dbcws.value.type) {
	case DBVT_BYTE:  data.iov_len = 2; break;
	case DBVT_WORD:  data.iov_len = 3; break;
	case DBVT_DWORD: data.iov_len = 5; break;

	case DBVT_ASCIIZ:
	case DBVT_UTF8:
		data.iov_len = 3 + dbcws.value.cchVal; break;

	case DBVT_BLOB:
	case DBVT_ENCRYPTED:
		data.iov_len = 3 + dbcws.value.cpbVal; break;

	default:
		return 1;
	}

	data.iov_base = _alloca(data.iov_len);

	uint8_t *pBlob = (uint8_t*)data.iov_base;
	*pBlob++ = dbcws.value.type;
	switch (dbcws.value.type) {
	case DBVT_BYTE:  *pBlob = dbcws.value.bVal; break;
	case DBVT_WORD:  *(uint16_t*)pBlob = dbcws.value.wVal; break;
	case DBVT_DWORD: *(uint32_t*)pBlob = dbcws.value.dVal; break;

	case DBVT_ASCIIZ:
	case DBVT_UTF8:
		*(uint16_t*)pBlob = dbcws.value.cchVal;
		pBlob += 2;
		memcpy(pBlob, dbcws.value.pszVal, dbcws.value.cchVal);
		break;

	case DBVT_BLOB:
	case DBVT_ENCRYPTED:
		*(uint16_t*)pBlob = dbcws.value.cpbVal;
		pBlob += 2;
		memcpy(pBlob, dbcws.value.pbVal, dbcws.value.cpbVal);
	}

	{
		txn_ptr trnlck(this);
		if (mdbx_put(trnlck, m_dbSettings, &key, &data, MDBX_UPSERT) != MDBX_SUCCESS)
			return 1;
	}

	DBFlush();
	return 0;
}

BOOL CDbxMDBX::DeleteContactSetting(MCONTACT contactID, const char *szModule, const char *szSetting)
{
	if (!szModule || !szSetting)
		return 1;

	size_t settingNameLen = strlen(szSetting);
	size_t moduleNameLen = strlen(szModule);
	{
		mir_cslock lck(m_csDbAccess);
		char *szCachedSettingName = m_cache->GetCachedSetting(szModule, szSetting, moduleNameLen, settingNameLen);

		// try to remove it from cache first. 
		// if there's nothing, don't try to remove a setting from database
		auto *pSetting = m_cache->GetCachedValuePtr(contactID, szCachedSettingName, -1);
		if (pSetting == nullptr)
			return 1;

		// if it's not a resident variable, delete it from database too
		if (szCachedSettingName[-1] == 0) {
			DBSettingKey *keyVal = (DBSettingKey*)_alloca(sizeof(DBSettingKey) + settingNameLen);
			keyVal->hContact = contactID;
			keyVal->dwModuleId = GetModuleID(szModule);
			memcpy(&keyVal->szSettingName, szSetting, settingNameLen + 1);

			txn_ptr trnlck(this);
			MDBX_val key = { keyVal,  sizeof(DBSettingKey) + settingNameLen };
			if (mdbx_del(trnlck, m_dbSettings, &key, nullptr) != MDBX_SUCCESS)
				return 1;
			DBFlush();
		}
	}

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
