/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
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

	VarDescr(LPCSTR var, LPSTR value) :
		szVar(mir_strdup(var)),
		szValue(value)
	{}

	VarDescr(LPCSTR var, PBYTE value, int len) :
		szVar(mir_strdup(var)),
		szValue((char*)memcpy(mir_alloc(len), value, len)),
		iLen(len)
	{}

	ptrA szVar, szValue;
	int  iLen;
};

struct SettingUgraderParam
{
	CDbxKyoto *db;
	LPCSTR    szModule;
	MCONTACT  contactID;
	OBJLIST<VarDescr>* pList;
};

int sttSettingUgrader(const char *szSetting, LPARAM lParam)
{
	SettingUgraderParam *param = (SettingUgraderParam*)lParam;
	if (param->db->IsSettingEncrypted(param->szModule, szSetting)) {
		DBVARIANT dbv = { DBVT_UTF8 };
		if (!param->db->GetContactSettingStr(param->contactID, param->szModule, szSetting, &dbv)) {
			if (dbv.type == DBVT_UTF8) {
				DecodeString(dbv.pszVal);
				param->pList->insert(new VarDescr(szSetting, (LPCSTR)dbv.pszVal));
			}
			param->db->FreeVariant(&dbv);
		}
	}
	return 0;
}

void sttContactEnum(MCONTACT contactID, const char *szModule, CDbxKyoto *db)
{
	OBJLIST<VarDescr> arSettings(1);
	SettingUgraderParam param = { db, szModule, contactID, &arSettings };

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
			db->WriteContactSetting(contactID, &dbcws);

			mir_free(pResult);
		}
	}
}

int sttModuleEnum(const char *szModule, DWORD, LPARAM lParam)
{
	CDbxKyoto *db = (CDbxKyoto*)lParam;
	sttContactEnum(NULL, szModule, db);

	for (MCONTACT contactID = db->FindFirstContact(); contactID; contactID = db->FindNextContact(contactID))
		sttContactEnum(contactID, szModule, db);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CDbxKyoto::InitCrypt()
{
	CRYPTO_PROVIDER *pProvider;
	bool bMissingKey = false;

	DBVARIANT dbv = { 0 };
	dbv.type = DBVT_BLOB;
	if (GetContactSetting(NULL, "CryptoEngine", "Provider", &dbv)) {
	LBL_CreateProvider:
		CRYPTO_PROVIDER **ppProvs;
		int iNumProvs;
		Crypto_EnumProviders(&iNumProvs, &ppProvs);
		if (iNumProvs == 0)
			return 1;

		pProvider = ppProvs[0];  //!!!!!!!!!!!!!!!!!!

		DBCONTACTWRITESETTING dbcws = { "CryptoEngine", "Provider" };
		dbcws.value.type = DBVT_BLOB;
		dbcws.value.pbVal = (PBYTE)pProvider->pszName;
		dbcws.value.cpbVal = (int)strlen(pProvider->pszName) + 1;
		WriteContactSetting(NULL, &dbcws);
	}
	else {
		if (dbv.type != DBVT_BLOB) { // old version, clean it up
			bMissingKey = true;
			goto LBL_CreateProvider;
		}

		pProvider = Crypto_GetProvider(LPCSTR(dbv.pbVal));
		FreeVariant(&dbv);
		if (pProvider == NULL)
			goto LBL_CreateProvider;
	}

	if ((m_crypto = pProvider->pFactory()) == NULL)
		return 3;

	dbv.type = DBVT_BLOB;
	if (GetContactSetting(NULL, "CryptoEngine", "StoredKey", &dbv)) {
		bMissingKey = true;

	LBL_SetNewKey:
		m_crypto->generateKey(); // unencrypted key
		StoreKey();
	}
	else {
		size_t iKeyLength = m_crypto->getKeyLength();
		if (dbv.cpbVal != (WORD)iKeyLength)
			goto LBL_SetNewKey;

		if (!m_crypto->setKey(dbv.pbVal, iKeyLength))
			if (!EnterPassword(dbv.pbVal, iKeyLength))  // password protected?
				return 4;

		FreeVariant(&dbv);
	}

	if (bMissingKey)
		EnumModuleNames(sttModuleEnum, this);

	dbv.type = DBVT_BYTE;
	if (!GetContactSetting(NULL, "CryptoEngine", "DatabaseEncryption", &dbv))
		m_bEncrypted = dbv.bVal != 0;

	InitDialogs();
	return 0;
}

void CDbxKyoto::StoreKey()
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

void CDbxKyoto::SetPassword(LPCTSTR ptszPassword)
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

/////////////////////////////////////////////////////////////////////////////////////////

void CDbxKyoto::ToggleEncryption()
{
	HANDLE hSave1 = hSettingChangeEvent;    hSettingChangeEvent = NULL;
	HANDLE hSave2 = hEventAddedEvent;       hEventAddedEvent = NULL;
	HANDLE hSave3 = hEventDeletedEvent;     hEventDeletedEvent = NULL;
	HANDLE hSave4 = hEventFilterAddedEvent; hEventFilterAddedEvent = NULL;

	mir_cslock lck(m_csDbAccess);
	ToggleSettingsEncryption(NULL);
	ToggleEventsEncryption(NULL);

	for (MCONTACT contactID = FindFirstContact(); contactID; contactID = FindNextContact(contactID)) {
		ToggleSettingsEncryption(contactID);
		ToggleEventsEncryption(contactID);
	}

	m_bEncrypted = !m_bEncrypted;

	DBCONTACTWRITESETTING dbcws = { "CryptoEngine", "DatabaseEncryption" };
	dbcws.value.type = DBVT_BYTE;
	dbcws.value.bVal = m_bEncrypted;
	WriteContactSetting(NULL, &dbcws);

	hSettingChangeEvent = hSave1;
	hEventAddedEvent = hSave2;
	hEventDeletedEvent = hSave3;
	hEventFilterAddedEvent = hSave4;
}

void CDbxKyoto::ToggleSettingsEncryption(MCONTACT contactID)
{
}

void CDbxKyoto::ToggleEventsEncryption(MCONTACT contactID)
{
}
