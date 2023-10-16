#ifndef __DEBUG_H
#define __DEBUG_H

#ifdef _DEBUG

//#define DEBUG_COMM               // debug communication to a file
//#define DEBUG_DECODE             // debug header decoding to a file
//#define DEBUG_DECODECODEPAGE     // add info about codepage used in conversion
//#define DEBUG_DECODEQUOTED       // add info about quoted printable result
//#define DEBUG_FILEREAD           // debug file reading to message boxes
//#define DEBUG_FILEREADMESSAGES   // debug file reading messages to message boxes

#ifdef DEBUG_COMM
// Used for communication debug
extern HANDLE CommFile;
#endif

#ifdef DEBUG_DECODE
// Used for decoding debug
extern HANDLE DecodeFile;
#endif

#if defined(DEBUG_FILEREAD) || defined(DEBUG_FILEREADMESSAGES)
uint32_t ReadStringFromMemory(char **Parser, char *End, char **StoreTo, char *DebugString);

uint32_t ReadStringFromMemoryW(char **Parser, wchar_t *End, char **StoreTo, wchar_t *DebugString);

#else
uint32_t ReadStringFromMemory(char **Parser, char *End, char **StoreTo);

uint32_t ReadStringFromMemoryW(wchar_t **Parser, wchar_t *End, wchar_t **StoreTo);
	
#endif

#endif	//YAMN_DEBUG
#endif	//_DEBUG_H
