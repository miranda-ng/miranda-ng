#include "stdafx.h"

enum {
	SQL_EVT_STMT_COUNT = 0,
	SQL_EVT_STMT_ADDEVENT,
	SQL_EVT_STMT_ADDCONTACTEVENT,
	SQL_EVT_STMT_DELETE,
	SQL_EVT_STMT_EDIT,
	SQL_EVT_STMT_BLOBSIZE,
	SQL_EVT_STMT_GET,
	SQL_EVT_STMT_GETFLAGS,
	SQL_EVT_STMT_SETFLAGS,
	SQL_EVT_STMT_GETCONTACT,
	SQL_EVT_STMT_FINDFIRST,
	SQL_EVT_STMT_FINDFIRSTUNREAD,
	SQL_EVT_STMT_FINDLAST,
	SQL_EVT_STMT_FINDNEXT,
	SQL_EVT_STMT_FINDPREV,
	SQL_EVT_STMT_GETIDBYSRVID,
	SQL_EVT_STMT_SETSRVID,
	SQL_EVT_STMT_MERGE,
	SQL_EVT_STMT_SPLIT,
	SQL_EVT_STMT_NUM
};

static char *evt_stmts[SQL_EVT_STMT_NUM] = {
	"select count(1) from contact_events where contactid = ? limit 1;",
	"insert into events(module, timestamp, type, flags, size, blob) values (?, ?, ?, ?, ?, ?);",
	"insert into contact_events(contactid, eventid) values (?, ?);",
	"delete from events where id = ?; delete from contact_events where contactid = ? and eventid = ?;",
	"update events set module = ?, timestamp = ?, type = ?, flags = ?, size = ?, blob = ? where id = ?;",
	"select size from events where id = ? limit 1;",
	"select module, timestamp, type, flags, size, blob from events where id = ? limit 1;",
	"select flags from events where id = ? limit 1;",
	"update events set flag = ? where id = ?;",
	"select contactid from contact_events where eventid = ? limit 1;",
	"select min(id) from contact_events where contactid = ? limit 1;",
	"select flags, id from events where contactid = ? order by id;",
	"select max(eventid) from contact_events where contactid = ? limit 1;",
	"select eventid from contact_events where contactid = ? and eventid > ? order by eventid limit 1;",
	"select eventid from contact_events where contactid = ? and eventid < ? order by eventid desc limit 1;",
	"select id from events where module = ? and serverid = ? limit 1;",
	"update events set serverid = ? where id = ?;",
	"insert into contact_events(contactid, eventid) select ?, eventid from contact_events where contactid = ?;",
	"delete from contact_events where contactid = ? and eventid in (select eventid from contact_events where contactid = ?);",
};

static sqlite3_stmt *evt_stmts_prep[SQL_EVT_STMT_NUM] = { 0 };

void CDbxSQLite::InitEvents()
{
	for (size_t i = 0; i < SQL_EVT_STMT_NUM; i++)
		sqlite3_prepare_v3(m_db, evt_stmts[i], -1, SQLITE_PREPARE_PERSISTENT, &evt_stmts_prep[i], nullptr);
}

void CDbxSQLite::UninitEvents()
{
	for (size_t i = 0; i < SQL_EVT_STMT_NUM; i++)
		sqlite3_finalize(evt_stmts_prep[i]);
}

LONG CDbxSQLite::GetEventCount(MCONTACT hContact)
{
	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_COUNT];
	sqlite3_bind_int64(stmt, 1, hContact);
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		return 0;
	}
	LONG count = sqlite3_column_int64(stmt, 0);
	sqlite3_reset(stmt);
	return count;
}

MEVENT CDbxSQLite::AddEvent(MCONTACT hContact, DBEVENTINFO *dbei)
{
	if (dbei == nullptr)
		return 0;

	if (dbei->timestamp == 0)
		return 0;

	DBCachedContact *cc = m_cache->GetCachedContact(hContact);
	if (cc == nullptr)
		return 0;

	if (NotifyEventHooks(hEventFilterAddedEvent, hContact, (LPARAM)dbei))
		return 0;

	MEVENT hDbEvent = 0;
	{
		mir_cslock lock(m_csDbAccess);
		sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_ADDEVENT];
		sqlite3_bind_text(stmt, 1, dbei->szModule, mir_strlen(dbei->szModule), nullptr);
		sqlite3_bind_int64(stmt, 2, dbei->timestamp);
		sqlite3_bind_int(stmt, 3, dbei->eventType);
		sqlite3_bind_int64(stmt, 4, dbei->flags);
		sqlite3_bind_int64(stmt, 5, dbei->cbBlob);
		sqlite3_bind_blob(stmt, 6, dbei->pBlob, dbei->cbBlob, nullptr);
		int rc = sqlite3_step(stmt);
		sqlite3_reset(stmt);
		if (rc != SQLITE_DONE)
			return 0;
		hDbEvent = sqlite3_last_insert_rowid(m_db);

		stmt = evt_stmts_prep[SQL_EVT_STMT_ADDCONTACTEVENT];
		sqlite3_bind_int64(stmt, 1, hContact);
		sqlite3_bind_int64(stmt, 2, hDbEvent);
		rc = sqlite3_step(stmt);
		sqlite3_reset(stmt);
		if (rc != SQLITE_DONE)
			return 0;
	}

	bool neednotify = false;
	if (!(dbei->flags & (DBEF_READ | DBEF_SENT)))
		neednotify = true;
	//else neednotify = m_safetyMode;

	// notify only in safe mode or on really new events
	if (neednotify)
		NotifyEventHooks(hEventAddedEvent, hContact, (LPARAM)hDbEvent);

	return hDbEvent;
}

BOOL CDbxSQLite::DeleteEvent(MCONTACT hContact, MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return 1;

	DBCachedContact *cc = m_cache->GetCachedContact(hContact);
	if (cc == nullptr)
		return 1;

	{
		mir_cslock lock(m_csDbAccess);
		sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_DELETE];
		sqlite3_bind_int64(stmt, 1, hDbEvent);
		sqlite3_bind_int64(stmt, 2, hContact);
		sqlite3_bind_int64(stmt, 3, hDbEvent);
		int rc = sqlite3_step(stmt);
		sqlite3_reset(stmt);
		if (rc != SQLITE_DONE)
			return 1;
	}

	NotifyEventHooks(hEventDeletedEvent, hContact, (LPARAM)hDbEvent);

	return 0;
}

BOOL CDbxSQLite::EditEvent(MCONTACT hContact, MEVENT hDbEvent, DBEVENTINFO *dbei)
{
	if (dbei == nullptr)
		return 1;

	if (dbei->timestamp == 0)
		return 1;

	DBCachedContact *cc = m_cache->GetCachedContact(hContact);
	if (cc == nullptr)
		return 1;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_EDIT];
	sqlite3_bind_text(stmt, 1, dbei->szModule, mir_strlen(dbei->szModule), nullptr);
	sqlite3_bind_int64(stmt, 2, dbei->timestamp);
	sqlite3_bind_int(stmt, 3, dbei->eventType);
	sqlite3_bind_int64(stmt, 4, dbei->flags);
	sqlite3_bind_int64(stmt, 5, dbei->cbBlob);
	sqlite3_bind_blob(stmt, 6, dbei->pBlob, dbei->cbBlob, nullptr);
	sqlite3_bind_int64(stmt, 7, hDbEvent);
	int rc = sqlite3_step(stmt);
	if (rc == SQLITE_DONE)
		NotifyEventHooks(hEventEditedEvent, hContact, (LPARAM)hDbEvent);
	sqlite3_reset(stmt);
	return (rc != SQLITE_DONE);
}

LONG CDbxSQLite::GetBlobSize(MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return -1;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_BLOBSIZE];
	sqlite3_bind_int(stmt, 1, hDbEvent);
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		return -1;
	}
	LONG res = sqlite3_column_int64(stmt, 0);
	sqlite3_reset(stmt);
	return res;
}

BOOL CDbxSQLite::GetEvent(MEVENT hDbEvent, DBEVENTINFO *dbei)
{
	if (hDbEvent == 0)
		return 1;

	if (dbei == nullptr)
		return 1;

	if (dbei->cbBlob > 0 && dbei->pBlob == nullptr) {
		dbei->cbBlob = 0;
		return 1;
	}

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_GET];
	sqlite3_bind_int64(stmt, 1, hDbEvent);
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		return 1;
	}
	dbei->szModule = (char*)sqlite3_column_text(stmt, 0);
	dbei->timestamp = sqlite3_column_int64(stmt, 1);
	dbei->eventType = sqlite3_column_int(stmt, 2);
	dbei->flags = sqlite3_column_int64(stmt, 3);

	DWORD cbBlob = sqlite3_column_int64(stmt, 4);
	int bytesToCopy = (dbei->cbBlob < cbBlob) ? dbei->cbBlob : cbBlob;
	dbei->cbBlob = cbBlob;
	if (bytesToCopy && dbei->pBlob) {
		BYTE *data = (BYTE*)sqlite3_column_blob(stmt, 5);
		memcpy(dbei->pBlob, data, dbei->cbBlob);
	}
	sqlite3_reset(stmt);
	return 0;
}

BOOL CDbxSQLite::MarkEventRead(MCONTACT hContact, MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return -1;

	DBCachedContact *cc = m_cache->GetCachedContact(hContact);
	if (cc == nullptr)
		return -1;

	DWORD flags = 0;
	{
		mir_cslock lock(m_csDbAccess);
		sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_GETFLAGS];
		sqlite3_bind_int64(stmt, 1, hDbEvent);
		int rc = sqlite3_step(stmt);
		if (rc != SQLITE_ROW) {
			sqlite3_reset(stmt);
			return -1;
		}
		flags = sqlite3_column_int64(stmt, 0);
		sqlite3_reset(stmt);
	}

	if ((flags & DBEF_READ) == DBEF_READ)
		return flags;

	flags |= DBEF_READ;
	{
		mir_cslock lock(m_csDbAccess);
		sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_SETFLAGS];
		sqlite3_bind_int(stmt, 1, flags);
		sqlite3_bind_int64(stmt, 2, hDbEvent);
		int rc = sqlite3_step(stmt);
			sqlite3_reset(stmt);
		if (rc != SQLITE_DONE)
			return -1;
	}

	NotifyEventHooks(hEventMarkedRead, hContact, (LPARAM)hDbEvent);

	return flags;
}

MCONTACT CDbxSQLite::GetEventContact(MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return INVALID_CONTACT_ID;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_GETCONTACT];
	sqlite3_bind_int64(stmt, 1, hDbEvent);
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		return INVALID_CONTACT_ID;
	}
	MCONTACT hContact = sqlite3_column_int64(stmt, 0);
	sqlite3_reset(stmt);
	return hContact;
}

MEVENT CDbxSQLite::FindFirstEvent(MCONTACT hContact)
{
	DBCachedContact *cc = m_cache->GetCachedContact(hContact);
	if (cc == nullptr)
		return 0;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_FINDFIRST];
	sqlite3_bind_int64(stmt, 1, hContact);
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		return 0;
	}
	MEVENT hDbEvent = sqlite3_column_int64(stmt, 0);
	sqlite3_reset(stmt);
	return hDbEvent;
}

MEVENT CDbxSQLite::FindFirstUnreadEvent(MCONTACT hContact)
{
	DBCachedContact *cc = m_cache->GetCachedContact(hContact);
	if (cc == nullptr)
		return 0;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_FINDFIRSTUNREAD];
	sqlite3_bind_int64(stmt, 1, hContact);
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		DWORD flags = sqlite3_column_int64(stmt, 0);
		if (!(flags & (DBEF_READ | DBEF_SENT))) {
			MEVENT hDbEvent = sqlite3_column_int64(stmt, 1);
			sqlite3_reset(stmt);
			return hDbEvent;
			break;
		}
	}
	sqlite3_reset(stmt);
	return 0;
}

MEVENT CDbxSQLite::FindLastEvent(MCONTACT hContact)
{
	DBCachedContact *cc = m_cache->GetCachedContact(hContact);
	if (cc == nullptr)
		return 0;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_FINDLAST];
	sqlite3_bind_int64(stmt, 1, hContact);
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		return 0;
	}
	MEVENT hDbEvent = sqlite3_column_int64(stmt, 0);
	sqlite3_reset(stmt);
	return hDbEvent;
}

MEVENT CDbxSQLite::FindNextEvent(MCONTACT hContact, MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return 0;

	DBCachedContact *cc = m_cache->GetCachedContact(hContact);
	if (cc == nullptr)
		return 0;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_FINDNEXT];
	sqlite3_bind_int64(stmt, 1, hContact);
	sqlite3_bind_int64(stmt, 2, hDbEvent);
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		return 0;
	}
	hDbEvent = sqlite3_column_int64(stmt, 0);
	sqlite3_reset(stmt);
	return hDbEvent;
}

MEVENT CDbxSQLite::FindPrevEvent(MCONTACT hContact, MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return 0;

	DBCachedContact *cc = m_cache->GetCachedContact(hContact);
	if (cc == nullptr)
		return 0;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_FINDPREV];
	sqlite3_bind_int64(stmt, 1, hContact);
	sqlite3_bind_int64(stmt, 2, hDbEvent);
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		return 0;
	}
	hDbEvent = sqlite3_column_int64(stmt, 0);
	sqlite3_reset(stmt);
	return hDbEvent;
}

MEVENT CDbxSQLite::GetEventById(LPCSTR szModule, LPCSTR szId)
{
	if (szModule == nullptr || szId == nullptr)
		return 0;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_GETIDBYSRVID];
	sqlite3_bind_text(stmt, 1, szModule, mir_strlen(szModule), nullptr);
	sqlite3_bind_text(stmt, 2, szId, mir_strlen(szId), nullptr);
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		return 0;
	}
	MEVENT hDbEvent = sqlite3_column_int64(stmt, 0);
	sqlite3_reset(stmt);
	return hDbEvent;
}

BOOL CDbxSQLite::SetEventId(LPCSTR, MEVENT hDbEvent, LPCSTR szId)
{
	if (hDbEvent == 0 || szId == nullptr)
		return 1;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_SETSRVID];
	sqlite3_bind_text(stmt, 1, szId, mir_strlen(szId), nullptr);
	sqlite3_bind_int64(stmt, 2, hDbEvent);
	int rc = sqlite3_step(stmt);
	sqlite3_reset(stmt);
	return (rc != SQLITE_DONE);
}


BOOL CDbxSQLite::MetaMergeHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_MERGE];
	sqlite3_bind_int64(stmt, 1, ccMeta->contactID);
	sqlite3_bind_int64(stmt, 2, ccSub->contactID);
	int rc = sqlite3_step(stmt);
	sqlite3_reset(stmt);
	return (rc != SQLITE_DONE);
}

BOOL CDbxSQLite::MetaSplitHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts_prep[SQL_EVT_STMT_MERGE];
	sqlite3_bind_int64(stmt, 1, ccMeta->contactID);
	sqlite3_bind_int64(stmt, 2, ccSub->contactID);
	int rc = sqlite3_step(stmt);
	sqlite3_reset(stmt);
	return (rc != SQLITE_DONE);
}