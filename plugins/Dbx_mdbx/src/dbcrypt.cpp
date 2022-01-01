/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

char DBKey_Crypto_Provider[] = "Provider";
char DBKey_Crypto_Key[] = "Key";
char DBKey_Crypto_IsEncrypted[] = "EncryptedDB";

/////////////////////////////////////////////////////////////////////////////////////////
// Saving encryption key in a database

STDMETHODIMP_(BOOL) CDbxMDBX::ReadCryptoKey(MBinBuffer &buf)
{
	MDBX_val key = { DBKey_Crypto_Key, sizeof(DBKey_Crypto_Key) }, value;
	int rc = mdbx_get(StartTran(), m_dbCrypto, &key, &value);
	if (rc != MDBX_SUCCESS)
		return FALSE;

	buf.append(value.iov_base, value.iov_len);
	return TRUE;
}

STDMETHODIMP_(BOOL) CDbxMDBX::StoreCryptoKey()
{
	size_t iKeyLength = m_crypto->getKeyLength();
	uint8_t *pKey = (uint8_t*)_alloca(iKeyLength);
	m_crypto->getKey(pKey, iKeyLength);
	{
		txn_ptr trnlck(this);
		MDBX_val key = { DBKey_Crypto_Key, sizeof(DBKey_Crypto_Key) }, value = { pKey, iKeyLength };
		int rc = mdbx_put(trnlck, m_dbCrypto, &key, &value, MDBX_UPSERT);
		/* FIXME: throw an exception */
		assert(rc == MDBX_SUCCESS);
		UNREFERENCED_PARAMETER(rc);
	}

	SecureZeroMemory(pKey, iKeyLength);
	DBFlush();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Saving encryption flag

STDMETHODIMP_(BOOL) CDbxMDBX::ReadEncryption()
{
	MDBX_val key = { DBKey_Crypto_IsEncrypted, sizeof(DBKey_Crypto_IsEncrypted) }, value;
	if (mdbx_get(StartTran(), m_dbCrypto, &key, &value) == MDBX_SUCCESS)
		return *(const bool *)value.iov_base;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Saving provider in a database

STDMETHODIMP_(CRYPTO_PROVIDER *) CDbxMDBX::ReadProvider()
{
	MDBX_val key = { DBKey_Crypto_Provider, sizeof(DBKey_Crypto_Provider) }, value;
	int rc = mdbx_get(StartTran(), m_dbCrypto, &key, &value);
	return (rc == MDBX_SUCCESS) ? Crypto_GetProvider((const char *)value.iov_base) : nullptr;
}

STDMETHODIMP_(BOOL) CDbxMDBX::StoreProvider(CRYPTO_PROVIDER *pProv)
{
	txn_ptr trnlck(this);
	MDBX_val key = { DBKey_Crypto_Provider, sizeof(DBKey_Crypto_Provider) }, value = { pProv->pszName, mir_strlen(pProv->pszName) + 1 };
	if (mdbx_put(trnlck, m_dbCrypto, &key, &value, MDBX_UPSERT) != MDBX_SUCCESS)
		return FALSE;

	DBFlush();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP_(BOOL) CDbxMDBX::EnableEncryption(BOOL bEncrypted)
{
	if (m_bEncrypted == bEncrypted)
		return TRUE;

	std::vector<MEVENT> lstEvents;

	MDBX_stat st;
	mdbx_dbi_stat(StartTran(), m_dbEvents, &st, sizeof(st));
	lstEvents.reserve(st.ms_entries);
	{
		MDBX_val key, data;
		cursor_ptr pCursor(StartTran(), m_dbEvents);

		while (mdbx_cursor_get(pCursor, &key, &data, MDBX_NEXT) == MDBX_SUCCESS) {
			const MEVENT hDbEvent = *(const MEVENT *)key.iov_base;
			lstEvents.push_back(hDbEvent);
		}
	}

	do {
		size_t portion = min(lstEvents.size(), 1000);

		txn_ptr trnlck(this);
		for (size_t i = 0; i < portion; i++) {
			MEVENT &hDbEvent = lstEvents[i];
			MDBX_val key = { &hDbEvent, sizeof(MEVENT) }, data;
			int rc = mdbx_get(trnlck, m_dbEvents, &key, &data);
			if (rc != MDBX_SUCCESS) {
				if (rc != MDBX_NOTFOUND)
					assert(rc == MDBX_SUCCESS);
				continue;
			}

			const DBEvent *dbEvent = (const DBEvent*)data.iov_base;
			const uint8_t    *pBlob = (uint8_t*)(dbEvent + 1);

			if (((dbEvent->flags & DBEF_ENCRYPTED) != 0) != bEncrypted) {
				mir_ptr<uint8_t> pNewBlob;
				size_t nNewBlob;
				uint32_t dwNewFlags;

				if (dbEvent->flags & DBEF_ENCRYPTED) {
					pNewBlob = (uint8_t*)m_crypto->decodeBuffer(pBlob, dbEvent->cbBlob, &nNewBlob);
					dwNewFlags = dbEvent->flags & (~DBEF_ENCRYPTED);
				}
				else {
					pNewBlob = m_crypto->encodeBuffer(pBlob, dbEvent->cbBlob, &nNewBlob);
					dwNewFlags = dbEvent->flags | DBEF_ENCRYPTED;
				}

				data.iov_len = sizeof(DBEvent) + nNewBlob;
				mir_ptr<uint8_t> pData((uint8_t*)mir_alloc(data.iov_len));
				data.iov_base = pData.get();

				DBEvent *pNewDBEvent = (DBEvent *)data.iov_base;
				*pNewDBEvent = *dbEvent;
				pNewDBEvent->cbBlob = (uint16_t)nNewBlob;
				pNewDBEvent->flags = dwNewFlags;
				memcpy(pNewDBEvent + 1, pNewBlob, nNewBlob);

				if (mdbx_put(trnlck, m_dbEvents, &key, &data, MDBX_UPSERT) != MDBX_SUCCESS)
					return FALSE;
			}
		}

		lstEvents.erase(lstEvents.begin(), lstEvents.begin()+portion);
	}
		while (lstEvents.size() > 0);

	{
		txn_ptr trnlck(this);
		MDBX_val key = { DBKey_Crypto_IsEncrypted, sizeof(DBKey_Crypto_IsEncrypted) }, value = { &bEncrypted, sizeof(bool) };
		if (mdbx_put(trnlck, m_dbCrypto, &key, &value, MDBX_UPSERT) != MDBX_SUCCESS)
			return FALSE;
	}

	DBFlush();
	m_bEncrypted = bEncrypted;
	return TRUE;
}
