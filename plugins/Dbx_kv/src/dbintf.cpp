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

static int compareSettings(ham_db_t*, const uint8_t *p1, uint32_t, const uint8_t *p2, uint32_t)
{
	DBSettingSortingKey *k1 = (DBSettingSortingKey*)p1, *k2 = (DBSettingSortingKey*)p2;
	if (k1->dwContactID < k2->dwContactID) return -1;
	if (k1->dwContactID > k2->dwContactID) return 1;

	if (k1->dwOfsModule < k2->dwOfsModule) return -1;
	if (k1->dwOfsModule > k2->dwOfsModule) return 1;

	return strcmp(k1->szSettingName, k2->szSettingName);
}

static int compareEvents(ham_db_t*, const uint8_t *p1, uint32_t, const uint8_t *p2, uint32_t)
{
	DBEventSortingKey *k1 = (DBEventSortingKey*)p1, *k2 = (DBEventSortingKey*)p2;
	if (k1->dwContactId < k2->dwContactId) return -1;
	if (k1->dwContactId > k2->dwContactId) return 1;

	if (k1->ts < k2->ts) return -1;
	if (k1->ts > k2->ts) return 1;

	if (k1->dwEventId < k2->dwEventId) return -1;
	if (k1->dwEventId > k2->dwEventId) return 1;

	return 0;
}

static int ModCompare(const ModuleName *mn1, const ModuleName *mn2)
{
	return strcmp(mn1->name, mn2->name);
}

static int OfsCompare(const ModuleName *mn1, const ModuleName *mn2)
{
	return (mn1->ofs - mn2->ofs);
}

static int stringCompare2(const char *p1, const char *p2)
{
	return strcmp(p1, p2);
}

CDbxKV::CDbxKV(const TCHAR *tszFileName, int iMode) :
	m_safetyMode(true),
	m_bReadOnly((iMode & DBMODE_READONLY) != 0),
	m_bShared((iMode & DBMODE_SHARED) != 0),
	m_dwMaxContactId(1),
	m_lMods(50, ModCompare),
	m_lOfs(50, OfsCompare),
	m_lResidentSettings(50, stringCompare2)
{
	m_tszProfileName = mir_tstrdup(tszFileName);
	InitDbInstance(this);

	m_codePage = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
	m_hModHeap = HeapCreate(0, 0, 0);
}

CDbxKV::~CDbxKV()
{
	// destroy modules
	HeapDestroy(m_hModHeap);

	// automatically closes all tables
	ham_env_close(m_pMdbEnv, HAM_AUTO_CLEANUP);

	DestroyServiceFunction(hService);
	UnhookEvent(hHook);

	if (m_crypto)
		m_crypto->destroy();

	DestroyHookableEvent(hContactDeletedEvent);
	DestroyHookableEvent(hContactAddedEvent);
	DestroyHookableEvent(hSettingChangeEvent);
	DestroyHookableEvent(hEventMarkedRead);

	DestroyHookableEvent(hEventAddedEvent);
	DestroyHookableEvent(hEventDeletedEvent);
	DestroyHookableEvent(hEventFilterAddedEvent);

	DestroyDbInstance(this);
	mir_free(m_tszProfileName);
}

int CDbxKV::Load(bool bSkipInit)
{
	int mode = HAM_ENABLE_FSYNC | HAM_DISABLE_RECOVERY;
	if (m_bReadOnly)
		mode += HAM_READ_ONLY;

	if (ham_env_open(&m_pMdbEnv, _T2A(m_tszProfileName), mode, NULL) != HAM_SUCCESS)
		return EGROKPRF_CANTREAD;

	if (!bSkipInit) {
		int iFlags = (m_bReadOnly) ? HAM_READ_ONLY : 0;
		if (ham_env_open_db(m_pMdbEnv, &m_dbGlobal,     1, iFlags, 0)) return EGROKPRF_DAMAGED;
		if (ham_env_open_db(m_pMdbEnv, &m_dbContacts,   2, iFlags, 0)) return EGROKPRF_DAMAGED;
		if (ham_env_open_db(m_pMdbEnv, &m_dbModules,    3, iFlags, 0)) return EGROKPRF_DAMAGED;
		if (ham_env_open_db(m_pMdbEnv, &m_dbEvents,     4, iFlags, 0)) return EGROKPRF_DAMAGED;
		if (ham_env_open_db(m_pMdbEnv, &m_dbEventsSort, 5, iFlags, 0)) return EGROKPRF_DAMAGED;
		if (ham_env_open_db(m_pMdbEnv, &m_dbSettings,   6, iFlags, 0)) return EGROKPRF_DAMAGED;

		ham_db_set_compare_func(m_dbEventsSort, compareEvents);
		ham_db_set_compare_func(m_dbSettings, compareSettings);

		DWORD keyVal = 1;
		ham_key_t key = { sizeof(DWORD), &keyVal };
		ham_record_t rec = { 0 };
		if (ham_db_find(m_dbGlobal, NULL, &key, &rec, HAM_FIND_EXACT_MATCH) == HAM_SUCCESS) {
			DBHeader *hdr = (DBHeader*)rec.data;
			if (hdr->dwSignature != DBHEADER_SIGNATURE)
				DatabaseCorruption(NULL);

			memcpy(&m_header, rec.data, sizeof(m_header));
		}
		else {
			m_header.dwSignature = DBHEADER_SIGNATURE;
			m_header.dwVersion = 1;
			rec.data = &m_header; rec.size = sizeof(m_header);
			ham_db_insert(m_dbGlobal, NULL, &key, &rec, HAM_OVERWRITE);

			keyVal = 0;
			DBContact dbc = { DBCONTACT_SIGNATURE, 0, 0, 0 };
			rec.data = &dbc; rec.size = sizeof(dbc);
			ham_db_insert(m_dbContacts, NULL, &key, &rec, HAM_OVERWRITE);
		}

		if (InitModuleNames()) return EGROKPRF_CANTREAD;
		if (InitCrypt())       return EGROKPRF_CANTREAD;

		// everything is ok, go on
		if (!m_bReadOnly) {
			// we don't need events in the service mode
			if (ServiceExists(MS_DB_SETSAFETYMODE)) {
				hContactDeletedEvent = CreateHookableEvent(ME_DB_CONTACT_DELETED);
				hContactAddedEvent = CreateHookableEvent(ME_DB_CONTACT_ADDED);
				hSettingChangeEvent = CreateHookableEvent(ME_DB_CONTACT_SETTINGCHANGED);
				hEventMarkedRead = CreateHookableEvent(ME_DB_EVENT_MARKED_READ);

				hEventAddedEvent = CreateHookableEvent(ME_DB_EVENT_ADDED);
				hEventDeletedEvent = CreateHookableEvent(ME_DB_EVENT_DELETED);
				hEventFilterAddedEvent = CreateHookableEvent(ME_DB_EVENT_FILTER_ADD);
			}
		}

		FillContacts();
	}

	return ERROR_SUCCESS;
}

int CDbxKV::Create(void)
{
	int flags = HAM_ENABLE_FSYNC | HAM_ENABLE_RECOVERY | HAM_ENABLE_FSYNC;
	if (ham_env_create(&m_pMdbEnv, _T2A(m_tszProfileName), flags, 0664, NULL) != HAM_SUCCESS)
		return EGROKPRF_CANTREAD;

	ham_parameter_t paramPrimKey32[] = { { HAM_PARAM_KEY_TYPE, HAM_TYPE_UINT32 }, { 0, 0 } };
	if (ham_env_create_db(m_pMdbEnv, &m_dbGlobal, 1, 0, paramPrimKey32)) return EGROKPRF_DAMAGED;
	if (ham_env_create_db(m_pMdbEnv, &m_dbContacts, 2, 0, paramPrimKey32)) return EGROKPRF_DAMAGED;
	if (ham_env_create_db(m_pMdbEnv, &m_dbModules, 3, 0, paramPrimKey32)) return EGROKPRF_DAMAGED;
	if (ham_env_create_db(m_pMdbEnv, &m_dbEvents, 4, 0, paramPrimKey32)) return EGROKPRF_DAMAGED;

	ham_parameter_t paramEventsPrimKey[] = { 
		{ HAM_PARAM_KEY_TYPE, HAM_TYPE_CUSTOM }, 
		{ HAM_PARAM_KEY_SIZE, sizeof(DBEventSortingKey) },
		{ 0, 0 } };
	if (ham_env_create_db(m_pMdbEnv, &m_dbEventsSort, 5, 0, paramEventsPrimKey)) return EGROKPRF_DAMAGED;
	ham_db_set_compare_func(m_dbEventsSort, compareEvents);

	ham_parameter_t paramSettingsPrimKey[] = {
		{ HAM_PARAM_KEY_TYPE, HAM_TYPE_CUSTOM },
		{ 0, 0 } };
	if (ham_env_create_db(m_pMdbEnv, &m_dbSettings, 6, 0, paramSettingsPrimKey)) return EGROKPRF_DAMAGED;
	ham_db_set_compare_func(m_dbSettings, compareSettings);

	return 0;
}

int CDbxKV::Check(void)
{
	HANDLE hFile = CreateFile(m_tszProfileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return EGROKPRF_CANTREAD;

	DWORD dummy = 0;
	char buf[32];
	if (!ReadFile(hFile, buf, sizeof(buf), &dummy, NULL)) {
		CloseHandle(hFile);
		return EGROKPRF_CANTREAD;
	}

	CloseHandle(hFile);
	return (memcmp(buf + 16, "HAM\x00", 4)) ? EGROKPRF_UNKHEADER : 0;
}

int CDbxKV::PrepareCheck(int*)
{
	InitModuleNames();
	return InitCrypt();
}

STDMETHODIMP_(void) CDbxKV::SetCacheSafetyMode(BOOL bIsSet)
{
	mir_cslock lck(m_csDbAccess);
	m_safetyMode = bIsSet != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static DWORD DatabaseCorrupted = 0;
static const TCHAR *msg = NULL;
static DWORD dwErr = 0;
static TCHAR tszPanic[] = LPGENT("Miranda has detected corruption in your database. This corruption may be fixed by DbChecker plugin. Please download it from http://miranda-ng.org/p/DbChecker/. Miranda will now shut down.");

void __cdecl dbpanic(void *)
{
	if (msg) {
		if (dwErr == ERROR_DISK_FULL)
			msg = TranslateT("Disk is full. Miranda will now shut down.");

		TCHAR err[256];
		mir_sntprintf(err, SIZEOF(err), msg, TranslateT("Database failure. Miranda will now shut down."), dwErr);

		MessageBox(0, err, TranslateT("Database Error"), MB_SETFOREGROUND | MB_TOPMOST | MB_APPLMODAL | MB_ICONWARNING | MB_OK);
	}
	else MessageBox(0, TranslateTS(tszPanic), TranslateT("Database Panic"), MB_SETFOREGROUND | MB_TOPMOST | MB_APPLMODAL | MB_ICONWARNING | MB_OK);
	TerminateProcess(GetCurrentProcess(), 255);
}

void CDbxKV::DatabaseCorruption(const TCHAR *text)
{
	int kill = 0;

	mir_cslockfull lck(m_csDbAccess);
	if (DatabaseCorrupted == 0) {
		DatabaseCorrupted++;
		kill++;
		msg = text;
		dwErr = GetLastError();
	}
	else {
		/* db is already corrupted, someone else is dealing with it, wait here
		so that we don't do any more damage */
		Sleep(INFINITE);
		return;
	}
	lck.unlock();

	if (kill) {
		_beginthread(dbpanic, 0, NULL);
		Sleep(INFINITE);
	}
}

///////////////////////////////////////////////////////////////////////////////
// MIDatabaseChecker

typedef int (CDbxKV::*CheckWorker)(int);

int CDbxKV::Start(DBCHeckCallback *callback)
{
	cb = callback;
	return ERROR_SUCCESS;
}

int CDbxKV::CheckDb(int, int)
{
	return ERROR_OUT_OF_PAPER;

	// return (this->*Workers[phase])(firstTime);
}

void CDbxKV::Destroy()
{
	delete this;
}
