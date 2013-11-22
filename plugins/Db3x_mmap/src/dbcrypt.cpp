/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012-13 Miranda NG project,
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

#include "commonheaders.h"

int CDb3Base::InitCrypt()
{
	CRYPTO_PROVIDER *pProvider;

	DBVARIANT dbv = { 0 };
	dbv.type = DBVT_ASCIIZ;
	DBCONTACTGETSETTING dbcgs = { "CryptoEngine", "Provider", &dbv };
	if (GetContactSettingStr(NULL, &dbcgs)) {
		CRYPTO_PROVIDER **ppProvs;
		int iNumProvs;
		Crypto_EnumProviders(&iNumProvs, &ppProvs);
		if (iNumProvs == 0)
			return 1;

		pProvider = ppProvs[0];  //!!!!!!!!!!!!!!!!!!
		
		DBCONTACTWRITESETTING dbcws = { "CryptoEngine", "Provider" };
		dbcws.value.type = DBVT_ASCIIZ;
		dbcws.value.pszVal = pProvider->pszName;
		WriteContactSetting(NULL, &dbcws);
	}
	else {
		pProvider = Crypto_GetProvider(dbv.pszVal);
		FreeVariant(&dbv);
		if (pProvider == NULL)
			return 2;
	}

	if ((m_crypto = pProvider->pFactory()) == NULL)
		return 3;

	size_t iKeyLength = m_crypto->getKeyLength();

	dbv.type = DBVT_BLOB;
	dbcgs.szSetting = "StoredKey";
	if (GetContactSetting(NULL, &dbcgs)) {
LBL_SetNewKey:
		m_crypto->generateKey(); // unencrypted key
		
		BYTE *pKey = (BYTE*)_alloca(iKeyLength);
		m_crypto->getKey(pKey, iKeyLength);

		DBCONTACTWRITESETTING dbcws = { "CryptoEngine", "StoredKey" };
		dbcws.value.type = DBVT_BLOB;
		dbcws.value.cpbVal = (WORD)iKeyLength;
		dbcws.value.pbVal = pKey;
		WriteContactSetting(NULL, &dbcws);

		memset(pKey, 0, iKeyLength);
	}
	else {
		if (dbv.cpbVal != (WORD)iKeyLength)
			goto LBL_SetNewKey;

		m_crypto->setKey(dbv.pbVal, iKeyLength);
		FreeVariant(&dbv);
	}
	return 0;
}
