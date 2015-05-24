/*
 * YAMN plugin main file
 * Miranda homepage: http://miranda-icq.sourceforge.net/
 *
 * Debug functions used in DEBUG release (you need to global #define DEBUG to get debug version)
 *
 * (c) majvan 2002-2004
 */

#include "yamn.h"

#ifdef _DEBUG

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

TCHAR DebugUserDirectory[MAX_PATH] = _T(".");
CRITICAL_SECTION FileAccessCS;

#ifdef DEBUG_SYNCHRO
TCHAR DebugSynchroFileName2[]=_T("%s\\yamn-debug.synchro.log");
HANDLE SynchroFile;
#endif

#ifdef DEBUG_COMM
TCHAR DebugCommFileName2[]=_T("%s\\yamn-debug.comm.log");
HANDLE CommFile;
#endif

#ifdef DEBUG_DECODE
TCHAR DebugDecodeFileName2[]=_T("%s\\yamn-debug.decode.log");
HANDLE DecodeFile;
#endif

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void InitDebug()
{
#if defined (DEBUG_SYNCHRO) || defined (DEBUG_COMM) || defined (DEBUG_DECODE)
	TCHAR DebugFileName[MAX_PATH];
#endif
	InitializeCriticalSection(&FileAccessCS);

#ifdef DEBUG_SYNCHRO
	mir_sntprintf(DebugFileName, SIZEOF(DebugFileName), DebugSynchroFileName2, DebugUserDirectory);
	
	SynchroFile=CreateFile(DebugFileName,GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
	DebugLog(SynchroFile,"Synchro debug file created by %s\n",YAMN_VER);
#endif

#ifdef DEBUG_COMM
	mir_sntprintf(DebugFileName, SIZEOF(DebugFileName), DebugCommFileName2, DebugUserDirectory);

	CommFile=CreateFile(DebugFileName,GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
	DebugLog(CommFile,"Communication debug file created by %s\n",YAMN_VER);
#endif

#ifdef DEBUG_DECODE
	mir_sntprintf(DebugFileName, SIZEOF(DebugFileName), DebugDecodeFileName2, DebugUserDirectory);

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
	mir_snprintf(tids, SIZEOF(tids), "[%x]",GetCurrentThreadId());
	while(mir_vsnprintf(str, strsize, fmt, vararg)==-1)
		str=(char *)realloc(str,strsize+=65536);
	va_end(vararg);
	EnterCriticalSection(&FileAccessCS);
	WriteFile(File,tids,(DWORD)mir_strlen(tids),&Written,NULL);
	WriteFile(File,str,(DWORD)mir_strlen(str),&Written,NULL);
	LeaveCriticalSection(&FileAccessCS);
	free(str);
}

void DebugLogW(HANDLE File,const WCHAR *fmt,...)
{
	WCHAR *str;
	char tids[32];
	va_list vararg;
	int strsize;
	DWORD Written;

	va_start(vararg,fmt);
	str=(WCHAR *)malloc((strsize=65536)*sizeof(WCHAR));
	mir_snprintf(tids, SIZEOF(tids), "[%x]",GetCurrentThreadId());
	while(mir_vsnwprintf(str, strsize, fmt, vararg)==-1)
		str=(WCHAR *)realloc(str,(strsize+=65536)*sizeof(WCHAR));
	va_end(vararg);
	EnterCriticalSection(&FileAccessCS);
	WriteFile(File,tids,(DWORD)mir_strlen(tids),&Written,NULL);
	WriteFile(File,str,(DWORD)mir_wstrlen(str)*sizeof(WCHAR),&Written,NULL);
	LeaveCriticalSection(&FileAccessCS);
	free(str);
}

#endif	//ifdef DEBUG