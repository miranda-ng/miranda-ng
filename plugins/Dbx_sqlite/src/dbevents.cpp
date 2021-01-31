#include "stdafx.h"

enum {
	SQL_EVT_STMT_COUNT = 0,
	SQL_EVT_STMT_ADDEVENT,
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
	SQL_EVT_STMT_GETIDBYSRVID,
	SQL_EVT_STMT_ADDEVENT_SRT,
	SQL_EVT_STMT_DELETE_SRT,
	SQL_EVT_STMT_META_SPLIT,
	SQL_EVT_STMT_META_MERGE_SELECT,
};

//TODO: hide it inside cursor class
static const char* normal_order_query = 
	"SELECT id FROM events_srt WHERE contact_id = ? ORDER BY timestamp;";
static const char* normal_order_pos_query =
	"SELECT id FROM events_srt WHERE contact_id = ? AND id >= ? ORDER BY timestamp;";

static const char* reverse_order_query =
	"SELECT id FROM events_srt WHERE contact_id = ? ORDER BY timestamp desc, id DESC;";
static const char* reverse_order_pos_query =
	"SELECT id FROM events_srt WHERE contact_id = ? AND id <= ? ORDER BY timestamp desc, id DESC;";

static CQuery evt_stmts[] =
{
	{ "SELECT COUNT(1) FROM events_srt WHERE contact_id = ? LIMIT 1;" },
	{ "INSERT INTO events(contact_id, module, timestamp, type, flags, data, server_id) VALUES (?, ?, ?, ?, ?, ?, ?);" },
	{ "DELETE FROM events WHERE id = ?;" },
	{ "UPDATE events SET module = ?, timestamp = ?, type = ?, flags = ?, data = ? WHERE id = ?;" },
	{ "SELECT LENGTH(data) FROM events WHERE id = ? LIMIT 1;" },
	{ "SELECT module, timestamp, type, flags, length(data), data FROM events WHERE id = ? LIMIT 1;" },
	{ "SELECT flags FROM events WHERE id = ? LIMIT 1;" },
	{ "UPDATE events SET flags = ? WHERE id = ?;" },
	{ "SELECT contact_id FROM events WHERE id = ? LIMIT 1;" },
	{ normal_order_query },
	{ "SELECT id, timestamp FROM events WHERE contact_id = ? AND (flags & ?) = 0 ORDER BY timestamp, id LIMIT 1;" },
	{ reverse_order_query },
	{ "SELECT id, timestamp FROM events WHERE module = ? AND server_id = ? LIMIT 1;" },
	{ "INSERT INTO events_srt(id, contact_id, timestamp) VALUES (?, ?, ?);" },
	{ "DELETE FROM events_srt WHERE id = ?;" },
	{ "DELETE FROM events_srt WHERE contact_id = ?;" },
	{ "SELECT id, timestamp FROM events WHERE contact_id = ?;" },
};

void CDbxSQLite::InitEvents()
{
	for (auto &it : evt_stmts)
		sqlite3_prepare_v3(m_db, it.szQuery, -1, SQLITE_PREPARE_PERSISTENT, &it.pQuery, nullptr);

	sqlite3_stmt *stmt = nullptr;
	sqlite3_prepare_v2(m_db, "SELECT DISTINCT module FROM events;", -1, &stmt, nullptr);
	int rc = 0;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		const char *module = (char*)sqlite3_column_text(stmt, 0);
		if (mir_strlen(module) > 0)
			m_modules.insert(mir_strdup(module));
	}
	logError(rc, __FILE__, __LINE__);
	sqlite3_finalize(stmt);
}

void CDbxSQLite::UninitEvents()
{
	for (auto module : m_modules.rev_iter()) {
		m_modules.removeItem(&module);
		mir_free(module);
	}

	for (auto &it : evt_stmts)
		sqlite3_finalize(it.pQuery);
}

LONG CDbxSQLite::GetEventCount(MCONTACT hContact)
{
	DBCachedContact *cc = (hContact) ? m_cache->GetCachedContact(hContact) : &m_system;
	if (cc->HasCount())
		return cc->m_count;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts[SQL_EVT_STMT_COUNT].pQuery;
	sqlite3_bind_int64(stmt, 1, hContact);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	cc->m_count = (rc != SQLITE_ROW) ? 0 : sqlite3_column_int64(stmt, 0);
	sqlite3_reset(stmt);
	return cc->m_count;
}

MEVENT CDbxSQLite::AddEvent(MCONTACT hContact, const DBEVENTINFO *dbei)
{
	if (dbei == nullptr)
		return 0;

	if (dbei->timestamp == 0)
		return 0;

	MCONTACT hNotifyContact = hContact;
	DBCachedContact *cc, *ccSub = nullptr;
	if (hContact != 0) {
		if ((cc = m_cache->GetCachedContact(hContact)) == nullptr)
			return 0;

		if (cc->IsSub()) {
			ccSub = cc;
			if ((cc = m_cache->GetCachedContact(cc->parentID)) == nullptr)
				return 0;

			// set default sub to the event's source
			if (!(dbei->flags & DBEF_SENT))
				db_mc_setDefault(cc->contactID, hContact, false);
			if (db_mc_isEnabled())
				hNotifyContact = cc->contactID; // and add an event to a metahistory
		}
	}
	else cc = &m_system;

	if (cc == nullptr)
		return 0;

	if (m_safetyMode)
		if (NotifyEventHooks(g_hevEventFiltered, hNotifyContact, (LPARAM)dbei))
			return 0;

	const char *szEventId;
	DWORD dwFlags = dbei->flags;
	if (dbei->szId != nullptr) {
		dwFlags |= DBEF_HAS_ID;
		szEventId = dbei->szId;
	}
	else szEventId = "";

	mir_cslockfull lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts[SQL_EVT_STMT_ADDEVENT].pQuery;
	sqlite3_bind_int64(stmt, 1, hContact);
	sqlite3_bind_text(stmt, 2, dbei->szModule, (int)mir_strlen(dbei->szModule), nullptr);
	sqlite3_bind_int64(stmt, 3, dbei->timestamp);
	sqlite3_bind_int(stmt, 4, dbei->eventType);
	sqlite3_bind_int64(stmt, 5, dwFlags);
	sqlite3_bind_blob(stmt, 6, dbei->pBlob, dbei->cbBlob, nullptr);
	sqlite3_bind_text(stmt, 7, szEventId, (int)mir_strlen(szEventId), nullptr);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);

	MEVENT hDbEvent = sqlite3_last_insert_rowid(m_db);

	stmt = evt_stmts[SQL_EVT_STMT_ADDEVENT_SRT].pQuery;
	sqlite3_bind_int64(stmt, 1, hDbEvent);
	sqlite3_bind_int64(stmt, 2, cc->contactID);
	sqlite3_bind_int64(stmt, 3, dbei->timestamp);
	rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);

	cc->AddEvent(hDbEvent, dbei->timestamp, !dbei->markedRead());
	if (ccSub != nullptr) {
		stmt = evt_stmts[SQL_EVT_STMT_ADDEVENT_SRT].pQuery;
		sqlite3_bind_int64(stmt, 1, hDbEvent);
		sqlite3_bind_int64(stmt, 2, ccSub->contactID);
		sqlite3_bind_int64(stmt, 3, dbei->timestamp);
		rc = sqlite3_step(stmt);
		logError(rc, __FILE__, __LINE__);
		sqlite3_reset(stmt); //is this necessary ?

		ccSub->AddEvent(hDbEvent, dbei->timestamp, !dbei->markedRead());
	}

	char *module = m_modules.find((char *)dbei->szModule);
	if (module == nullptr)
		m_modules.insert(mir_strdup(dbei->szModule));

	lock.unlock();

	DBFlush();
	if (m_safetyMode && !(dbei->flags & DBEF_TEMPORARY))
		NotifyEventHooks(g_hevEventAdded, hNotifyContact, (LPARAM)hDbEvent);

	return hDbEvent;
}

BOOL CDbxSQLite::DeleteEvent(MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return 1;

	MEVENT hContact = GetEventContact(hDbEvent);
	DBCachedContact *cc = (hContact) ? m_cache->GetCachedContact(hContact) : &m_system;
	if (cc == nullptr)
		return 1;

	mir_cslockfull lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts[SQL_EVT_STMT_DELETE].pQuery;
	sqlite3_bind_int64(stmt, 1, hDbEvent);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);
	if (rc != SQLITE_DONE)
		return 1;

	stmt = evt_stmts[SQL_EVT_STMT_DELETE_SRT].pQuery;
	sqlite3_bind_int64(stmt, 1, hDbEvent);
	rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);
	if (rc != SQLITE_DONE)
		return 1;

	cc->DeleteEvent(hDbEvent);
	if (cc->IsSub() && (cc = m_cache->GetCachedContact(cc->parentID)))
		cc->DeleteEvent(hDbEvent);

	lock.unlock();

	DBFlush();
	NotifyEventHooks(g_hevEventDeleted, hContact, hDbEvent);
	return 0;
}

BOOL CDbxSQLite::EditEvent(MCONTACT hContact, MEVENT hDbEvent, const DBEVENTINFO *dbei)
{
	if (dbei == nullptr)
		return 1;

	if (dbei->timestamp == 0)
		return 1;

	DBCachedContact *cc = (hContact) ? m_cache->GetCachedContact(hContact) : &m_system;
	if (cc == nullptr)
		return 1;

	mir_cslockfull lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts[SQL_EVT_STMT_EDIT].pQuery;
	sqlite3_bind_text(stmt, 1, dbei->szModule, (int)mir_strlen(dbei->szModule), nullptr);
	sqlite3_bind_int64(stmt, 2, dbei->timestamp);
	sqlite3_bind_int(stmt, 3, dbei->eventType);
	sqlite3_bind_int64(stmt, 4, dbei->flags);
	sqlite3_bind_blob(stmt, 5, dbei->pBlob, dbei->cbBlob, nullptr);
	sqlite3_bind_int64(stmt, 6, hDbEvent);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);

	cc->EditEvent(hDbEvent, dbei->timestamp, !dbei->markedRead());
	if (cc->IsSub() && (cc = m_cache->GetCachedContact(cc->parentID)))
		cc->EditEvent(hDbEvent, dbei->timestamp, !dbei->markedRead());

	char *module = m_modules.find((char *)dbei->szModule);
	if (module == nullptr)
		m_modules.insert(mir_strdup(dbei->szModule));

	lock.unlock();

	DBFlush();
	NotifyEventHooks(g_hevEventEdited, hContact, (LPARAM)hDbEvent);
	return 0;
}

LONG CDbxSQLite::GetBlobSize(MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return -1;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts[SQL_EVT_STMT_BLOBSIZE].pQuery;
	sqlite3_bind_int(stmt, 1, hDbEvent);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
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
	sqlite3_stmt *stmt = evt_stmts[SQL_EVT_STMT_GET].pQuery;
	sqlite3_bind_int64(stmt, 1, hDbEvent);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		return 1;
	}

	char *module = (char *)sqlite3_column_text(stmt, 0);
	dbei->szModule = m_modules.find(module);
	if (dbei->szModule == nullptr)
		return 1;

	dbei->timestamp = sqlite3_column_int64(stmt, 1);
	dbei->eventType = sqlite3_column_int(stmt, 2);
	dbei->flags = sqlite3_column_int64(stmt, 3);

	DWORD cbBlob = sqlite3_column_int64(stmt, 4);
	int bytesToCopy = (dbei->cbBlob < cbBlob) ? dbei->cbBlob : cbBlob;
	dbei->cbBlob = cbBlob;
	if (bytesToCopy && dbei->pBlob) {
		BYTE *data = (BYTE *)sqlite3_column_blob(stmt, 5);
		memcpy(dbei->pBlob, data, bytesToCopy);
	}
	sqlite3_reset(stmt);
	return 0;
}

BOOL CDbxSQLite::MarkEventRead(MCONTACT hContact, MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return -1;

	DBCachedContact *cc = (hContact) ? m_cache->GetCachedContact(hContact) : &m_system;
	if (cc == nullptr)
		return -1;

	DWORD flags = 0;
	{
		mir_cslock lock(m_csDbAccess);
		sqlite3_stmt *stmt = evt_stmts[SQL_EVT_STMT_GETFLAGS].pQuery;
		sqlite3_bind_int64(stmt, 1, hDbEvent);
		int rc = sqlite3_step(stmt);
		logError(rc, __FILE__, __LINE__);
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
		sqlite3_stmt *stmt = evt_stmts[SQL_EVT_STMT_SETFLAGS].pQuery;
		sqlite3_bind_int(stmt, 1, flags);
		sqlite3_bind_int64(stmt, 2, hDbEvent);
		int rc = sqlite3_step(stmt);
		logError(rc, __FILE__, __LINE__);
		sqlite3_reset(stmt);
		if (rc != SQLITE_DONE)
			return -1;

		cc->MarkRead(hDbEvent);
		if (cc->IsSub() && (cc = m_cache->GetCachedContact(cc->parentID)))
			cc->MarkRead(hDbEvent);
	}

	DBFlush();
	NotifyEventHooks(g_hevMarkedRead, hContact, (LPARAM)hDbEvent);
	return flags;
}

MCONTACT CDbxSQLite::GetEventContact(MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return INVALID_CONTACT_ID;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts[SQL_EVT_STMT_GETCONTACT].pQuery;
	sqlite3_bind_int64(stmt, 1, hDbEvent);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
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
	DBCachedContact *cc = (hContact) ? m_cache->GetCachedContact(hContact) : &m_system;
	if (cc == nullptr)
		return 0;

	evt_cnt_fwd = hContact;

	mir_cslock lock(m_csDbAccess);

	if (evt_cur_fwd)
		sqlite3_reset(evt_cur_fwd);

	evt_cur_fwd = evt_stmts[SQL_EVT_STMT_FINDFIRST].pQuery;
	sqlite3_bind_int64(evt_cur_fwd, 1, hContact);

	int rc = sqlite3_step(evt_cur_fwd);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_ROW) {
		//empty response
		//reset sql cursor
		sqlite3_reset(evt_cur_fwd);
		evt_cur_fwd = 0;
		//reset current contact
		evt_cnt_fwd = 0;
		return 0;
	}
	return sqlite3_column_int64(evt_cur_fwd, 0);
}

MEVENT CDbxSQLite::FindFirstUnreadEvent(MCONTACT hContact)
{
	DBCachedContact *cc = (hContact) ? m_cache->GetCachedContact(hContact) : &m_system;
	if (cc == nullptr)
		return 0;

	if (cc->m_unread)
		return cc->m_unread;

	mir_cslock lock(m_csDbAccess);

	if (cc->IsMeta()) {
		if (cc->nSubs == 0) {
			cc->m_unread = 0;
			cc->m_unreadTimestamp = 0;
			return 0;
		}

		CMStringA query(FORMAT, "SELECT id FROM events WHERE (flags & %d) = 0 AND contact_id IN (", DBEF_READ | DBEF_SENT);
		for (int k = 0; k < cc->nSubs; k++)
			query.AppendFormat("%lu, ", cc->pSubs[k]);
		query.Delete(query.GetLength() - 2, 2);
		query.Append(") ORDER BY timestamp, id LIMIT 1;");

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(m_db, query, -1, &stmt, nullptr);
		int rc = sqlite3_step(stmt);
		logError(rc, __FILE__, __LINE__);
		if (rc != SQLITE_ROW) {
			sqlite3_finalize(stmt);
			return 0;
		}
		cc->m_unread = sqlite3_column_int64(stmt, 0);
		cc->m_unreadTimestamp = sqlite3_column_int64(stmt, 1);
		sqlite3_finalize(stmt);
		return cc->m_unread;
	}

	sqlite3_stmt *stmt = evt_stmts[SQL_EVT_STMT_FINDFIRSTUNREAD].pQuery;
	sqlite3_bind_int64(stmt, 1, hContact);
	sqlite3_bind_int(stmt, 2, DBEF_READ | DBEF_SENT);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		return 0;
	}
	cc->m_unread = sqlite3_column_int64(stmt, 0);
	cc->m_unreadTimestamp = sqlite3_column_int64(stmt, 1);
	sqlite3_reset(stmt);
	return cc->m_unread;
}

MEVENT CDbxSQLite::FindLastEvent(MCONTACT hContact)
{
	DBCachedContact *cc = (hContact) ? m_cache->GetCachedContact(hContact) : &m_system;
	if (cc == nullptr)
		return 0;

	evt_cnt_backwd = hContact;

	mir_cslock lock(m_csDbAccess);

	if (evt_cur_backwd)
		sqlite3_reset(evt_cur_backwd);

	evt_cur_backwd = evt_stmts[SQL_EVT_STMT_FINDLAST].pQuery;
	sqlite3_bind_int64(evt_cur_backwd, 1, hContact);
	int rc = sqlite3_step(evt_cur_backwd);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_ROW) {
		//empty response
		//reset sql cursor
		sqlite3_reset(evt_cur_backwd);
		evt_cur_backwd = 0;
		//reset current contact
		evt_cnt_backwd = 0;
		return 0;
	}
	return sqlite3_column_int64(evt_cur_backwd, 0);
}

MEVENT CDbxSQLite::FindNextEvent(MCONTACT hContact, MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return 0;

	DBCachedContact *cc = m_cache->GetCachedContact(hContact);
	if (cc == nullptr)
		return 0;

	if (!evt_cur_fwd) {
		evt_cur_fwd = evt_stmts[SQL_EVT_STMT_FINDFIRST].pQuery;
		sqlite3_bind_int64(evt_cur_fwd, 1, hContact);
		evt_cnt_fwd = hContact;
	}
	else if (hContact != evt_cnt_fwd) {
		sqlite3_reset(evt_cur_fwd);
		evt_cur_fwd = evt_stmts[SQL_EVT_STMT_FINDFIRST].pQuery;
		sqlite3_bind_int64(evt_cur_fwd, 1, hContact);
		evt_cnt_fwd = hContact;
	}

	while (hDbEvent != sqlite3_column_int64(evt_cur_fwd, 0)) {
		int rc = sqlite3_step(evt_cur_fwd);
		logError(rc, __FILE__, __LINE__);
		if (rc == SQLITE_DONE) {
			//reset sql cursor
			sqlite3_reset(evt_cur_fwd);
			evt_cur_fwd = 0;
			//reset current contact
			evt_cnt_fwd = 0;
			return 0;
		}
	}

	int rc = sqlite3_step(evt_cur_fwd);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_ROW) {
		//reset sql cursor
		sqlite3_reset(evt_cur_fwd);
		evt_cur_fwd = 0;
		//reset current contact
		evt_cnt_fwd = 0;
		return 0;
	}
	hDbEvent = sqlite3_column_int64(evt_cur_fwd, 0);

	return hDbEvent;
}

MEVENT CDbxSQLite::FindPrevEvent(MCONTACT hContact, MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return 0;

	DBCachedContact *cc = m_cache->GetCachedContact(hContact);
	if (cc == nullptr)
		return 0;

	if (!evt_cur_backwd) {
		evt_cur_backwd = evt_stmts[SQL_EVT_STMT_FINDLAST].pQuery;
		sqlite3_bind_int64(evt_cur_backwd, 1, hContact);
		evt_cnt_backwd = hContact;
	}
	else if (hContact != evt_cnt_backwd) {
		sqlite3_reset(evt_cur_fwd);
		evt_cur_backwd = evt_stmts[SQL_EVT_STMT_FINDLAST].pQuery;
		sqlite3_bind_int64(evt_cur_backwd, 1, hContact);
		evt_cnt_backwd = hContact;
	}

	while (hDbEvent != sqlite3_column_int64(evt_cur_backwd, 0)) {
		int rc = sqlite3_step(evt_cur_backwd);
		logError(rc, __FILE__, __LINE__);
		if (rc == SQLITE_DONE) {
			//reset sql cursor
			sqlite3_reset(evt_cur_backwd);
			evt_cur_backwd = 0;
			//reset current contact
			evt_cnt_backwd = 0;
			return 0;
		}
	}

	int rc = sqlite3_step(evt_cur_backwd);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_ROW) {
		//reset sql cursor
		sqlite3_reset(evt_cur_backwd);
		evt_cur_backwd = 0;
		//reset current contact
		evt_cnt_backwd = 0;
		return 0;
	}
	hDbEvent = sqlite3_column_int64(evt_cur_backwd, 0);

	return hDbEvent;
}

MEVENT CDbxSQLite::GetEventById(LPCSTR szModule, LPCSTR szId)
{
	if (szModule == nullptr || szId == nullptr)
		return 0;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts[SQL_EVT_STMT_GETIDBYSRVID].pQuery;
	sqlite3_bind_text(stmt, 1, szModule, (int)mir_strlen(szModule), nullptr);
	sqlite3_bind_text(stmt, 2, szId, (int)mir_strlen(szId), nullptr);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		return 0;
	}
	MEVENT hDbEvent = sqlite3_column_int64(stmt, 0);
	sqlite3_reset(stmt);
	return hDbEvent;
}

BOOL CDbxSQLite::MetaMergeHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	//TODO: test this
	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts[SQL_EVT_STMT_META_MERGE_SELECT].pQuery;
	sqlite3_bind_int64(stmt, 1, ccSub->contactID);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	while (rc == SQLITE_ROW) {
		sqlite3_stmt *stmt2 = evt_stmts[SQL_EVT_STMT_ADDEVENT_SRT].pQuery;
		sqlite3_bind_int64(stmt2, 1, sqlite3_column_int64(stmt, 0));
		sqlite3_bind_int64(stmt2, 2, ccMeta->contactID);
		sqlite3_bind_int64(stmt2, 3, sqlite3_column_int64(stmt, 1));
		int rc2 = sqlite3_step(stmt2);
		logError(rc2, __FILE__, __LINE__);
		sqlite3_reset(stmt2);
		rc = sqlite3_step(stmt);
		logError(rc, __FILE__, __LINE__);
	}

	sqlite3_reset(stmt);
	DBFlush();
	return TRUE;
}

BOOL CDbxSQLite::MetaSplitHistory(DBCachedContact *ccMeta, DBCachedContact *)
{
	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = evt_stmts[SQL_EVT_STMT_META_SPLIT].pQuery;
	sqlite3_bind_int64(stmt, 1, ccMeta->contactID);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);
	if (rc != SQLITE_DONE)
		return 1;

	DBFlush();
	return TRUE;
}

STDMETHODIMP_(DB::EventCursor *) CDbxSQLite::EventCursor(MCONTACT hContact, MEVENT hDbEvent)
{
	return new CDbxSQLiteEventCursor(hContact, m_db, hDbEvent);
}

STDMETHODIMP_(DB::EventCursor *) CDbxSQLite::EventCursorRev(MCONTACT hContact, MEVENT hDbEvent)
{
	return new CDbxSQLiteEventCursor(hContact, m_db, hDbEvent, true);
}

CDbxSQLiteEventCursor::CDbxSQLiteEventCursor(MCONTACT _1, sqlite3 *_db, MEVENT hDbEvent, bool reverse) :
	EventCursor(_1), m_db(_db)
{
	if (reverse) {
		if (!hDbEvent)
			sqlite3_prepare_v2(m_db, reverse_order_query, -1, &cursor, nullptr);
		else
			sqlite3_prepare_v2(m_db, reverse_order_pos_query, -1, &cursor, nullptr);
	}
	else {
		if (!hDbEvent)
			sqlite3_prepare_v2(m_db, normal_order_query, -1, &cursor, nullptr);
		else
			sqlite3_prepare_v2(m_db, normal_order_pos_query, -1, &cursor, nullptr);
	}
	sqlite3_bind_int64(cursor, 1, hContact);
	if (hDbEvent)
		sqlite3_bind_int64(cursor, 2, hDbEvent);
}

CDbxSQLiteEventCursor::~CDbxSQLiteEventCursor()
{
	if (cursor)
		sqlite3_reset(cursor);
}

MEVENT CDbxSQLiteEventCursor::FetchNext()
{
	if (!cursor)
		return 0;

	int rc = sqlite3_step(cursor);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_ROW) {
		//empty response
		//reset sql cursor
		sqlite3_reset(cursor);
		cursor = nullptr;
		return 0;
	}
	return sqlite3_column_int64(cursor, 0);
}
