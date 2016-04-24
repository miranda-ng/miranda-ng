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

static int ModCompare(const ModuleName *mn1, const ModuleName *mn2)
{
	return strcmp(mn1->name, mn2->name);
}

static int OfsCompare(const ModuleName *mn1, const ModuleName *mn2)
{
	return (mn1->ofs - mn2->ofs);
}

CDbxMdb::CDbxMdb(const TCHAR *tszFileName, int iMode) :
	m_safetyMode(true),
	m_bReadOnly((iMode & DBMODE_READONLY) != 0),
	m_bShared((iMode & DBMODE_SHARED) != 0),
	m_dwMaxContactId(1),
	m_lMods(50, ModCompare),
	m_lOfs(50, OfsCompare),
	m_lResidentSettings(50, strcmp)
{
	m_tszProfileName = mir_tstrdup(tszFileName);
	InitDbInstance(this);

	mdb_env_create(&m_pMdbEnv);
	mdb_env_set_maxdbs(m_pMdbEnv, 10);
	mdb_env_set_userctx(m_pMdbEnv, this);

	m_codePage = Langpack_GetDefaultCodePage();
	m_hModHeap = HeapCreate(0, 0, 0);
}

CDbxMdb::~CDbxMdb()
{
	// destroy modules
	HeapDestroy(m_hModHeap);

	mdb_env_close(m_pMdbEnv);

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

int CDbxMdb::Load(bool bSkipInit)
{
	if (!Map())
		return EGROKPRF_CANTREAD;

	if (!bSkipInit) {
		txn_ptr trnlck(m_pMdbEnv);

		unsigned int defFlags = MDB_CREATE;

		mdb_dbi_open(trnlck, "global", defFlags | MDB_INTEGERKEY, &m_dbGlobal);
		mdb_dbi_open(trnlck, "crypto", defFlags, &m_dbCrypto);
		mdb_dbi_open(trnlck, "contacts", defFlags | MDB_INTEGERKEY, &m_dbContacts);
		mdb_dbi_open(trnlck, "modules", defFlags | MDB_INTEGERKEY, &m_dbModules);
		mdb_dbi_open(trnlck, "events", defFlags | MDB_INTEGERKEY, &m_dbEvents);
		mdb_dbi_open(trnlck, "eventsrt", defFlags | MDB_INTEGERKEY, &m_dbEventsSort);
		mdb_dbi_open(trnlck, "settings", defFlags, &m_dbSettings);

		DWORD keyVal = 1;
		MDB_val key = { sizeof(DWORD), &keyVal }, data;
		if (mdb_get(trnlck, m_dbGlobal, &key, &data) == MDB_SUCCESS) 
		{
			const DBHeader *hdr = (const DBHeader*)data.mv_data;
			if (hdr->dwSignature != DBHEADER_SIGNATURE)
				DatabaseCorruption(NULL);
			if (hdr->dwVersion != DBHEADER_VERSION)
				return EGROKPRF_OBSOLETE;

			memcpy(&m_header, data.mv_data, sizeof(m_header));
		}
		else 
		{
			m_header.dwSignature = DBHEADER_SIGNATURE;
			m_header.dwVersion = DBHEADER_VERSION;
			data.mv_data = &m_header; data.mv_size = sizeof(m_header);
			mdb_put(trnlck, m_dbGlobal, &key, &data, 0);

			keyVal = 0;
			DBContact dbc = { DBCONTACT_SIGNATURE, 0, 0, 0 };
			data.mv_data = &dbc; data.mv_size = sizeof(dbc);
			mdb_put(trnlck, m_dbContacts, &key, &data, 0);
		}
		trnlck.commit();

		{
			MDB_val key, val;

			mdb_txn_begin(m_pMdbEnv, nullptr, MDB_RDONLY, &m_txn);

			mdb_cursor_open(m_txn, m_dbEvents, &m_curEvents);
			if (mdb_cursor_get(m_curEvents, &key, &val, MDB_LAST) == MDB_SUCCESS)
				m_dwMaxEventId = *(MEVENT*)key.mv_data + 1;

			mdb_cursor_open(m_txn, m_dbEventsSort, &m_curEventsSort);
			mdb_cursor_open(m_txn, m_dbSettings, &m_curSettings);
			mdb_cursor_open(m_txn, m_dbModules, &m_curModules);

			mdb_cursor_open(m_txn, m_dbContacts, &m_curContacts);
			if (mdb_cursor_get(m_curContacts, &key, &val, MDB_LAST) == MDB_SUCCESS)
				m_dwMaxContactId = *(DWORD*)key.mv_data + 1;

			mdb_txn_reset(m_txn);
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

	return EGROKPRF_NOERROR;
}

int CDbxMdb::Create(void)
{
	m_dwFileSize = 0;
	return (Map()) ? 0 : EGROKPRF_CANTREAD;
}

int CDbxMdb::Check(void)
{
	HANDLE hFile = CreateFile(m_tszProfileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return EGROKPRF_CANTREAD;

	LARGE_INTEGER iFileSize;
	GetFileSizeEx(hFile, &iFileSize);
	m_dwFileSize = (iFileSize.LowPart & 0xFFFFF000);

	DWORD dummy = 0;
	char buf[32];
	if (!ReadFile(hFile, buf, sizeof(buf), &dummy, NULL)) {
		CloseHandle(hFile);
		return EGROKPRF_CANTREAD;
	}

	CloseHandle(hFile);
	return (memcmp(buf + 16, "\xDE\xC0\xEF\xBE", 4)) ? EGROKPRF_UNKHEADER : 0;
}

int CDbxMdb::PrepareCheck(int*)
{
	InitModuleNames();
	return InitCrypt();
}

STDMETHODIMP_(void) CDbxMdb::SetCacheSafetyMode(BOOL bIsSet)
{
	mir_cslock lck(m_csDbAccess);
	m_safetyMode = bIsSet != 0;
}

bool CDbxMdb::Map()
{
	m_dwFileSize += 0x100000;
	mdb_env_set_mapsize(m_pMdbEnv, m_dwFileSize);

	unsigned int mode = MDB_NOSYNC | MDB_NOSUBDIR | MDB_NOLOCK;
//	if (m_bReadOnly)
//		mode |= MDB_RDONLY;
//	else
		mode |= MDB_WRITEMAP;
	return mdb_env_open(m_pMdbEnv, _T2A(m_tszProfileName), mode, 0664) == MDB_SUCCESS;
}

bool CDbxMdb::Remap()
{
	mir_cslock lck(m_csDbAccess);
	m_dwFileSize += 0x100000;
	return mdb_env_set_mapsize(m_pMdbEnv, m_dwFileSize) == MDB_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////

static DWORD DatabaseCorrupted = 0;
static const TCHAR *msg = NULL;
static DWORD dwErr = 0;
static TCHAR tszPanic[] = LPGENT("Miranda has detected corruption in your database. This corruption may be fixed by DbChecker plugin. Please download it from http://miranda-ng.org/p/DbChecker/. Miranda will now shut down.");

EXTERN_C void __cdecl dbpanic(void *)
{
	if (msg) {
		if (dwErr == ERROR_DISK_FULL)
			msg = TranslateT("Disk is full. Miranda will now shut down.");

		TCHAR err[256];
		mir_sntprintf(err, _countof(err), msg, TranslateT("Database failure. Miranda will now shut down."), dwErr);

		MessageBox(0, err, TranslateT("Database Error"), MB_SETFOREGROUND | MB_TOPMOST | MB_APPLMODAL | MB_ICONWARNING | MB_OK);
	}
	else MessageBox(0, TranslateTS(tszPanic), TranslateT("Database Panic"), MB_SETFOREGROUND | MB_TOPMOST | MB_APPLMODAL | MB_ICONWARNING | MB_OK);
	TerminateProcess(GetCurrentProcess(), 255);
}


EXTERN_C void LMDB_FailAssert(void *p, const char *text)
{
	((CDbxMdb*)p)->DatabaseCorruption(_A2T(text));
}

void CDbxMdb::DatabaseCorruption(const TCHAR *text)
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

typedef int (CDbxMdb::*CheckWorker)(int);

int CDbxMdb::Start(DBCHeckCallback *callback)
{
	cb = callback;
	return ERROR_SUCCESS;
}

int CDbxMdb::CheckDb(int, int)
{
	return ERROR_OUT_OF_PAPER;
}

void CDbxMdb::Destroy()
{
	delete this;
}
