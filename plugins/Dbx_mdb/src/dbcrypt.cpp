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

#define DBKEY_PROVIDER "Provider"
#define DBKEY_KEY      "Key"
#define DBKEY_IS_ENCRYPTED "EncryptedDB"

CRYPTO_PROVIDER* CDbxMdb::SelectProvider()
{
	CRYPTO_PROVIDER **ppProvs, *pProv;
	int iNumProvs;
	Crypto_EnumProviders(&iNumProvs, &ppProvs);
	if (iNumProvs == 0)
		return nullptr;

	if (iNumProvs > 1)
	{
		CSelectCryptoDialog dlg(ppProvs, iNumProvs);
		dlg.DoModal();
		pProv = dlg.GetSelected();
	}
	else pProv = ppProvs[0];

	for (;; Remap())
	{
		txn_ptr txn(m_pMdbEnv);
		MDB_val key = { sizeof(DBKEY_PROVIDER), DBKEY_PROVIDER }, value = { mir_strlen(pProv->pszName) + 1, pProv->pszName };
		mdb_put(txn, m_dbCrypto, &key, &value, 0);
		if (txn.commit())
			break;
	}

	return pProv;
}

int CDbxMdb::InitCrypt()
{
	CRYPTO_PROVIDER *pProvider;

	txn_ptr_ro txn(m_txn);

	MDB_val key = { sizeof(DBKEY_PROVIDER), DBKEY_PROVIDER }, value;
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

	key.mv_size = sizeof(DBKEY_KEY); key.mv_data = DBKEY_KEY;
	if (mdb_get(txn, m_dbCrypto, &key, &value) == MDB_SUCCESS)
	{
		if (value.mv_size != m_crypto->getKeyLength())
		{
			if (!m_crypto->generateKey())
				return 6;
			StoreKey();
		}
		else
		{
			if (!m_crypto->setKey((const BYTE*)value.mv_data, value.mv_size))
				if (!EnterPassword((const BYTE*)value.mv_data, value.mv_size))  // password protected?
					return 4;
		}
	}
	else
	{
		if (!m_crypto->generateKey())
			return 6;
		StoreKey();
	}

	key.mv_size = sizeof(DBKEY_IS_ENCRYPTED); key.mv_data = DBKEY_IS_ENCRYPTED;
	
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
		MDB_val key = { sizeof(DBKEY_KEY), DBKEY_KEY }, value = { iKeyLength, pKey };
		mdb_put(txn, m_dbCrypto, &key, &value, 0);
		if (txn.commit())
			break;
	}
	SecureZeroMemory(pKey, iKeyLength);
}

void CDbxMdb::SetPassword(LPCTSTR ptszPassword)
{
	if (ptszPassword == NULL || *ptszPassword == 0) {
		m_bUsesPassword = false;
		m_crypto->setPassword(NULL);
	}
	else {
		m_bUsesPassword = true;
		m_crypto->setPassword(pass_ptrA(mir_utf8encodeT(ptszPassword)));
	}
	UpdateMenuItem();
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDbxMdb::EnableEncryption(bool bEncrypted)
{
	if (m_bEncrypted == bEncrypted)
		return 0;

	mir_cslock lck(m_csDbAccess);
	for (MCONTACT contactID = FindFirstContact(); contactID; contactID = FindNextContact(contactID)) 
	{
		EnableContactEncryption(contactID, bEncrypted);
	}

	for (;; Remap())
	{
		txn_ptr txn(m_pMdbEnv);
		MDB_val key = { sizeof(DBKEY_IS_ENCRYPTED), DBKEY_IS_ENCRYPTED }, value = { sizeof(bool), &bEncrypted };
		MDB_CHECK(mdb_put(txn, m_dbCrypto, &key, &value, 0), 1);
		if (txn.commit())
			break;
	}
	m_bEncrypted = bEncrypted;
	return 0;
}

int CDbxMdb::EnableContactEncryption(MCONTACT hContact, bool bEncrypted)
{
	//TODO: encrypt/decrypt all contact events and settings
	return 0;
}