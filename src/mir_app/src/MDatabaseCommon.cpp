/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-20 Miranda NG team,
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
// Encryption support

BOOL MDatabaseCommon::IsSettingEncrypted(LPCSTR szModule, LPCSTR szSetting)
{
	if (!_strnicmp(szSetting, "password", 8))      return true;
	if (!mir_strcmp(szSetting, "NLProxyAuthPassword")) return true;
	if (!mir_strcmp(szSetting, "LNPassword"))          return true;
	if (!mir_strcmp(szSetting, "FileProxyPassword"))   return true;
	if (!mir_strcmp(szSetting, "TokenSecret"))         return true;

	if (!mir_strcmp(szModule, "SecureIM")) {
		if (!mir_strcmp(szSetting, "pgp"))              return true;
		if (!mir_strcmp(szSetting, "pgpPrivKey"))       return true;
	}
	return false;
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

STDMETHODIMP_(MIDatabaseChecker *) MDatabaseCommon::GetChecker()
{
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Event cursors

STDMETHODIMP_(DB::EventCursor *) MDatabaseCommon::EventCursor(MCONTACT, MEVENT)
{
	return nullptr;
}

STDMETHODIMP_(DB::EventCursor *) MDatabaseCommon::EventCursorRev(MCONTACT, MEVENT)
{
	return nullptr;
}
