#include "stdafx.h"

enum {
	SQL_CTC_STMT_COUNT = 0,
	SQL_CTC_STMT_ADD,
	SQL_CTC_STMT_DELETE,
	SQL_CTC_STMT_NUM
};

static char *ctc_stmts[SQL_CTC_STMT_NUM] = {
	"select count(1) from contacts limit 1;",
	"insert into contacts values (null);",
	"delete from events where contactid = ?; delete from settings where contactid = ?; delete from contacts where id = ?;"
};

static sqlite3_stmt *ctc_stmts_prep[SQL_CTC_STMT_NUM] = { 0 };

void CDbxSQLite::InitContacts()
{
	for (size_t i = 0; i < SQL_CTC_STMT_NUM; i++)
		sqlite3_prepare_v3(m_db, ctc_stmts[i], -1, SQLITE_PREPARE_PERSISTENT, &ctc_stmts_prep[i], nullptr);

	// add global contact
	//m_cache->AddContactToCache(0);

	sqlite3_stmt *stmt = nullptr;
	sqlite3_prepare_v2(m_db, "select id from contacts;", -1, &stmt, nullptr);
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		MCONTACT hContact = sqlite3_column_int64(stmt, 0);
		DBCachedContact *cc = m_cache->AddContactToCache(hContact);

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
		sqlite3_reset(stmt);
		if (rc != SQLITE_DONE)
			return INVALID_CONTACT_ID;
		hContact = sqlite3_last_insert_rowid(m_db);
	}

	DBCachedContact *cc = m_cache->AddContactToCache(hContact);
	NotifyEventHooks(hContactAddedEvent, hContact);

	return hContact;
}

LONG CDbxSQLite::DeleteContact(MCONTACT hContact)
{
	// global contact cannot be removed
	if (hContact == 0)
		return 1;

	{
		mir_cslock lock(m_csDbAccess);
		sqlite3_stmt *stmt = ctc_stmts_prep[SQL_CTC_STMT_DELETE];
		sqlite3_bind_int64(stmt, 1, hContact);
		int rc = sqlite3_step(stmt);
		sqlite3_reset(stmt);
		if (rc != SQLITE_DONE)
			return 1;
	}

	m_cache->FreeCachedContact(hContact);
	NotifyEventHooks(hContactDeletedEvent, hContact);

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
