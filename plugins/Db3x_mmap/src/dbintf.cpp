/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012 Miranda NG project,
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

DBSignature dbSignature = {"Miranda ICQ DB",0x1A};

static int stringCompare(const char* p1, const char* p2)
{
	return strcmp(p1+1, p2+1);
}

static int stringCompare2(const char* p1, const char* p2)
{
	return strcmp(p1, p2);
}

static int compareGlobals(const DBCachedGlobalValue* p1, const DBCachedGlobalValue* p2)
{
	return strcmp(p1->name, p2->name);
}

static int ModCompare(const ModuleName *mn1, const ModuleName *mn2 )
{
	return strcmp( mn1->name, mn2->name );
}

static int OfsCompare(const ModuleName *mn1, const ModuleName *mn2 )
{
	return ( mn1->ofs - mn2->ofs );
}

CDb3Base::CDb3Base(const TCHAR* tszFileName) :
	m_hDbFile(INVALID_HANDLE_VALUE),
	m_safetyMode(TRUE),
	m_lSettings(100, stringCompare),
	m_lContacts(50, LIST<DBCachedContactValueList>::FTSortFunc(HandleKeySort)),
	m_lGlobalSettings(50, compareGlobals),
	m_lResidentSettings(50, stringCompare2),
	m_lMods(50, ModCompare),
	m_lOfs(50, OfsCompare)
{
	m_tszProfileName = mir_tstrdup(tszFileName);

	InitializeCriticalSection(&m_csDbAccess);

	SYSTEM_INFO sinf;
	GetSystemInfo(&sinf);
	m_ChunkSize = sinf.dwAllocationGranularity;

	m_codePage = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
	m_hCacheHeap = HeapCreate(0, 0, 0);
	m_hModHeap = HeapCreate(0,0,0);
}

CDb3Base::~CDb3Base()
{
	// destroy settings
	HeapDestroy(m_hCacheHeap);
	m_lContacts.destroy();
	m_lSettings.destroy();
	m_lGlobalSettings.destroy();
	m_lResidentSettings.destroy();

	// destroy modules
	HeapDestroy(m_hModHeap);
	m_lMods.destroy();
	m_lOfs.destroy();

	// destroy map
	KillTimer(NULL,m_flushBuffersTimerId);
	if (m_pDbCache) {
		FlushViewOfFile(m_pDbCache, 0);
		UnmapViewOfFile(m_pDbCache);
	}

	// update profile last modified time
	DWORD bytesWritten;
	SetFilePointer(m_hDbFile, 0, NULL, FILE_BEGIN);
	WriteFile(m_hDbFile, &dbSignature, 1, &bytesWritten, NULL);
	CloseHandle(m_hDbFile);

	DeleteCriticalSection(&m_csDbAccess);

	mir_free(m_tszProfileName);
}

int CDb3Base::Load(bool bSkipInit)
{
	log0("DB logging running");
	
	DWORD dummy = 0;
	m_hDbFile = CreateFile(m_tszProfileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
	if ( m_hDbFile == INVALID_HANDLE_VALUE )
		return 1;

	if ( !ReadFile(m_hDbFile,&m_dbHeader,sizeof(m_dbHeader),&dummy,NULL)) {
		CloseHandle(m_hDbFile);
		return 1;
	}

	if ( !bSkipInit) {
		if (InitCache()) return 1;
		if (InitModuleNames()) return 1;

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
	return ( m_hDbFile == INVALID_HANDLE_VALUE );
}

STDMETHODIMP_(void) CDb3Base::SetCacheSafetyMode(BOOL bIsSet)
{
	{	mir_cslock lck(m_csDbAccess);
		m_safetyMode = bIsSet;
	}
	DBFlush(1);
}

void CDb3Base::EncodeCopyMemory(void *dst, void *src, size_t size)
{
	MoveMemory(dst, src, size);
}

void CDb3Base::DecodeCopyMemory(void *dst, void *src, size_t size)
{
	MoveMemory(dst, src, size);
}

void CDb3Base::EncodeDBWrite(DWORD ofs, void *src, int size)
{
	DBWrite(ofs, src, size);
}

void CDb3Base::DecodeDBWrite(DWORD ofs, void *src, int size)
{
	DBWrite(ofs, src, size);
}

