// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#define _CRT_SECURE_NO_WARNINGS

#pragma once
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define BZ_NO_STDIO

#ifdef _WIN32
# define _WIN32_WINNT 0x0500
# define _WIN32_IE	0x0501
#endif

#ifdef _DEBUG
# define _STLP_DEBUG 1
#endif

#ifndef _WIN64
	#define _USE_32BIT_TIME_T
#endif

#define _STLP_DONT_USE_SHORT_STRING_OPTIM 1	// Uses small string buffer, so it saves memory for a lot of strings
#define _STLP_USE_PTR_SPECIALIZATIONS 1		// Reduces some code bloat
#define _STLP_USE_TEMPLATE_EXPRESSION 1		// Speeds up string concatenation
#define _STLP_NO_ANACHRONISMS 1

#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <Winsock2.h>

#include <newpluginapi.h>
#include <m_system_cpp.h>
#include <m_database.h>
#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_avatars.h>
#include <m_netlib.h>
#include <m_clistint.h>
#include <win2k.h>

#include <m_folders.h>
#include <m_flash.h>

#include "CriticalSection.h"
#include "TigerHash.h"
#include "Version.h"
#include "resource.h"

#ifdef _DEBUG

inline void _cdecl debugTrace(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	
	char buf[512];
	
	mir_vsnprintf(buf, sizeof(buf), format, args);
	OutputDebugStringA(buf);
	va_end(args);
}

# define debug debugTrace
#define assert(exp) \
do { if (!(exp)) { \
	debug("Assertion hit in %s(%d): " #exp "\n", __FILE__, __LINE__); \
	if(1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, #exp)) \
_CrtDbgBreak(); } } while(false)

#else
# define debug
# define assert
#endif


