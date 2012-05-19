/*

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

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <vdmdbg.h>
#include "EnumProc.h"


BOOL findFilename(char *);
char *filename(char *);
BOOL WINAPI Enum16(DWORD, WORD, WORD, char *, char *, LPARAM);


// Globals
extern double dWinVer;
extern BOOL bWindowsNT;
extern PROCESS_LIST ProcessList;

HINSTANCE hInstLib, hInstLib2;
HANDLE (WINAPI *lpfCreateToolhelp32Snapshot)(DWORD, DWORD);
BOOL (WINAPI *lpfProcess32First)(HANDLE, LPPROCESSENTRY32);
BOOL (WINAPI *lpfProcess32Next)(HANDLE, LPPROCESSENTRY32);
BOOL (WINAPI *lpfEnumProcesses)(DWORD *, DWORD, DWORD *);
BOOL (WINAPI *lpfEnumProcessModules)(HANDLE, HMODULE *, DWORD, LPDWORD);
DWORD (WINAPI *lpfGetModuleBaseName)(HANDLE, HMODULE, LPTSTR, DWORD);
INT (WINAPI *lpfVDMEnumTaskWOWEx)(DWORD, TASKENUMPROCEX, LPARAM);


void LoadProcsLibrary(void)
{
	if (bWindowsNT && dWinVer < 5) {

		if (!(hInstLib = LoadLibraryA("PSAPI.DLL")))
			return;

		if (!(hInstLib2 = LoadLibraryA("VDMDBG.DLL")))
			return;

		lpfEnumProcesses = (BOOL (WINAPI *)(DWORD *, DWORD, DWORD*)) GetProcAddress(hInstLib, "EnumProcesses");
		lpfEnumProcessModules = (BOOL (WINAPI *)(HANDLE, HMODULE *, DWORD, LPDWORD)) GetProcAddress(hInstLib, "EnumProcessModules");
		lpfGetModuleBaseName = (DWORD (WINAPI *)(HANDLE, HMODULE, LPTSTR, DWORD)) GetProcAddress(hInstLib, "GetModuleBaseNameA");

		lpfVDMEnumTaskWOWEx = (INT (WINAPI *)(DWORD, TASKENUMPROCEX, LPARAM)) GetProcAddress(hInstLib2, "VDMEnumTaskWOWEx");
	} else {

		if(!(hInstLib = LoadLibraryA("Kernel32.DLL")))
			return;

		if (bWindowsNT && !(hInstLib2 = LoadLibraryA("VDMDBG.DLL")))
			return;
 
		lpfCreateToolhelp32Snapshot = (HANDLE (WINAPI *)(DWORD,DWORD)) GetProcAddress(hInstLib, "CreateToolhelp32Snapshot");
		lpfProcess32First = (BOOL (WINAPI *)(HANDLE,LPPROCESSENTRY32)) GetProcAddress(hInstLib, "Process32First");
		lpfProcess32Next = (BOOL (WINAPI *)(HANDLE,LPPROCESSENTRY32)) GetProcAddress(hInstLib, "Process32Next");

		if (bWindowsNT)
			lpfVDMEnumTaskWOWEx = (INT (WINAPI *)(DWORD, TASKENUMPROCEX, LPARAM)) GetProcAddress(hInstLib2, "VDMEnumTaskWOWEx");
	}
}


void UnloadProcsLibrary(void)
{
	if (hInstLib)
		FreeLibrary(hInstLib);
	if (hInstLib2)
		FreeLibrary(hInstLib2);

	hInstLib = hInstLib = NULL;
	lpfCreateToolhelp32Snapshot = NULL;
	lpfProcess32First = (void *)lpfProcess32Next = (void *)lpfEnumProcesses = (void *)lpfEnumProcessModules = (void *)lpfGetModuleBaseName = (void *)lpfVDMEnumTaskWOWEx = NULL;
}


BOOL areThereProcessesRunning(void)
{
	HANDLE         hSnapShot = NULL;
	LPDWORD        lpdwPIDs  = NULL;
	PROCESSENTRY32 procentry;
	BOOL           bFlag;
	DWORD          dwSize;
	DWORD          dwSize2;
	DWORD          dwIndex;
	HMODULE        hMod;
	HANDLE         hProcess;
	char           szFileName[MAX_PATH+1];


	if (!ProcessList.count) // Process list is empty
		return FALSE;

	// If Windows NT 4.0
	if (bWindowsNT && dWinVer < 5) {

		if (!lpfEnumProcesses || !lpfEnumProcessModules || !lpfGetModuleBaseName || !lpfVDMEnumTaskWOWEx)
			return FALSE;

		//
		// Call the PSAPI function EnumProcesses to get all of the ProcID's currently in the system.
		//
		// NOTE: In the documentation, the third parameter of EnumProcesses is named cbNeeded, which implies that you
		// can call the function once to find out how much space to allocate for a buffer and again to fill the buffer.
		// This is not the case. The cbNeeded parameter returns the number of PIDs returned, so if your buffer size is
		// zero cbNeeded returns zero.
		//
		// NOTE: The "HeapAlloc" loop here ensures that we actually allocate a buffer large enough for all the
		// PIDs in the system.
		//
		dwSize2 = 256 * sizeof(DWORD);
		do {
			if (lpdwPIDs) {
				HeapFree(GetProcessHeap(), 0, lpdwPIDs);
				dwSize2 *= 2;
			}
        		if (!(lpdwPIDs = (LPDWORD)HeapAlloc(GetProcessHeap(), 0, dwSize2)))
               			return FALSE;
			if (!lpfEnumProcesses(lpdwPIDs, dwSize2, &dwSize)) {
				HeapFree(GetProcessHeap(), 0, lpdwPIDs);
				return FALSE;
			}
		} while (dwSize == dwSize2);

		// How many ProcID's did we get?
		dwSize /= sizeof(DWORD);

		// Loop through each ProcID.
		for (dwIndex = 0; dwIndex < dwSize; dwIndex++) {
			char *szFileNameAux;
			szFileName[0] = '\0';

			// Open the process (if we can... security does not permit every process in the system to be opened).
			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, lpdwPIDs[dwIndex]);
			if (hProcess) {
				// Here we call EnumProcessModules to get only the first module in the process. This will be the
				// EXE module for which we will retrieve the name.
				if (lpfEnumProcessModules(hProcess, &hMod, sizeof(hMod), &dwSize2)) {
					// Get the module name
					if (!lpfGetModuleBaseName(hProcess, hMod, szFileName, sizeof(szFileName)))
						szFileName[0] = '\0';
				}
				CloseHandle(hProcess);
 			}
			szFileNameAux = filename(szFileName);

			// Search szFileName in user-defined list
			if (findFilename(szFileNameAux)) {
				HeapFree(GetProcessHeap(), 0, lpdwPIDs);
				return TRUE;
			}

			// Did we just bump into an NTVDM?
			if (!_stricmp(szFileNameAux, "NTVDM.EXE")) {
				BOOL bFound = FALSE;

				// Enum the 16-bit stuff.
				lpfVDMEnumTaskWOWEx(lpdwPIDs[dwIndex], (TASKENUMPROCEX) Enum16, (LPARAM)&bFound);

				// Did we find any user-defined process?
				if (bFound) {
					HeapFree(GetProcessHeap(), 0, lpdwPIDs);
					return TRUE;
				}
			}
		}
		HeapFree(GetProcessHeap(), 0, lpdwPIDs);

	// If any OS other than Windows NT 4.0.
	} else {

		if (!lpfProcess32Next || !lpfProcess32First || !lpfCreateToolhelp32Snapshot)
			return FALSE;

			// Get a handle to a Toolhelp snapshot of all processes.
         	if ((hSnapShot = lpfCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) == INVALID_HANDLE_VALUE)
				return FALSE;

         	// While there are processes, keep looping.
         	for (procentry.dwSize=sizeof(PROCESSENTRY32), bFlag=lpfProcess32First(hSnapShot, &procentry); bFlag; procentry.dwSize=sizeof(PROCESSENTRY32), bFlag=lpfProcess32Next(hSnapShot, &procentry)) {
				char *szFileNameAux = filename(procentry.szExeFile);

				// Search szFileName in user-defined list
				if (findFilename(szFileNameAux))
					return TRUE;

				// Did we just bump into an NTVDM?
				if (lpfVDMEnumTaskWOWEx && !_stricmp(szFileNameAux, "NTVDM.EXE")) {
					BOOL bFound = FALSE;

					// Enum the 16-bit stuff.
					lpfVDMEnumTaskWOWEx(procentry.th32ProcessID, (TASKENUMPROCEX)Enum16, (LPARAM)&bFound);

					// Did we find any user-defined process?
					if (bFound)
						return TRUE;
				}
			}
	}

	return FALSE;
}


BOOL WINAPI Enum16(DWORD dwThreadId, WORD hMod16, WORD hTask16, char *szModName, char *szFileName, LPARAM lpUserDefined)
{
	BOOL bRet;
	BOOL *pbFound = (BOOL *)lpUserDefined;

	if ((bRet = findFilename(filename(szFileName))))
		*pbFound = TRUE;

	return bRet;
}


BOOL findFilename(char *fileName)
{
	unsigned int i;

	for (i=0; i < ProcessList.count; i++)
		if (ProcessList.szFileName[i] && !_stricmp(ProcessList.szFileName[i], fileName))
			return TRUE;

	return FALSE;
}


char *filename(char *fullFileName)
{
	char *str;

	str = strrchr(fullFileName, '\\');
	if (!str)
		return fullFileName;

	return ++str;
}