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

CDbxMdb::CDbxMdb(const TCHAR *tszFileName, int iMode) :
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

	mdb_env_create(&m_pMdbEnv);
	mdb_env_set_maxdbs(m_pMdbEnv, 10);

	m_codePage = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
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
	int mode = MDB_NOSYNC | MDB_NOSUBDIR;
	if (m_bReadOnly)
		mode += MDB_RDONLY;
	else
		mode += MDB_WRITEMAP;

	if (mdb_env_open(m_pMdbEnv, _T2A(m_tszProfileName), mode, 0664) != 0)
		return EGROKPRF_CANTREAD;

	if (!bSkipInit) {
		mdb_txn_begin(m_pMdbEnv, NULL, 0, &m_txn);

		mdb_open(m_txn, "modules",  MDB_CREATE | MDB_INTEGERKEY, &m_dbModules);
		mdb_open(m_txn, "contacts", MDB_CREATE | MDB_INTEGERKEY, &m_dbContacts);
		mdb_open(m_txn, "events",   MDB_CREATE | MDB_INTEGERKEY, &m_dbEvents);

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

int CDbxMdb::Create(void)
{
	int mode = MDB_NOSYNC | MDB_NOSUBDIR;
	if (m_bReadOnly)
		mode += MDB_RDONLY;
	else
		mode += MDB_WRITEMAP;

	if (mdb_env_open(m_pMdbEnv, _T2A(m_tszProfileName), mode, 0664) != 0)
		return EGROKPRF_CANTREAD;

	return 0;
}

int CDbxMdb::Check(void)
{
	HANDLE hFile = CreateFile(m_tszProfileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return EGROKPRF_CANTREAD;

	DWORD dummy = 0;
	char buf[32];
	if (!ReadFile(hFile, buf, sizeof(buf), &dummy, NULL)) {
		CloseHandle(hFile);
		return EGROKPRF_CANTREAD;
	}

	CloseHandle(hFile);
	return (memcmp(buf + 16, "\xDE\xC0\xEF\xBE", 4)) ? EGROKPRF_UNKHEADER : 0;
}

int CDbxMdb::PrepareCheck(int *error)
{
	InitModuleNames();
	return InitCrypt();
}

STDMETHODIMP_(void) CDbxMdb::SetCacheSafetyMode(BOOL bIsSet)
{
	mir_cslock lck(m_csDbAccess);
	m_safetyMode = bIsSet != 0;
}

///////////////////////////////////////////////////////////////////////////////
// MIDatabaseChecker

typedef int (CDbxMdb::*CheckWorker)(int);

int CDbxMdb::Start(DBCHeckCallback *callback)
{
	cb = callback;
	return ERROR_SUCCESS;
}

int CDbxMdb::CheckDb(int phase, int firstTime)
{
	return ERROR_OUT_OF_PAPER;

	// return (this->*Workers[phase])(firstTime);
}

void CDbxMdb::Destroy()
{
	delete this;
}
