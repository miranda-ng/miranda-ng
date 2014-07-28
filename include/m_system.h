/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_SYSTEM_H__
#define M_SYSTEM_H__ 1

#include <tchar.h>

#ifndef MIRANDANAME
	#define MIRANDANAME "Miranda NG"
#endif
#ifndef MIRANDACLASS
	#define MIRANDACLASS	"Miranda"
#endif

// set the default compatibility lever for Miranda 0.4.x
#ifndef MIRANDA_VER
	#define MIRANDA_VER    0x0A00
#endif

#ifndef _MSC_VER
	#define __forceinline inline __attribute__ ((always_inline))
#else
	#pragma warning(disable:4244 4245)
#endif

#define SIZEOF(X) (sizeof(X)/sizeof(X[0]))

#define NEWSTR_ALLOCA(A) (A == NULL)?NULL:strcpy((char*)alloca(strlen(A)+1), A)
#define NEWWSTR_ALLOCA(A) ((A==NULL)?NULL:wcscpy((wchar_t*)alloca(sizeof(wchar_t)*(wcslen(A)+1)),A))
#define NEWTSTR_ALLOCA(A) (A == NULL)?NULL:_tcscpy((TCHAR*)alloca((_tcslen(A)+1)* sizeof(TCHAR)), A)

#include <m_core.h>

//miranda/system/modulesloaded
//called after all modules have been successfully initialised
//wParam = lParam = 0
//used to resolve double-dependencies in the module load order
#define ME_SYSTEM_MODULESLOADED  "Miranda/System/ModulesLoaded"

//miranda/system/shutdown event
//called just before the application terminates
//the database is still guaranteed to be running during this hook.
//wParam = lParam = 0
#define ME_SYSTEM_SHUTDOWN   "Miranda/System/Shutdown"

//restarts miranda (0.8+)
//wParam = 0 or 1. 1 - restart with current profile, 0 - restart in default profile or profile manager
//lParam = 0
#define MS_SYSTEM_RESTART    "Miranda/System/Restart"

//miranda/system/oktoexit event
//called before the app goes into shutdown routine to make sure everyone is
//happy to exit
//wParam = lParam = 0
//return nonzero to stop the exit cycle
#define ME_SYSTEM_OKTOEXIT   "Miranda/System/OkToExitEvent"

//miranda/system/oktoexit service
//Check if everyone is happy to exit
//wParam = lParam = 0
//if everyone acknowleges OK to exit then returns true, otherwise false
#define MS_SYSTEM_OKTOEXIT   "Miranda/System/OkToExit"

//gets the version number of Miranda encoded as a DWORD     v0.1.0.1+
//wParam = lParam = 0
//returns the version number, encoded as one version per byte, therefore
//version 1.2.3.10 is 0x0102030a
#define MS_SYSTEM_GETVERSION "Miranda/System/GetVersion"

//gets the version number of Miranda encoded as four WORDs   v0.92.2+
//wParam = 0
//lParam = WORD[4]*
//returns the version number, encoded as one version per word, therefore
//version 1.2.3.3210 is 0x0001, 0x0002, 0x0003, 0x0C8a
#define MS_SYSTEM_GETFILEVERSION "Miranda/System/GetFileVersion"

//gets the version of Miranda encoded as text   v0.1.0.1+
//wParam = cch
//lParam = (LPARAM)(char*)pszVersion
//cch is the size of the buffer pointed to by pszVersion, in bytes
//may return a build qualifier, such as "0.1.0.1 alpha"
//returns 0 on success, nonzero on failure
#define MS_SYSTEM_GETVERSIONTEXT "Miranda/System/GetVersionText"

//Adds a HANDLE to the list to be checked in the main message loop  v0.1.2.0+
//wParam = (WPARAM)(HANDLE)hObject
//lParam = (LPARAM)(const char*)pszService
//returns 0 on success or nonzero on failure
//Causes pszService to be CallService()d (wParam = hObject, lParam = 0) from the
//main thread whenever hObject is signalled.
//The Miranda message loop has a MsgWaitForMultipleObjects() call in it to
//implement this feature. See the documentation for that function for
//information on what objects are supported.
//There is a limit of MAXIMUM_WAIT_OBJECTS minus one (MWO is defined in winnt.h
//to be 64) on the number of handles MSFMO() can process. This service will
//return nonzero if that many handles are already being waited on.

//As of writing, the following parts of Miranda are thread-safe, so can be
//called from any thread:
//All of modules.h except NotifyEventHooks()
//Read-only parts of m_database.h (since the write parts will call hooks)
//All of m_langpack.h
//for all other routines your mileage may vary, but I would strongly recommend
//that you call them from the main thread, or ask about it on plugin-dev if you
//think it really ought to work.

//Update during 0.1.2.0 development, 16/10/01:
//NotifyEventHooks() now translates all calls into the context of the main
//thread, which means that all of m_database.h is now completely safe.

//Update during 0.1.2.2 development, 17/4/02:
//The main thread's message loop now also deals with asynchronous procedure
//calls. Loop up QueueUserAPC() for a neater way to accomplish a lot of the
//things that used to require ms_system_waitonhandle.

//Miranda is compiled with the multithreaded runtime - don't forget to do the
//same with your plugin.
#define MS_SYSTEM_WAITONHANDLE   "Miranda/System/WaitOnHandle"

//Removes a HANDLE from the wait list   v0.1.2.0+
//wParam = (WPARAM)(HANDLE)hObject
//lParam = 0
//returns 0 on success or nonzero on failure.
#define MS_SYSTEM_REMOVEWAIT     "Miranda/System/RemoveWait"

/*
wParam = 0
lParam = 0

This hook is fired just before the thread unwind stack is used,
it allows MT plugins to shutdown threads if they have any special
processing to do, etc.

*/
#define ME_SYSTEM_PRESHUTDOWN		"Miranda/System/PShutdown"

/*
wParam = 0
lParam = 0

Returns TRUE when Miranda has got WM_QUIT and is in the process
of shutting down
*/
#define MS_SYSTEM_TERMINATED		"Miranda/SysTerm"

/*
	wParam : 0
	lParam : (address) void (__cdecl *callback) (void)
	Affect : Setup a function pointer to be called after main loop iterations, it allows for
		     idle processing, See notes
	Returns: 1 on success, 0 on failure

	Notes  : This service will only allow one function to be registered, if one is registered, 0 will be returned
		     Remember that this uses __cdecl.
	Version: Added during 0.3.4+

*/
#define MS_SYSTEM_SETIDLECALLBACK	"Miranda/SetIdleCallback"

/*
	wParam : 0
	lParam : &tick
	Affect : return the last window tick where a monitored event was seen, currently WM_CHAR/WM_MOUSEMOVE
	Returns: Always returns 0
	Version: Added during 0.3.4+ (2004/09/12)
*/
#define MS_SYSTEM_GETIDLE "Miranda/GetIdle"

/*
	wParam: cchMax (max length of buffer)
	lParam: pointer to buffer to fill
	Affect: Returns the build timestamp of the core, as a string of YYYYMMDDhhmmss, this service might
		not exist and therefore the build is before 2004-09-30
	Returns: zero on success, non zero on failure
	Version: 0.3.4a+ (2004/09/30)
	DEFUNCT: This service was removed on 0.3.4.3+ (2004/11/19) use APILEVEL
*/
#define MS_SYSTEM_GETBUILDSTRING "Miranda/GetBuildString"

#ifdef _STATIC
INT_PTR MirandaIsTerminated(WPARAM, LPARAM);

__forceinline INT_PTR Miranda_Terminated(void)
{
	return MirandaIsTerminated(0, 0);
}
#else
__forceinline INT_PTR Miranda_Terminated(void)
{
	return CallService(MS_SYSTEM_TERMINATED, 0, 0);
}
#endif

/* Missing service catcher
Is being called when one calls the non-existent service.
All parameters are stored in the special structure

The event handler takes 0 as wParam and TMissingServiceParams* as lParam.

0.4.3+ addition (2006/03/27)
*/

typedef struct
{
	const char* name;
	WPARAM      wParam;
	LPARAM      lParam;
}
	MISSING_SERVICE_PARAMS;

#define ME_SYSTEM_MISSINGSERVICE "System/MissingService"

/* Unhandled exceptions filter
Is being called inside any thread launched via mir_forkthread, including the main thread.
If a plugin's author executes a large piece of code inside __try/__except, he should
obtain this filter and call it inside the __except section

0.8.0+ addition (2008/07/20)
*/

#define MS_SYSTEM_GETEXCEPTFILTER "System/GetExceptFilter"

__forceinline pfnExceptionFilter Miranda_GetExceptFilter(void)
{	return GetExceptionFilter();
}

#define MS_SYSTEM_SETEXCEPTFILTER "System/SetExceptFilter"

__forceinline pfnExceptionFilter Miranda_SetExceptFilter(pfnExceptionFilter foo)
{	return SetExceptionFilter(foo);
}

#endif // M_SYSTEM_H
