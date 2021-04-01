#include "stdafx.h"

enum
{
	SQL_CRYPT_GET_MODE,
	SQL_CRYPT_SET_MODE,
	SQL_CRYPT_GET_PROVIDER,
	SQL_CRYPT_SET_PROVIDER,
	SQL_CRYPT_GET_KEY,
	SQL_CRYPT_SET_KEY,
	SQL_CRYPT_ENCRYPT,
};

static CQuery crypto_stmts[] =
{
	{ "SELECT data FROM crypto WHERE id=1;" },          // SQL_CRYPT_GET_MODE
	{ "REPLACE INTO crypto VALUES (1, ?);" },           // SQL_CRYPT_SET_MODE 
	{ "SELECT data FROM crypto WHERE id=2;" },          // SQL_CRYPT_GET_PROVIDER
	{ "REPLACE INTO crypto VALUES(2, ?);" },            // SQL_CRYPT_SET_PROVIDER 
	{ "SELECT data FROM crypto WHERE id=3;" },          // SQL_CRYPT_GET_KEY 
	{ "REPLACE INTO crypto VALUES(3, ?);" },            // SQL_CRYPT_SET_KEY
	{ "UPDATE events SET flags=?, data=? WHERE id=?;"}, // SQL_CRYPT_ENCRYPT
};

static char szCreateQuery[] =
	"CREATE TABLE crypto (id INTEGER NOT NULL PRIMARY KEY, data ANY NOT NULL);\r\n"
	"INSERT INTO crypto VALUES (1, 0), (2, 'AES (Rjindale)'), (3, (SELECT value FROM settings WHERE contact_id=0 AND module='CryptoEngine' AND setting='StoredKey'));\r\n"
	"DELETE FROM settings WHERE contact_id=0 AND module='CryptoEngine';\r\n";

void CDbxSQLite::InitEncryption()
{
	int rc = sqlite3_exec(m_db, "SELECT COUNT(1) FROM crypto;", nullptr, nullptr, nullptr);
	if (rc == SQLITE_ERROR) // table doesn't exist, fill it with existing data
		sqlite3_exec(m_db, szCreateQuery, nullptr, nullptr, nullptr);

	for (auto &it : crypto_stmts)
		sqlite3_prepare_v3(m_db, it.szQuery, -1, SQLITE_PREPARE_PERSISTENT, &it.pQuery, nullptr);
}

void CDbxSQLite::UninintEncryption()
{
	for (auto &it : crypto_stmts)
		sqlite3_finalize(it.pQuery);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Saving encryption key in a database

STDMETHODIMP_(BOOL) CDbxSQLite::ReadCryptoKey(MBinBuffer &buf)
{
	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = crypto_stmts[SQL_CRYPT_GET_KEY].pQuery;
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		return FALSE;
	}

	buf.append((BYTE*)sqlite3_column_blob(stmt, 0), sqlite3_column_bytes(stmt, 0));
	sqlite3_reset(stmt);
	return TRUE;
}

STDMETHODIMP_(BOOL) CDbxSQLite::StoreCryptoKey()
{
	size_t iKeyLength = m_crypto->getKeyLength();
	BYTE *pKey = (BYTE*)_alloca(iKeyLength);
	m_crypto->getKey(pKey, iKeyLength);

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = crypto_stmts[SQL_CRYPT_SET_KEY].pQuery;
	sqlite3_bind_blob(stmt, 1, pKey, (int)iKeyLength, nullptr);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);

	SecureZeroMemory(pKey, iKeyLength);
	DBFlush();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Reads encryption flag

STDMETHODIMP_(BOOL) CDbxSQLite::ReadEncryption()
{
	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = crypto_stmts[SQL_CRYPT_GET_MODE].pQuery;
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		return FALSE;
	}

	int ret = sqlite3_column_int(stmt, 0);
	sqlite3_reset(stmt);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Reads crypto provider's name

STDMETHODIMP_(CRYPTO_PROVIDER*) CDbxSQLite::ReadProvider()
{
	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = crypto_stmts[SQL_CRYPT_GET_PROVIDER].pQuery;
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);
	return (rc != SQLITE_ROW) ? nullptr : Crypto_GetProvider((char*)sqlite3_column_text(stmt, 0));
}

STDMETHODIMP_(BOOL) CDbxSQLite::StoreProvider(CRYPTO_PROVIDER *pProvider)
{
	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = crypto_stmts[SQL_CRYPT_SET_PROVIDER].pQuery;
	sqlite3_bind_text(stmt, 1, pProvider->pszName, (int)strlen(pProvider->pszName), 0);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);

	DBFlush();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Toggles full/partial encryption mode

STDMETHODIMP_(BOOL) CDbxSQLite::EnableEncryption(BOOL bEncrypted)
{
	if (m_bEncrypted == (bEncrypted != 0))
		return TRUE;

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = nullptr;
	int rc = sqlite3_prepare_v2(m_db, "SELECT id, flags, data FROM events;", -1, &stmt, 0);
	logError(rc, __FILE__, __LINE__);
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int dwFlags = sqlite3_column_int(stmt, 1);
		if (((dwFlags & DBEF_ENCRYPTED) != 0) == bEncrypted)
			continue;
	
		int id = sqlite3_column_int(stmt, 0);
		auto *pBlob = (const BYTE *)sqlite3_column_blob(stmt, 2);
		unsigned cbBlob = sqlite3_column_bytes(stmt, 2);

		mir_ptr<BYTE> pNewBlob;
		size_t nNewBlob;

		if (dwFlags & DBEF_ENCRYPTED) {
			pNewBlob = (BYTE*)m_crypto->decodeBuffer(pBlob, cbBlob, &nNewBlob);
			dwFlags &= (~DBEF_ENCRYPTED);
		}
		else {
			pNewBlob = m_crypto->encodeBuffer(pBlob, cbBlob, &nNewBlob);
			dwFlags |= DBEF_ENCRYPTED;
		}

		sqlite3_stmt *upd = crypto_stmts[SQL_CRYPT_ENCRYPT].pQuery;
		sqlite3_bind_int(upd, 1, dwFlags);
		sqlite3_bind_blob(upd, 2, pNewBlob, (int)nNewBlob, 0);
		sqlite3_bind_int(upd, 3, id);
		rc = sqlite3_step(upd);
		logError(rc, __FILE__, __LINE__);
		sqlite3_reset(upd);
	}
	sqlite3_finalize(stmt);

	// Finally update flag
	stmt = crypto_stmts[SQL_CRYPT_SET_MODE].pQuery;
	sqlite3_bind_int(stmt, 1, bEncrypted);
	rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);

	m_bEncrypted = bEncrypted;
	return TRUE;
}
