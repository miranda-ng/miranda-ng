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

#include "Common.h"

BOOL findFilename(TCHAR *);
TCHAR *filename(TCHAR *);

// Globals
extern PROCESS_LIST ProcessList;

BOOL areThereProcessesRunning(void)
{
	HANDLE         hSnapShot = NULL;
	LPDWORD        lpdwPIDs  = NULL;
	PROCESSENTRY32 procentry;
	BOOL           bFlag;


	if (!ProcessList.count) // Process list is empty
		return FALSE;

	// Get a handle to a Toolhelp snapshot of all processes.
    if ((hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) == INVALID_HANDLE_VALUE)
		return FALSE;

    // While there are processes, keep looping.
    for (procentry.dwSize=sizeof(PROCESSENTRY32), bFlag=Process32First(hSnapShot, &procentry); bFlag; procentry.dwSize=sizeof(PROCESSENTRY32), bFlag=Process32Next(hSnapShot, &procentry)) {
		TCHAR *szFileNameAux = filename(procentry.szExeFile);

		// Search szFileName in user-defined list
		if (findFilename(szFileNameAux))
			return TRUE;
	}

	return FALSE;
}

BOOL findFilename(TCHAR *fileName)
{
	for (int i=0; i < ProcessList.count; i++)
		if (ProcessList.szFileName[i] && !_wcsicmp(ProcessList.szFileName[i], fileName))
			return TRUE;

	return FALSE;
}


TCHAR *filename(TCHAR *fullFileName)
{
	TCHAR *str;

	str = wcsrchr(fullFileName, '\\');
	if (!str)
		return fullFileName;

	return ++str;
}