#ifndef __DEBUG_H
#define __DEBUG_H

//	#define YAMN_DEBUG

//#define YAMN_VER_BETA
//#define YAMN_VER_BETA_CRASHONLY

#ifdef YAMN_DEBUG

//#pragma comment(lib, "th32.lib")

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0501 // WinXP only
#endif
#define VC_EXTRALEAN
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <shlwapi.h>

//#define DEBUG_SYNCHRO			//debug synchro to a file
//#define DEBUG_COMM			//debug communiation to a file
//#define DEBUG_DECODE			//debug header decoding to a file
//#define DEBUG_DECODECODEPAGE		//add info about codepage used in conversion
//#define DEBUG_DECODEBASE64		//add info about base64 result
//#define DEBUG_DECODEQUOTED		//add info about quoted printable result
//#define DEBUG_FILEREAD			//debug file reading to message boxes
//#define DEBUG_FILEREADMESSAGES		//debug file reading messages to message boxes

void DebugLog(HANDLE,const char *fmt,...);
void DebugLogW(HANDLE File,const WCHAR *fmt,...);

#ifdef DEBUG_SYNCHRO
// Used for synchronization debug
extern HANDLE SynchroFile;
#endif

#ifdef DEBUG_COMM
// Used for communication debug
extern HANDLE CommFile;
#endif

#ifdef DEBUG_DECODE
// Used for decoding debug
extern HANDLE DecodeFile;
#endif

#if defined(DEBUG_FILEREAD) || defined(DEBUG_FILEREADMESSAGES)
DWORD ReadStringFromMemory(TCHAR **Parser,TCHAR *End,TCHAR **StoreTo,TCHAR *DebugString);
	#ifndef UNICODE
DWORD ReadStringFromMemoryW(TCHAR **Parser,TCHAR *End,TCHAR **StoreTo,TCHAR *DebugString);
	#else
#define ReadStringFromMemoryW	ReadStringFromMemory
	#endif
#endif

//#ifdef DEBUG_ACCOUNTS
//int GetAccounts();
//void WriteAccounts();
//#endif

#endif	//YAMN_DEBUG
#endif	//_DEBUG_H
