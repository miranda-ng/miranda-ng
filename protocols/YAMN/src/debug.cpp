/*
 * YAMN plugin main file
 * Miranda homepage: http://miranda-icq.sourceforge.net/
 *
 * Debug functions used in DEBUG release (you need to global #define DEBUG to get debug version)
 *
 * (c) majvan 2002-2004
 */

#include "stdafx.h"

#ifdef _DEBUG

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

wchar_t DebugUserDirectory[MAX_PATH] = L".";
CRITICAL_SECTION FileAccessCS;

#ifdef DEBUG_SYNCHRO
wchar_t DebugSynchroFileName2[]=L"%s\\yamn-debug.synchro.log";
HANDLE SynchroFile;
#endif

#ifdef DEBUG_COMM
wchar_t DebugCommFileName2[]=L"%s\\yamn-debug.comm.log";
HANDLE CommFile;
#endif

#ifdef DEBUG_DECODE
wchar_t DebugDecodeFileName2[]=L"%s\\yamn-debug.decode.log";
HANDLE DecodeFile;
#endif

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void InitDebug()
{
#if defined (DEBUG_SYNCHRO) || defined (DEBUG_COMM) || defined (DEBUG_DECODE)
	wchar_t DebugFileName[MAX_PATH];
#endif
	InitializeCriticalSection(&FileAccessCS);

#ifdef DEBUG_SYNCHRO
	mir_snwprintf(DebugFileName, DebugSynchroFileName2, DebugUserDirectory);
	
	SynchroFile=CreateFile(DebugFileName,GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
	DebugLog(SynchroFile,"Synchro debug file created by %s\n",YAMN_VER);
#endif

#ifdef DEBUG_COMM
	mir_snwprintf(DebugFileName, DebugCommFileName2, DebugUserDirectory);

	CommFile=CreateFile(DebugFileName,GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
	DebugLog(CommFile,"Communication debug file created by %s\n",YAMN_VER);
#endif

#ifdef DEBUG_DECODE
	mir_snwprintf(DebugFileName, DebugDecodeFileName2, DebugUserDirectory);

	DecodeFile=CreateFile(DebugFileName,GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
	DebugLog(DecodeFile,"Decoding kernel debug file created by %s\n",YAMN_VER);
#endif
}

void UnInitDebug()
{
	DeleteCriticalSection(&FileAccessCS);
#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"File is being closed normally.");
	CloseHandle(SynchroFile);
#endif
#ifdef DEBUG_COMM
	DebugLog(CommFile,"File is being closed normally.");
	CloseHandle(CommFile);
#endif
#ifdef DEBUG_DECODE
	DebugLog(DecodeFile,"File is being closed normally.");
	CloseHandle(DecodeFile);
#endif
}


void DebugLog(HANDLE File,const char *fmt,...)
{
	char *str;
	char tids[32];
	va_list vararg;
	int strsize;
	DWORD Written;

	va_start(vararg,fmt);
	str=(char *)malloc(strsize=65536);
	mir_snprintf(tids, "[%x]",GetCurrentThreadId());
	while(mir_vsnprintf(str, strsize, fmt, vararg)==-1)
		str=(char *)realloc(str,strsize+=65536);
	va_end(vararg);
	EnterCriticalSection(&FileAccessCS);
	WriteFile(File,tids,(uint32_t)mir_strlen(tids),&Written,nullptr);
	WriteFile(File,str,(uint32_t)mir_strlen(str),&Written,nullptr);
	LeaveCriticalSection(&FileAccessCS);
	free(str);
}

void DebugLogW(HANDLE File,const wchar_t *fmt,...)
{
	wchar_t *str;
	char tids[32];
	va_list vararg;
	int strsize;
	DWORD Written;

	va_start(vararg,fmt);
	str=(wchar_t *)malloc((strsize=65536)*sizeof(wchar_t));
	mir_snprintf(tids, "[%x]",GetCurrentThreadId());
	while(mir_vsnwprintf(str, strsize, fmt, vararg)==-1)
		str=(wchar_t *)realloc(str,(strsize+=65536)*sizeof(wchar_t));
	va_end(vararg);
	EnterCriticalSection(&FileAccessCS);
	WriteFile(File,tids,(uint32_t)mir_strlen(tids),&Written,nullptr);
	WriteFile(File,str,(uint32_t)mir_wstrlen(str)*sizeof(wchar_t),&Written,nullptr);
	LeaveCriticalSection(&FileAccessCS);
	free(str);
}

#endif	//ifdef DEBUG