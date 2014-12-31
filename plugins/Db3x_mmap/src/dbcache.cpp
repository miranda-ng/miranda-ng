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

void CDb3Mmap::Map()
{
	DWORD dwProtectMode, dwAccess;
	if (cb && cb->bAggressive)
		dwProtectMode = PAGE_WRITECOPY, dwAccess = FILE_MAP_COPY;
	else if (m_bReadOnly)
		dwProtectMode = PAGE_READONLY, dwAccess = FILE_MAP_READ;
	else
		dwProtectMode = PAGE_READWRITE, dwAccess = FILE_MAP_ALL_ACCESS;

	m_hMap = CreateFileMapping(m_hDbFile, NULL, dwProtectMode, 0, m_dwFileSize, NULL);
	if (m_hMap) {
		m_pDbCache = (PBYTE)MapViewOfFile(m_hMap, dwAccess, 0, 0, 0);
		if (!m_pDbCache)
			DatabaseCorruption(_T("%s (MapViewOfFile failed. Code: %d)"));
	}
	else DatabaseCorruption(_T("%s (CreateFileMapping failed. Code: %d)"));
}

void CDb3Mmap::ReMap(DWORD needed)
{
	KillTimer(NULL, m_flushBuffersTimerId);

	log3("remapping %d + %d (file end: %d)", m_dwFileSize, needed, m_dbHeader.ofsFileEnd);

	if (needed > 0) {
		if (needed > m_ChunkSize) {
			if (needed + m_dwFileSize > m_dbHeader.ofsFileEnd + m_ChunkSize)
				DatabaseCorruption(_T("%s (Too large increment)"));
			else {
				DWORD x = m_dbHeader.ofsFileEnd / m_ChunkSize;
				m_dwFileSize = (x + 1)*m_ChunkSize;
			}
		}
		else m_dwFileSize += m_ChunkSize;
	}

	UnmapViewOfFile(m_pDbCache);
	m_pDbCache = NULL;
	CloseHandle(m_hMap);

	Map();
}

void CDb3Mmap::DBMoveChunk(DWORD ofsDest, DWORD ofsSource, int bytes)
{
	int x = 0;
	//log3("move %d %08x->%08x",bytes,ofsSource,ofsDest);
	if (ofsDest + bytes > m_dwFileSize)
		ReMap(ofsDest + bytes - m_dwFileSize);

	if (ofsSource + bytes > m_dwFileSize) {
		x = ofsSource + bytes - m_dwFileSize;
		log0("buggy move!");
	}
	if (x > 0)
		memset((m_pDbCache + ofsDest + bytes - x), 0, x);
	if (ofsSource < m_dwFileSize)
		memmove(m_pDbCache + ofsDest, m_pDbCache + ofsSource, bytes - x);

	logg();
}

//we are assumed to be in a mutex here
PBYTE CDb3Mmap::DBRead(DWORD ofs, int *bytesAvail)
{
	// buggy read
	if (ofs >= m_dwFileSize) {
		//log2("read from outside %d@%08x",bytesRequired,ofs);
		if (bytesAvail != NULL)
			*bytesAvail = m_ChunkSize;
		return m_pNull;
	}
	//log3((ofs+bytesRequired > m_dwFileSize)?"read %d@%08x, only %d avaliable":"read %d@%08x",bytesRequired,ofs,m_dwFileSize-ofs);
	if (bytesAvail != NULL)
		*bytesAvail = m_dwFileSize - ofs;
	return m_pDbCache + ofs;
}

//we are assumed to be in a mutex here
void CDb3Mmap::DBWrite(DWORD ofs, PVOID pData, int bytes)
{
	//log2("write %d@%08x",bytes,ofs);
	if (ofs + bytes > m_dwFileSize)
		ReMap(ofs + bytes - m_dwFileSize);
	memmove(m_pDbCache + ofs, pData, bytes);
	logg();
}

//we are assumed to be in a mutex here
void CDb3Mmap::DBFill(DWORD ofs, int bytes)
{
	//log2("zerofill %d@%08x",bytes,ofs);
	if ((ofs + bytes) <= m_dwFileSize)
		memset((m_pDbCache + ofs), 0, bytes);
	logg();
}

static VOID CALLBACK DoBufferFlushTimerProc(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	for (int i = 0; i < g_Dbs.getCount(); i++) {
		CDb3Mmap *db = g_Dbs[i];
		if (db->m_flushBuffersTimerId != idEvent)
			continue;

		if (!db->m_pDbCache)
			return;

		KillTimer(NULL, db->m_flushBuffersTimerId);
		log0("tflush1");
		if (FlushViewOfFile(db->m_pDbCache, 0) == 0) {
			if (db->m_flushFailTick == 0)
				db->m_flushFailTick = GetTickCount();
			else if (GetTickCount() - db->m_flushFailTick > 5000)
				db->DatabaseCorruption(NULL);
		}
		else db->m_flushFailTick = 0;
		log0("tflush2");
	}
}

void CDb3Mmap::DBFlush(int setting)
{
	if (!setting) {
		log0("nflush1");
		if (m_safetyMode && m_pDbCache) {
			if (FlushViewOfFile(m_pDbCache, 0) == 0) {
				if (m_flushFailTick == 0)
					m_flushFailTick = GetTickCount();
				else if (GetTickCount() - m_flushFailTick > 5000)
					DatabaseCorruption(NULL);
			}
			else m_flushFailTick = 0;
		}
		log0("nflush2");
		return;
	}
	KillTimer(NULL, m_flushBuffersTimerId);
	m_flushBuffersTimerId = SetTimer(NULL, m_flushBuffersTimerId, 50, DoBufferFlushTimerProc);
}

int CDb3Mmap::InitMap(void)
{
	m_dwFileSize = GetFileSize(m_hDbFile, NULL);

	// Align to chunk
	if (!m_bReadOnly) {
		DWORD x = m_dwFileSize % m_ChunkSize;
		if (x)
			m_dwFileSize += m_ChunkSize - x;
	}

	Map();

	// zero region for reads outside the file
	m_pNull = (PBYTE)calloc(m_ChunkSize, 1);
	return 0;
}

DWORD CDb3Mmap::GetSettingsGroupOfsByModuleNameOfs(DBContact *dbc, DWORD ofsModuleName)
{
	DWORD ofsThis = dbc->ofsFirstSettings;
	while (ofsThis) {
		DBContactSettings *dbcs = (DBContactSettings*)DBRead(ofsThis, NULL);
		if (dbcs->signature != DBCONTACTSETTINGS_SIGNATURE) DatabaseCorruption(NULL);
		if (dbcs->ofsModuleName == ofsModuleName)
			return ofsThis;

		ofsThis = dbcs->ofsNext;
	}
	return 0;
}
