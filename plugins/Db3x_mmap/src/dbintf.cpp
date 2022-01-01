/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
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

DBSignature dbSignatureU = { "Miranda NG DBu", 0x1A }; // unencrypted database
DBSignature dbSignatureE = { "Miranda NG DBe", 0x1A }; // encrypted database
DBSignature dbSignatureIM = { "Miranda ICQ DB", 0x1A };
DBSignature dbSignatureSA = { "Miranda ICQ SA", 0x1A };
DBSignature dbSignatureSD = { "Miranda ICQ SD", 0x1A };

static int ModCompare(const ModuleName *mn1, const ModuleName *mn2)
{
	return mir_strcmp(mn1->name, mn2->name);
}

static int OfsCompare(const ModuleName *mn1, const ModuleName *mn2)
{
	return (mn1->ofs - mn2->ofs);
}

CDb3Mmap::CDb3Mmap(const wchar_t *tszFileName, int iMode) :
	m_hDbFile(INVALID_HANDLE_VALUE),
	m_safetyMode(true),
	m_bReadOnly((iMode & DBMODE_READONLY) != 0),
	m_bShared((iMode & DBMODE_SHARED) != 0),
	m_dwMaxContactId(1),
	m_lMods(50, ModCompare),
	m_lOfs(50, OfsCompare)
{
	m_tszProfileName = mir_wstrdup(tszFileName);

	SYSTEM_INFO sinf;
	GetSystemInfo(&sinf);
	m_ChunkSize = sinf.dwAllocationGranularity;

	m_hModHeap = HeapCreate(0, 0, 0);
}

CDb3Mmap::~CDb3Mmap()
{
	g_Dbs.remove(this);

	// destroy modules
	HeapDestroy(m_hModHeap);

	// destroy map
	KillTimer(nullptr, m_flushBuffersTimerId);
	if (m_pDbCache) {
		FlushViewOfFile(m_pDbCache, 0);
		UnmapViewOfFile(m_pDbCache);
	}

	DestroyServiceFunction(hService);
	UnhookEvent(hHook);

	if (m_hMap)
		CloseHandle(m_hMap);

	// update profile last modified time
	if (!m_bReadOnly) {
		DWORD bytesWritten;
		SetFilePointer(m_hDbFile, 0, nullptr, FILE_BEGIN);
		WriteFile(m_hDbFile, &dbSignatureU, 1, &bytesWritten, nullptr);
	}

	if (m_hDbFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hDbFile);

	mir_free(m_tszProfileName);

	free(m_pNull);
}

int CDb3Mmap::Load(bool bSkipInit)
{
	log0("DB logging running");

	DWORD dummy = 0, dwMode = FILE_SHARE_READ;
	if (m_bShared)
		dwMode |= FILE_SHARE_WRITE;
	if (m_bReadOnly)
		m_hDbFile = CreateFile(m_tszProfileName, GENERIC_READ, dwMode, nullptr, OPEN_EXISTING, 0, nullptr);
	else
		m_hDbFile = CreateFile(m_tszProfileName, GENERIC_READ | GENERIC_WRITE, dwMode, nullptr, OPEN_ALWAYS, 0, nullptr);

	if (m_hDbFile == INVALID_HANDLE_VALUE)
		return EGROKPRF_CANTREAD;

	if (!ReadFile(m_hDbFile, &m_dbHeader, sizeof(m_dbHeader), &dummy, nullptr)) {
		CloseHandle(m_hDbFile);
		return EGROKPRF_CANTREAD;
	}

	if (!bSkipInit) {
		if (InitMap()) return 1;
		if (InitModuleNames()) return 1;
		if (InitCrypt()) return EGROKPRF_CANTREAD;

		// everything is ok, go on
		if (!m_bReadOnly)
			if (m_dbHeader.version < DB_095_1_VERSION)
				return EGROKPRF_CANTREAD;

		FillContacts();
	}

	return ERROR_SUCCESS;
}

int CDb3Mmap::Create()
{
	m_hDbFile = CreateFile(m_tszProfileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, 0, nullptr);
	return (m_hDbFile == INVALID_HANDLE_VALUE);
}

STDMETHODIMP_(void) CDb3Mmap::SetCacheSafetyMode(BOOL bIsSet)
{
	{
		mir_cslock lck(m_csDbAccess);
		m_safetyMode = bIsSet != 0;
	}
	DBFlush(1);
}
