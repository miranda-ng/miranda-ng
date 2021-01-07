#include "stdafx.h"

enum {
	SQL_CTC_STMT_COUNT = 0,
	SQL_CTC_STMT_ADD,
	SQL_CTC_STMT_DELETE,
	SQL_CTC_STMT_DELETESETTINGS,
	SQL_CTC_STMT_DELETEEVENTS,
	SQL_CTC_STMT_DELETEEVENTS_SRT,
	SQL_CTC_STMT_NUM
};

static char *ctc_stmts[SQL_CTC_STMT_NUM] = {
	"SELECT COUNT(1) FROM contacts LIMIT 1;",
	"INSERT INTO contacts VALUES (null);",
	"DELETE FROM contacts WHERE id = ?;",
	"DELETE FROM settings WHERE contact_id = ?;",
	"DELETE FROM events WHERE contact_id = ?;",
	"DELETE FROM events_srt WHERE contact_id = ?;"
};

static sqlite3_stmt *ctc_stmts_prep[SQL_CTC_STMT_NUM] = { 0 };

void CDbxSQLite::InitContacts()
{
	for (size_t i = 0; i < SQL_CTC_STMT_NUM; i++)
		sqlite3_prepare_v3(m_db, ctc_stmts[i], -1, SQLITE_PREPARE_PERSISTENT, &ctc_stmts_prep[i], nullptr);

	sqlite3_stmt *stmt = nullptr;
	sqlite3_prepare_v2(m_db, "SELECT contacts.id, COUNT(es.id) FROM contacts LEFT JOIN events_srt es ON es.contact_id = contacts.id GROUP BY contacts.id;", -1, &stmt, nullptr);
	int rc = 0;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		MCONTACT hContact = sqlite3_column_int64(stmt, 0);
		DBCachedContact *cc = (hContact)
			? m_cache->AddContactToCache(hContact)
			: &m_system;
		cc->m_count = sqlite3_column_int64(stmt, 1);

		DBVARIANT dbv = { DBVT_DWORD };
		cc->nSubs = (0 != GetContactSetting(cc->contactID, META_PROTO, "NumContacts", &dbv)) ? -1 : dbv.dVal;
		if (cc->nSubs != -1) {
			cc->pSubs = (MCONTACT*)mir_alloc(cc->nSubs * sizeof(MCONTACT));
			for (int k = 0; k < cc->nSubs; k++) {
				char setting[100];
				mir_snprintf(setting, _countof(setting), "Handle%d", k);
				cc->pSubs[k] = (0 != GetContactSetting(cc->contactID, META_PROTO, setting, &dbv)) ? 0 : dbv.dVal;
			}
		}
		cc->nDefault = (0 != GetContactSetting(cc->contactID, META_PROTO, "Default", &dbv)) ? -1 : dbv.dVal;
		cc->parentID = (0 != GetContactSetting(cc->contactID, META_PROTO, "ParentMeta", &dbv)) ? 0 : dbv.dVal;
	}
	assert(rc == SQLITE_ROW || rc == SQLITE_DONE);
	sqlite3_finalize(stmt);
}

void CDbxSQLite::UninitContacts()
{
	for (size_t i = 0; i < SQL_CTC_STMT_NUM; i++)
		sqlite3_finalize(ctc_stmts_prep[i]);
}

LONG CDbxSQLite::GetContactCount()
{
	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = ctc_stmts_prep[SQL_CTC_STMT_COUNT];
	int rc = sqlite3_step(stmt);
	assert(rc == SQLITE_ROW || rc == SQLITE_DONE);
	int count = sqlite3_column_int(stmt, 0);
	sqlite3_reset(stmt);
	return count;
}

MCONTACT CDbxSQLite::AddContact()
{
	MCONTACT hContact = INVALID_CONTACT_ID;
	{
		mir_cslock lock(m_csDbAccess);
		sqlite3_stmt *stmt = ctc_stmts_prep[SQL_CTC_STMT_ADD];
		int rc = sqlite3_step(stmt);
		assert(rc == SQLITE_ROW || rc == SQLITE_DONE);
		sqlite3_reset(stmt);
		if (rc != SQLITE_DONE)
			return INVALID_CONTACT_ID;
		hContact = sqlite3_last_insert_rowid(m_db);
	}

	DBCachedContact *cc = m_cache->AddContactToCache(hContact);
	if (cc == nullptr)
		return INVALID_CONTACT_ID;

	NotifyEventHooks(g_hevContactAdded, hContact);
	return hContact;
}

LONG CDbxSQLite::DeleteContact(MCONTACT hContact)
{
	// global contact cannot be removed
	if (hContact == 0)
		return 1;

	mir_cslockfull lock(m_csDbAccess);

	sqlite3_stmt *stmt = ctc_stmts_prep[SQL_CTC_STMT_DELETEEVENTS];
	sqlite3_bind_int64(stmt, 1, hContact);
	int rc = sqlite3_step(stmt);
	assert(rc == SQLITE_DONE);
	sqlite3_reset(stmt);
	if (rc != SQLITE_DONE)
		return 1;

	stmt = ctc_stmts_prep[SQL_CTC_STMT_DELETEEVENTS_SRT];
	sqlite3_bind_int64(stmt, 1, hContact);
	int rc = sqlite3_step(stmt);
	assert(rc == SQLITE_DONE);
	sqlite3_reset(stmt);
	if (rc != SQLITE_DONE)
		return 1;

	stmt = ctc_stmts_prep[SQL_CTC_STMT_DELETESETTINGS];
	sqlite3_bind_int64(stmt, 1, hContact);
	rc = sqlite3_step(stmt);
	assert(rc == SQLITE_DONE);
	sqlite3_reset(stmt);
	if (rc != SQLITE_DONE)
		return 1;

	stmt = ctc_stmts_prep[SQL_CTC_STMT_DELETE];
	sqlite3_bind_int64(stmt, 1, hContact);
	rc = sqlite3_step(stmt);
	assert(rc == SQLITE_DONE);
	sqlite3_reset(stmt);
	if (rc != SQLITE_DONE)
		return 1;

	m_cache->FreeCachedContact(hContact);

	lock.unlock();
	NotifyEventHooks(g_hevContactDeleted, hContact);
	return 0;
}

BOOL CDbxSQLite::IsDbContact(MCONTACT hContact)
{
	DBCachedContact *cc = m_cache->GetCachedContact(hContact);
	return (cc != nullptr);
}

LONG CDbxSQLite::GetContactSize(void)
{
	return sizeof(DBCachedContact);
}

/////////////////////////////////////

void DBCachedContact::AddEvent(MEVENT hDbEvent, uint32_t timestamp, bool unread)
{
	m_count = HasCount() ? m_count + 1 : 1;
	
	if (unread && timestamp > m_unreadTimestamp) {
		m_unread = hDbEvent;
		m_unreadTimestamp = timestamp;
	}
}

void DBCachedContact::EditEvent(MEVENT hDbEvent, uint32_t timestamp, bool unread)
{
	if (m_unread = hDbEvent && (!unread || m_unreadTimestamp != timestamp)) {
		m_unread = 0;
		m_unreadTimestamp = 0;
	}
	else if (unread && timestamp > m_unreadTimestamp) {
		m_unread = hDbEvent;
		m_unreadTimestamp = timestamp;
	}
}

void DBCachedContact::DeleteEvent(MEVENT hDbEvent)
{
	if (m_count > 0)
		m_count--;
	if (m_unread == hDbEvent) {
		m_unread = 0;
		m_unreadTimestamp = 0;
	}
}

void DBCachedContact::MarkRead(MEVENT hDbEvent)
{
	if (m_unread == hDbEvent) {
		m_unread = 0;
		m_unreadTimestamp = 0;
	}
}
