#include "stdafx.h"

STDMETHODIMP_(BOOL) CDbxSQLite::StoreProvider(CRYPTO_PROVIDER *pProvider)
{
	DBCONTACTWRITESETTING dbcws = { "CryptoEngine", "Provider" };
	dbcws.value.type = DBVT_BLOB;
	dbcws.value.pbVal = (PBYTE)pProvider->pszName;
	dbcws.value.cpbVal = (WORD)mir_strlen(pProvider->pszName) + 1;
	WriteContactSetting(0, &dbcws);
	return TRUE;
}
