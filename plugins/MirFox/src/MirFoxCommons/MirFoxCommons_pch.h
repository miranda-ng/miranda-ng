
//used in:	all
#pragma once

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#if defined( UNICODE ) && !defined( _UNICODE )
#define _UNICODE
#endif

#define WIN32_LEAN_AND_MEAN											// Exclude rarely-used stuff from Windows headers
#include <stdint.h>													//for	[u]intNN_t types
#ifdef _WIN64 // [
#  define SCNoPTR     L"%I64o"
#  define SCNuPTR     L"%I64u"
#  define SCNxPTR     L"%I64x"
#  define SCNXPTR     L"%I64X"
#else  // WIN64 ][
#  define SCNoPTR     L"%lo"
#  define SCNuPTR     L"%lu"
#  define SCNxPTR     L"%lx"
#  define SCNXPTR     L"%lX"
#endif  // WIN64 ]
#include <windows.h>												//for	NULL
#include <string>													//for	std::string, std::wstring, TEXT and _T macros, operator<<
#include <sstream>

//used in:	MirFoxCommons_messageQueueBySM.h and MirFoxCommons_sharedMemory.h
#define BOOST_DATE_TIME_NO_LIB
#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>

//used in:	MirFoxCommons_sharedMemory.h
#include <time.h>													//for	time
#include <boost/ptr_container/ptr_list.hpp>							//for	boost::ptr_list
#include <boost/tokenizer.hpp>										//for	boost::tokenizer
#include <boost/foreach.hpp>										//for	BOOST_FOREACH


