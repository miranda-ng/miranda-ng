/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project,
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

BOOL safetyMode = TRUE;
static UINT_PTR flushBuffersTimerId;

static PBYTE pNull = 0;
static PBYTE pDbCache = NULL;
static HANDLE hMap = NULL;
static DWORD dwFileSize = 0;
static DWORD ChunkSize = 65536;
static DWORD flushFailTick = 0;


void Map()
{
	hMap = CreateFileMapping(hDbFile, NULL, PAGE_READWRITE, 0, dwFileSize, NULL);

	if (hMap)
	{
		pDbCache = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS/*FILE_MAP_WRITE*/, 0, 0 ,0);
		if (!pDbCache)
			DatabaseCorruption( _T("%s (MapViewOfFile failed. Code: %d)"));
	}
	else
		DatabaseCorruption( _T("%s (CreateFileMapping failed. Code: %d)"));
}

void ReMap(DWORD needed)
{
	KillTimer(NULL,flushBuffersTimerId);

	log3("remapping %d + %d (file end: %d)",dwFileSize,needed,dbHeader.ofsFileEnd);

	if (needed > ChunkSize)
	{
		if (needed + dwFileSize > dbHeader.ofsFileEnd + ChunkSize)
			DatabaseCorruption( _T("%s (Too large increment)"));
		else
		{
			DWORD x = dbHeader.ofsFileEnd/ChunkSize;
			dwFileSize = (x+1)*ChunkSize;
		}
	}
	else
		dwFileSize += ChunkSize;

//	FlushViewOfFile(pDbCache, 0);
	UnmapViewOfFile(pDbCache);
	pDbCache = NULL;
	CloseHandle(hMap);

	Map();
}

void DBMoveChunk(DWORD ofsDest,DWORD ofsSource,int bytes)
{
    int x = 0;
	log3("move %d %08x->%08x",bytes,ofsSource,ofsDest);
	if (ofsDest+bytes>dwFileSize) ReMap(ofsDest+bytes-dwFileSize);
	if (ofsSource+bytes>dwFileSize) {
		x = ofsSource+bytes-dwFileSize;
		log0("buggy move!");
		_ASSERT(0);
	}
	if (x > 0)
		ZeroMemory(pDbCache+ofsDest+bytes-x, x);
	if (ofsSource < dwFileSize)
		MoveMemory(pDbCache+ofsDest,pDbCache+ofsSource, bytes-x);

	logg();
}

//we are assumed to be in a mutex here
PBYTE DBRead(DWORD ofs,int bytesRequired,int *bytesAvail)
{
	// buggy read
	if (ofs>=dwFileSize) {
		log2("read from outside %d@%08x",bytesRequired,ofs);
		if (bytesAvail!=NULL) *bytesAvail = ChunkSize;
		return pNull;
	}
	log3((ofs+bytesRequired>dwFileSize)?"read %d@%08x, only %d avaliable":"read %d@%08x",bytesRequired,ofs,dwFileSize-ofs);
	if (bytesAvail!=NULL) *bytesAvail = dwFileSize - ofs;
	return pDbCache+ofs;
}

//we are assumed to be in a mutex here
void DBWrite(DWORD ofs,PVOID pData,int bytes)
{
	log2("write %d@%08x",bytes,ofs);
	if (ofs+bytes>dwFileSize) ReMap(ofs+bytes-dwFileSize);
	MoveMemory(pDbCache+ofs,pData,bytes);
	logg();
}

//we are assumed to be in a mutex here
void DBFill(DWORD ofs,int bytes)
{
	log2("zerofill %d@%08x",bytes,ofs);
	if (ofs+bytes<=dwFileSize)
		ZeroMemory(pDbCache+ofs,bytes);
	logg();
}

static VOID CALLBACK DoBufferFlushTimerProc(HWND hwnd, UINT message, UINT_PTR idEvent, DWORD dwTime)
{
    if (!pDbCache) return;

	KillTimer(NULL,flushBuffersTimerId);
	log0("tflush1");
	if (FlushViewOfFile(pDbCache, 0) == 0) {
		if (flushFailTick == 0)
			flushFailTick = GetTickCount();
		else if (GetTickCount() - flushFailTick > 5000) 
			DatabaseCorruption(NULL);
	}
	else
		flushFailTick = 0;
	log0("tflush2");
}

void DBFlush(int setting)
{
	if(!setting) {
		log0("nflush1");
		if(safetyMode && pDbCache) {
			if (FlushViewOfFile(pDbCache, 0) == 0) {
				if (flushFailTick == 0)
					flushFailTick = GetTickCount();
				else if (GetTickCount() - flushFailTick > 5000) 
					DatabaseCorruption(NULL);
			}
			else
				flushFailTick = 0;
		}
		log0("nflush2");
		return;
	}
	KillTimer(NULL,flushBuffersTimerId);
	flushBuffersTimerId=SetTimer(NULL,flushBuffersTimerId,50,DoBufferFlushTimerProc);
}

static INT_PTR CacheSetSafetyMode(WPARAM wParam,LPARAM lParam)
{
	EnterCriticalSection(&csDbAccess);
	safetyMode=wParam;
	LeaveCriticalSection(&csDbAccess);
	DBFlush(1);
	return 0;
}

int InitCache(void)
{
	DWORD x;
	SYSTEM_INFO sinf;

	GetSystemInfo(&sinf);
	ChunkSize = sinf.dwAllocationGranularity;

	dwFileSize = GetFileSize(hDbFile,  NULL);

	// Align to chunk
	x = dwFileSize % ChunkSize;
	if (x) dwFileSize += ChunkSize - x;

	Map();

	// zero region for reads outside the file
	pNull = calloc(ChunkSize,1);

	CreateServiceFunction(MS_DB_SETSAFETYMODE,CacheSetSafetyMode);

	return 0;
}

void UninitCache(void)
{
	KillTimer(NULL,flushBuffersTimerId);
	FlushViewOfFile(pDbCache, 0);
	UnmapViewOfFile(pDbCache);
	CloseHandle(hMap);
	if (pNull) free(pNull);
}
