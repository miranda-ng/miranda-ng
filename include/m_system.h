/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-19 Miranda NG team (https://miranda-ng.org)
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

#define NEWSTR_ALLOCA(A) (A == NULL)?NULL:strcpy((char*)alloca(strlen(A)+1), A)
#define NEWWSTR_ALLOCA(A) ((A==NULL)?NULL:wcscpy((wchar_t*)alloca(sizeof(wchar_t)*(wcslen(A)+1)),A))

#include <m_core.h>

// miranda/system/modulesloaded
// called after all modules have been successfully initialised
// wParam = lParam = 0
// used to resolve double-dependencies in the module load order
#define ME_SYSTEM_MODULESLOADED  "Miranda/System/ModulesLoaded"

// miranda/system/shutdown event
// called just before the application terminates
// the database is still guaranteed to be running during this hook.
// wParam = lParam = 0
#define ME_SYSTEM_SHUTDOWN   "Miranda/System/Shutdown"

// restarts miranda (0.8+)
// wParam = 0 or 1. 1 - restart with current profile, 0 - restart in default profile or profile manager
// lParam = (wchar_t*)path to a new miranda32.exe binary or NULL to use current
#define MS_SYSTEM_RESTART    "Miranda/System/Restart"

// miranda/system/oktoexit event
// called before the app goes into shutdown routine to make sure everyone is
// happy to exit
// wParam = lParam = 0
// return nonzero to stop the exit cycle
#define ME_SYSTEM_OKTOEXIT   "Miranda/System/OkToExitEvent"

// gets the version number of Miranda encoded as a DWORD
// returns the version number, encoded as one version per byte, therefore
// version 1.2.3.10 is 0x0102030a
EXTERN_C MIR_APP_DLL(DWORD) Miranda_GetVersion(void);

// gets the version number of Miranda encoded as four WORDs   v0.92.2+
// returns the version number, encoded as one version per word, therefore
// version 1.2.3.3210 is 0x0001, 0x0002, 0x0003, 0x0C8a
typedef WORD MFileVersion[4];
EXTERN_C MIR_APP_DLL(void) Miranda_GetFileVersion(MFileVersion*);

// gets the version of Miranda encoded as text
// cch is the size of the buffer pointed to by pszVersion, in bytes
// may return a build qualifier, such as "0.1.0.1 alpha"
// returns 0 on success, nonzero on failure
EXTERN_C MIR_APP_DLL(void) Miranda_GetVersionText(char *pDest, size_t cbSize);

// Adds an event to the list to be checked in the main message loop
// when a handle gets triggered, an appopriate stub gets called
typedef void (CALLBACK *MWaitableStub)(void);
typedef void (CALLBACK *MWaitableStubEx)(void*);

EXTERN_C MIR_APP_DLL(void) Miranda_WaitOnHandle(MWaitableStub pFunc, HANDLE hEvent = nullptr);
EXTERN_C MIR_APP_DLL(void) Miranda_WaitOnHandleEx(MWaitableStubEx pFunc, void *pInfo);

// wParam = 0 (ignored)
// lParam = 0 (ignored)
//
// This hook is fired just before the thread unwind stack is used,
// it allows MT plugins to shutdown threads if they have any special
// processing to do, etc.
#define ME_SYSTEM_PRESHUTDOWN		"Miranda/System/PShutdown"

//	Returns true when Miranda has got WM_QUIT and is in the process of shutting down
EXTERN_C MIR_APP_DLL(bool) Miranda_IsTerminated(void);

// Check if everyone is happy to exit
// if everyone acknowleges OK to exit then returns true, otherwise false
EXTERN_C MIR_APP_DLL(bool) Miranda_OkToExit(void);

// Used by contact lists inside CloseAction
// Waits for a permission to exit and destroys contact list
EXTERN_C MIR_APP_DLL(void) Miranda_Close(void);

// Sets up a function pointer to be called after main loop iterations, suitable for idle processing
EXTERN_C MIR_APP_DLL(void) Miranda_SetIdleCallback(void(__cdecl *pfnCallback)(void));

// returns the last window tick where a monitored event was seen, currently WM_CHAR/WM_MOUSEMOVE
EXTERN_C MIR_APP_DLL(DWORD) Miranda_GetIdle(void);

/////////////////////////////////////////////////////////////////////////////////////////

#if defined(__cplusplus)
	#include <m_system_cpp.h>
#endif

#endif // M_SYSTEM_H
