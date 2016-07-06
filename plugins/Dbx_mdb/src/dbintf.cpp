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

CDbxMdb::CDbxMdb(const TCHAR *tszFileName, int iMode) :
	m_safetyMode(true),
	m_bReadOnly((iMode & DBMODE_READONLY) != 0),
	m_bShared((iMode & DBMODE_SHARED) != 0),
	m_lResidentSettings(50, strcmp),
	m_maxContactId(1)
{
	m_tszProfileName = mir_tstrdup(tszFileName);
	InitDbInstance(this);

	mdb_env_create(&m_pMdbEnv);
	mdb_env_set_maxdbs(m_pMdbEnv, 10);
	mdb_env_set_userctx(m_pMdbEnv, this);
//	mdb_env_set_assert(m_pMdbEnv, LMDB_FailAssert);

	m_codePage = Langpack_GetDefaultCodePage();
}

CDbxMdb::~CDbxMdb()
{
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
	if (Map() != MDB_SUCCESS)
		return EGROKPRF_CANTREAD;

	if (!bSkipInit) {
		txn_ptr trnlck(m_pMdbEnv);

		unsigned int defFlags = MDB_CREATE;

		mdb_dbi_open(trnlck, "global", defFlags | MDB_INTEGERKEY, &m_dbGlobal);
		mdb_dbi_open(trnlck, "crypto", defFlags, &m_dbCrypto);
		mdb_dbi_open(trnlck, "contacts", defFlags | MDB_INTEGERKEY, &m_dbContacts);
		mdb_dbi_open(trnlck, "modules", defFlags | MDB_INTEGERKEY, &m_dbModules);
		mdb_dbi_open(trnlck, "events", defFlags | MDB_INTEGERKEY, &m_dbEvents);

		mdb_dbi_open(trnlck, "eventsrt", defFlags, &m_dbEventsSort);
		mdb_set_compare(trnlck, m_dbEventsSort, DBEventSortingKey::Compare);

		mdb_dbi_open(trnlck, "settings", defFlags, &m_dbSettings);
		mdb_set_compare(trnlck, m_dbSettings, DBSettingKey::Compare);

		uint32_t keyVal = 1;
		MDB_val key = { sizeof(keyVal), &keyVal }, data;
		if (mdb_get(trnlck, m_dbGlobal, &key, &data) == MDB_SUCCESS) 
		{
			const DBHeader *hdr = (const DBHeader*)data.mv_data;
			if (hdr->dwSignature != DBHEADER_SIGNATURE)
				return EGROKPRF_DAMAGED;
			if (hdr->dwVersion != DBHEADER_VERSION)
				return EGROKPRF_OBSOLETE;

			m_header = *hdr;
		}
		else 
		{
			m_header.dwSignature = DBHEADER_SIGNATURE;
			m_header.dwVersion = DBHEADER_VERSION;
			data.mv_data = &m_header; data.mv_size = sizeof(m_header);
			mdb_put(trnlck, m_dbGlobal, &key, &data, 0);

			keyVal = 0;
			DBContact dbc = { 0, 0, 0 };
			data.mv_data = &dbc; data.mv_size = sizeof(dbc);
			mdb_put(trnlck, m_dbContacts, &key, &data, 0);
		}
		trnlck.commit();

		{
			MDB_val key, val;

			mdb_txn_begin(m_pMdbEnv, nullptr, MDB_RDONLY, &m_txn);

			mdb_cursor_open(m_txn, m_dbEvents, &m_curEvents);
			if (mdb_cursor_get(m_curEvents, &key, &val, MDB_LAST) == MDB_SUCCESS)
				m_dwMaxEventId = *(MEVENT*)key.mv_data;

			mdb_cursor_open(m_txn, m_dbEventsSort, &m_curEventsSort);
			mdb_cursor_open(m_txn, m_dbSettings, &m_curSettings);
			mdb_cursor_open(m_txn, m_dbModules, &m_curModules);

			mdb_cursor_open(m_txn, m_dbContacts, &m_curContacts);
			if (mdb_cursor_get(m_curContacts, &key, &val, MDB_LAST) == MDB_SUCCESS)
				m_maxContactId = *(MCONTACT*)key.mv_data;

			MDB_stat st;
			mdb_stat(m_txn, m_dbContacts, &st);
			m_contactCount = st.ms_entries;

			mdb_txn_reset(m_txn);
		}


		if (InitModules()) return EGROKPRF_DAMAGED;
		if (InitCrypt())       return EGROKPRF_DAMAGED;

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
	return (Map() == MDB_SUCCESS) ? 0 : EGROKPRF_CANTREAD;
}

size_t iDefHeaderOffset = 16;
BYTE bDefHeader[] = { 0xDE, 0xC0, 0xEF, 0xBE };

int CDbxMdb::Check(void)
{
	FILE *pFile = _tfopen(m_tszProfileName, _T("rb"));
	if (pFile == nullptr)
		return EGROKPRF_CANTREAD;

	fseek(pFile, iDefHeaderOffset, SEEK_SET);
	BYTE buf[_countof(bDefHeader)];
	if (fread(buf, 1, _countof(buf), pFile) != _countof(buf))
		return EGROKPRF_DAMAGED;

	fclose(pFile);

	return (memcmp(buf, bDefHeader, _countof(bDefHeader))) ? EGROKPRF_UNKHEADER : 0;
}

int CDbxMdb::PrepareCheck(int*)
{
	InitModules();
	return InitCrypt();
}

STDMETHODIMP_(void) CDbxMdb::SetCacheSafetyMode(BOOL bIsSet)
{
	m_safetyMode = bIsSet != 0;
}

int CDbxMdb::Map()
{
	unsigned int mode = MDB_NOSYNC | MDB_NOSUBDIR | /*MDB_NOLOCK |*/ MDB_NOTLS | MDB_WRITEMAP;
	if (m_bReadOnly)
		mode |= MDB_RDONLY;
	return mdb_env_open(m_pMdbEnv, _T2A(m_tszProfileName), mode, 0664);
}

bool CDbxMdb::Remap()
{
	MDB_envinfo ei;
	mdb_env_info(m_pMdbEnv, &ei);
	return mdb_env_set_mapsize(m_pMdbEnv, ei.me_mapsize + 0x100000) == MDB_SUCCESS;
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


EXTERN_C void LMDB_FailAssert(MDB_env *env, const char *text)
{
	((CDbxMdb*)mdb_env_get_userctx(env))->DatabaseCorruption(_A2T(text));
}

EXTERN_C void LMDB_Log(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	CallService(MS_NETLIB_LOG, 0, (LPARAM)(CMStringA().FormatV(fmt, args)));
	va_end(args);
}

void CDbxMdb::DatabaseCorruption(const TCHAR *text)
{
	int kill = 0;

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
