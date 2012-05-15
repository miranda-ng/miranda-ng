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

int ProfileManager(char *szDbDest,int cbDbDest);
int ShouldAutoCreate(void);
int CreateDbHeaders(HANDLE hFile);
int InitialiseDbHeaders(void);
int InitSettings(void);
void UninitSettings(void);
int InitContacts(void);
void UninitContacts(void);
int InitEvents(void);
void UninitEvents(void);
int InitCrypt(void);
int InitModuleNames(void);
void UninitModuleNames(void);
int InitCache(void);
void UninitCache(void);
int InitIni(void);
void UninitIni(void);

HANDLE hDbFile=INVALID_HANDLE_VALUE;
CRITICAL_SECTION csDbAccess;
struct DBHeader dbHeader;
char szDbPath[MAX_PATH];

static void UnloadDatabase(void)
{
	// update profile last modified time
	DWORD bytesWritten;
	SetFilePointer(hDbFile,0,NULL,FILE_BEGIN);
	WriteFile(hDbFile,&dbSignature,1,&bytesWritten,NULL);

	CloseHandle(hDbFile);
}

DWORD CreateNewSpace(int bytes)
{
	DWORD ofsNew;
	ofsNew=dbHeader.ofsFileEnd;
	dbHeader.ofsFileEnd+=bytes;
	DBWrite(0,&dbHeader,sizeof(dbHeader));
	log2("newspace %d@%08x",bytes,ofsNew);
	return ofsNew;
}

void DeleteSpace(DWORD ofs,int bytes)
{
	if (ofs+bytes == dbHeader.ofsFileEnd)	{
		log2("freespace %d@%08x",bytes,ofs);
		dbHeader.ofsFileEnd=ofs;
	} else	{
		log2("deletespace %d@%08x",bytes,ofs);
		dbHeader.slackSpace+=bytes;
	}
	DBWrite(0,&dbHeader,sizeof(dbHeader));
	DBFill(ofs,bytes);
}

DWORD ReallocSpace(DWORD ofs,int oldSize,int newSize)
{
	DWORD ofsNew;

	if (oldSize >= newSize) return ofs;

	if (ofs+oldSize == dbHeader.ofsFileEnd) {
		ofsNew = ofs;
		dbHeader.ofsFileEnd+=newSize-oldSize;
		DBWrite(0,&dbHeader,sizeof(dbHeader));
		log3("adding newspace %d@%08x+%d",newSize,ofsNew,oldSize);
	} else {
		ofsNew=CreateNewSpace(newSize);
		DBMoveChunk(ofsNew,ofs,oldSize);
		DeleteSpace(ofs,oldSize);
	}
	return ofsNew;
}

void UnloadDatabaseModule(void)
{
	//UninitIni();
	UninitEvents();
	UninitSettings();
	UninitContacts();
	UninitModuleNames();
	UninitCache();
	UnloadDatabase();
	DeleteCriticalSection(&csDbAccess);
}

int LoadDatabaseModule(void)
{
	InitializeCriticalSection(&csDbAccess);
	log0("DB logging running");
	{
		DWORD dummy=0;
		hDbFile=CreateFileA(szDbPath,GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
		if ( hDbFile == INVALID_HANDLE_VALUE ) {
			return 1;
		}
		if ( !ReadFile(hDbFile,&dbHeader,sizeof(dbHeader),&dummy,NULL) ) {
			CloseHandle(hDbFile);
			return 1;
		}
	}
	//if(ParseCommandLine()) return 1;
	if(InitCache()) return 1;
	if(InitModuleNames()) return 1;
	if(InitContacts()) return 1;
	if(InitSettings()) return 1;
	if(InitEvents()) return 1;
	if(InitCrypt()) return 1;
	return 0;
}

static DWORD DatabaseCorrupted=0;
static TCHAR *msg = NULL;
static DWORD dwErr = 0;

void __cdecl dbpanic(void *arg)
{
	if (msg)
	{
		TCHAR err[256];

		if (dwErr==ERROR_DISK_FULL)
			msg = TranslateT("Disk is full. Miranda will now shutdown.");

		mir_sntprintf(err, SIZEOF(err), msg, TranslateT("Database failure. Miranda will now shutdown."), dwErr);

		MessageBox(0,err,TranslateT("Database Error"),MB_SETFOREGROUND|MB_TOPMOST|MB_APPLMODAL|MB_ICONWARNING|MB_OK);
	}
	else
		MessageBox(0,TranslateT("Miranda has detected corruption in your database. This corruption maybe fixed by DBTool.  Please download it from http://www.miranda-im.org. Miranda will now shutdown."),
					TranslateT("Database Panic"),MB_SETFOREGROUND|MB_TOPMOST|MB_APPLMODAL|MB_ICONWARNING|MB_OK);
	TerminateProcess(GetCurrentProcess(),255);
}

void DatabaseCorruption(TCHAR *text)
{
	int kill=0;

	EnterCriticalSection(&csDbAccess);
	if (DatabaseCorrupted==0) {
		DatabaseCorrupted++;
		kill++;
		msg = text;
		dwErr = GetLastError();
	} else {
		/* db is already corrupted, someone else is dealing with it, wait here
		so that we don't do any more damage */
		LeaveCriticalSection(&csDbAccess);
		Sleep(INFINITE);
		return;
	}
	LeaveCriticalSection(&csDbAccess);
	if (kill) {
		_beginthread(dbpanic,0,NULL);
		Sleep(INFINITE);
	}
}

#ifdef DBLOGGING
void DBLog(const char *file,int line,const char *fmt,...)
{
	FILE *fp;
	va_list vararg;
	char str[1024];

	va_start(vararg,fmt);
	mir_vsnprintf(str,sizeof(str),fmt,vararg);
	va_end(vararg);
	fp=fopen("c:\\mirandadatabase.log.txt","at");
	fprintf(fp,"%u: %s %d: %s\n",GetTickCount(),file,line,str);
	fclose(fp);
}
#endif
