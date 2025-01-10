#include "stdafx.h"

static const char normal_order_query[] = "SELECT id FROM events_srt WHERE contact_id = ? ORDER BY timestamp;";
static const char reverse_order_query[] = "SELECT id FROM events_srt WHERE contact_id = ? ORDER BY timestamp DESC;";

void CDbxSQLite::InitEvents()
{
	int rc = 0;
	{
		sqlite3_stmt *stmt = nullptr;
		sqlite3_prepare_v2(m_db, "SELECT DISTINCT module FROM events;", -1, &stmt, nullptr);
		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
			const char *module = (char *)sqlite3_column_text(stmt, 0);
			if (mir_strlen(module) > 0)
				m_modules.insert(mir_strdup(module));
		}
		logError(rc, __FILE__, __LINE__);
		sqlite3_finalize(stmt);
	}

	// Events convertor
	DBVARIANT dbv = { DBVT_BYTE };
	if (GetContactSettingWorker(0, "Compatibility", "DbEvents", &dbv, 0))
		dbv.bVal = 0;
	
	if (dbv.bVal < 1) {
		sqlite3_stmt *stmt = nullptr;
		sqlite3_prepare_v2(m_db, "SELECT id FROM events WHERE type=1002;", -1, &stmt, nullptr);
		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
			MEVENT hEvent = sqlite3_column_int(stmt, 0);

			DB::EventInfo dbei;
			dbei.cbBlob = -1;
			GetEvent(hEvent, &dbei);

			const char *p = (const char*)dbei.pBlob + sizeof(uint32_t);
			DB::FILE_BLOB blob(dbei.getString(p), dbei.getString(p + mir_strlen(p) + 1));
			blob.write(dbei);
			EditEvent(hEvent, &dbei);
		}
		logError(rc, __FILE__, __LINE__);
		sqlite3_finalize(stmt);

		dbv.bVal = 1;
		WriteContactSetting(0, "Compatibility", "DbEvents", &dbv);
	}
}

void CDbxSQLite::UninitEvents()
{
	for (auto &module : m_modules)
		mir_free(module);
	m_modules.destroy();
}

int CDbxSQLite::GetEventCount(MCONTACT hContact)
{
	DBCachedContact *cc = (hContact) ? m_cache->GetCachedContact(hContact) : &m_system;
	return (cc == nullptr) ? 0 : cc->m_count;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDbxSQLite::AddEventSrt(MEVENT hDbEvent, MCONTACT hContact, int64_t ts)
{
	int rc;

	do {
		auto *stmt = InitQuery("INSERT INTO events_srt(id, contact_id, timestamp) VALUES (?, ?, ?);", qEvAddSrt);
		sqlite3_bind_int64(stmt, 1, hDbEvent);
		sqlite3_bind_int64(stmt, 2, hContact);
		sqlite3_bind_int64(stmt, 3, ts);
		rc = sqlite3_step(stmt);
		logError(rc, __FILE__, __LINE__);
		sqlite3_reset(stmt);

		ts++;
	} while (rc != SQLITE_DONE);
	
	return rc;
}

MEVENT CDbxSQLite::AddEvent(MCONTACT hContact, const DBEVENTINFO *dbei)
{
	if (dbei == nullptr)
		return 0;

	if (dbei->iTimestamp == 0)
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
			if (!dbei->bSent)
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

	DBEVENTINFO tmp = *dbei;
	const char *szEventId;
	if (tmp.szId != nullptr) {
		tmp.flags |= DBEF_HAS_ID;
		szEventId = tmp.szId;
	}
	else szEventId = "";

	mir_ptr<char> pCryptBlob;
	if (m_bEncrypted) {
		size_t len;
		char *pResult = (char*)m_crypto->encodeBuffer(tmp.pBlob, tmp.cbBlob, &len);
		if (pResult != nullptr) {
			pCryptBlob = tmp.pBlob = pResult;
			tmp.cbBlob = (uint16_t)len;
			tmp.flags |= DBEF_ENCRYPTED;
		}
	}

	mir_cslockfull lock(m_csDbAccess);
	sqlite3_stmt *stmt = InitQuery(
		"INSERT INTO events(contact_id, module, timestamp, type, flags, data, server_id, user_id, is_read, reply_id) "
		"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);", qEvAdd);
	sqlite3_bind_int64(stmt, 1, hContact);
	sqlite3_bind_text(stmt, 2, tmp.szModule, (int)mir_strlen(tmp.szModule), nullptr);
	sqlite3_bind_int64(stmt, 3, tmp.iTimestamp);
	sqlite3_bind_int(stmt, 4, tmp.eventType);
	sqlite3_bind_int64(stmt, 5, tmp.flags);
	sqlite3_bind_blob(stmt, 6, tmp.pBlob, tmp.cbBlob, nullptr);
	sqlite3_bind_text(stmt, 7, tmp.szId, (int)mir_strlen(tmp.szId), nullptr);
	sqlite3_bind_text(stmt, 8, tmp.szUserId, (int)mir_strlen(tmp.szUserId), nullptr);
	sqlite3_bind_int(stmt, 9, tmp.markedRead());
	sqlite3_bind_text(stmt, 10, tmp.szReplyId, (int)mir_strlen(tmp.szReplyId), nullptr);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);

	MEVENT hDbEvent = sqlite3_last_insert_rowid(m_db);

	int64_t tsSort = tmp.bMsec ? tmp.iTimestamp : tmp.iTimestamp * 1000;
	AddEventSrt(hDbEvent, cc->contactID, tsSort);

	cc->m_count++;
	if (ccSub != nullptr) {
		AddEventSrt(hDbEvent, ccSub->contactID, tsSort);
		ccSub->m_count++;
	}

	char *module = m_modules.find((char *)tmp.szModule);
	if (module == nullptr)
		m_modules.insert(mir_strdup(tmp.szModule));

	lock.unlock();

	DBFlush();
	if (m_safetyMode && !(tmp.flags & DBEF_TEMPORARY))
		NotifyEventHooks(g_hevEventAdded, hNotifyContact, (LPARAM)hDbEvent);

	return hDbEvent;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDbxSQLite::DeleteEventMain(MEVENT hDbEvent)
{
	auto *stmt = InitQuery("DELETE FROM events WHERE id = ?;", qEvDel);
	sqlite3_bind_int64(stmt, 1, hDbEvent);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);
	return rc;
}

int CDbxSQLite::DeleteEventSrt(MCONTACT hContact, int64_t ts)
{
	auto *stmt = InitQuery("DELETE FROM events_srt WHERE contact_id = ? AND timestamp = ?;", qEvDelSrt);
	sqlite3_bind_int64(stmt, 1, hContact);
	sqlite3_bind_int64(stmt, 2, ts);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);
	return rc;
}

int CDbxSQLite::DeleteEventSrt2(MEVENT hDbEvent)
{
	auto *stmt = InitQuery("DELETE FROM events_srt WHERE id = ?;", qEvDelSrt2);
	sqlite3_bind_int64(stmt, 1, hDbEvent);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);
	return rc;
}

BOOL CDbxSQLite::DeleteEvent(MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return 1;

	MEVENT hContact;
	{
		sqlite3_stmt *stmt = InitQuery("SELECT contact_id FROM events WHERE id = ? LIMIT 1;", qEvGetContact2);
		sqlite3_bind_int64(stmt, 1, hDbEvent);
		int rc = sqlite3_step(stmt);
		logError(rc, __FILE__, __LINE__);
		if (rc != SQLITE_ROW) {
			sqlite3_reset(stmt);
			return 2;
		}
		hContact = sqlite3_column_int64(stmt, 0);
		sqlite3_reset(stmt);
	}

	DBCachedContact *cc = (hContact) ? m_cache->GetCachedContact(hContact) : &m_system;
	if (cc == nullptr)
		return 1;

	NotifyEventHooks(g_hevEventDeleted, hContact, hDbEvent);

	mir_cslockfull lock(m_csDbAccess);
	int rc = DeleteEventMain(hDbEvent);
	if (rc != SQLITE_DONE)
		return 1;

	rc = DeleteEventSrt2(hDbEvent);
	if (rc != SQLITE_DONE)
		return 1;

	cc->m_count--;
	if (cc->IsSub())
		if (auto *ccSub = m_cache->GetCachedContact(cc->parentID))
			ccSub->m_count--;

	lock.unlock();

	DBFlush();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL CDbxSQLite::EditEvent(MEVENT hDbEvent, const DBEVENTINFO *dbei)
{
	if (dbei == nullptr)
		return 1;

	if (dbei->iTimestamp == 0)
		return 1;

	DBEVENTINFO tmp = *dbei;
	mir_ptr<char> pCryptBlob;
	if (m_bEncrypted) {
		if (tmp.pBlob) {
			size_t len;
			char *pResult = (char *)m_crypto->encodeBuffer(tmp.pBlob, tmp.cbBlob, &len);
			if (pResult != nullptr) {
				pCryptBlob = tmp.pBlob = pResult;
				tmp.cbBlob = (uint16_t)len;
				tmp.flags |= DBEF_ENCRYPTED;
			}
		}
		else tmp.flags |= DBEF_ENCRYPTED;
	}

	mir_cslockfull lock(m_csDbAccess);
	sqlite3_stmt *stmt;
	if (tmp.pBlob)
		stmt = InitQuery("UPDATE events SET module = ?, timestamp = ?, type = ?, flags = ?, data = ?, is_read = ?, server_id = ?, user_id = ?, reply_id = ? WHERE id = ?;", qEvEdit1);
	else
		stmt = InitQuery("UPDATE events SET module = ?, timestamp = ?, type = ?, flags = ?, is_read = ?, server_id = ?, user_id = ?, reply_id = ? WHERE id = ?;", qEvEdit2);

	int i = 1;
	sqlite3_bind_text(stmt, i++, tmp.szModule, (int)mir_strlen(tmp.szModule), nullptr);
	sqlite3_bind_int64(stmt, i++, tmp.iTimestamp);
	sqlite3_bind_int(stmt, i++, tmp.eventType);
	sqlite3_bind_int64(stmt, i++, tmp.flags);
	if (tmp.pBlob)
		sqlite3_bind_blob(stmt, i++, tmp.pBlob, tmp.cbBlob, nullptr);
	sqlite3_bind_int(stmt, i++, tmp.markedRead());
	sqlite3_bind_text(stmt, i++, tmp.szId, (int)mir_strlen(tmp.szId), nullptr);
	sqlite3_bind_text(stmt, i++, tmp.szUserId, (int)mir_strlen(tmp.szUserId), nullptr);
	sqlite3_bind_text(stmt, i++, tmp.szReplyId, (int)mir_strlen(tmp.szReplyId), nullptr);
	sqlite3_bind_int64(stmt, i++, hDbEvent);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);

	char *module = m_modules.find((char *)tmp.szModule);
	if (module == nullptr)
		m_modules.insert(mir_strdup(tmp.szModule));

	lock.unlock();

	DBFlush();

	if (m_safetyMode) {
		MCONTACT hContact = GetEventContact(hDbEvent);
		if (auto *cc = m_cache->GetCachedContact(hContact))
			if (cc->IsSub())
				hContact = cc->parentID;

		NotifyEventHooks(g_hevEventEdited, hContact, hDbEvent);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void str2json(CMStringA &str)
{
	str.Replace("\\", "\\\\");
}

int CDbxSQLite::SetEventJson(MEVENT hDbEvent, const char *szSetting, DBVARIANT *dbv)
{
	if (hDbEvent == 0)
		return 1;

	CMStringA tmp(FORMAT, "$.%s", szSetting);
	{	
		mir_cslock lock(m_csDbAccess);
		sqlite3_stmt *stmt = InitQuery("UPDATE events SET data=json_set(cast(data as text), ?, ?) WHERE id = ?;", qEvSetJson);

		sqlite3_bind_text(stmt, 1, tmp, tmp.GetLength(), nullptr);
		switch (dbv->type) {
		case DBVT_BYTE:
			sqlite3_bind_int(stmt, 2, dbv->bVal);
			break;
		case DBVT_WORD:
			sqlite3_bind_int(stmt, 2, dbv->wVal);
			break;
		case DBVT_DWORD:
			sqlite3_bind_int(stmt, 2, dbv->dVal);
			break;
		case DBVT_ASCIIZ:
		case DBVT_UTF8:
			{
				tmp = dbv->pszVal;
				str2json(tmp);
				sqlite3_bind_text(stmt, 2, tmp, tmp.GetLength(), nullptr);
			}
			break;
		case DBVT_WCHAR:
			{
				tmp = T2Utf(dbv->pwszVal).get();
				str2json(tmp);
				sqlite3_bind_text(stmt, 2, tmp, tmp.GetLength(), nullptr);
			}
			break;
		default:
			return 2;
		}
		sqlite3_bind_int64(stmt, 3, hDbEvent);
		int rc = sqlite3_step(stmt);
		logError(rc, __FILE__, __LINE__);
		sqlite3_reset(stmt);
	}

	DBFlush();

	if (m_safetyMode) {
		MCONTACT hContact = GetEventContact(hDbEvent);
		if (auto *cc = m_cache->GetCachedContact(hContact))
			if (cc->IsSub())
				hContact = cc->parentID;

		NotifyEventHooks(g_hevEventSetJson, hContact, hDbEvent);
	}
	return 0;
}

int CDbxSQLite::GetBlobSize(MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return -1;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = InitQuery("SELECT LENGTH(CAST(data AS BLOB)) AS l FROM events WHERE id = ? LIMIT 1;", qEvBlobSize);
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

static char g_szId[100], g_szUserId[100], g_szReplyId[100];

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
	sqlite3_stmt *stmt = InitQuery("SELECT module, timestamp, type, flags, server_id, user_id, reply_id, LENGTH(CAST(data AS BLOB)) AS l, data, contact_id FROM events WHERE id = ? LIMIT 1;", qEvGet);
	sqlite3_bind_int64(stmt, 1, hDbEvent);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		return 1;
	}

	char *p = (char *)sqlite3_column_text(stmt, 0);
	dbei->szModule = m_modules.find(p);
	if (dbei->szModule == nullptr)
		return 1;

	dbei->iTimestamp = sqlite3_column_int64(stmt, 1);
	dbei->eventType = sqlite3_column_int(stmt, 2);
	dbei->flags = sqlite3_column_int64(stmt, 3);
	dbei->hContact = sqlite3_column_int(stmt, 9);

	p = (char *)sqlite3_column_text(stmt, 4);
	if (mir_strlen(p)) {
		mir_strncpy(g_szId, p, sizeof(g_szId));
		dbei->szId = g_szId;
	}
	else dbei->szId = nullptr;

	p = (char *)sqlite3_column_text(stmt, 5);
	if (mir_strlen(p)) {
		mir_strncpy(g_szUserId, p, sizeof(g_szUserId));
		dbei->szUserId = g_szUserId;
	}
	else dbei->szUserId = nullptr;	

	p = (char *)sqlite3_column_text(stmt, 6);
	if (mir_strlen(p)) {
		mir_strncpy(g_szReplyId, p, sizeof(g_szReplyId));
		dbei->szReplyId = g_szReplyId;
	}
	else dbei->szReplyId = nullptr;

	int32_t cbBlob = sqlite3_column_int64(stmt, 7);
	size_t bytesToCopy = cbBlob;
	if (dbei->cbBlob == -1)
		dbei->pBlob = (char *)mir_calloc(cbBlob + 2);
	else if (dbei->cbBlob < cbBlob)
		bytesToCopy = dbei->cbBlob;

	dbei->cbBlob = cbBlob;
	if (bytesToCopy && dbei->pBlob) {
		uint8_t *data = (uint8_t *)sqlite3_column_blob(stmt, 8);

		if (dbei->flags & DBEF_ENCRYPTED) {
			dbei->flags &= ~DBEF_ENCRYPTED;
			size_t len;
			uint8_t* pBlob = (uint8_t*)m_crypto->decodeBuffer(data, cbBlob, &len);
			if (pBlob == nullptr)
				return 1;

			memcpy(dbei->pBlob, pBlob, bytesToCopy);
			if (bytesToCopy > len)
				memset(dbei->pBlob + len, 0, bytesToCopy - len);

			mir_free(pBlob);
		}
		else memcpy(dbei->pBlob, data, bytesToCopy);		
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

	int rows;
	{
		mir_cslock lock(m_csDbAccess);
		sqlite3_stmt *stmt = InitQuery("UPDATE events SET flags = flags | 4, is_read = 1 WHERE id = ? AND is_read = 0;", qEvSetFlags);
		sqlite3_bind_int64(stmt, 1, hDbEvent);
		int rc = sqlite3_step(stmt);
		rows = sqlite3_changes(m_db);
		logError(rc, __FILE__, __LINE__);
		sqlite3_reset(stmt);
		if (rc != SQLITE_DONE)
			return -1;
	}

	DBFlush();

	if (rows == 0)
		return 0;

	NotifyEventHooks(g_hevMarkedRead, hContact, (LPARAM)hDbEvent);
	return 1;
}

MCONTACT CDbxSQLite::GetEventContact(MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return INVALID_CONTACT_ID;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = InitQuery("SELECT contact_id FROM events WHERE id = ? LIMIT 1;", qEvGetContact);
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

MEVENT CDbxSQLite::FindFirstUnreadEvent(MCONTACT hContact)
{
	DBCachedContact *cc = (hContact) ? m_cache->GetCachedContact(hContact) : &m_system;
	if (cc == nullptr)
		return 0;

	mir_cslock lock(m_csDbAccess);

	if (cc->IsMeta()) {
		if (cc->nSubs == 0)
			return 0;

		CMStringA query("SELECT id FROM events WHERE is_read = 0 AND contact_id IN (");
		for (int k = 0; k < cc->nSubs; k++)
			query.AppendFormat("%lu, ", cc->pSubs[k]);
		query.Delete(query.GetLength() - 2, 2);
		query.Append(") ORDER BY timestamp LIMIT 1;");

		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(m_db, query, -1, &stmt, nullptr);
		int rc = sqlite3_step(stmt);
		logError(rc, __FILE__, __LINE__);
		if (rc != SQLITE_ROW) {
			sqlite3_finalize(stmt);
			return 0;
		}
		
		MEVENT ret = sqlite3_column_int64(stmt, 0);
		sqlite3_finalize(stmt);
		return ret;
	}

	sqlite3_stmt *stmt = InitQuery("SELECT id FROM events WHERE contact_id = ? AND is_read = 0 ORDER BY timestamp LIMIT 1;", qEvFindUnread);
	sqlite3_bind_int64(stmt, 1, hContact);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		return 0;
	}

	MEVENT ret = sqlite3_column_int64(stmt, 0);
	sqlite3_reset(stmt);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////
// First/next event

MEVENT CDbxSQLite::FindFirstEvent(MCONTACT hContact)
{
	DBCachedContact *cc = (hContact) ? m_cache->GetCachedContact(hContact) : &m_system;
	if (cc == nullptr)
		return 0;

	mir_cslock lock(m_csDbAccess);

	if (fwd.cur)
		sqlite3_reset(fwd.cur);

	fwd.hContact = hContact;
	fwd.cur = InitQuery(normal_order_query, qEvFindFirst);
	sqlite3_bind_int64(fwd.cur, 1, hContact);

	int rc = sqlite3_step(fwd.cur);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_ROW) {	
		fwd.clear();
		return 0;
	}
	return fwd.hEvent = sqlite3_column_int64(fwd.cur, 0);
}

MEVENT CDbxSQLite::FindNextEvent(MCONTACT hContact, MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return 0;

	DBCachedContact *cc = (hContact) ? m_cache->GetCachedContact(hContact) : &m_system;
	if (cc == nullptr)
		return 0;

	if (hContact != fwd.hContact || hDbEvent != fwd.hEvent) {
		if (fwd.cur)
			sqlite3_reset(fwd.cur);

		fwd.hContact = hContact;
		fwd.cur = InitQuery("SELECT id FROM events_srt WHERE contact_id = ? AND timestamp > (SELECT timestamp FROM events_srt WHERE id=?) ORDER BY timestamp;", qEvFindNext);
		sqlite3_bind_int64(fwd.cur, 1, hContact);
		sqlite3_bind_int64(fwd.cur, 2, hDbEvent);
	}

	int rc = sqlite3_step(fwd.cur);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_ROW) {
		fwd.clear();
		return 0;
	}

	return fwd.hEvent = sqlite3_column_int64(fwd.cur, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Last/prev event

MEVENT CDbxSQLite::FindLastEvent(MCONTACT hContact)
{
	DBCachedContact *cc = (hContact) ? m_cache->GetCachedContact(hContact) : &m_system;
	if (cc == nullptr)
		return 0;

	mir_cslock lock(m_csDbAccess);

	if (back.cur)
		sqlite3_reset(back.cur);

	back.hContact = hContact;
	back.cur = InitQuery(reverse_order_query, qEvFindLast);
	sqlite3_bind_int64(back.cur, 1, hContact);
	int rc = sqlite3_step(back.cur);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_ROW) {
		back.clear();
		return 0;
	}
	
	return back.hEvent = sqlite3_column_int64(back.cur, 0);
}

MEVENT CDbxSQLite::FindPrevEvent(MCONTACT hContact, MEVENT hDbEvent)
{
	if (hDbEvent == 0)
		return 0;

	DBCachedContact *cc = (hContact) ? m_cache->GetCachedContact(hContact) : &m_system;
	if (cc == nullptr)
		return 0;

	if (hContact != back.hContact || hDbEvent != back.hEvent) {
		if (back.cur)
			sqlite3_reset(back.cur);

		back.hContact = hContact;
		back.cur = InitQuery("SELECT id FROM events_srt WHERE contact_id = ? AND timestamp < (SELECT timestamp FROM events_srt WHERE id=?) ORDER BY timestamp DESC;", qEvFindPrev);
		sqlite3_bind_int64(back.cur, 1, hContact);
		sqlite3_bind_int64(back.cur, 2, hDbEvent);
	}

	int rc = sqlite3_step(back.cur);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_ROW) {
		back.clear();
		return 0;
	}

	return back.hEvent = sqlite3_column_int64(back.cur, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Metacontacts

BOOL CDbxSQLite::MetaMergeHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	{	mir_cslock lock(m_csDbAccess);
		sqlite3_stmt *stmt = InitQuery(
			"INSERT INTO events_srt(id, contact_id, timestamp) "
				"SELECT id, ?, timestamp from events_srt where contact_id = ?;", qEvMetaMerge);
		sqlite3_bind_int64(stmt, 1, ccMeta->contactID);
		sqlite3_bind_int64(stmt, 2, ccSub->contactID);
		int rc = sqlite3_step(stmt);
		logError(rc, __FILE__, __LINE__);
		sqlite3_reset(stmt);
		if (rc != SQLITE_DONE)
			return FALSE;

		ccMeta->m_count = GetContactEventCount(ccMeta->contactID);
	}

	DBFlush();
	return TRUE;
}

BOOL CDbxSQLite::MetaSplitHistory(DBCachedContact *ccMeta, DBCachedContact *ccSub)
{
	{	mir_cslock lock(m_csDbAccess);
		sqlite3_stmt *stmt = InitQuery(
			"DELETE FROM events_srt WHERE contact_id = ? "
				"AND id IN (SELECT id from events_srt WHERE contact_id = ?);", qEvMetaSplit);
		sqlite3_bind_int64(stmt, 1, ccMeta->contactID);
		sqlite3_bind_int64(stmt, 2, ccSub->contactID);
		int rc = sqlite3_step(stmt);
		logError(rc, __FILE__, __LINE__);
		sqlite3_reset(stmt);
		if (rc != SQLITE_DONE)
			return FALSE;
	}

	DBFlush();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Server ids

MEVENT CDbxSQLite::GetEventById(LPCSTR szModule, LPCSTR szId)
{
	if (szModule == nullptr || szId == nullptr)
		return 0;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = InitQuery("SELECT id, timestamp FROM events WHERE module = ? AND server_id = ? LIMIT 1;", qEvGetById);
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

int CDbxSQLite::UpdateEventId(MEVENT hDbEvent, LPCSTR szId)
{
	if (hDbEvent == 0 || mir_strlen(szId) == 0)
		return 1;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = InitQuery("UPDATE events SET server_id = ? WHERE id = ?;", qEvUpdateId);
	sqlite3_bind_text(stmt, 1, szId, (int)mir_strlen(szId), nullptr);
	sqlite3_bind_int64(stmt, 2, hDbEvent);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	int rows = sqlite3_changes(m_db);
	sqlite3_reset(stmt);
	return (rows == 0) ? 2 : 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Event cursors

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
			sqlite3_prepare_v2(m_db, "SELECT id FROM events_srt WHERE contact_id = ? AND timestamp <= (SELECT timestamp FROM events_srt WHERE id=?) ORDER BY timestamp DESC;", -1, &cursor, nullptr);
	}
	else {
		if (!hDbEvent)
			sqlite3_prepare_v2(m_db, normal_order_query, -1, &cursor, nullptr);
		else
			sqlite3_prepare_v2(m_db, "SELECT id FROM events_srt WHERE contact_id = ? AND timestamp >= (SELECT timestamp FROM events_srt WHERE id=?) ORDER BY timestamp;", -1, &cursor, nullptr);
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
