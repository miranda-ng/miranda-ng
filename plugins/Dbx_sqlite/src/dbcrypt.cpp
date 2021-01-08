#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Saving encryption key in a database

STDMETHODIMP_(BOOL) CDbxSQLite::ReadCryptoKey(MBinBuffer &buf)
{
	DBVARIANT dbv = {};
	dbv.type = DBVT_BLOB;
	if (GetContactSetting(0, "CryptoEngine", "StoredKey", &dbv))
		return FALSE;

	buf.append(dbv.pbVal, dbv.cpbVal);
	return TRUE;
}

STDMETHODIMP_(BOOL) CDbxSQLite::StoreCryptoKey()
{
	size_t iKeyLength = m_crypto->getKeyLength();
	BYTE *pKey = (BYTE*)_alloca(iKeyLength);
	m_crypto->getKey(pKey, iKeyLength);

	DBCONTACTWRITESETTING dbcws = { "CryptoEngine", "StoredKey" };
	dbcws.value.type = DBVT_BLOB;
	dbcws.value.cpbVal = (WORD)iKeyLength;
	dbcws.value.pbVal = pKey;
	WriteContactSetting(0, &dbcws);

	SecureZeroMemory(pKey, iKeyLength);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Saving encryption flag

STDMETHODIMP_(BOOL) CDbxSQLite::ReadEncryption()
{
	DBVARIANT dbv = {};
	dbv.type = DBVT_BYTE;
	return (GetContactSetting(0, "CryptoEngine", "DatabaseEncryption", &dbv)) ? false : dbv.bVal != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Saving provider in a database

STDMETHODIMP_(CRYPTO_PROVIDER*) CDbxSQLite::ReadProvider()
{
	DBVARIANT dbv = {};
	dbv.type = DBVT_BLOB;
	if (GetContactSetting(0, "CryptoEngine", "Provider", &dbv))
		return nullptr;

	if (dbv.type != DBVT_BLOB) 
		return nullptr;

	auto *pProvider = Crypto_GetProvider(LPCSTR(dbv.pbVal));
	FreeVariant(&dbv);
	return pProvider;
}

STDMETHODIMP_(BOOL) CDbxSQLite::StoreProvider(CRYPTO_PROVIDER *pProvider)
{
	DBCONTACTWRITESETTING dbcws = { "CryptoEngine", "Provider" };
	dbcws.value.type = DBVT_BLOB;
	dbcws.value.pbVal = (PBYTE)pProvider->pszName;
	dbcws.value.cpbVal = (WORD)mir_strlen(pProvider->pszName) + 1;
	WriteContactSetting(0, &dbcws);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Toggles full/partial encryption mode

STDMETHODIMP_(BOOL) CDbxSQLite::EnableEncryption(BOOL)
{
	return FALSE;
}
