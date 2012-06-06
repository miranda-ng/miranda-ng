/*
 * Copied from YAMN plugin
 *
 * (c) majvan 2002-2004
 */
#ifdef DEBUG_FILTER

#include <windows.h>
#include <tchar.h>
#include <stdio.h>


//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

TCHAR DebugUserDirectory[MAX_PATH]=".";
LPCRITICAL_SECTION FileAccessCS;

void DebugLog(HANDLE File,const char *fmt,...);

#ifdef DEBUG_FILTER
TCHAR DebugFilterFileName2[]=_T("%s\\yamn-debug.basefilter.log");
HANDLE FilterFile=INVALID_HANDLE_VALUE;
#endif

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void InitDebug()
{
	TCHAR DebugFileName[MAX_PATH];

	if(FileAccessCS==NULL)
	{
		FileAccessCS=new CRITICAL_SECTION;
		InitializeCriticalSection(FileAccessCS);
	}

	_stprintf(DebugFileName,DebugFilterFileName2,DebugUserDirectory);

	FilterFile=CreateFile(DebugFileName,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,0,NULL);

	DebugLog(FilterFile,"Base filter plugin for YAMN - debug file\n");
}

void UnInitDebug()
{
	DebugLog(FilterFile,"File is being closed normally.");
	CloseHandle(FilterFile);
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

#endif	//ifdef DEBUG