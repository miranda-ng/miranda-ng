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

#ifdef DEBUG_COMM
HANDLE CommFile;
#endif

#ifdef DEBUG_DECODE
HANDLE DecodeFile;
#endif

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void InitDebug()
{
#ifdef DEBUG_COMM
	CommFile = mir_createLog("YAMN_comm", L"", VARSW(L"%miranda_userdata%\\yamn-debug.comm.log"), 0);
	mir_writeLogA(CommFile,"Communication debug file created by %s\n",YAMN_VER);
#endif

#ifdef DEBUG_DECODE
	DecodeFile = mir_createLog("YAMN_comm", L"", VARSW(L"%miranda_userdata%\\yamn-debug.decode.log"), 0);
	mir_writeLogA(DecodeFile, "Decoding kernel debug file created by %s\n", YAMN_VER);
#endif
}

void UnInitDebug()
{
#ifdef DEBUG_COMM
	mir_writeLogA(CommFile,"File is being closed normally.");
#endif
#ifdef DEBUG_DECODE
	mir_writeLogA(DecodeFile,"File is being closed normally.");
#endif
}

#endif	//ifdef DEBUG