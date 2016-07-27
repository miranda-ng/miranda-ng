/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-16 Miranda NG project (http://miranda-ng.org)
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

/////////////////////////////////////////////////////////////////////////////////////////

char DBKey_Crypto_Provider   [] = "Provider";
char DBKey_Crypto_Key        [] = "Key";
char DBKey_Crypto_IsEncrypted[] = "EncryptedDB";

CRYPTO_PROVIDER* CDbxMdb::SelectProvider()
{
	CRYPTO_PROVIDER **ppProvs, *pProv;
	int iNumProvs;
	Crypto_EnumProviders(&iNumProvs, &ppProvs);

	if (iNumProvs == 0)
		return nullptr;

	bool bTotalCrypt = false;

	if (iNumProvs > 1)
	{
		CSelectCryptoDialog dlg(ppProvs, iNumProvs);
		dlg.DoModal();
		pProv = dlg.GetSelected();
		bTotalCrypt = dlg.TotalSelected();
	}
	else pProv = ppProvs[0];

	for (;; Remap())
	{
		txn_ptr txn(m_pMdbEnv);

		MDB_val key = { sizeof(DBKey_Crypto_Provider), DBKey_Crypto_Provider }, value = { mir_strlen(pProv->pszName) + 1, pProv->pszName };
		MDB_CHECK(mdb_put(txn, m_dbCrypto, &key, &value, 0), nullptr);

		key.mv_size = sizeof(DBKey_Crypto_IsEncrypted); key.mv_data = DBKey_Crypto_IsEncrypted; value.mv_size = sizeof(bool); value.mv_data = &bTotalCrypt;
		MDB_CHECK(mdb_put(txn, m_dbCrypto, &key, &value, 0), nullptr);

		if (txn.commit() == MDB_SUCCESS)
			break;
	}

	return pProv;
}

int CDbxMdb::InitCrypt()
{
	CRYPTO_PROVIDER *pProvider;

	txn_ptr_ro txn(m_txn);

	MDB_val key = { sizeof(DBKey_Crypto_Provider), DBKey_Crypto_Provider }, value;
	if (mdb_get(txn, m_dbCrypto, &key, &value) == MDB_SUCCESS)
	{
		pProvider = Crypto_GetProvider((const char*)value.mv_data);
		if (pProvider == nullptr)
			pProvider = SelectProvider();
	}
	else
	{
		pProvider = SelectProvider();
	}
	if (pProvider == nullptr) 
		return 1;

	if ((m_crypto = pProvider->pFactory()) == nullptr)
		return 3;

	key.mv_size = sizeof(DBKey_Crypto_Key); key.mv_data = DBKey_Crypto_Key;
	if (mdb_get(txn, m_dbCrypto, &key, &value) == MDB_SUCCESS && (value.mv_size == m_crypto->getKeyLength()))
	{
		if (!m_crypto->setKey((const BYTE*)value.mv_data, value.mv_size))
		{
			DlgChangePassParam param = { this };
			CEnterPasswordDialog dlg(&param);
			while (true)
			{
				if (-128 != dlg.DoModal())
					return 4;
				m_crypto->setPassword(pass_ptrA(mir_utf8encodeW(param.newPass)));
				if (m_crypto->setKey((const BYTE*)value.mv_data, value.mv_size))
				{
					m_bUsesPassword = true;
					SecureZeroMemory(&param, sizeof(param));
					break;
				}
				param.wrongPass++;
			}
		}
	}
	else
	{
		if (!m_crypto->generateKey())
			return 6;
		StoreKey();
	}

	key.mv_size = sizeof(DBKey_Crypto_IsEncrypted); key.mv_data = DBKey_Crypto_IsEncrypted;
	
	if (mdb_get(txn, m_dbCrypto, &key, &value) == MDB_SUCCESS)
		m_bEncrypted = *(const bool*)value.mv_data;
	else 
		m_bEncrypted = false;

	InitDialogs();
	return 0;
}

void CDbxMdb::StoreKey()
{
	size_t iKeyLength = m_crypto->getKeyLength();
	BYTE *pKey = (BYTE*)_alloca(iKeyLength);
	m_crypto->getKey(pKey, iKeyLength);

	for (;; Remap())
	{
		txn_ptr txn(m_pMdbEnv);
		MDB_val key = { sizeof(DBKey_Crypto_Key), DBKey_Crypto_Key }, value = { iKeyLength, pKey };
		mdb_put(txn, m_dbCrypto, &key, &value, 0);
		if (txn.commit() == MDB_SUCCESS)
			break;
	}
	SecureZeroMemory(pKey, iKeyLength);
}

void CDbxMdb::SetPassword(LPCTSTR ptszPassword)
{
	if (ptszPassword == NULL || *ptszPassword == 0) 
	{
		m_bUsesPassword = false;
		m_crypto->setPassword(NULL);
	}
	else 
	{
		m_bUsesPassword = true;
		m_crypto->setPassword(pass_ptrA(mir_utf8encodeW(ptszPassword)));
	}
	UpdateMenuItem();
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDbxMdb::EnableEncryption(bool bEncrypted)
{
	if (m_bEncrypted == bEncrypted)
		return 0;


	{
		txn_ptr_ro txn(m_txn);

		MDB_stat st;
		mdb_stat(txn, m_dbEvents, &st);

		std::vector<MEVENT> lstEvents;
		lstEvents.reserve(st.ms_entries);

		{
			cursor_ptr_ro cursor(m_curEvents);
			MDB_val key, data;
			while (mdb_cursor_get(cursor, &key, &data, MDB_NEXT) == MDB_SUCCESS)
			{
				const MEVENT hDbEvent = *(const MEVENT*)key.mv_data;
				lstEvents.push_back(hDbEvent);
			}
		}
		for (auto it = lstEvents.begin(); it != lstEvents.end(); ++it)
		{
			MEVENT &hDbEvent = *it;
			MDB_val key = { sizeof(MEVENT), &hDbEvent }, data;
			mdb_get(txn, m_dbEvents, &key, &data);

			const DBEvent *dbEvent = (const DBEvent*)data.mv_data;
			const BYTE    *pBlob = (BYTE*)(dbEvent + 1);

			if (((dbEvent->flags & DBEF_ENCRYPTED) != 0) != bEncrypted)
			{
				mir_ptr<BYTE> pNewBlob;
				size_t nNewBlob;
				uint32_t dwNewFlags;

				if (dbEvent->flags & DBEF_ENCRYPTED)
				{
					pNewBlob = (BYTE*)m_crypto->decodeBuffer(pBlob, dbEvent->cbBlob, &nNewBlob);
					dwNewFlags = dbEvent->flags & (~DBEF_ENCRYPTED);
				}
				else
				{
					pNewBlob = m_crypto->encodeBuffer(pBlob, dbEvent->cbBlob, &nNewBlob);
					dwNewFlags = dbEvent->flags | DBEF_ENCRYPTED;
				}

				for (;; Remap())
				{
					txn_ptr txn(m_pMdbEnv);
					data.mv_size = sizeof(DBEvent)+nNewBlob;
					MDB_CHECK(mdb_put(txn, m_dbEvents, &key, &data, MDB_RESERVE), 1);

					DBEvent *pNewDBEvent = (DBEvent *)data.mv_data;
					*pNewDBEvent = *dbEvent;
					pNewDBEvent->cbBlob = nNewBlob;
					pNewDBEvent->flags = dwNewFlags;
					memcpy(pNewDBEvent + 1, pNewBlob, nNewBlob);


					if (txn.commit() == MDB_SUCCESS)
						break;
				}
			}
		}
	}

	for (;; Remap())
	{
		txn_ptr txn(m_pMdbEnv);
		MDB_val key = { sizeof(DBKey_Crypto_IsEncrypted), DBKey_Crypto_IsEncrypted }, value = { sizeof(bool), &bEncrypted };
		MDB_CHECK(mdb_put(txn, m_dbCrypto, &key, &value, 0), 1);
		if (txn.commit() == MDB_SUCCESS)
			break;
	}
	m_bEncrypted = bEncrypted;
	return 0;
}