/*
    Variables Plugin for Miranda-IM (www.miranda-im.org)
    Copyright 2003-2006 P. Boon

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
// This file has been copied from msdn.microsoft.com
//
// EnumProc.c
//

#include "variables.h"

typedef struct {
	DWORD          dwPID;
	PROCENUMPROC   lpProc;
	DWORD          lParam;
	BOOL           bEnd;
} EnumInfoStruct;

// The EnumProcs function takes a pointer to a callback function
// that will be called once per process with the process filename
// and process ID.
//
// lpProc -- Address of callback routine.
//
// lParam -- A user-defined LPARAM value to be passed to
//           the callback routine.
//
// Callback function definition:
// BOOL CALLBACK Proc(DWORD dw, WORD w, LPCSTR lpstr, LPARAM lParam);

BOOL WINAPI EnumProcs(PROCENUMPROC lpProc, LPARAM lParam)
{
	LPDWORD        lpdwPIDs  = NULL;

	// Retrieve the OS version
	OSVERSIONINFO  osver;
	osver.dwOSVersionInfoSize = sizeof(osver);
	if (!GetVersionEx(&osver))
		return FALSE;

	if (osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS || (osver.dwPlatformId == VER_PLATFORM_WIN32_NT && osver.dwMajorVersion > 4)) {
		// Get a handle to a Toolhelp snapshot of all processes.
		HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapShot == INVALID_HANDLE_VALUE)
			return FALSE;

		// Get the first process' information.
		PROCESSENTRY32 procentry;
		procentry.dwSize = sizeof(PROCESSENTRY32);
		BOOL bFlag = Process32First(hSnapShot, &procentry);

		// While there are processes, keep looping.
		while (bFlag) {
			// Call the enum func with the filename and ProcID.
			if (lpProc(procentry.th32ProcessID, 0, (char *)procentry.szExeFile, lParam)) {
				procentry.dwSize = sizeof(PROCESSENTRY32);
				bFlag = Process32Next(hSnapShot, &procentry);

			} else
				bFlag = FALSE;
		}
	} else
		return FALSE;

	return TRUE;
}