#include "stdafx.h"

void CDbxSQLite::InitContacts()
{
	sqlite3_stmt *stmt = nullptr;
	sqlite3_prepare_v2(m_db, "SELECT contacts.id, COUNT(es.id) FROM contacts LEFT JOIN events_srt es ON es.contact_id = contacts.id GROUP BY contacts.id;", -1, &stmt, nullptr);
	int rc = 0;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		MCONTACT hContact = sqlite3_column_int64(stmt, 0);
		DBCachedContact *cc = (hContact) ? m_cache->AddContactToCache(hContact) : &m_system;
		cc->m_count = sqlite3_column_int64(stmt, 1);
	}
	logError(rc, __FILE__, __LINE__);
	sqlite3_finalize(stmt);
}

int CDbxSQLite::GetContactCount()
{
	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = InitQuery("SELECT COUNT(1) FROM contacts LIMIT 1;", qCntCount);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	int count = sqlite3_column_int(stmt, 0);
	sqlite3_reset(stmt);
	return count;
}

MCONTACT CDbxSQLite::AddContact()
{
	MCONTACT hContact = INVALID_CONTACT_ID;
	{
		mir_cslock lock(m_csDbAccess);
		sqlite3_stmt *stmt = InitQuery("INSERT INTO contacts VALUES (null);", qCntAdd);
		int rc = sqlite3_step(stmt);
		logError(rc, __FILE__, __LINE__);
		sqlite3_reset(stmt);
		if (rc != SQLITE_DONE)
			return INVALID_CONTACT_ID;
		hContact = sqlite3_last_insert_rowid(m_db);
		DBFlush();
	}

	DBCachedContact *cc = m_cache->AddContactToCache(hContact);
	if (cc == nullptr)
		return INVALID_CONTACT_ID;

	NotifyEventHooks(g_hevContactAdded, hContact);
	return hContact;
}

int CDbxSQLite::DeleteContact(MCONTACT hContact)
{
	// global contact cannot be removed
	if (hContact == 0)
		return 1;

	DBCachedContact *cc = m_cache->GetCachedContact(hContact);
	if (cc == nullptr)
		return 1;

	NotifyEventHooks(g_hevContactDeleted, hContact);

	mir_cslockfull lock(m_csDbAccess);

	sqlite3_stmt *stmt = InitQuery("DELETE FROM events WHERE contact_id = ?;", qCntDelEvents);
	sqlite3_bind_int64(stmt, 1, hContact);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);
	if (rc != SQLITE_DONE)
		return 1;

	stmt = InitQuery("DELETE FROM events_srt WHERE contact_id = ?;", qCntDelEventSrt);
	sqlite3_bind_int64(stmt, 1, hContact);
	rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);
	if (rc != SQLITE_DONE)
		return 1;

	stmt = InitQuery("DELETE FROM settings WHERE contact_id = ?;", qCntDelSettings);
	sqlite3_bind_int64(stmt, 1, hContact);
	rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);
	if (rc != SQLITE_DONE)
		return 1;

	stmt = InitQuery("DELETE FROM contacts WHERE id = ?;", qCntDel);
	sqlite3_bind_int64(stmt, 1, hContact);
	rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);
	if (rc != SQLITE_DONE)
		return 1;

	m_cache->FreeCachedContact(hContact);
	lock.unlock();

	DBFlush();
	return 0;
}

BOOL CDbxSQLite::IsDbContact(MCONTACT hContact)
{
	DBCachedContact *cc = m_cache->GetCachedContact(hContact);
	return (cc != nullptr);
}

int CDbxSQLite::GetContactSize(void)
{
	return sizeof(DBCachedContact);
}

/////////////////////////////////////

void DBCachedContact::AddEvent(MEVENT hDbEvent, uint32_t timestamp, bool unread)
{
	m_count = HasCount() ? m_count + 1 : 1;
	
	if (unread && m_unreadTimestamp > timestamp) {
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
