/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team,
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
#include "database.h"

static int stringCompare2(const char *p1, const char *p2)
{
	return mir_strcmp(p1, p2);
}

MDatabaseCommon::MDatabaseCommon() :
	m_lResidentSettings(50, stringCompare2)
{
	m_codePage = Langpack_GetDefaultCodePage();
	m_cache = new MDatabaseCache(this);
}

MDatabaseCommon::~MDatabaseCommon()
{
	if (m_crypto)
		m_crypto->destroy();

	UnlockName();
	delete (MDatabaseCache*)m_cache;
}

/////////////////////////////////////////////////////////////////////////////////////////

int MDatabaseCommon::CheckProto(DBCachedContact *cc, const char *proto)
{
	if (cc->szProto == nullptr) {
		char protobuf[MAX_PATH] = { 0 };
		DBVARIANT dbv;
		dbv.type = DBVT_ASCIIZ;
		dbv.pszVal = protobuf;
		dbv.cchVal = sizeof(protobuf);
		if (GetContactSettingStatic(cc->contactID, "Protocol", "p", &dbv) != 0 || (dbv.type != DBVT_ASCIIZ))
			return 0;

		cc->szProto = m_cache->GetCachedSetting(nullptr, protobuf, 0, mir_strlen(protobuf));
	}

	return !mir_strcmp(cc->szProto, proto);
}

void MDatabaseCommon::FillContactSettings()
{
	for (DBCachedContact *cc = m_cache->GetFirstContact(); cc; cc = m_cache->GetNextContact(cc->contactID)) {
		CheckProto(cc, "");

		DBVARIANT dbv; dbv.type = DBVT_DWORD;
		cc->nSubs = (0 != GetContactSetting(cc->contactID, META_PROTO, "NumContacts", &dbv)) ? -1 : dbv.dVal;
		if (cc->nSubs != -1) {
			cc->pSubs = (MCONTACT*)mir_alloc(cc->nSubs * sizeof(MCONTACT));
			for (int k = 0; k < cc->nSubs; k++) {
				char setting[100];
				mir_snprintf(setting, _countof(setting), "Handle%d", k);
				cc->pSubs[k] = (0 != GetContactSetting(cc->contactID, META_PROTO, setting, &dbv)) ? 0 : dbv.dVal;
			}
		}
		cc->nDefault = (0 != GetContactSetting(cc->contactID, META_PROTO, "Default", &dbv)) ? -1 : dbv.dVal;
		cc->parentID = (0 != GetContactSetting(cc->contactID, META_PROTO, "ParentMeta", &dbv)) ? 0 : dbv.dVal;
	}
}

bool MDatabaseCommon::LockName(const wchar_t *pwszProfileName)
{
	if (m_hLock != nullptr)
		return true;

	if (pwszProfileName == nullptr)
		return false;

	CMStringW wszPhysName(pwszProfileName);
	wszPhysName.Replace(L"\\", L"_");
	wszPhysName.Insert(0, L"Global\\");

	HANDLE hMutex = ::CreateMutexW(nullptr, false, wszPhysName);
	if (hMutex == nullptr)
		return false;

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		::CloseHandle(hMutex);
		return false;
	}

	m_hLock = hMutex;
	return true;
}

void MDatabaseCommon::UnlockName()
{
	if (m_hLock) {
		CloseHandle(m_hLock);
		m_hLock = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Modules

static int sttEnumVars(const char *szVarName, void *param)
{
	LIST<char>* vars = (LIST<char>*)param;
	vars->insert(mir_strdup(szVarName));
	return 0;
}

BOOL MDatabaseCommon::DeleteModule(MCONTACT hContact, LPCSTR szModule)
{
	LIST<char> vars(20);
	EnumContactSettings(hContact, sttEnumVars, szModule, &vars);

	for (auto &it : vars.rev_iter()) {
		DeleteContactSetting(hContact, szModule, it);
		mir_free(it);
	}
	return 0;
}

BOOL MDatabaseCommon::Compact(void)
{
	return ERROR_NOT_SUPPORTED;
}

BOOL MDatabaseCommon::Backup(LPCWSTR)
{
	return ERROR_NOT_SUPPORTED;
}

BOOL MDatabaseCommon::Flush(void)
{
	return ERROR_NOT_SUPPORTED;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Contacts

STDMETHODIMP_(MCONTACT) MDatabaseCommon::FindFirstContact(const char *szProto)
{
	mir_cslock lck(m_csDbAccess);
	DBCachedContact *cc = m_cache->GetFirstContact();
	if (cc == nullptr)
		return 0;

	if (!szProto || CheckProto(cc, szProto))
		return cc->contactID;

	return FindNextContact(cc->contactID, szProto);
}

STDMETHODIMP_(MCONTACT) MDatabaseCommon::FindNextContact(MCONTACT contactID, const char *szProto)
{
	mir_cslock lck(m_csDbAccess);
	while (contactID) {
		DBCachedContact *cc = m_cache->GetNextContact(contactID);
		if (cc == nullptr)
			break;

		if (!szProto || CheckProto(cc, szProto))
			return cc->contactID;

		contactID = cc->contactID;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Meta-contacts support

BOOL MDatabaseCommon::MetaDetouchSub(DBCachedContact *cc, int nSub)
{
	return DeleteModule(cc->pSubs[nSub], META_PROTO);
}

BOOL MDatabaseCommon::MetaSetDefault(DBCachedContact *cc)
{
	DBCONTACTWRITESETTING cws;
	cws.szModule = META_PROTO;
	cws.szSetting = "Default";
	cws.value.type = DBVT_DWORD;
	cws.value.dVal = cc->nDefault;
	return WriteContactSetting(cc->contactID, &cws);
}

BOOL MDatabaseCommon::MetaRemoveSubHistory(DBCachedContact*)
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Getting settings

STDMETHODIMP_(BOOL) MDatabaseCommon::GetContactSetting(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv)
{
	dbv->type = 0;
	if (GetContactSettingWorker(contactID, szModule, szSetting, dbv, 0))
		return 1;

	if (dbv->type == DBVT_UTF8) {
		wchar_t *tmp = mir_utf8decodeW(dbv->pszVal);
		if (tmp != nullptr) {
			mir_free(dbv->pszVal);
			dbv->type = DBVT_WCHAR;
			dbv->pwszVal = tmp;
		}
		else {
			dbv->type = DBVT_ASCIIZ;
			mir_free(tmp);
		}
	}

	return 0;
}

STDMETHODIMP_(BOOL) MDatabaseCommon::GetContactSettingStr(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv)
{
	int iSaveType = dbv->type;

	if (GetContactSettingWorker(contactID, szModule, szSetting, dbv, 0))
		return 1;

	if (iSaveType == 0 || iSaveType == dbv->type)
		return 0;

	if (dbv->type != DBVT_ASCIIZ && dbv->type != DBVT_UTF8)
		return 1;

	if (iSaveType == DBVT_WCHAR) {
		if (dbv->type != DBVT_UTF8) {
			int len = MultiByteToWideChar(CP_ACP, 0, dbv->pszVal, -1, nullptr, 0);
			wchar_t* wszResult = (wchar_t*)mir_alloc((len + 1) * sizeof(wchar_t));
			if (wszResult == nullptr)
				return 1;

			MultiByteToWideChar(CP_ACP, 0, dbv->pszVal, -1, wszResult, len);
			wszResult[len] = 0;
			mir_free(dbv->pszVal);
			dbv->pwszVal = wszResult;
		}
		else {
			char* savePtr = NEWSTR_ALLOCA(dbv->pszVal);
			mir_free(dbv->pszVal);
			if (!mir_utf8decode(savePtr, &dbv->pwszVal))
				return 1;
		}
	}
	else if (iSaveType == DBVT_UTF8) {
		char* tmpBuf = mir_utf8encode(dbv->pszVal);
		if (tmpBuf == nullptr)
			return 1;

		mir_free(dbv->pszVal);
		dbv->pszVal = tmpBuf;
	}
	else if (iSaveType == DBVT_ASCIIZ)
		mir_utf8decode(dbv->pszVal, nullptr);

	dbv->type = iSaveType;
	return 0;
}

STDMETHODIMP_(BOOL) MDatabaseCommon::GetContactSettingStatic(MCONTACT contactID, LPCSTR szModule, LPCSTR szSetting, DBVARIANT *dbv)
{
	bool bNeedsWchars;
	size_t cbSaved = 0;

	if (dbv->type == DBVT_WCHAR) { // there's no wchar_t strings in a database, we need conversion
		cbSaved = dbv->cchVal - 1;
		dbv->cchVal *= sizeof(wchar_t); // extend a room for the utf8 string
		dbv->type = DBVT_UTF8;
		bNeedsWchars = true;
	}
	else bNeedsWchars = false;

	if (GetContactSettingWorker(contactID, szModule, szSetting, dbv, 1))
		return 1;

	if (bNeedsWchars) {
		char *pBuf = NEWSTR_ALLOCA(dbv->pszVal);
		int cbLen = Utf8toUcs2(pBuf, dbv->cchVal, dbv->pwszVal, cbSaved);
		if (cbLen < 0)
			return 1;

		dbv->pwszVal[cbLen] = 0;
	}
	else if (dbv->type == DBVT_UTF8) {
		mir_utf8decode(dbv->pszVal, nullptr);
		dbv->type = DBVT_ASCIIZ;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool ValidLookupName(const char *szModule, const char *szSetting)
{
	if (!strcmp(szModule, META_PROTO))
		return strcmp(szSetting, "IsSubcontact") && strcmp(szSetting, "ParentMetaID");

	return false;
}

STDMETHODIMP_(int) MDatabaseCommon::GetContactSettingWorker(MCONTACT contactID, const char *szModule, const char *szSetting, DBVARIANT *dbv, int isStatic)
{
	if (szSetting == nullptr || szModule == nullptr)
		return 1;

	DBVARIANT *pCachedValue;
	size_t settingNameLen = strlen(szSetting);
	size_t moduleNameLen = strlen(szModule);
	{
		mir_cslock lck(m_csDbAccess);

LBL_Seek:
		char *szCachedSettingName = m_cache->GetCachedSetting(szModule, szSetting, moduleNameLen, settingNameLen);

		pCachedValue = m_cache->GetCachedValuePtr(contactID, szCachedSettingName, 0);
		if (pCachedValue == nullptr) {
			// if nothing was faund, try to lookup the same setting from meta's default contact
			if (contactID) {
				DBCachedContact *cc = m_cache->GetCachedContact(contactID);
				if (cc && cc->IsMeta() && ValidLookupName(szModule, szSetting)) {
					if (contactID = db_mc_getDefault(contactID)) {
						szModule = Proto_GetBaseAccountName(contactID);
						if (szModule == nullptr) // smth went wrong
							return 1;

						moduleNameLen = strlen(szModule);
						goto LBL_Seek;
					}
				}
			}

			// otherwise fail
			return 1;
		}
	}

	switch(pCachedValue->type) {
	case DBVT_ASCIIZ:
	case DBVT_UTF8:
		dbv->type = pCachedValue->type;
		if (isStatic) {
			int cbLen = (int)mir_strlen(pCachedValue->pszVal);
			int cbOrigLen = dbv->cchVal;
			cbOrigLen--;
			if (cbLen < cbOrigLen)
				cbOrigLen = cbLen;
			memcpy(dbv->pszVal, pCachedValue->pszVal, cbOrigLen);
			dbv->pszVal[cbOrigLen] = 0;
			dbv->cchVal = cbLen;
		}
		else {
			dbv->pszVal = (char *)mir_alloc(strlen(pCachedValue->pszVal) + 1);
			strcpy(dbv->pszVal, pCachedValue->pszVal);
			dbv->cchVal = pCachedValue->cchVal;
		}
		break;

	case DBVT_BLOB:
		dbv->type = DBVT_BLOB;
		if (isStatic) {
			if (pCachedValue->cpbVal < dbv->cpbVal)
				dbv->cpbVal = pCachedValue->cpbVal;
			memcpy(dbv->pbVal, pCachedValue->pbVal, dbv->cpbVal);
		}
		else {
			dbv->pbVal = (uint8_t *)mir_alloc(pCachedValue->cpbVal);
			memcpy(dbv->pbVal, pCachedValue->pbVal, pCachedValue->cpbVal);
		}
		dbv->cpbVal = pCachedValue->cpbVal;
		break;

	case DBVT_ENCRYPTED:
		if (m_crypto != nullptr) {
			size_t realLen;
			ptrA decoded(m_crypto->decodeString(pCachedValue->pbVal, pCachedValue->cpbVal, &realLen));
			if (decoded == nullptr)
				return 1;

			dbv->type = DBVT_UTF8;
			if (isStatic) {
				dbv->cchVal--;
				if (realLen < dbv->cchVal)
					dbv->cchVal = uint16_t(realLen);
				memcpy(dbv->pszVal, decoded, dbv->cchVal);
				dbv->pszVal[dbv->cchVal] = 0;
				dbv->cchVal = uint16_t(realLen);
			}
			else {
				dbv->pszVal = (char *)mir_alloc(1 + realLen);
				memcpy(dbv->pszVal, decoded, realLen);
				dbv->pszVal[realLen] = 0;
			}
			break;
		}
		return 1;

	default:
		memcpy(dbv, pCachedValue, sizeof(DBVARIANT));
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP_(BOOL) MDatabaseCommon::FreeVariant(DBVARIANT *dbv)
{
	if (dbv == nullptr) return 1;

	switch (dbv->type) {
	case DBVT_ASCIIZ:
	case DBVT_UTF8:
	case DBVT_WCHAR:
		if (dbv->pszVal) mir_free(dbv->pszVal);
		dbv->pszVal = nullptr;
		break;
	case DBVT_BLOB:
		if (dbv->pbVal) mir_free(dbv->pbVal);
		dbv->pbVal = nullptr;
		break;
	}
	dbv->type = 0;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP_(BOOL) MDatabaseCommon::WriteContactSetting(MCONTACT contactID, DBCONTACTWRITESETTING *dbcws)
{
	if (dbcws == nullptr || dbcws->szSetting == nullptr || dbcws->szModule == nullptr)
		return 1;

	// the db format can't tolerate more than 255 bytes of space (incl. null) for settings+module name
	size_t settingNameLen = strlen(dbcws->szSetting);
	size_t moduleNameLen = strlen(dbcws->szModule);

	// used for notifications
	DBCONTACTWRITESETTING dbcwNotif = *dbcws;
	if (dbcwNotif.value.type == DBVT_WCHAR) {
		if (dbcwNotif.value.pszVal != nullptr) {
			T2Utf val(dbcwNotif.value.pwszVal);
			if (!val)
				return 1;

			dbcwNotif.value.pszVal = NEWSTR_ALLOCA(val);
			dbcwNotif.value.type = DBVT_UTF8;
		}
		else return 1;
	}

	if (dbcwNotif.szModule == nullptr || dbcwNotif.szSetting == nullptr)
		return 1;

	DBCONTACTWRITESETTING dbcwWork = dbcwNotif;

	mir_ptr<uint8_t> pEncoded(nullptr);
	bool bIsEncrypted = false;
	switch (dbcwWork.value.type) {
	case DBVT_BYTE: case DBVT_WORD: case DBVT_DWORD:
		break;

	case DBVT_ASCIIZ:
	case DBVT_UTF8:
		bIsEncrypted = m_bEncrypted || IsSettingEncrypted(dbcws->szModule, dbcws->szSetting);
		if (dbcwWork.value.pszVal == nullptr)
			return 1;

		dbcwWork.value.cchVal = (uint16_t)strlen(dbcwWork.value.pszVal);
		if (bIsEncrypted && m_crypto) {
			size_t len;
			uint8_t *pResult = m_crypto->encodeString(dbcwWork.value.pszVal, &len);
			if (pResult != nullptr) {
				pEncoded = dbcwWork.value.pbVal = pResult;
				dbcwWork.value.cpbVal = (uint16_t)len;
				dbcwWork.value.type = DBVT_ENCRYPTED;
			}
		}
		break;

	case DBVT_BLOB:
	case DBVT_ENCRYPTED:
		if (dbcwWork.value.pbVal == nullptr)
			return 1;
		break;

	default:
		return 1;
	}

	mir_cslockfull lck(m_csDbAccess);
	char *szCachedSettingName = m_cache->GetCachedSetting(dbcwWork.szModule, dbcwWork.szSetting, moduleNameLen, settingNameLen);

	DBVARIANT *pCachedValue = m_cache->GetCachedValuePtr(contactID, szCachedSettingName, 1);
	if (pCachedValue != nullptr) {
		bool bIsIdentical = false;
		if (pCachedValue->type == dbcwWork.value.type) {
			switch (dbcwWork.value.type) {
			case DBVT_BYTE:   bIsIdentical = pCachedValue->bVal == dbcwWork.value.bVal;  break;
			case DBVT_WORD:   bIsIdentical = pCachedValue->wVal == dbcwWork.value.wVal;  break;
			case DBVT_DWORD:  bIsIdentical = pCachedValue->dVal == dbcwWork.value.dVal;  break;
			case DBVT_UTF8:
			case DBVT_ASCIIZ: bIsIdentical = strcmp(pCachedValue->pszVal, dbcwWork.value.pszVal) == 0; break;
			case DBVT_BLOB:
			case DBVT_ENCRYPTED:
				if (pCachedValue->cpbVal == dbcwWork.value.cchVal)
					bIsIdentical = memcmp(pCachedValue->pbVal, dbcwWork.value.pbVal, dbcwWork.value.cchVal);
				break;
			}
			if (bIsIdentical)
				return 0;
		}
		m_cache->SetCachedVariant(&dbcwWork.value, pCachedValue);
	}

	// for non-resident settings we call a write worker
	if (szCachedSettingName[-1] == 0)
		if (WriteContactSettingWorker(contactID, dbcwWork))
			return 1;

	lck.unlock();
	NotifyEventHooks(g_hevSettingChanged, contactID, (LPARAM)&dbcwNotif);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Resident settings

STDMETHODIMP_(BOOL) MDatabaseCommon::EnumResidentSettings(DBMODULEENUMPROC pFunc, void *pParam)
{
	for (auto &it : m_lResidentSettings) {
		int ret = pFunc(it, pParam);
		if (ret)
			return ret;
	}
	return 0;
}

STDMETHODIMP_(BOOL) MDatabaseCommon::SetSettingResident(BOOL bIsResident, const char *pszSettingName)
{
	char *szSetting = m_cache->GetCachedSetting(nullptr, pszSettingName, 0, mir_strlen(pszSettingName));
	szSetting[-1] = (char)bIsResident;

	mir_cslock lck(m_csDbAccess);
	int idx = m_lResidentSettings.getIndex(szSetting);
	if (idx == -1) {
		if (bIsResident)
			m_lResidentSettings.insert(szSetting);
	}
	else if (!bIsResident)
		m_lResidentSettings.remove(idx);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

class MDefaultChecker : public MIDatabaseChecker
{
	STDMETHODIMP_(int) Start(DBCHeckCallback*) override
	{	return ERROR_SUCCESS;
	}

	STDMETHODIMP_(BOOL) CheckDb(int /*phase*/) override
	{	return ERROR_OUT_OF_PAPER;
	}
	
	STDMETHODIMP_(VOID) Destroy() override
	{}
};

static MDefaultChecker sttDefaultChecker;

STDMETHODIMP_(MIDatabaseChecker *) MDatabaseCommon::GetChecker()
{
	return &sttDefaultChecker;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Event cursors

class CCompatiblityCursor : public DB::EventCursor
{
	MDatabaseCommon *db;
	MEVENT curr;

public:
	CCompatiblityCursor(MDatabaseCommon *pDb, MCONTACT hContact, MEVENT hEvent) :
		DB::EventCursor(hContact),
		db(pDb)
	{
		curr = (hEvent == 0) ? db->FindFirstEvent(hContact) : db->FindNextEvent(hContact, hEvent);
	}

	MEVENT FetchNext() override
	{
		if (curr == 0)
			return 0;
		
		MEVENT ret = curr; curr = db->FindNextEvent(hContact, curr);
		return ret;
	}
};

STDMETHODIMP_(DB::EventCursor*) MDatabaseCommon::EventCursor(MCONTACT hContact, MEVENT hEvent)
{
	return new CCompatiblityCursor(this, hContact, hEvent);
}

/////////////////////////////////////////////////////////////////////////////////////////

class CCompatiblityCursorRev : public DB::EventCursor
{
	MDatabaseCommon *db;
	MEVENT curr;

public:
	CCompatiblityCursorRev(MDatabaseCommon *pDb, MCONTACT hContact, MEVENT hEvent) :
		DB::EventCursor(hContact),
		db(pDb)
	{
		curr = (hEvent == 0) ? db->FindLastEvent(hContact) : db->FindPrevEvent(hContact, hEvent);
	}

	MEVENT FetchNext() override
	{
		if (curr == 0)
			return 0;

		MEVENT ret = curr; curr = db->FindPrevEvent(hContact, curr);
		return ret;
	}
};

STDMETHODIMP_(DB::EventCursor*) MDatabaseCommon::EventCursorRev(MCONTACT hContact, MEVENT hEvent)
{
	return new CCompatiblityCursorRev(this, hContact, hEvent);
}
