#include "stdafx.h"

enum {
	SQL_SET_STMT_ENUM = 0,
	SQL_SET_STMT_GET,
	SQL_SET_STMT_REPLACE,
	SQL_SET_STMT_DELETE,
	SQL_SET_STMT_ENUMMODULE,
	SQL_SET_STMT_CHANGES
};

static CQuery settings_stmts[] =
{
	{ "SELECT DISTINCT module FROM settings;" },
	{ "SELECT type, value FROM settings WHERE contact_id = ? AND module = ? AND setting = ? LIMIT 1;" },
	{ "REPLACE INTO settings(contact_id, module, setting, type, value) VALUES (?, ?, ?, ?, ?);" },
	{ "DELETE FROM settings WHERE contact_id = ? AND module = ? AND setting = ?;" },
	{ "SELECT setting FROM settings WHERE contact_id = ? AND module = ?;" },
	{ "SELECT CHANGES() FROM settings;" },
};

void CDbxSQLite::InitSettings()
{
	for (auto &it : settings_stmts)
		sqlite3_prepare_v3(m_db, it.szQuery, -1, SQLITE_PREPARE_PERSISTENT, &it.pQuery, nullptr);

	sqlite3_stmt *stmt = nullptr;
	sqlite3_prepare_v2(m_db, "SELECT type, value, contact_id, module, setting FROM settings;", -1, &stmt, nullptr);
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		MCONTACT hContact = sqlite3_column_int64(stmt, 2);
		auto *szModule = (const char *)sqlite3_column_text(stmt, 3);
		auto *szSetting = (const char *)sqlite3_column_text(stmt, 4);

		size_t settingNameLen = strlen(szSetting);
		size_t moduleNameLen = strlen(szModule);

		char *szCachedSettingName = m_cache->GetCachedSetting(szModule, szSetting, moduleNameLen, settingNameLen);

		DBVARIANT *dbv = m_cache->GetCachedValuePtr(hContact, szCachedSettingName, 1);
		if (dbv == nullptr) // garbage! a setting for removed/non-existent contact
			continue;

		dbv->type = (int)sqlite3_column_int(stmt, 0);
		switch (dbv->type) {
		case DBVT_BYTE:
			dbv->bVal = sqlite3_column_int(stmt, 1);
			break;

		case DBVT_WORD:
			dbv->wVal = sqlite3_column_int(stmt, 1);
			break;

		case DBVT_DWORD:
			dbv->dVal = sqlite3_column_int64(stmt, 1);
			break;

		case DBVT_ASCIIZ:
		case DBVT_UTF8:
			dbv->cchVal = sqlite3_column_bytes(stmt, 1);
			{
				const char *value = (const char *)sqlite3_column_text(stmt, 1);
				dbv->pszVal = (char *)mir_alloc(dbv->cchVal + 1);
				memcpy(dbv->pszVal, value, dbv->cchVal);
				dbv->pszVal[dbv->cchVal] = 0;
			}
			break;

		case DBVT_ENCRYPTED:
		case DBVT_BLOB:
			dbv->cpbVal = sqlite3_column_bytes(stmt, 1);
			{
				const char *data = (const char *)sqlite3_column_blob(stmt, 1);
				dbv->pbVal = (BYTE *)mir_alloc(dbv->cpbVal + 1);
				memcpy(dbv->pbVal, data, dbv->cpbVal);
				dbv->pbVal[dbv->cpbVal] = 0;
			}
			break;
		}
	}
	sqlite3_finalize(stmt);

	FillContactSettings();
}

void CDbxSQLite::UninitSettings()
{
	for (auto &it : settings_stmts)
		sqlite3_finalize(it.pQuery);
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL CDbxSQLite::EnumModuleNames(DBMODULEENUMPROC pFunc, void *param)
{
	LIST<char> modules(100);
	{
		sqlite3_stmt *stmt = settings_stmts[SQL_SET_STMT_ENUM].pQuery;
		int rc = 0;
		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
			const char *value = (const char *)sqlite3_column_text(stmt, 0);
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

/////////////////////////////////////////////////////////////////////////////////////////

BOOL CDbxSQLite::WriteContactSettingWorker(MCONTACT hContact, DBCONTACTWRITESETTING &dbcws)
{
	sqlite3_stmt *stmt = settings_stmts[SQL_SET_STMT_REPLACE].pQuery;
	sqlite3_bind_int64(stmt, 1, hContact);
	sqlite3_bind_text(stmt, 2, dbcws.szModule, (int)mir_strlen(dbcws.szModule), nullptr);
	sqlite3_bind_text(stmt, 3, dbcws.szSetting, (int)mir_strlen(dbcws.szSetting), nullptr);
	sqlite3_bind_int(stmt, 4, dbcws.value.type);
	switch (dbcws.value.type) {
	case DBVT_BYTE:
		sqlite3_bind_int(stmt, 5, dbcws.value.bVal);
		break;
	case DBVT_WORD:
		sqlite3_bind_int(stmt, 5, dbcws.value.wVal);
		break;
	case DBVT_DWORD:
		sqlite3_bind_int64(stmt, 5, dbcws.value.dVal);
		break;
	case DBVT_ASCIIZ:
	case DBVT_UTF8:
		sqlite3_bind_text(stmt, 5, dbcws.value.pszVal, dbcws.value.cchVal, nullptr);
		break;
	case DBVT_ENCRYPTED:
	case DBVT_BLOB:
		sqlite3_bind_blob(stmt, 5, dbcws.value.pbVal, dbcws.value.cpbVal, nullptr);
		break;
	}

	int rc = sqlite3_step(stmt);
	assert(rc == SQLITE_ROW || rc == SQLITE_DONE);
	sqlite3_reset(stmt);
	if (rc != SQLITE_DONE)
		return 1;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

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
	if (szCachedSettingName[-1] == 0) { // it's not a resident variable
		mir_cslock lock(m_csDbAccess);
		sqlite3_stmt *stmt = settings_stmts[SQL_SET_STMT_DELETE].pQuery;
		sqlite3_bind_int64(stmt, 1, hContact);
		sqlite3_bind_text(stmt, 2, szModule, (int)mir_strlen(szModule), nullptr);
		sqlite3_bind_text(stmt, 3, szSetting, (int)mir_strlen(szSetting), nullptr);
		int rc = sqlite3_step(stmt);
		assert(rc == SQLITE_DONE);
		sqlite3_reset(stmt);
		stmt = settings_stmts[SQL_SET_STMT_CHANGES].pQuery;
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

/////////////////////////////////////////////////////////////////////////////////////////

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
		sqlite3_stmt *stmt = settings_stmts[SQL_SET_STMT_ENUMMODULE].pQuery;
		sqlite3_bind_int64(stmt, 1, hContact);
		sqlite3_bind_text(stmt, 2, szModule, (int)mir_strlen(szModule), nullptr);
		int rc = 0;
		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
			const char *value = (const char *)sqlite3_column_text(stmt, 0);
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
