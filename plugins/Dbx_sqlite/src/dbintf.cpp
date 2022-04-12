#include "stdafx.h"

CDbxSQLite::CDbxSQLite(const wchar_t *pwszFileName, bool bReadOnly, bool bShared) :
	m_impl(*this),
	m_wszFileName(mir_wstrdup(pwszFileName)),
	m_safetyMode(true),
	m_bReadOnly(bReadOnly),
	m_bShared(bShared),
	m_modules(1, strcmp)
{
}

CDbxSQLite::~CDbxSQLite()
{
	if (m_bTranStarted) {
		int rc = sqlite3_exec(m_db, "commit;", nullptr, nullptr, nullptr);
		logError(rc, __FILE__, __LINE__);
	}

	UninitEvents();

	if (m_db) {
		int rc = sqlite3_close(m_db);
		logError(rc, __FILE__, __LINE__);

		m_db = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDbxSQLite::Create()
{
	ptrA path(mir_utf8encodeW(m_wszFileName));
	int rc = sqlite3_open_v2(path, &m_db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_EXCLUSIVE, nullptr);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_OK) {
		logError(rc, __FILE__, __LINE__);
		return 1;
	}

	rc = sqlite3_exec(m_db, "CREATE TABLE contacts (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT);", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);

	rc = sqlite3_exec(m_db, "CREATE TABLE crypto (id INTEGER NOT NULL PRIMARY KEY, data NOT NULL);", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);

	rc = sqlite3_exec(m_db, "CREATE TABLE events (id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, contact_id INTEGER NOT NULL, module TEXT NOT NULL,"
		"timestamp INTEGER NOT NULL, type INTEGER NOT NULL, flags INTEGER NOT NULL, data BLOB, server_id TEXT);", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);

	rc = sqlite3_exec(m_db, "CREATE INDEX idx_events_contactid_timestamp ON events(contact_id, timestamp);", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);

	rc = sqlite3_exec(m_db, "CREATE INDEX idx_events_module_serverid ON events(module, server_id);", nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK)
		logError(rc, __FILE__, __LINE__);

	rc = sqlite3_exec(m_db, "CREATE TABLE events_srt (id INTEGER NOT NULL, contact_id INTEGER NOT NULL, timestamp INTEGER, PRIMARY KEY(contact_id, timestamp, id));", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);

	rc = sqlite3_exec(m_db, "CREATE TABLE settings (contact_id INTEGER NOT NULL, module TEXT NOT NULL, setting TEXT NOT NULL, type INTEGER NOT NULL, value NOT NULL,"
		"PRIMARY KEY(contact_id, module, setting)) WITHOUT ROWID;", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);

	rc = sqlite3_exec(m_db, "CREATE INDEX idx_settings_module ON settings(module);", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDbxSQLite::Check()
{
	FILE *hFile = _wfopen(m_wszFileName, L"rb");
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
	ptrA path(mir_utf8encodeW(m_wszFileName));
	int rc = sqlite3_open_v2(path, &database, SQLITE_OPEN_READONLY | SQLITE_OPEN_EXCLUSIVE, nullptr);
	if (rc != SQLITE_OK) {
		logError(rc, __FILE__, __LINE__);
		return EGROKPRF_DAMAGED;
	}

	sqlite3_close(database);

	return EGROKPRF_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDbxSQLite::Load()
{
	if (!LockName(m_wszFileName))
		return EGROKPRF_CANTREAD;

	ptrA path(mir_utf8encodeW(m_wszFileName));
	int flags = 0;
	if (!m_bShared)
		flags |= SQLITE_OPEN_EXCLUSIVE;
	if (m_bReadOnly)
		flags |= SQLITE_OPEN_READONLY;
	else
		flags |= SQLITE_OPEN_READWRITE;

	int rc = sqlite3_open_v2(path, &m_db, flags, nullptr);
	if (rc != SQLITE_OK) {
		logError(rc, __FILE__, __LINE__);
		return EGROKPRF_CANTREAD;
	}

	rc = sqlite3_exec(m_db, "pragma locking_mode = EXCLUSIVE;", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);
	rc = sqlite3_exec(m_db, "pragma synchronous = NORMAL;", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);
	rc = sqlite3_exec(m_db, "pragma foreign_keys = OFF;", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);
	rc = sqlite3_exec(m_db, "pragma journal_mode = OFF;", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);
	if (rc == SQLITE_BUSY) {
		sqlite3_close(m_db);
		return EGROKPRF_CANTREAD;
	}

	InitContacts();
	InitEncryption();
	InitSettings();
	InitEvents();

	if (InitCrypt())
		return EGROKPRF_CANTREAD;

	m_bTranStarted = true;
	rc = sqlite3_exec(m_db, "begin transaction;", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);
	return EGROKPRF_NOERROR;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL CDbxSQLite::Backup(LPCWSTR profile)
{
	sqlite3 *database = nullptr;
	ptrA path(mir_utf8encodeW(profile));
	int rc = sqlite3_open_v2(path, &database, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_EXCLUSIVE, nullptr);
	if (rc != SQLITE_OK) {
		logError(rc, __FILE__, __LINE__);
		return rc;
	}

	mir_cslock lock(m_csDbAccess);

	sqlite3_backup *backup = sqlite3_backup_init(database, "main", m_db, "main");
	if (backup == nullptr) {
		sqlite3_close(database);
		DeleteFileW(profile);
		return ERROR_BACKUP_CONTROLLER;
	}

	rc = sqlite3_exec(m_db, "commit;", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);

	logError(sqlite3_backup_step(backup, -1), __FILE__, __LINE__);
	logError(sqlite3_backup_finish(backup), __FILE__, __LINE__);
	sqlite3_close(database);

	rc = sqlite3_exec(m_db, "begin transaction;", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);
	return 0;
}

BOOL CDbxSQLite::Compact()
{
	mir_cslock lck(m_csDbAccess);
	int rc = sqlite3_exec(m_db, "pragma optimize;", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);

	rc = sqlite3_exec(m_db, "commit;", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);

	rc = sqlite3_exec(m_db, "vacuum;", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);

	rc = sqlite3_exec(m_db, "begin transaction;", nullptr, nullptr, nullptr);
	logError(rc, __FILE__, __LINE__);
	return 0;
}

void CDbxSQLite::DBFlush(bool bForce)
{
	if (bForce) {
		mir_cslock lck(m_csDbAccess);

		int rc = sqlite3_exec(m_db, "commit;", nullptr, nullptr, nullptr);
		logError(rc, __FILE__, __LINE__);

		rc = sqlite3_exec(m_db, "begin transaction;", nullptr, nullptr, nullptr);
		logError(rc, __FILE__, __LINE__);
	}
	else if (m_safetyMode)
		m_impl.m_timer.Start(50);
}

BOOL CDbxSQLite::Flush()
{
	DBFlush(true);
	sqlite3_db_cacheflush(m_db);
	return ERROR_SUCCESS;
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
