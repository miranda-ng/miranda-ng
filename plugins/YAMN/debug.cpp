/*
 * YAMN plugin main file
 * Miranda homepage: http://miranda-icq.sourceforge.net/
 *
 * Debug functions used in DEBUG release (you need to global #define DEBUG to get debug version)
 *
 * (c) majvan 2002-2004
 */

/*#include <windows.h>
#include <tchar.h>
#include <stdio.h>*/
#include "debug.h"
#ifdef YAMN_DEBUG
#include "yamn.h"
#include "version.h"

#if defined (WIN9X)
	#define YAMN_VER	"YAMN " YAMN_VERSION_C " (Win9x)"
#elif defined(WIN2IN1)
	#define YAMN_VER	"YAMN " YAMN_VERSION_C " (2in1)"
#else
	#define YAMN_VER	"YAMN " YAMN_VERSION_C " (WinNT)"
#endif

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

TCHAR DebugUserDirectory[MAX_PATH]=".";
LPCRITICAL_SECTION FileAccessCS;

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
	if(FileAccessCS==NULL)
	{
		FileAccessCS=new CRITICAL_SECTION;
		InitializeCriticalSection(FileAccessCS);
	}

#ifdef DEBUG_SYNCHRO
	_stprintf(DebugFileName,DebugSynchroFileName2,DebugUserDirectory);
	
	SynchroFile=CreateFile(DebugFileName,GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
	DebugLog(SynchroFile,"Synchro debug file created by %s\n",YAMN_VER);
#endif

#ifdef DEBUG_COMM
	_stprintf(DebugFileName,DebugCommFileName2,DebugUserDirectory);

	CommFile=CreateFile(DebugFileName,GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
	DebugLog(CommFile,"Communication debug file created by %s\n",YAMN_VER);
#endif

#ifdef DEBUG_DECODE
	_stprintf(DebugFileName,DebugDecodeFileName2,DebugUserDirectory);

	DecodeFile=CreateFile(DebugFileName,GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
	DebugLog(DecodeFile,"Decoding kernel debug file created by %s\n",YAMN_VER);
#endif
}

void UnInitDebug()
{
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
	_stprintf(tids,_T("[%x]"),GetCurrentThreadId());
	while(_vsnprintf(str,strsize,fmt,vararg)==-1)
		str=(char *)realloc(str,strsize+=65536);
	va_end(vararg);
	EnterCriticalSection(FileAccessCS);
	WriteFile(File,tids,(DWORD)strlen(tids),&Written,NULL);
	WriteFile(File,str,(DWORD)strlen(str),&Written,NULL);
	LeaveCriticalSection(FileAccessCS);
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
	_stprintf(tids,_T("[%x]"),GetCurrentThreadId());
	while(_vsnwprintf(str,strsize,fmt,vararg)==-1)
		str=(WCHAR *)realloc(str,(strsize+=65536)*sizeof(WCHAR));
	va_end(vararg);
	EnterCriticalSection(FileAccessCS);
	WriteFile(File,tids,(DWORD)strlen(tids),&Written,NULL);
	WriteFile(File,str,(DWORD)wcslen(str)*sizeof(WCHAR),&Written,NULL);
	LeaveCriticalSection(FileAccessCS);
	free(str);
}

#endif	//ifdef DEBUG