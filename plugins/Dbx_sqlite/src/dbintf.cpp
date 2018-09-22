#include "stdafx.h"

CDbxSQLite::CDbxSQLite(sqlite3 *database)
	: m_db(database)
{
	hContactAddedEvent = CreateHookableEvent(ME_DB_CONTACT_ADDED);
	hContactDeletedEvent = CreateHookableEvent(ME_DB_CONTACT_DELETED);
	hEventAddedEvent = CreateHookableEvent(ME_DB_EVENT_ADDED);
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
	DestroyHookableEvent(hEventDeletedEvent);
	DestroyHookableEvent(hEventFilterAddedEvent);
	DestroyHookableEvent(hEventMarkedRead);
	DestroyHookableEvent(hSettingChangeEvent);

	InitContacts();
	InitEvents();
	InitSettings();

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

	sqlite3_exec(database, "create table contacts (id integer not null primary key);", nullptr, nullptr, nullptr);
	sqlite3_exec(database, "create table events (id integer not null primary key, contactid integer not null, module varchar(255) not null, timestamp integer not null, type integer not null, flags integer not null, size integer not null, blob any, serverid varchar(64));", nullptr, nullptr, nullptr);
	sqlite3_exec(database, "create index idx_events1 on events(id, contactid);", nullptr, nullptr, nullptr);
	sqlite3_exec(database, "create index idx_events2 on events(module, serverid);", nullptr, nullptr, nullptr);
	sqlite3_exec(database, "create table settings (contactid integer not null, module varchar(255) not null, setting varchar(255) not null, type integer not null, value any, primary key(contactid, module, setting)) without rowid;", nullptr, nullptr, nullptr);
	sqlite3_exec(database, "create index idx_settings on settings(contactid, module, setting);", nullptr, nullptr, nullptr);

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
	sqlite3_exec(database, "commit;", nullptr, nullptr, nullptr);

	CDbxSQLite *db = new CDbxSQLite(database);
	db->InitContacts();
	db->InitEvents();
	db->InitSettings();
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
	return 0;
}

void CDbxSQLite::SetCacheSafetyMode(BOOL)
{
}

BOOL CDbxSQLite::MetaMergeHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	return 0;
}

BOOL CDbxSQLite::MetaSplitHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	return 0;
}
