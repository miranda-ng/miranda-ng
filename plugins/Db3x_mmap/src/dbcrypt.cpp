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

/////////////////////////////////////////////////////////////////////////////////////////

bool isEncrypted(LPCSTR szModule, LPCSTR szSetting);

//VERY VERY VERY BASIC ENCRYPTION FUNCTION

static void Encrypt(char *msg, BOOL up)
{
	int jump = (up) ? 5 : -5;
	for (int i = 0; msg[i]; i++)
		msg[i] = msg[i] + jump;
}

__forceinline void DecodeString(LPSTR buf)
{
	Encrypt(buf, FALSE);
}

struct VarDescr
{
	VarDescr(LPCSTR var, LPCSTR value) :
		szVar(mir_strdup(var)),
		szValue(mir_strdup(value))
		{}
		
	ptrA szVar, szValue;
};

struct SettingUgraderParam
{
	CDb3Mmap *db;
	LPCSTR    szModule;
	HANDLE    hContact;
	OBJLIST<VarDescr>* pList;
};

int sttSettingUgrader(const char *szSetting, LPARAM lParam)
{
	SettingUgraderParam *param = (SettingUgraderParam*)lParam;
	if (isEncrypted(param->szModule, szSetting)) {
		DBVARIANT dbv = { DBVT_UTF8 };
		DBCONTACTGETSETTING dbcgs = { param->szModule, szSetting, &dbv };
		if (!param->db->GetContactSettingStr(param->hContact, &dbcgs)) {
			if (dbv.type == DBVT_UTF8) {
				DecodeString(dbv.pszVal);
				param->pList->insert(new VarDescr(szSetting, dbv.pszVal));
			}
			param->db->FreeVariant(&dbv);
		}
	}
	return 0;
}

void sttContactEnum(HANDLE hContact, const char *szModule, CDb3Mmap *db)
{
	OBJLIST<VarDescr> arSettings(1);
	SettingUgraderParam param = { db, szModule, hContact, &arSettings };

	DBCONTACTENUMSETTINGS dbces = { 0 };
	dbces.pfnEnumProc = sttSettingUgrader;
	dbces.szModule = szModule;
	dbces.lParam = (LPARAM)&param;
	db->EnumContactSettings(NULL, &dbces);

	for (int i = 0; i < arSettings.getCount(); i++) {
		VarDescr &p = arSettings[i];

		size_t len;
		BYTE *pResult = db->m_crypto->encodeString(p.szValue, &len);
		if (pResult != NULL) {
			DBCONTACTWRITESETTING dbcws = { szModule, p.szVar };
			dbcws.value.type = DBVT_ENCRYPTED;
			dbcws.value.pbVal = pResult;
			dbcws.value.cpbVal = (WORD)len;
			db->WriteContactSetting(hContact, &dbcws);

			mir_free(pResult);
		}
	}
}

int sttModuleEnum(const char *szModule, DWORD, LPARAM lParam)
{
	CDb3Mmap *db = (CDb3Mmap*)lParam;
	sttContactEnum(NULL, szModule, db);

	for (HANDLE hContact = db->FindFirstContact(); hContact; hContact = db->FindNextContact(hContact))
		sttContactEnum(hContact, szModule, db);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDb3Mmap::InitCrypt()
{
	CRYPTO_PROVIDER *pProvider;

	DBVARIANT dbv = { 0 };
	dbv.type = DBVT_ASCIIZ;
	DBCONTACTGETSETTING dbcgs = { "CryptoEngine", "Provider", &dbv };
	if (GetContactSettingStr(NULL, &dbcgs)) {
LBL_CreateProvider:
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
			goto LBL_CreateProvider;
	}

	if ((m_crypto = pProvider->pFactory()) == NULL)
		return 3;

	dbv.type = DBVT_BLOB;
	dbcgs.szSetting = "StoredKey";
	if (GetContactSetting(NULL, &dbcgs)) {
LBL_SetNewKey:
		m_crypto->generateKey(); // unencrypted key
		StoreKey();		
	}
	else {
		size_t iKeyLength = m_crypto->getKeyLength();
		if (dbv.cpbVal != (WORD)iKeyLength)
			goto LBL_SetNewKey;

		if (!m_crypto->setKey(dbv.pbVal, iKeyLength))
			if (!EnterPassword(dbv.pbVal, iKeyLength)) // password protected?
				return 4;

		FreeVariant(&dbv);
	}

	if (memcmp(&m_dbHeader.signature, &dbSignatureU, sizeof(m_dbHeader.signature)) &&
		 memcmp(&m_dbHeader.signature, &dbSignatureE, sizeof(m_dbHeader.signature)))
	{
		EnumModuleNames(sttModuleEnum, this);

		// upgrade signature
		memcpy(&m_dbHeader.signature, &dbSignatureU, sizeof(dbSignatureU));
		DBWrite(0, &dbSignatureU, sizeof(dbSignatureU));
	}

	InitDialogs();
	return 0;
}

void CDb3Mmap::StoreKey()
{
	size_t iKeyLength = m_crypto->getKeyLength();
	BYTE *pKey = (BYTE*)_alloca(iKeyLength);
	m_crypto->getKey(pKey, iKeyLength);

	DBCONTACTWRITESETTING dbcws = { "CryptoEngine", "StoredKey" };
	dbcws.value.type = DBVT_BLOB;
	dbcws.value.cpbVal = (WORD)iKeyLength;
	dbcws.value.pbVal = pKey;
	WriteContactSetting(NULL, &dbcws);

	SecureZeroMemory(pKey, iKeyLength);
}

void CDb3Mmap::SetPassword(LPCTSTR ptszPassword)
{
	if (ptszPassword == NULL || *ptszPassword == 0) {
		m_bUsesPassword = false;
		m_crypto->setPassword(NULL);
	}
	else {
		m_bUsesPassword = true;
		m_crypto->setPassword(ptrA(mir_utf8encodeT(ptszPassword)));
	}
	UpdateMenuItem();
}
