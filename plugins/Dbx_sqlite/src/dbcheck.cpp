#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// first try to find events with wrong contact ids

int CDbxSQLite::CheckPhase1()
{
	sqlite3_stmt *pQuery;
	int rc = sqlite3_prepare_v2(m_db, "SELECT id, contact_id FROM events WHERE contact_id <> 0 AND contact_id NOT IN (SELECT id FROM contacts)", -1, &pQuery, nullptr);
	logError(rc, __FILE__, __LINE__);
	if (rc)
		return rc;

	while (sqlite3_step(pQuery) == SQLITE_ROW) {
		MEVENT hDbEvent = sqlite3_column_int(pQuery, 0);
		MCONTACT hContact = sqlite3_column_int(pQuery, 1);
		cb->pfnAddLogMessage(STATUS_ERROR, CMStringW(FORMAT, TranslateT("Orphaned event with wrong contact ID %d, deleting"), hContact));
		DeleteEvent(hDbEvent);
	}
	
	sqlite3_finalize(pQuery);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// then we're wiping orphaned records from Events_srt which have no parent record in Events

int CDbxSQLite::CheckPhase2()
{
	sqlite3_stmt *pQuery;
	int rc = sqlite3_prepare_v2(m_db, "SELECT id, contact_id, timestamp FROM events_srt WHERE id NOT IN (SELECT id FROM events)", -1, &pQuery, nullptr);
	logError(rc, __FILE__, __LINE__);
	if (rc)
		return rc;

	while (sqlite3_step(pQuery) == SQLITE_ROW) {
		MEVENT hDbEvent = sqlite3_column_int(pQuery, 0);
		MCONTACT hContact = sqlite3_column_int(pQuery, 1);
		uint32_t ts = sqlite3_column_int(pQuery, 2);

		DeleteEventSrt(hDbEvent, hContact, ts);
		cb->pfnAddLogMessage(STATUS_ERROR, CMStringW(FORMAT, TranslateT("Orphaned sorting event with wrong event ID %d:%08X, deleting"), hContact, hDbEvent));
	}

	sqlite3_finalize(pQuery);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// now try to find orphans in backward direction: from Events_srt to Events

int CDbxSQLite::CheckPhase3()
{
	sqlite3_stmt *pQuery;
	int rc = sqlite3_prepare_v2(m_db, "SELECT id, contact_id FROM events WHERE id NOT IN (SELECT id FROM events_srt)", -1, &pQuery, nullptr);
	logError(rc, __FILE__, __LINE__);
	if (rc)
		return rc;

	while (sqlite3_step(pQuery) == SQLITE_ROW) {
		MEVENT hDbEvent = sqlite3_column_int(pQuery, 0);
		MCONTACT hContact = sqlite3_column_int(pQuery, 1);

		DeleteEventMain(hDbEvent);
		cb->pfnAddLogMessage(STATUS_ERROR, CMStringW(FORMAT, TranslateT("Orphaned event with wrong event ID %d:%08X, deleting"), hContact, hDbEvent));
	}

	sqlite3_finalize(pQuery);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// remove settings with wrong contact ids

int CDbxSQLite::CheckPhase4()
{
	sqlite3_stmt *pQuery;
	int rc = sqlite3_prepare_v2(m_db, "SELECT contact_id,module,setting FROM settings WHERE contact_id <> 0 AND contact_id NOT IN (SELECT id FROM contacts)", -1, &pQuery, nullptr);
	logError(rc, __FILE__, __LINE__);
	if (rc)
		return rc;

	while (sqlite3_step(pQuery) == SQLITE_ROW) {
		MCONTACT hContact = sqlite3_column_int(pQuery, 0);
		auto *szModule = (const char *)sqlite3_column_text(pQuery, 1);
		auto *szSetting = (const char *)sqlite3_column_text(pQuery, 2);

		cb->pfnAddLogMessage(STATUS_ERROR, CMStringW(FORMAT, TranslateT("Orphaned setting [%S:%S] with wrong contact ID %d, deleting"), szModule, szSetting, hContact));
		DeleteContactSettingWorker(hContact, szModule, szSetting);
	}

	sqlite3_finalize(pQuery);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// MIDatabaseChecker

int CDbxSQLite::CheckDb(int phase)
{
	switch (phase) {
	case 0: return CheckPhase1();
	case 1: return CheckPhase2();
	case 2: return CheckPhase3();
	case 3: return CheckPhase4();
	}

	DBFlush(true);
	return ERROR_OUT_OF_PAPER;
}
