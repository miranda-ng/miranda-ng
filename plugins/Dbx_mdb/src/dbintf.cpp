/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org)
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
	m_tszProfileName = mir_wstrdup(tszFileName);
	InitDbInstance(this);

	mdbx_env_create(&m_pMdbEnv);
	mdbx_env_set_maxdbs(m_pMdbEnv, 10);
	mdbx_env_set_userctx(m_pMdbEnv, this);
//	mdbx_env_set_assert(m_pMdbEnv, LMDBX_FailAssert);

	m_codePage = Langpack_GetDefaultCodePage();
}

CDbxMdb::~CDbxMdb()
{
	mdbx_env_close(m_pMdbEnv);

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
	if (Map() != MDBX_SUCCESS)
		return EGROKPRF_CANTREAD;

	if (!bSkipInit) {
		txn_ptr trnlck(m_pMdbEnv);

		unsigned int defFlags = MDBX_CREATE;

		mdbx_dbi_open(trnlck, "global", defFlags | MDBX_INTEGERKEY, &m_dbGlobal);
		mdbx_dbi_open(trnlck, "crypto", defFlags, &m_dbCrypto);
		mdbx_dbi_open(trnlck, "contacts", defFlags | MDBX_INTEGERKEY, &m_dbContacts);
		mdbx_dbi_open(trnlck, "modules", defFlags | MDBX_INTEGERKEY, &m_dbModules);
		mdbx_dbi_open(trnlck, "events", defFlags | MDBX_INTEGERKEY, &m_dbEvents);

		mdbx_dbi_open_ex(trnlck, "eventsrt", defFlags, &m_dbEventsSort, DBEventSortingKey::Compare, nullptr);
		mdbx_dbi_open_ex(trnlck, "settings", defFlags, &m_dbSettings, DBSettingKey::Compare, nullptr);

		uint32_t keyVal = 1;
		MDBX_val key = { &keyVal, sizeof(keyVal) }, data;
		if (mdbx_get(trnlck, m_dbGlobal, &key, &data) == MDBX_SUCCESS) 
		{
			const DBHeader *hdr = (const DBHeader*)data.iov_base;
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
			data.iov_base = &m_header; data.iov_len = sizeof(m_header);
			mdbx_put(trnlck, m_dbGlobal, &key, &data, 0);

			keyVal = 0;
			DBContact dbc = { 0, 0, 0 };
			data.iov_base = &dbc; data.iov_len = sizeof(dbc);
			mdbx_put(trnlck, m_dbContacts, &key, &data, 0);
		}
		trnlck.commit();

		{
			MDBX_val key, val;

			mdbx_txn_begin(m_pMdbEnv, nullptr, MDBX_RDONLY, &m_txn);

			mdbx_cursor_open(m_txn, m_dbEvents, &m_curEvents);
			if (mdbx_cursor_get(m_curEvents, &key, &val, MDBX_LAST) == MDBX_SUCCESS)
				m_dwMaxEventId = *(MEVENT*)key.iov_base;

			mdbx_cursor_open(m_txn, m_dbEventsSort, &m_curEventsSort);
			mdbx_cursor_open(m_txn, m_dbSettings, &m_curSettings);
			mdbx_cursor_open(m_txn, m_dbModules, &m_curModules);

			mdbx_cursor_open(m_txn, m_dbContacts, &m_curContacts);
			if (mdbx_cursor_get(m_curContacts, &key, &val, MDBX_LAST) == MDBX_SUCCESS)
				m_maxContactId = *(MCONTACT*)key.iov_base;

			MDBX_stat st;
			mdbx_dbi_stat(m_txn, m_dbContacts, &st, sizeof(st));
			m_contactCount = st.ms_entries;

			mdbx_txn_reset(m_txn);
		}


		if (InitModules()) return EGROKPRF_DAMAGED;
		if (InitCrypt())       return EGROKPRF_DAMAGED;

		// everything is ok, go on
		if (!m_bReadOnly) {
			// retrieve the event handles
			hContactDeletedEvent = CreateHookableEvent(ME_DB_CONTACT_DELETED);
			hContactAddedEvent = CreateHookableEvent(ME_DB_CONTACT_ADDED);
			hSettingChangeEvent = CreateHookableEvent(ME_DB_CONTACT_SETTINGCHANGED);
			hEventMarkedRead = CreateHookableEvent(ME_DB_EVENT_MARKED_READ);

			hEventAddedEvent = CreateHookableEvent(ME_DB_EVENT_ADDED);
			hEventDeletedEvent = CreateHookableEvent(ME_DB_EVENT_DELETED);	  
			hEventFilterAddedEvent = CreateHookableEvent(ME_DB_EVENT_FILTER_ADD);
		}

		FillContacts();
	}

	return EGROKPRF_NOERROR;
}

int CDbxMdb::Create(void)
{
	return (Map() == MDBX_SUCCESS) ? 0 : EGROKPRF_CANTREAD;
}

size_t iDefHeaderOffset = 0;
BYTE bDefHeader[] = { 0 };

int CDbxMdb::Check(void)
{
	FILE *pFile = _wfopen(m_tszProfileName, L"rb");
	if (pFile == nullptr)
		return EGROKPRF_CANTREAD;

	fseek(pFile, (LONG)iDefHeaderOffset, SEEK_SET);
	BYTE buf[_countof(bDefHeader)];
	size_t cbRead = fread(buf, 1, _countof(buf), pFile);
	fclose(pFile);
	if (cbRead != _countof(buf))
		return EGROKPRF_DAMAGED;

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
	unsigned int mode = MDBX_NOSUBDIR | MDBX_NOTLS | MDBX_MAPASYNC | MDBX_WRITEMAP | MDBX_NOSYNC;
	if (m_bReadOnly)
		mode |= MDBX_RDONLY;
	mdbx_env_open(m_pMdbEnv, _T2A(m_tszProfileName), mode, 0664);
	mdbx_env_set_mapsize(m_pMdbEnv, 0x1000000);
	return MDBX_SUCCESS;

}

bool CDbxMdb::Remap()
{
	MDBX_envinfo ei;
	mdbx_env_info(m_pMdbEnv, &ei, sizeof(ei));
	return mdbx_env_set_mapsize(m_pMdbEnv, ei.mi_mapsize + 0x100000) == MDBX_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////////////////

static DWORD DatabaseCorrupted = 0;
static const TCHAR *msg = NULL;
static DWORD dwErr = 0;
static wchar_t tszPanic[] = LPGENW("Miranda has detected corruption in your database. This corruption may be fixed by DbChecker plugin. Please download it from https://miranda-ng.org/p/DbChecker/. Miranda will now shut down.");

EXTERN_C void __cdecl dbpanic(void *)
{
	if (msg) {
		if (dwErr == ERROR_DISK_FULL)
			msg = TranslateT("Disk is full. Miranda will now shut down.");

		TCHAR err[256];
		mir_snwprintf(err, msg, TranslateT("Database failure. Miranda will now shut down."), dwErr);

		MessageBox(0, err, TranslateT("Database Error"), MB_SETFOREGROUND | MB_TOPMOST | MB_APPLMODAL | MB_ICONWARNING | MB_OK);
	}
	else MessageBox(0, TranslateW(tszPanic), TranslateT("Database Panic"), MB_SETFOREGROUND | MB_TOPMOST | MB_APPLMODAL | MB_ICONWARNING | MB_OK);
	TerminateProcess(GetCurrentProcess(), 255);
}


EXTERN_C void LMDBX_FailAssert(MDBX_env *env, const char *text)
{
	((CDbxMdb*)mdbx_env_get_userctx(env))->DatabaseCorruption(_A2T(text));
}

EXTERN_C void LMDBX_Log(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	Netlib_Log(0, CMStringA().FormatV(fmt, args));
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
