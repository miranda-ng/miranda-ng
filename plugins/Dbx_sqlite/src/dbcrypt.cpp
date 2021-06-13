#include "stdafx.h"

static char szCreateQuery[] =
	"CREATE TABLE crypto (id INTEGER NOT NULL PRIMARY KEY, data ANY NOT NULL);\r\n"
	"INSERT INTO crypto VALUES (1, 0), (2, 'AES (Rjindale)'), (3, (SELECT value FROM settings WHERE contact_id=0 AND module='CryptoEngine' AND setting='StoredKey'));\r\n"
	"DELETE FROM settings WHERE contact_id=0 AND module='CryptoEngine';\r\n";

void CDbxSQLite::InitEncryption()
{
	int rc = sqlite3_exec(m_db, "SELECT COUNT(1) FROM crypto;", nullptr, nullptr, nullptr);
	if (rc == SQLITE_ERROR) // table doesn't exist, fill it with existing data
		sqlite3_exec(m_db, szCreateQuery, nullptr, nullptr, nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Saving encryption key in a database

STDMETHODIMP_(BOOL) CDbxSQLite::ReadCryptoKey(MBinBuffer &buf)
{
	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = InitQuery("SELECT data FROM crypto WHERE id=3;", qCryptGetKey);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	if (rc != SQLITE_ROW) {
		sqlite3_reset(stmt);
		return FALSE;
	}

	buf.append(sqlite3_column_blob(stmt, 0), sqlite3_column_bytes(stmt, 0));
	sqlite3_reset(stmt);
	return TRUE;
}

STDMETHODIMP_(BOOL) CDbxSQLite::StoreCryptoKey()
{
	size_t iKeyLength = m_crypto->getKeyLength();
	BYTE *pKey = (BYTE*)_alloca(iKeyLength);
	m_crypto->getKey(pKey, iKeyLength);

	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = InitQuery("REPLACE INTO crypto VALUES(3, ?);", qCryptSetKey);
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
	sqlite3_stmt *stmt = InitQuery("SELECT data FROM crypto WHERE id=1;", qCryptGetMode);
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
	sqlite3_stmt *stmt = InitQuery("SELECT data FROM crypto WHERE id=2;", qCryptGetProvider);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);
	return (rc != SQLITE_ROW) ? nullptr : Crypto_GetProvider((char*)sqlite3_column_text(stmt, 0));
}

STDMETHODIMP_(BOOL) CDbxSQLite::StoreProvider(CRYPTO_PROVIDER *pProvider)
{
	mir_cslock lock(m_csDbAccess);
	sqlite3_stmt *stmt = InitQuery("REPLACE INTO crypto VALUES(2, ?);", qCryptSetProvider);
	sqlite3_bind_text(stmt, 1, pProvider->pszName, (int)strlen(pProvider->pszName), 0);
	int rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);

	DBFlush();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Toggles full/partial encryption mode

STDMETHODIMP_(BOOL) CDbxSQLite::EnableEncryption(BOOL bEncrypt)
{
	if (m_bEncrypted == (bEncrypt != 0))
		return TRUE;

	mir_cslock lock(m_csDbAccess);

	// encrypt all histories
	sqlite3_stmt *stmt = nullptr;
	int rc = sqlite3_prepare_v2(m_db, "SELECT id, flags, data FROM events;", -1, &stmt, 0);
	logError(rc, __FILE__, __LINE__);
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int dwFlags = sqlite3_column_int(stmt, 1);
		if (((dwFlags & DBEF_ENCRYPTED) != 0) == bEncrypt)
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

		sqlite3_stmt *upd = InitQuery("UPDATE events SET flags=?, data=? WHERE id=?;", qCryptEnc1);
		sqlite3_bind_int(upd, 1, dwFlags);
		sqlite3_bind_blob(upd, 2, pNewBlob, (int)nNewBlob, 0);
		sqlite3_bind_int(upd, 3, id);
		rc = sqlite3_step(upd);
		logError(rc, __FILE__, __LINE__);
		sqlite3_reset(upd);
	}
	sqlite3_finalize(stmt);
	DBFlush(true);
	
	// if database is encrypted, decrypt all settings with type = DBVT_ENCRYPTED
	CMStringA query(FORMAT, "SELECT contact_id, module, setting, value FROM settings WHERE type=%d", (bEncrypt) ? DBVT_UTF8 : DBVT_ENCRYPTED);
	rc = sqlite3_prepare_v2(m_db, query, -1, &stmt, 0);
	logError(rc, __FILE__, __LINE__);
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int hContact = sqlite3_column_int(stmt, 0);
		auto *pszModule = (char *)sqlite3_column_text(stmt, 1);
		auto *pszSetting = (char *)sqlite3_column_text(stmt, 2);

		// all passwords etc should remain encrypted
		if (!bEncrypt && IsSettingEncrypted(pszModule, pszSetting))
			continue;

		sqlite3_stmt *upd = InitQuery("UPDATE settings SET type=?, value=? WHERE contact_id=? AND module=? AND setting=?;", qCryptEnc2);
		sqlite3_bind_int(upd, 1, (bEncrypt) ? DBVT_ENCRYPTED : DBVT_UTF8);

		size_t resultLen;
		ptrA pBuf;
		if (bEncrypt) {
			pBuf = (char*)m_crypto->encodeString((char*)sqlite3_column_text(stmt, 3), &resultLen);
			sqlite3_bind_blob(upd, 2, pBuf, (int)resultLen, 0);
		}
		else {
			pBuf = (char*)m_crypto->decodeString(sqlite3_column_text(stmt, 3), sqlite3_column_bytes(stmt, 3), &resultLen);
			sqlite3_bind_text(upd, 2, pBuf, (int)strlen(pBuf), 0);
		}
		
		sqlite3_bind_int(upd, 3, hContact);
		sqlite3_bind_text(upd, 4, pszModule, (int)strlen(pszModule), 0);
		sqlite3_bind_text(upd, 5, pszSetting, (int)strlen(pszSetting), 0);
		rc = sqlite3_step(upd);
		logError(rc, __FILE__, __LINE__);
		sqlite3_reset(upd);
	}

	// Finally update flag
	stmt = InitQuery("REPLACE INTO crypto VALUES (1, ?);", qCryptSetMode);
	sqlite3_bind_int(stmt, 1, bEncrypt);
	rc = sqlite3_step(stmt);
	logError(rc, __FILE__, __LINE__);
	sqlite3_reset(stmt);
	DBFlush(true);

	m_bEncrypted = bEncrypt;
	return TRUE;
}
