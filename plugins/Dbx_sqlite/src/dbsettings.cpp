#include "stdafx.h"

enum {
	SQL_SET_STMT_ENUM = 0,
	SQL_SET_STMT_GET,
	SQL_SET_STMT_REPLACE,
	SQL_SET_STMT_DELETE,
	SQL_SET_STMT_ENUMMODULE,
	SQL_SET_STMT_CHANGES,
	SQL_SET_STMT_NUM
};

static char *settings_stmts[SQL_SET_STMT_NUM] = {
	"SELECT DISTINCT module FROM settings;",
	"SELECT type, value FROM settings WHERE contact_id = ? AND module = ? AND setting = ? LIMIT 1;",
	"REPLACE INTO settings(contact_id, module, setting, type, value) VALUES (?, ?, ?, ?, ?);",
	"DELETE FROM settings WHERE contact_id = ? AND module = ? AND setting = ?;",
	"SELECT setting FROM settings WHERE contact_id = ? AND module = ?;",
	"SELECT changes() FROM settings;"
};

static sqlite3_stmt *settings_stmts_prep[SQL_SET_STMT_NUM] = { 0 };

void CDbxSQLite::InitSettings()
{
	for (size_t i = 0; i < SQL_SET_STMT_NUM; i++)
		sqlite3_prepare_v3(m_db, settings_stmts[i], -1, SQLITE_PREPARE_PERSISTENT, &settings_stmts_prep[i], nullptr);
}

void CDbxSQLite::UninitSettings()
{
	for (size_t i = 0; i < SQL_SET_STMT_NUM; i++)
		sqlite3_finalize(settings_stmts_prep[i]);
}

BOOL CDbxSQLite::EnumModuleNames(DBMODULEENUMPROC pFunc, void *param)
{
	LIST<char> modules(100);
	{
		sqlite3_stmt *stmt = settings_stmts_prep[SQL_SET_STMT_ENUM];
		int rc = 0;
		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
			const char *value = (const char*)sqlite3_column_text(stmt, 0);
			modules.insert(mir_strdup(value));
		}
		assert(rc == SQLITE_ROW || rc == SQLITE_DONE);
		sqlite3_reset(stmt);
	}

	int result = -1;
	for (auto &module : modules) {
		result = pFunc(module, param);
		mir_free(module);
	}
	return result;
}

static bool ValidLookupName(LPCSTR szModule, LPCSTR szSetting)
{
	if (!strcmp(szModule, META_PROTO))
		return strcmp(szSetting, "IsSubcontact") && strcmp(szSetting, "ParentMetaID");

	if (!strcmp(szModule, "Ignore"))
		return false;

	return true;
}

BOOL CDbxSQLite::GetContactSettingWorker(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv, int isStatic)
{
	if (szSetting == nullptr || szModule == nullptr)
		return 1;

	DBCachedContact *cc = nullptr;
	if (hContact) {
		cc = m_cache->GetCachedContact(hContact);
		if (cc == nullptr)
			return 1;
	}

	mir_cslock lock(m_csDbAccess);

LBL_Seek:
	char *cachedSettingName = m_cache->GetCachedSetting(szModule, szSetting, mir_strlen(szModule), mir_strlen(szSetting));
	DBVARIANT *pCachedValue = m_cache->GetCachedValuePtr(hContact, cachedSettingName, 0);
	if (pCachedValue != nullptr) {
		if (pCachedValue->type == DBVT_UTF8) {
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

		return (pCachedValue->type == DBVT_DELETED) ? 1 : 0;
	}

	// never look db for the resident variable
	if (cachedSettingName[-1] != 0)
		return 1;

	sqlite3_stmt *stmt = settings_stmts_prep[SQL_SET_STMT_GET];
	sqlite3_bind_int64(stmt, 1, hContact);
	sqlite3_bind_text(stmt, 2, szModule, (int)mir_strlen(szModule), nullptr);
	sqlite3_bind_text(stmt, 3, szSetting, (int)mir_strlen(szSetting), nullptr);
	int rc = sqlite3_step(stmt);
	assert(rc == SQLITE_ROW || rc == SQLITE_DONE);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		if (rc == SQLITE_DONE && cc && cc->IsMeta() && ValidLookupName(szModule, szSetting)) {
			if (hContact = db_mc_getDefault(hContact)) {
				if (szModule = Proto_GetBaseAccountName(hContact))
					goto LBL_Seek;
			}
		}
		return 1;
	}
	dbv->type = (int)sqlite3_column_int(stmt, 0);
	switch (dbv->type) {
	case DBVT_DELETED:
		dbv->type = DBVT_DELETED;
		sqlite3_reset(stmt);
		return 2;
	case DBVT_BYTE:
		dbv->bVal = sqlite3_column_int(stmt, 1);
		break;
	case DBVT_WORD:
		dbv->wVal = sqlite3_column_int(stmt, 1);
		break;
	case DBVT_DWORD:
		dbv->dVal = sqlite3_column_int64(stmt, 1);
		break;
	case DBVT_UTF8:
	{
		dbv->cchVal = sqlite3_column_bytes(stmt, 1);
		const char *value = (const char*)sqlite3_column_text(stmt, 1);
		if (!isStatic)
			dbv->pszVal = (char*)mir_alloc(dbv->cchVal + 1);
		memcpy(dbv->pszVal, value, dbv->cchVal);
		dbv->pszVal[dbv->cchVal] = 0;
		break;
	}
	case DBVT_BLOB:
	{
		dbv->cpbVal = sqlite3_column_bytes(stmt, 1);
		const char *data = (const char*)sqlite3_column_blob(stmt, 1);
		if (!isStatic)
			dbv->pbVal = (BYTE*)mir_alloc(dbv->cpbVal + 1);
		memcpy(dbv->pbVal, data, dbv->cpbVal);
		break;
	}
	}
	sqlite3_reset(stmt);

	// add to cache
	if (dbv->type != DBVT_BLOB) {
		pCachedValue = m_cache->GetCachedValuePtr(hContact, cachedSettingName, 1);
		if (pCachedValue != nullptr)
			m_cache->SetCachedVariant(dbv, pCachedValue);
	}

	return 0;
}

BOOL CDbxSQLite::WriteContactSetting(MCONTACT hContact, DBCONTACTWRITESETTING *dbcws)
{
	if (dbcws == nullptr || dbcws->szSetting == nullptr || dbcws->szModule == nullptr)
		return 1;

	if (hContact) {
		DBCachedContact *cc = m_cache->GetCachedContact(hContact);
		if (cc == nullptr)
			return 1;
	}

	DBCONTACTWRITESETTING dbcwNotif = *dbcws;
	// we work only with utf-8 inside
	switch (dbcwNotif.value.type) {
	case DBVT_UTF8:
		dbcwNotif.value.pszVal = mir_strdup(dbcws->value.pszVal);
		break;
	case DBVT_ASCIIZ:
	{
		ptrA value(mir_utf8encode(dbcws->value.pszVal));
		dbcwNotif.value.pszVal = NEWSTR_ALLOCA(value);
		dbcwNotif.value.type = DBVT_UTF8;
		break;
	}
	case DBVT_WCHAR:
	{
		T2Utf value(dbcwNotif.value.pwszVal);
		dbcwNotif.value.pszVal = NEWSTR_ALLOCA(value);
		dbcwNotif.value.type = DBVT_UTF8;
		break;
	}
	}
	DBCONTACTWRITESETTING dbcwWork = dbcwNotif;
	if (dbcwWork.value.type == DBVT_UTF8)
		dbcwWork.value.cchVal = (WORD)strlen(dbcwWork.value.pszVal);

	mir_cslockfull lock(m_csDbAccess);

	char *cachedSettingName = m_cache->GetCachedSetting(dbcwWork.szModule, dbcwWork.szSetting, mir_strlen(dbcwWork.szModule), mir_strlen(dbcwWork.szSetting));
	bool isResident = cachedSettingName[-1] != 0;

	// we don't cache blobs
	if (dbcwWork.value.type != DBVT_BLOB) {
		DBVARIANT *cachedValue = m_cache->GetCachedValuePtr(hContact, cachedSettingName, 1);
		if (cachedValue != nullptr) {
			bool isIdentical = false;
			if (cachedValue->type == dbcwWork.value.type) {
				switch (dbcwWork.value.type) {
				case DBVT_BYTE:
					isIdentical = cachedValue->bVal == dbcwWork.value.bVal;
					break;
				case DBVT_WORD:
					isIdentical = cachedValue->wVal == dbcwWork.value.wVal;
					break;
				case DBVT_DWORD:
					isIdentical = cachedValue->dVal == dbcwWork.value.dVal;
					break;
				case DBVT_UTF8:
					isIdentical = mir_strcmp(cachedValue->pszVal, dbcwWork.value.pszVal) == 0;
					break;
				}
				if (isIdentical)
					return 0;
			}
			m_cache->SetCachedVariant(&dbcwWork.value, cachedValue);
		}
		if (isResident) {
			lock.unlock();
			NotifyEventHooks(g_hevSettingChanged, hContact, (LPARAM)&dbcwWork);
			return 0;
		}
	}
	else m_cache->GetCachedValuePtr(hContact, cachedSettingName, -1);

	sqlite3_stmt *stmt = settings_stmts_prep[SQL_SET_STMT_REPLACE];
	sqlite3_bind_int64(stmt, 1, hContact);
	sqlite3_bind_text(stmt, 2, dbcwWork.szModule, (int)mir_strlen(dbcwWork.szModule), nullptr);
	sqlite3_bind_text(stmt, 3, dbcwWork.szSetting, (int)mir_strlen(dbcwWork.szSetting), nullptr);
	sqlite3_bind_int(stmt, 4, dbcwWork.value.type);
	switch (dbcwWork.value.type) {
	case DBVT_BYTE:
		sqlite3_bind_int(stmt, 5, dbcwWork.value.bVal);
		break;
	case DBVT_WORD:
		sqlite3_bind_int(stmt, 5, dbcwWork.value.wVal);
		break;
	case DBVT_DWORD:
		sqlite3_bind_int64(stmt, 5, dbcwWork.value.dVal);
		break;
	case DBVT_UTF8:
		sqlite3_bind_text(stmt, 5, dbcwWork.value.pszVal, dbcwWork.value.cchVal, nullptr);
		break;
	case DBVT_BLOB:
		sqlite3_bind_blob(stmt, 5, dbcwWork.value.pbVal, dbcwWork.value.cpbVal, nullptr);
		break;
	}
	int rc = sqlite3_step(stmt);
	assert(rc == SQLITE_ROW || rc == SQLITE_DONE);
	sqlite3_reset(stmt);
	if (rc != SQLITE_DONE)
		return 1;

	lock.unlock();

	NotifyEventHooks(g_hevSettingChanged, hContact, (LPARAM)&dbcwNotif);

	return 0;
}

BOOL CDbxSQLite::DeleteContactSetting(MCONTACT hContact, LPCSTR szModule, LPCSTR szSetting)
{
	if (szSetting == nullptr || szModule == nullptr)
		return 1;

	if (hContact) {
		DBCachedContact *cc = m_cache->GetCachedContact(hContact);
		if (cc == nullptr)
			return 1;
	}
	
	char *szCachedSettingName = m_cache->GetCachedSetting(szModule, szSetting, mir_strlen(szModule), mir_strlen(szSetting));
	if (szCachedSettingName[-1] == 0)  // it's not a resident variable
	{
		mir_cslock lock(m_csDbAccess);
		sqlite3_stmt *stmt = settings_stmts_prep[SQL_SET_STMT_DELETE];
		sqlite3_bind_int64(stmt, 1, hContact);
		sqlite3_bind_text(stmt, 2, szModule, (int)mir_strlen(szModule), nullptr);
		sqlite3_bind_text(stmt, 3, szSetting, (int)mir_strlen(szSetting), nullptr);
		int rc = sqlite3_step(stmt);
		assert(rc == SQLITE_DONE);
		sqlite3_reset(stmt);
		stmt = settings_stmts_prep[SQL_SET_STMT_CHANGES];
		rc = sqlite3_step(stmt);
		assert(rc == SQLITE_ROW);
		int deleted = sqlite3_column_int(stmt, 0);
		sqlite3_reset(stmt);
		if (deleted == 0)
			return 1;
	}
	m_cache->GetCachedValuePtr(hContact, szCachedSettingName, -1);

	// notify
	DBCONTACTWRITESETTING dbcws = { 0 };
	dbcws.szModule = szModule;
	dbcws.szSetting = szSetting;
	dbcws.value.type = DBVT_DELETED;
	NotifyEventHooks(g_hevSettingChanged, hContact, (LPARAM)&dbcws);

	return 0;
}

BOOL CDbxSQLite::EnumContactSettings(MCONTACT hContact, DBSETTINGENUMPROC pfnEnumProc, const char *szModule, void *param)
{
	if (szModule == nullptr)
		return -1;

	if (hContact) {
		DBCachedContact *cc = m_cache->GetCachedContact(hContact);
		if (cc == nullptr)
			return -1;
	}

	LIST<char> settings(100);
	{
		mir_cslock lock(m_csDbAccess);
		sqlite3_stmt *stmt = settings_stmts_prep[SQL_SET_STMT_ENUMMODULE];
		sqlite3_bind_int64(stmt, 1, hContact);
		sqlite3_bind_text(stmt, 2, szModule, (int)mir_strlen(szModule), nullptr);
		int rc = 0;
		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
			const char *value = (const char*)sqlite3_column_text(stmt, 0);
			settings.insert(mir_strdup(value));
		}
		assert(rc == SQLITE_ROW || rc == SQLITE_DONE);
		sqlite3_reset(stmt);
	}

	int result = -1;
	for (auto &setting : settings) {
		result = pfnEnumProc(setting, param);
		mir_free(setting);
	}
	return result;
}
