#ifndef __DEBUG_H
#define __DEBUG_H

#ifdef _DEBUG

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
DWORD ReadStringFromMemory(char **Parser,char *End,char **StoreTo,char *DebugString);
	
DWORD ReadStringFromMemoryW(char **Parser,TCHAR *End,char **StoreTo,TCHAR *DebugString);
	
#else
DWORD ReadStringFromMemory(char **Parser,char *End,char **StoreTo);
	
DWORD ReadStringFromMemoryW(WCHAR **Parser,WCHAR *End,WCHAR **StoreTo);
	
#endif

//#ifdef DEBUG_ACCOUNTS
//int GetAccounts();
//void WriteAccounts();
//#endif

#endif	//YAMN_DEBUG
#endif	//_DEBUG_H
