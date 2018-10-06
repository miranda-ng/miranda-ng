#include "stdafx.h"

CDbxSQLite::CDbxSQLite(sqlite3 *database)
	: m_db(database),
	m_safetyMode(true)
{
	hContactAddedEvent = CreateHookableEvent(ME_DB_CONTACT_ADDED);
	hContactDeletedEvent = CreateHookableEvent(ME_DB_CONTACT_DELETED);
	hEventAddedEvent = CreateHookableEvent(ME_DB_EVENT_ADDED);
	hEventEditedEvent = CreateHookableEvent(ME_DB_EVENT_EDITED);
	hEventDeletedEvent = CreateHookableEvent(ME_DB_EVENT_DELETED);
	hEventFilterAddedEvent = CreateHookableEvent(ME_DB_EVENT_FILTER_ADD);
	hEventMarkedRead = CreateHookableEvent(ME_DB_EVENT_MARKED_READ);
	hSettingChangeEvent = CreateHookableEvent(ME_DB_CONTACT_SETTINGCHANGED);
}

CDbxSQLite::~CDbxSQLite()
{
	DestroyHookableEvent(hContactAddedEvent);
	DestroyHookableEvent(hContactDeletedEvent);
	DestroyHookableEvent(hEventAddedEvent);
	DestroyHookableEvent(hEventEditedEvent);
	DestroyHookableEvent(hEventDeletedEvent);
	DestroyHookableEvent(hEventFilterAddedEvent);
	DestroyHookableEvent(hEventMarkedRead);
	DestroyHookableEvent(hSettingChangeEvent);

	UninitEvents();
	UninitContacts();
	UninitSettings();

	if (m_db) {
		sqlite3_close(m_db);
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

	sqlite3_exec(database, "create table contacts (id integer not null primary key autoincrement);", nullptr, nullptr, nullptr);
	sqlite3_exec(database, "create table events (id integer not null primary key autoincrement, timestamp integer not null, type integer not null, flags integer not null, size integer not null, data blob, module varchar(255) not null, server_id varchar(64));", nullptr, nullptr, nullptr);
	sqlite3_exec(database, "create index idx_events_module_serverid on events(module, server_id);", nullptr, nullptr, nullptr);
	sqlite3_exec(database, "create table contact_events (contact_id integer not null, event_id integer not null, timestamp integer not null, primary key(contact_id, event_id)) without rowid;", nullptr, nullptr, nullptr);
	sqlite3_exec(database, "create index idx_contact_events_eventid on contact_events(event_id);", nullptr, nullptr, nullptr);
	sqlite3_exec(database, "create table settings (contact_id integer not null, module varchar(255) not null, setting varchar(255) not null, type integer not null, value any, primary key(contact_id, module, setting)) without rowid;", nullptr, nullptr, nullptr);
	sqlite3_exec(database, "create index idx_settings_module on settings(module);", nullptr, nullptr, nullptr);

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

	sqlite3_exec(database, "begin transaction;", nullptr, nullptr, nullptr);
	sqlite3_exec(database, "pragma locking_mode = EXCLUSIVE;", nullptr, nullptr, nullptr);
	sqlite3_exec(database, "pragma synchronous = NORMAL;", nullptr, nullptr, nullptr);
	sqlite3_exec(database, "pragma foreign_keys = OFF;", nullptr, nullptr, nullptr);
	sqlite3_exec(database, "pragma journal_mode = OFF;", nullptr, nullptr, nullptr);
	sqlite3_exec(database, "commit;", nullptr, nullptr, nullptr);

	CDbxSQLite *db = new CDbxSQLite(database);
	db->InitSettings();
	db->InitContacts();
	db->InitEvents();
	return db;
}

BOOL CDbxSQLite::Compact()
{
	sqlite3_exec(m_db, "pragma optimize;", nullptr, nullptr, nullptr);
	sqlite3_exec(m_db, "vacuum;", nullptr, nullptr, nullptr);
	return 0;
}

BOOL CDbxSQLite::Backup(const wchar_t *profile)
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
	m_safetyMode = value != FALSE;
}
