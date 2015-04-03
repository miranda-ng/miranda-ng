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

struct SettingsComparator : public Comparator
{
	SettingsComparator() {}
	virtual int32_t compare(const char *akbuf, size_t, const char *bkbuf, size_t)
	{
		DBSettingSortingKey *k1 = (DBSettingSortingKey*)akbuf, *k2 = (DBSettingSortingKey*)bkbuf;
		if (k1->dwContactID < k2->dwContactID) return -1;
		if (k1->dwContactID > k2->dwContactID) return 1;

		if (k1->dwOfsModule < k2->dwOfsModule) return -1;
		if (k1->dwOfsModule > k2->dwOfsModule) return 1;

		return strcmp(k1->szSettingName, k2->szSettingName);
	}
}
static _settingsComparator;

struct EventsComparator : public Comparator
{
	EventsComparator() {}
	virtual int32_t compare(const char *akbuf, size_t, const char *bkbuf, size_t)
	{
		DBEventSortingKey *k1 = (DBEventSortingKey*)akbuf, *k2 = (DBEventSortingKey*)bkbuf;
		if (k1->dwContactId < k2->dwContactId) return -1;
		if (k1->dwContactId > k2->dwContactId) return 1;

		if (k1->ts < k2->ts) return -1;
		if (k1->ts > k2->ts) return 1;

		if (k1->dwEventId < k2->dwEventId) return -1;
		if (k1->dwEventId > k2->dwEventId) return 1;

		return 0;
	}
}
static _eventsComparator;

struct LongComparator : public Comparator
{
	LongComparator() {}
	virtual int32_t compare(const char *akbuf, size_t, const char *bkbuf, size_t)
	{
		return *(LONG*)akbuf - *(LONG*)bkbuf;
	}
}
static _longComparator;

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
	if (!bSkipInit) {
		int iFlags = TreeDB::OREADER | TreeDB::ONOREPAIR;
		if (!m_bReadOnly)
			iFlags |= TreeDB::OWRITER;

		std::string szFilename((char*)_T2A(m_tszProfileName));
		m_dbGlobal.tune_map(16384);
		if (!m_dbGlobal.open(szFilename, iFlags))
			return EGROKPRF_DAMAGED;

		m_dbContacts.tune_map(256 * 1024);
		if (!m_dbContacts.open(szFilename + ".cnt", iFlags))
			return EGROKPRF_DAMAGED;

		m_dbModules.tune_comparator(&_longComparator);
		if (!m_dbModules.open(szFilename + ".mod", iFlags))
			return EGROKPRF_DAMAGED;

		m_dbEvents.tune_comparator(&_longComparator);
		if (!m_dbEvents.open(szFilename + ".evt", iFlags))
			return EGROKPRF_DAMAGED;

		m_dbEventsSort.tune_comparator(&_eventsComparator);
		if (!m_dbEventsSort.open(szFilename + ".evs", iFlags))
			return EGROKPRF_DAMAGED;
		
		m_dbSettings.tune_comparator(&_settingsComparator);
		if (!m_dbSettings.open(szFilename + ".set", iFlags))
			return EGROKPRF_DAMAGED;

		DWORD keyVal = 1;
		if (-1 != m_dbGlobal.get((LPCSTR)&keyVal, sizeof(keyVal), (LPSTR)&m_header, sizeof(m_header))) {
			if (m_header.dwSignature != DBHEADER_SIGNATURE)
				DatabaseCorruption(NULL);
		}
		else {
			m_header.dwSignature = DBHEADER_SIGNATURE;
			m_header.dwVersion = 1;
			m_dbGlobal.set((LPCSTR)&keyVal, sizeof(keyVal), (LPCSTR)&m_header, sizeof(m_header));

			keyVal = 0;
			DBContact dbc = { DBCONTACT_SIGNATURE, 0, 0, 0 };
			m_dbContacts.set((LPCSTR)&keyVal, sizeof(keyVal), (LPCSTR)&dbc, sizeof(dbc));
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
	int iFlags = TreeDB::OREADER | TreeDB::OCREATE;
	if (!m_bReadOnly)
		iFlags |= TreeDB::OWRITER;

	std::string szFilename((char*)_T2A(m_tszProfileName));
	if (!m_dbGlobal.open(szFilename, iFlags)) return EGROKPRF_DAMAGED;
	if (!m_dbContacts.open(szFilename + ".cnt", iFlags)) return EGROKPRF_DAMAGED;
	if (!m_dbModules.open(szFilename + ".mod", iFlags)) return EGROKPRF_DAMAGED;
	if (!m_dbEvents.open(szFilename + ".evt", iFlags)) return EGROKPRF_DAMAGED;
	if (!m_dbEventsSort.open(szFilename + ".evs", iFlags)) return EGROKPRF_DAMAGED;
	if (!m_dbSettings.open(szFilename + ".set", iFlags)) return EGROKPRF_DAMAGED;
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
	return (memcmp(buf, "\x4B\x43\x0A\x00", 4)) ? EGROKPRF_UNKHEADER : 0;
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
