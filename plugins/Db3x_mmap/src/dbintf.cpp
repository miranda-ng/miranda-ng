/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012-14 Miranda NG project,
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

DBSignature dbSignatureU = { "Miranda NG DBu", 0x1A }; // unencrypted database
DBSignature dbSignatureE = { "Miranda NG DBe", 0x1A }; // encrypted database
DBSignature dbSignatureIM = { "Miranda ICQ DB", 0x1A };
DBSignature dbSignatureSA = { "Miranda ICQ SA", 0x1A };
DBSignature dbSignatureSD = { "Miranda ICQ SD", 0x1A };

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

CDb3Mmap::CDb3Mmap(const TCHAR *tszFileName, int iMode) :
	m_hDbFile(INVALID_HANDLE_VALUE),
	m_safetyMode(true),
	m_bReadOnly((iMode & DBMODE_READONLY) != 0),
	m_bShared((iMode & DBMODE_SHARED) != 0),
	m_dwMaxContactId(1),
	m_lMods(50, ModCompare),
	m_lOfs(50, OfsCompare),
	m_lResidentSettings(50, stringCompare2),
	m_contactsMap(50, NumericKeySortT)
{
	m_tszProfileName = mir_tstrdup(tszFileName);
	InitDbInstance(this);

	InitializeCriticalSection(&m_csDbAccess);

	SYSTEM_INFO sinf;
	GetSystemInfo(&sinf);
	m_ChunkSize = sinf.dwAllocationGranularity;

	m_codePage = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
	m_hModHeap = HeapCreate(0, 0, 0);
}

CDb3Mmap::~CDb3Mmap()
{
	// destroy modules
	HeapDestroy(m_hModHeap);

	// destroy map
	KillTimer(NULL, m_flushBuffersTimerId);
	if (m_pDbCache) {
		FlushViewOfFile(m_pDbCache, 0);
		UnmapViewOfFile(m_pDbCache);
	}

	DestroyServiceFunction(hService);
	UnhookEvent(hHook);

	if (m_crypto)
		m_crypto->destroy();

	if (m_hMap)
		CloseHandle(m_hMap);

	// update profile last modified time
	if (!m_bReadOnly) {
		DWORD bytesWritten;
		SetFilePointer(m_hDbFile, 0, NULL, FILE_BEGIN);
		WriteFile(m_hDbFile, &dbSignatureU, 1, &bytesWritten, NULL);
	}

	if (m_hDbFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hDbFile);

	DestroyHookableEvent(hContactDeletedEvent);
	DestroyHookableEvent(hContactAddedEvent);
	DestroyHookableEvent(hSettingChangeEvent);
	DestroyHookableEvent(hEventMarkedRead);

	DestroyHookableEvent(hEventAddedEvent);
	DestroyHookableEvent(hEventDeletedEvent);
	DestroyHookableEvent(hEventFilterAddedEvent);

	DeleteCriticalSection(&m_csDbAccess);

	DestroyDbInstance(this);
	mir_free(m_tszProfileName);

	free(m_pNull);
}

static TCHAR szMsgConvert[] = 
	LPGENT("Your database must be converted into the new format. This is potentially dangerous operation and might damage your profile, so please make a backup before.\n\nClick Yes to proceed with conversion or No to exit Miranda");

int CDb3Mmap::Load(bool bSkipInit)
{
	log0("DB logging running");

	DWORD dummy = 0, dwMode = FILE_SHARE_READ;
	if (m_bShared)
		dwMode |= FILE_SHARE_WRITE;
	if (m_bReadOnly)
		m_hDbFile = CreateFile(m_tszProfileName, GENERIC_READ, dwMode, NULL, OPEN_EXISTING, 0, NULL);
	else
		m_hDbFile = CreateFile(m_tszProfileName, GENERIC_READ | GENERIC_WRITE, dwMode, NULL, OPEN_ALWAYS, 0, NULL);

	if (m_hDbFile == INVALID_HANDLE_VALUE)
		return EGROKPRF_CANTREAD;

	if (!ReadFile(m_hDbFile, &m_dbHeader, sizeof(m_dbHeader), &dummy, NULL)) {
		CloseHandle(m_hDbFile);
		return EGROKPRF_CANTREAD;
	}

	if (!bSkipInit) {
		if (InitMap()) return 1;
		if (InitModuleNames()) return 1;
		if (InitCrypt()) return EGROKPRF_CANTREAD;

		// everything is ok, go on
		if (!m_bReadOnly) {
			if (m_dbHeader.version < DB_095_1_VERSION) {
				if (IDYES != MessageBox(NULL, TranslateTS(szMsgConvert), TranslateT("Database conversion required"), MB_YESNO | MB_ICONWARNING))
					return EGROKPRF_CANTREAD;

				if (m_dbHeader.version < DB_095_VERSION)
					ConvertContacts();
				ConvertEvents();

				m_dbHeader.version = DB_095_1_VERSION;
				DBWrite(sizeof(dbSignatureU), &m_dbHeader.version, sizeof(m_dbHeader.version));
			}

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

int CDb3Mmap::Create()
{
	m_hDbFile = CreateFile(m_tszProfileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
	return (m_hDbFile == INVALID_HANDLE_VALUE);
}

int CDb3Mmap::PrepareCheck()
{
	int ret = CheckDbHeaders(true);
	if (ret != ERROR_SUCCESS)
		return ret;

	InitMap();
	InitModuleNames();
	if ((ret = InitCrypt()) != ERROR_SUCCESS)
		return ret;

	return ERROR_SUCCESS;
}

STDMETHODIMP_(void) CDb3Mmap::SetCacheSafetyMode(BOOL bIsSet)
{
	{	mir_cslock lck(m_csDbAccess);
		m_safetyMode = bIsSet != 0;
	}
	DBFlush(1);
}

///////////////////////////////////////////////////////////////////////////////
// MIDatabaseChecker

typedef int (CDb3Mmap::*CheckWorker)(int);

static CheckWorker Workers[6] = 
{
	&CDb3Mmap::WorkInitialChecks,
	&CDb3Mmap::WorkModuleChain,
	&CDb3Mmap::WorkUser,
	&CDb3Mmap::WorkContactChain,
	&CDb3Mmap::WorkAggressive,
	&CDb3Mmap::WorkFinalTasks
};

int CDb3Mmap::Start(DBCHeckCallback *callback)
{
	cb = callback;
	ReMap(0);
	return ERROR_SUCCESS;
}

int CDb3Mmap::CheckDb(int phase, int firstTime)
{
	if (phase >= SIZEOF(Workers))
		return ERROR_OUT_OF_PAPER;

	return (this->*Workers[phase])(firstTime);
}

void CDb3Mmap::Destroy()
{
	delete this;
}
