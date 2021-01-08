#include "stdafx.h"

CDbxSQLite::CDbxSQLite(sqlite3 *database)
	: m_db(database),
	m_safetyMode(true),
	m_modules(1, strcmp)
{
}

CDbxSQLite::~CDbxSQLite()
{
	UninitEvents();
	UninitContacts();
	UninitSettings();

	if (m_db) {
		int rc = sqlite3_close(m_db);
		if (rc != SQLITE_OK)
		{
			//TODO:
		}
		m_db = nullptr;
	}
}

int CDbxSQLite::Create(const wchar_t *profile)
{
	sqlite3 *database = nullptr;
	ptrA path(mir_utf8encodeW(profile));
	int rc = sqlite3_open_v2(path, &database, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, nullptr);
	if (rc != SQLITE_OK)
		return 1;

	rc = sqlite3_exec(database, "CREATE TABLE contacts (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);", nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK)
	{
		//TODO: handle error
	}
	rc = sqlite3_exec(database, "CREATE TABLE events (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, contact_id INTEGER NOT NULL, module TEXT NOT NULL,"
		"timestamp INTEGER NOT NULL, type INTEGER NOT NULL, flags INTEGER NOT NULL, data BLOB, server_id TEXT);", nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK)
	{
		//TODO: handle error
	}
	rc = sqlite3_exec(database, "CREATE INDEX idx_events_contactid_timestamp ON events(contact_id, timestamp);", nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK)
	{
		//TODO: handle error
	}
	rc = sqlite3_exec(database, "CREATE INDEX idx_events_module_serverid ON events(module, server_id);", nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK)
	{
		//TODO: handle error
	}
    rc = sqlite3_exec(database, "CREATE TABLE events_srt (id INTEGER NOT NULL, contact_id INTEGER NOT NULL, timestamp INTEGER, PRIMARY KEY(contact_id, timestamp, id));",
		nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK)
	{
		//TODO: handle error
	}
	rc = sqlite3_exec(database, "CREATE TABLE settings (contact_id INTEGER NOT NULL, module TEXT NOT NULL, setting TEXT NOT NULL, type INTEGER NOT NULL, value ANY,"
		"PRIMARY KEY(contact_id, module, setting)) WITHOUT ROWID;", nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK)
	{
		//TODO: handle error
	}
	rc = sqlite3_exec(database, "CREATE INDEX idx_settings_module ON settings(module);", nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK)
	{
		//TODO: handle error
	}

	sqlite3_close(database);

	return 0;
}

int CDbxSQLite::Check(const wchar_t *profile)
{
	FILE *hFile = _wfopen(profile, L"rb");

	if (hFile == INVALID_HANDLE_VALUE)
		return EGROKPRF_CANTREAD;

	char header[16] = {};
	size_t size = sizeof(header);

	if (fread(header, sizeof(char), size, hFile) != size) {
		fclose(hFile);
		return EGROKPRF_CANTREAD;
	}

	fclose(hFile);

	if (memcmp(header, SQLITE_HEADER_STR, mir_strlen(SQLITE_HEADER_STR)) != 0)
		return EGROKPRF_UNKHEADER;

	sqlite3 *database = nullptr;
	ptrA path(mir_utf8encodeW(profile));
	int rc = sqlite3_open_v2(path, &database, SQLITE_OPEN_READONLY, nullptr);
	if (rc != SQLITE_OK)
		return EGROKPRF_DAMAGED;

	sqlite3_close(database);

	return EGROKPRF_NOERROR;
}

MDatabaseCommon* CDbxSQLite::Load(const wchar_t *profile, int readonly)
{
	sqlite3 *database = nullptr;
	ptrA path(mir_utf8encodeW(profile));
	int flags = SQLITE_OPEN_READWRITE;
	if (readonly)
		flags |= SQLITE_OPEN_READONLY;
	int rc = sqlite3_open_v2(path, &database, flags, nullptr);
	if (rc != SQLITE_OK)
		return nullptr;

	rc = sqlite3_exec(database, "begin transaction;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK)
	{
		//TODO: handle error
	}
	rc = sqlite3_exec(database, "pragma locking_mode = EXCLUSIVE;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
      // TODO: handle error
    }
    rc = sqlite3_exec(database, "pragma synchronous = NORMAL;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
      // TODO: handle error
    }
    sqlite3_exec(database, "pragma foreign_keys = OFF;", nullptr, nullptr, nullptr);
	rc = sqlite3_exec(database, "pragma journal_mode = OFF;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
      // TODO: handle error
    }
    rc = sqlite3_exec(database, "commit;", nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK) {
      // TODO: handle error
    }


	CDbxSQLite *db = new CDbxSQLite(database);
	if (!db->InitCrypt()) {
		delete db;
		return nullptr;
	}
	
	db->InitContacts();
	db->InitSettings();
	db->InitEvents();
	return db;
}

BOOL CDbxSQLite::Compact()
{
	sqlite3_exec(m_db, "pragma optimize;", nullptr, nullptr, nullptr);
	sqlite3_exec(m_db, "vacuum;", nullptr, nullptr, nullptr);
	return 0;
}

BOOL CDbxSQLite::Backup(LPCWSTR profile)
{
	sqlite3 *database = nullptr;
	ptrA path(mir_utf8encodeW(profile));
	int rc = sqlite3_open_v2(path, &database, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, nullptr);
	if (rc != SQLITE_OK)
		return FALSE;

	mir_cslock lock(m_csDbAccess);

	sqlite3_backup *backup = sqlite3_backup_init(database, "main", m_db, "main");
	if (backup) {
		sqlite3_backup_step(backup, -1);
		sqlite3_backup_finish(backup);
	}
	sqlite3_close(database);

	return 0;
}

BOOL CDbxSQLite::IsRelational()
{
	return TRUE;
}

void CDbxSQLite::SetCacheSafetyMode(BOOL value)
{
	// hack to increase import speed
	if (!value)
		sqlite3_exec(m_db, "pragma synchronous = OFF;", nullptr, nullptr, nullptr);
	else
		sqlite3_exec(m_db, "pragma synchronous = NORMAL;", nullptr, nullptr, nullptr);
	m_safetyMode = value != FALSE;
}
