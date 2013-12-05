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

DBSignature dbSignatureU = { "Miranda NG DBu", 0x1A }; // unencrypted database
DBSignature dbSignatureE = { "Miranda NG DBe", 0x1A }; // encrypted database
DBSignature dbSignatureIM = { "Miranda ICQ DB", 0x1A };
DBSignature dbSignatureSA = { "Miranda ICQ SA", 0x1A };

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

CDb3Base::CDb3Base(const TCHAR *tszFileName) :
	m_hDbFile(INVALID_HANDLE_VALUE),
	m_safetyMode(true),
	m_bReadOnly(true),
	m_lMods(50, ModCompare),
	m_lOfs(50, OfsCompare),
	m_lResidentSettings(50, stringCompare2)
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

CDb3Base::~CDb3Base()
{
	m_lResidentSettings.destroy();

	// destroy modules
	HeapDestroy(m_hModHeap);
	m_lMods.destroy();
	m_lOfs.destroy();

	// destroy map
	KillTimer(NULL, m_flushBuffersTimerId);
	if (m_pDbCache) {
		FlushViewOfFile(m_pDbCache, 0);
		UnmapViewOfFile(m_pDbCache);
	}

	if (m_crypto)
		m_crypto->destroy();

	if (m_hMap)
		CloseHandle(m_hMap);

	// update profile last modified time
	if (!m_bReadOnly) {
		DWORD bytesWritten;
		SetFilePointer(m_hDbFile, 0, NULL, FILE_BEGIN);
		WriteFile(m_hDbFile, &dbSignatureIM, 1, &bytesWritten, NULL);
	}
	CloseHandle(m_hDbFile);

	DestroyHookableEvent(hContactDeletedEvent);
	DestroyHookableEvent(hContactAddedEvent);
	DestroyHookableEvent(hSettingChangeEvent);

	DestroyHookableEvent(hEventAddedEvent);
	DestroyHookableEvent(hEventDeletedEvent);
	DestroyHookableEvent(hEventFilterAddedEvent);

	DeleteCriticalSection(&m_csDbAccess);

	DestroyDbInstance(this);
	mir_free(m_tszProfileName);
}

int CDb3Base::Load(bool bSkipInit)
{
	log0("DB logging running");
	
	DWORD dummy = 0;
	if (bSkipInit)
		m_hDbFile = CreateFile(m_tszProfileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	else
		m_hDbFile = CreateFile(m_tszProfileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
	if ( m_hDbFile == INVALID_HANDLE_VALUE )
		return EGROKPRF_CANTREAD;

	if (!ReadFile(m_hDbFile, &m_dbHeader, sizeof(m_dbHeader), &dummy, NULL)) {
		CloseHandle(m_hDbFile);
		return EGROKPRF_CANTREAD;
	}

	if (!bSkipInit) {
		if (InitCache())       return 1;
		if (InitModuleNames()) return 1;
		if (InitCrypt())       return 1;

		m_bReadOnly = false;

		hContactDeletedEvent = CreateHookableEvent(ME_DB_CONTACT_DELETED);
		hContactAddedEvent = CreateHookableEvent(ME_DB_CONTACT_ADDED);
		hSettingChangeEvent = CreateHookableEvent(ME_DB_CONTACT_SETTINGCHANGED);

		hEventAddedEvent = CreateHookableEvent(ME_DB_EVENT_ADDED);
		hEventDeletedEvent = CreateHookableEvent(ME_DB_EVENT_DELETED);
		hEventFilterAddedEvent = CreateHookableEvent(ME_DB_EVENT_FILTER_ADD);
	}
	return 0;
}

int CDb3Base::Create()
{
	m_hDbFile = CreateFile(m_tszProfileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
	return (m_hDbFile == INVALID_HANDLE_VALUE);
}

STDMETHODIMP_(void) CDb3Base::SetCacheSafetyMode(BOOL bIsSet)
{
	{	mir_cslock lck(m_csDbAccess);
		m_safetyMode = bIsSet != 0;
	}
	DBFlush(1);
}

///////////////////////////////////////////////////////////////////////////////
// MIDatabaseChecker

static CheckWorker Workers[6] = 
{
	&CDb3Base::WorkInitialChecks,
	&CDb3Base::WorkModuleChain,
	&CDb3Base::WorkUser,
	&CDb3Base::WorkContactChain,
	&CDb3Base::WorkAggressive,
	&CDb3Base::WorkFinalTasks
};

int CDb3Base::Start(DBCHeckCallback *callback)
{
	cb = callback;
	return ERROR_SUCCESS;
}

int CDb3Base::CheckDb(int phase, int firstTime)
{
	if (phase >= SIZEOF(Workers))
		return ERROR_OUT_OF_PAPER;

	return (this->*Workers[phase])(firstTime);
}

void CDb3Base::Destroy()
{
	delete this;
}
