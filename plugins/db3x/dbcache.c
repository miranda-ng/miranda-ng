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
#include "database.h"


#define CACHESECTIONSIZE   4096
#define CACHESECTIONCOUNT  32

extern HANDLE hDbFile;
extern CRITICAL_SECTION csDbAccess;

static BOOL safetyMode=TRUE;
static PBYTE pDbCache;
static DWORD lastUseCounter;
struct DBCacheSectionInfo {
	DWORD ofsBase;
	DWORD lastUsed;
} static cacheSectionInfo[CACHESECTIONCOUNT];

static __inline int FindSectionForOffset(const DWORD ofs)
{
	int i;
	for(i=0;i<CACHESECTIONCOUNT;i++)
		if(ofs>=cacheSectionInfo[i].ofsBase && ofs<cacheSectionInfo[i].ofsBase+CACHESECTIONSIZE)
			return i;
	return -1;
}

static __inline int FindLRUSection(void)
{
	int i,lru=0;
	DWORD lowestLastUse=cacheSectionInfo[0].lastUsed;
	for(i=1;i<CACHESECTIONCOUNT;i++) if(cacheSectionInfo[i].lastUsed<lowestLastUse) {lru=i; lowestLastUse=cacheSectionInfo[i].lastUsed;}
	return lru;
}

static __inline void LoadSection(const int i,DWORD ofs)
{
	cacheSectionInfo[i].ofsBase=ofs-ofs%CACHESECTIONSIZE;
	log1("readsect %08x",ofs);
	SetFilePointer(hDbFile,cacheSectionInfo[i].ofsBase,NULL,FILE_BEGIN);
	ReadFile(hDbFile,pDbCache+i*CACHESECTIONSIZE,CACHESECTIONSIZE,&ofs,NULL);
}

static __inline void MoveSection(int *sectId,int dest)
{
	CopyMemory(pDbCache+dest*CACHESECTIONSIZE,pDbCache+(*sectId)*CACHESECTIONSIZE,CACHESECTIONSIZE);
	cacheSectionInfo[dest].ofsBase=cacheSectionInfo[*sectId].ofsBase;
	*sectId=dest;
}



//we are assumed to be in a mutex here
PBYTE DBRead(DWORD ofs,int bytesRequired,int *bytesAvail)
{

	int part1sect;
	int part2sect;


	part1sect = FindSectionForOffset(ofs);
	if (ofs%CACHESECTIONSIZE+bytesRequired<CACHESECTIONSIZE) {
		//only one section required
		if(part1sect==-1) {
			part1sect=FindLRUSection();
			LoadSection(part1sect,ofs);
		}
		cacheSectionInfo[part1sect].lastUsed=++lastUseCounter;
		if(bytesAvail!=NULL) *bytesAvail=cacheSectionInfo[part1sect].ofsBase+CACHESECTIONSIZE-ofs;
		return pDbCache+part1sect*CACHESECTIONSIZE+(ofs-cacheSectionInfo[part1sect].ofsBase);
	}
	//two sections are required
	part2sect=FindSectionForOffset(ofs+CACHESECTIONSIZE);
	if(part1sect!=-1) {
		if(part2sect==-1) {  //first part in cache, but not second part
			if(part1sect==CACHESECTIONCOUNT-1) MoveSection(&part1sect,0);
			LoadSection(part1sect+1,ofs+CACHESECTIONSIZE);
		}
		else if(part2sect!=part1sect+1) {   //both parts are in cache, but not already consecutive
			if(part1sect==CACHESECTIONCOUNT-1) {
				//first part is at end, move to before second part
				if(part2sect==0) //second part is at start: need to move both
					MoveSection(&part2sect,1);
				MoveSection(&part1sect,part2sect-1);
			}
			else  //move second part to after first part
				MoveSection(&part2sect,part1sect+1);
		}
	}
	else {
		if(part2sect==-1) {  //neither section is in cache
			part1sect=0; part2sect=1;
			LoadSection(part1sect,ofs); LoadSection(part2sect,ofs+CACHESECTIONSIZE);
		}
		else {    //part 2 is in cache, but not part 1
			if(part2sect==0) MoveSection(&part2sect,1);
			part1sect=part2sect-1;
			LoadSection(part1sect,ofs);
		}
	}
	//both sections are now consecutive, starting at part1sect
	cacheSectionInfo[part1sect].lastUsed=++lastUseCounter;
	cacheSectionInfo[part1sect+1].lastUsed=++lastUseCounter;
	if(bytesAvail!=NULL) *bytesAvail=cacheSectionInfo[part1sect+1].ofsBase+CACHESECTIONSIZE-ofs;
	return pDbCache+part1sect*CACHESECTIONSIZE+(ofs-cacheSectionInfo[part1sect].ofsBase);
}



//we are assumed to be in a mutex here
void DBWrite(DWORD ofs,PVOID pData,int bytes)
{
	//write direct, and rely on Windows' write caching
	DWORD bytesWritten;
	int i;

	log2("write %d@%08x",bytes,ofs);
	SetFilePointer(hDbFile,ofs,NULL,FILE_BEGIN);
	if (WriteFile(hDbFile,pData,bytes,&bytesWritten,NULL)==0)
	{
		DatabaseCorruption();
	}
	logg();
	//check if any of the cache sections contain this bit
	for(i=0;i<CACHESECTIONCOUNT;i++) {
		if(ofs+bytes>=cacheSectionInfo[i].ofsBase && ofs<cacheSectionInfo[i].ofsBase+CACHESECTIONSIZE) {
			if(ofs<cacheSectionInfo[i].ofsBase) {  //don't start at beginning
				if(ofs+bytes>=cacheSectionInfo[i].ofsBase+CACHESECTIONSIZE)   //don't finish at end
					CopyMemory(pDbCache+i*CACHESECTIONSIZE,(PBYTE)pData+cacheSectionInfo[i].ofsBase-ofs,CACHESECTIONSIZE);
				else CopyMemory(pDbCache+i*CACHESECTIONSIZE,(PBYTE)pData+cacheSectionInfo[i].ofsBase-ofs,bytes-(cacheSectionInfo[i].ofsBase-ofs));
			}
			else {	  //start at beginning
				if(ofs+bytes>=cacheSectionInfo[i].ofsBase+CACHESECTIONSIZE)   //don't finish at end
					CopyMemory(pDbCache+i*CACHESECTIONSIZE+ofs-cacheSectionInfo[i].ofsBase,pData,cacheSectionInfo[i].ofsBase+CACHESECTIONSIZE-ofs);
				else CopyMemory(pDbCache+i*CACHESECTIONSIZE+ofs-cacheSectionInfo[i].ofsBase,pData,bytes);
			}
		}
	}
}

void DBMoveChunk(DWORD ofsDest,DWORD ofsSource,int bytes)
{
	DWORD bytesRead;
	PBYTE buf;

	log3("move %d %08x->%08x",bytes,ofsSource,ofsDest);
	buf=(PBYTE)mir_alloc(bytes);
	SetFilePointer(hDbFile,ofsSource,NULL,FILE_BEGIN);
	ReadFile(hDbFile,buf,bytes,&bytesRead,NULL);
	DBWrite(ofsDest,buf,bytes);
	mir_free(buf);
	logg();
}

static UINT_PTR flushBuffersTimerId;
static VOID CALLBACK DoBufferFlushTimerProc(HWND hwnd,UINT message,UINT_PTR idEvent,DWORD dwTime)
{
	KillTimer(NULL,flushBuffersTimerId);
	log0("tflush1");
	FlushFileBuffers(hDbFile);
	log0("tflush2");
}

void DBFlush(int setting)
{
	if(!setting) {
		log0("nflush1");
		if(safetyMode) FlushFileBuffers(hDbFile);
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
	if(safetyMode) FlushFileBuffers(hDbFile);
	return 0;
}

int InitCache(void)
{
	int i;
	DWORD bytesRead;
	
	CreateServiceFunction(MS_DB_SETSAFETYMODE,CacheSetSafetyMode);
	pDbCache=(PBYTE)mir_alloc(CACHESECTIONSIZE*CACHESECTIONCOUNT);
	lastUseCounter=CACHESECTIONCOUNT;
	for(i=0;i<CACHESECTIONCOUNT;i++) {
		cacheSectionInfo[i].ofsBase=0;
		cacheSectionInfo[i].lastUsed=i;
		SetFilePointer(hDbFile,cacheSectionInfo[i].ofsBase,NULL,FILE_BEGIN);
		ReadFile(hDbFile,pDbCache+i*CACHESECTIONSIZE,CACHESECTIONSIZE,&bytesRead,NULL);
	}
	return 0;
}

void UninitCache(void)
{
	mir_free(pDbCache);
	KillTimer(NULL,flushBuffersTimerId);
}
