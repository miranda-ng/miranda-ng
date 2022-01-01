/*

Import plugin for Miranda NG

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

#include "..\stdafx.h"

STDMETHODIMP_(BOOL) CDbxSQLite::EnumModuleNames(DBMODULEENUMPROC pFunc, void *pParam)
{
	mir_cslockfull lock(m_csDbAccess);

	int res = 0;
	while (sql_step(set_stmts_prep[SQL_SET_STMT_ENUMMODULES]) == SQLITE_ROW && !res) {
		const char *szModule = (const char *)sqlite3_column_text(set_stmts_prep[SQL_SET_STMT_ENUMMODULES], 0);
		lock.unlock();
		res = (pFunc)(szModule, pParam);
		lock.lock();
	}
	sql_reset(set_stmts_prep[SQL_SET_STMT_ENUMMODULES]);
	return res;
}

STDMETHODIMP_(BOOL) CDbxSQLite::GetContactSettingWorker(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, int isStatic)
{
	if (!szSetting || !szModule)
		return 1;

	size_t settingNameLen = strlen(szSetting);
	size_t moduleNameLen = strlen(szModule);

	mir_cslock lock(m_csDbAccess);

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

	sqlite3_bind_text(set_stmts_prep[SQL_SET_STMT_READ], 1, szSetting, -1, SQLITE_STATIC);
	sqlite3_bind_text(set_stmts_prep[SQL_SET_STMT_READ], 2, szModule, -1, SQLITE_STATIC);
	sqlite3_bind_int(set_stmts_prep[SQL_SET_STMT_READ], 3, contactID);
	if (sql_step(set_stmts_prep[SQL_SET_STMT_READ]) != SQLITE_ROW) {
		/*if (dbv->type != DBVT_BLOB) {
			DBVARIANT* pCachedValue = settings_getCachedValue(contactID, szCachedSettingName, 1);

			if (pCachedValue != NULL)
				pCachedValue->type = DBVT_DELETED;
		}*/
		sql_reset(set_stmts_prep[SQL_SET_STMT_READ]);
		return 1;
	}
	dbv->type = (int)sqlite3_column_int(set_stmts_prep[SQL_SET_STMT_READ], 0);
	switch (dbv->type) {
	case DBVT_BYTE:
		dbv->bVal = (uint8_t)sqlite3_column_int(set_stmts_prep[SQL_SET_STMT_READ], 1);
		break;
	case DBVT_WORD:
		dbv->wVal = (uint16_t)sqlite3_column_int(set_stmts_prep[SQL_SET_STMT_READ], 1);
		break;
	case DBVT_DWORD:
		dbv->dVal = (uint32_t)sqlite3_column_int(set_stmts_prep[SQL_SET_STMT_READ], 1);
		break;
	case DBVT_UTF8:
	case DBVT_ASCIIZ:
	{
		const char *p = (const char *)sqlite3_column_text(set_stmts_prep[SQL_SET_STMT_READ], 1);

		if (p != NULL) {
			size_t len = strlen(p) + 1;
			size_t copylen = isStatic ? (len < dbv->cchVal ? len : dbv->cchVal) : len;
			if (!isStatic)
				dbv->pszVal = (char*)mir_alloc(len);
			memmove(dbv->pszVal, p, copylen);
		}
		else
			dbv->pszVal = 0;
		break;
	}
	case DBVT_BLOB:
	{
		size_t len = sqlite3_column_bytes(set_stmts_prep[SQL_SET_STMT_READ], 1);

		if (len) {
			size_t copylen = isStatic ? (len < dbv->cpbVal ? len : dbv->cpbVal) : len;
			if (!isStatic)
				dbv->pbVal = (uint8_t*)mir_alloc(copylen);
			memcpy(dbv->pbVal, sqlite3_column_blob(set_stmts_prep[SQL_SET_STMT_READ], 1), copylen);
			dbv->cpbVal = (uint16_t)copylen;
		}
		else {
			dbv = 0;
		}
	}
	}
	sql_reset(set_stmts_prep[SQL_SET_STMT_READ]);
	// add to cache
	if (dbv->type != DBVT_BLOB/* && dbv->type != DBVT_ENCRYPTED*/) {
		pCachedValue = m_cache->GetCachedValuePtr(contactID, szCachedSettingName, 1);
		if (pCachedValue != nullptr)
			m_cache->SetCachedVariant(dbv, pCachedValue);
	}

	return 0;
}

STDMETHODIMP_(BOOL) CDbxSQLite::EnumContactSettings(MCONTACT hContact, DBSETTINGENUMPROC pfnEnumProc, const char *szModule, void *param)
{
	if (szModule == nullptr)
		return -1;

	mir_cslockfull lock(m_csDbAccess);

	int res = -1;
	sqlite3_bind_int(set_stmts_prep[SQL_SET_STMT_ENUM], 1, hContact);
	sqlite3_bind_text(set_stmts_prep[SQL_SET_STMT_ENUM], 2, szModule, -1, SQLITE_STATIC);
	while (sql_step(set_stmts_prep[SQL_SET_STMT_ENUM]) == SQLITE_ROW) {
		const char *sczSetting = (const char*)sqlite3_column_text(set_stmts_prep[SQL_SET_STMT_ENUM], 0);
		if (sczSetting) {
			lock.unlock();
			res = (pfnEnumProc)(sczSetting, param);
			lock.lock();
		}
	}
	sql_reset(set_stmts_prep[SQL_SET_STMT_ENUM]);
	return res;
}
