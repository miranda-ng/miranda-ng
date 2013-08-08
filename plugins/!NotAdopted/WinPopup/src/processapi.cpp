/*

WinPopup Protocol plugin for Miranda IM.

Copyright (C) 2004-2010 Nikolay Raspopov

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

#include "stdafx.h"

DWORD GetProcessId (LPCTSTR name)
{
	DWORD id = 0;
	int name_len = lstrlen (name);
	CProcessApi papi;
	if (papi.Init (true)) {
		DWORD pl = papi.ProcessesGetList();
		if (pl) {
			CProcessApi::tProcessInfo pi = {0};
			while (papi.ProcessesWalk (pl, &pi)) {
				int len = lstrlen (pi.FileName);
				if (len >= name_len &&
					lstrcmpi (pi.FileName + (len - name_len), name) == 0) {
						id = pi.pid;
						break;
					}
			}
		}
		papi.ProcessesFreeList(pl);
	}
	return id;
}

/* ----------------------------------------------------------------------------- 
 * Copyright (c) 2003 Elias Bachaalany <elias_bachaalany@yahoo.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ----------------------------------------------------------------------------- 
 */

#ifdef _UNICODE
	#define Modifier "W"
#else
	#define Modifier "A"
#endif

// custom macro to allow me to load functions dynamically
#define DynamicGetProcAddress(lib, prefix, mod_name, Mod) \
  { \
	PVOID p = GetProcAddress(lib, #mod_name Mod); \
	if (!p) { \
		FreeLibrary(lib); \
		return (LastError = paeNoEntryPoint, false); \
	} \
	##prefix##mod_name = (t_##prefix##mod_name)p; \
  }

//--------------------------------------------------------------------------------------------
// The constructor only initializes some internal variables
CProcessApi::CProcessApi() :
	m_hPsApi (NULL),
	m_hTlHlp (NULL),
	m_hNtApi (NULL),
	m_bPsApi (false),
	m_bToolHelp (false),
	m_bNtApi (false),
	LastError (paeSuccess)
{
}

//--------------------------------------------------------------------------------------------
// This is the destructor. It frees any process helper api that has been used
CProcessApi::~CProcessApi()
{
  if (m_hPsApi)
    FreeLibrary(m_hPsApi);
  if (m_hTlHlp)
    FreeLibrary(m_hTlHlp);
  if (m_hNtApi)
    FreeLibrary(m_hNtApi);
}

//--------------------------------------------------------------------------------------------
// Used to initialize the CProcessApi class
bool CProcessApi::Init (bool bNtApiFirst)
{
	bool loaded = m_bPsApi || m_bToolHelp || m_bNtApi;

	if (bNtApiFirst && !loaded) {
		loaded = Load_NtApi();
	}

	if (!loaded) {
		loaded = Load_PsApi();
	}

	if (!loaded) {
		loaded = Load_TlHlp();
	}

	if (!bNtApiFirst && !loaded) {
		loaded = Load_NtApi();
	}

	return (loaded ? (LastError = paeSuccess, true) : (LastError = paeNoApi, false));
}

//--------------------------------------------------------------------------------------------
// This function returns a list id (list id) that can be used w/ Walking functions
// in order to navigate through the process list
// This function decides what to use from the helper apis
DWORD CProcessApi::ModulesGetList(DWORD pid)
{
	tModulesData *md = new tModulesData;
	if ( ! md )
		return (LastError = paeNoMem, 0u);

	// create the list
	md->ml = new tModulesList;
	if ( ! md->ml )
	{
		delete md;
		return (LastError = paeNoMem, 0u);
	}

	// decide what to use
	if (m_bPsApi)
		LastError = ModulesPopulatePsApi(pid, md);
	else if (m_bToolHelp)
		LastError = ModulesPopulateToolHelp(pid, md);
	else if (m_bNtApi)
		LastError = ModulesPopulateNtApi(pid, md);

	return (DWORD) md;
}

//--------------------------------------------------------------------------------------------
// Populates the modules of a process using ToolHelp api
DWORD CProcessApi::ModulesPopulateToolHelp(DWORD pid, tModulesData *md)
{
  MODULEENTRY32 me32        = {sizeof(MODULEENTRY32), 0}; 
  tModuleInfo   mi = {0};

  // Take a snapshot of all modules in the specified process. 
  HANDLE hModuleSnap = tlhlp_CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid); 

  if (hModuleSnap == INVALID_HANDLE_VALUE) 
    return paeNoSnap; 
 
  // Start walking
  BOOL bMore = tlhlp_Module32First(hModuleSnap, &me32);
  do
  {
    lstrcpy (mi.FileName, me32.szExePath);
    mi.ImageBase = me32.modBaseAddr;
    mi.ImageSize = me32.modBaseSize;

    // save item
    md->ml->Add (mi);

    // search next
    bMore = tlhlp_Module32Next(hModuleSnap, &me32);
  } while (bMore);

  CloseHandle (hModuleSnap); 
  md->Pos = 0;
  return paeSuccess;
} 

//--------------------------------------------------------------------------------------------
// Populates the modules of a process using PsApi api
DWORD CProcessApi::ModulesPopulatePsApi(DWORD pid, tModulesData *md)
{
  DWORD nModules, nCount = 4096;
  HANDLE hProcess;

  // allocate memory for modules
  HMODULE *modules   = new HMODULE[nCount];

  // open process for querying only
  hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
  if (!hProcess)
    return paeNoSnap;
      
  // now try to enum all modules
  if (!psapi_EnumProcessModules(hProcess, modules, nCount * sizeof(DWORD), &nModules))    
  {
    CloseHandle(hProcess);
    return paeNoSnap;
  }

  // because nModules returned from EnumProcessModules() is in bytes, I divid by 4 to return n DWORDs
  nModules /= 4;

  tModuleInfo mi = {0};
  MODULEINFO psapiMi = {0};

  for (DWORD i=0; i < nModules; i++)
  {
    // get module name
    psapi_GetModuleFileNameEx(hProcess, modules[i], mi.FileName, sizeof(mi.FileName));    

    // get module information
    psapi_GetModuleInformation(hProcess, modules[i], &psapiMi, sizeof(MODULEINFO));

    // get relevant data
    mi.ImageBase = psapiMi.lpBaseOfDll;
    mi.ImageSize = psapiMi.SizeOfImage;

    // save item
    md->ml->Add (mi);
  }

  md->Pos = 0;
  CloseHandle(hProcess);
  delete [] modules;

  return paeSuccess;
}

//--------------------------------------------------------------------------------------------
// It frees a modules list by its ID
void CProcessApi::ModulesFreeList(DWORD_PTR lid)
{
  tModulesData *md = reinterpret_cast<tModulesData *>(lid);
  delete md->ml;
  delete md;
}

//--------------------------------------------------------------------------------------------
// This function allows you to retrieve information about a certain module in a process
// You can either use this function using the Pos parameter controlled by the ModulesCount()
// Or you can keep calling it till it returns false
bool CProcessApi::ModulesWalk(DWORD_PTR lid, tModuleInfo *mi, DWORD Pos)
{
  tModulesData *md = reinterpret_cast<tModulesData *>(lid);

  // auto position ?
  if (Pos == -1)
    Pos = md->Pos;

  // out of bounds?
  if (Pos > md->ml->GetCount())
    return (LastError = paeOutOfBounds, false);

  // end reached ?
  else if (Pos == md->ml->GetCount())
    return (LastError = paeNoMore, false);
    
  // copy information to user buffer
  *mi = md->ml->GetAt (Pos);

  // advance position to next item
  md->Pos++;

  return (LastError = paeSuccess, true);
}


//--------------------------------------------------------------------------------------------
// This function allows you to retrieve information about a certain process in the list
// You can either use this function using the Pos parameter controlled by the ProcessesCount()
// Or you can keep calling it till it returns false
bool CProcessApi::ProcessesWalk(DWORD_PTR lid, tProcessInfo *pi, DWORD Pos)
{
  tProcessesData *pd = reinterpret_cast<tProcessesData *>(lid);

  // auto position ?
  if (Pos == -1)
    Pos = pd->Pos;

  // out of bounds?
  if (Pos > pd->pl->GetCount())
    return (LastError = paeOutOfBounds, false);
  // end reached ?
  else if (Pos == pd->pl->GetCount())
    return (LastError = paeNoMore, false);
    
  // copy information to user buffer
  *pi = pd->pl->GetAt (Pos);

  // advance position to next item
  pd->Pos++;
  return (LastError = paeSuccess, true);
}


//--------------------------------------------------------------------------------------------
// This function returns a list id (list id) that can be used w/ Walking functions
// in order to navigate through the process list
// This function decides what to use from the helper apis
DWORD CProcessApi::ProcessesGetList()
{
  tProcessesData *pd = new tProcessesData;
  if (!pd)
    return (LastError = paeNoMem, 0u);

  // create the list
  pd->pl = new tProcessesList;
  if (!pd->pl)
  {
    delete pd;
    return (LastError = paeNoMem, 0u);
  }

  // decide what to use
  if (m_bPsApi)
    LastError = ProcessesPopulatePsApi(pd);
  else if (m_bToolHelp)
    LastError = ProcessesPopulateToolHelp(pd);
  else if (m_bNtApi)
    LastError = ProcessesPopulateNtApi(pd);

  return (DWORD) pd;
}

//--------------------------------------------------------------------------------------------
// It frees a process list by its ID
void CProcessApi::ProcessesFreeList(DWORD_PTR lid)
{
  tProcessesData *pd = reinterpret_cast<tProcessesData *>(lid);
  delete pd->pl;
  delete pd;
}

//--------------------------------------------------------------------------------------------
// Dynamically loads the PsApi functions
bool CProcessApi::Load_PsApi()
{
	if (m_bPsApi)
		return true;
	if (!m_hPsApi)
		m_hPsApi = LoadLibrary (_T("psapi.dll"));
	if (!m_hPsApi)
		return false; 

	DynamicGetProcAddress(m_hPsApi, psapi_, GetModuleFileNameEx, Modifier);
	DynamicGetProcAddress(m_hPsApi, psapi_, EnumProcessModules, "");
	DynamicGetProcAddress(m_hPsApi, psapi_, EnumProcesses, "");
	DynamicGetProcAddress(m_hPsApi, psapi_, GetModuleInformation, "");

	m_bPsApi = true;
	return true;
}


//--------------------------------------------------------------------------------------------
// Dynamically loads the ToolHelp functions
bool CProcessApi::Load_TlHlp()
{
	if (m_bToolHelp)
		return true;
	if (!m_hTlHlp)
		m_hTlHlp = LoadLibrary (_T("kernel32.dll"));
	if (!m_hTlHlp)
		return false; 

	DynamicGetProcAddress(m_hTlHlp, tlhlp_, CreateToolhelp32Snapshot, "");
	DynamicGetProcAddress(m_hTlHlp, tlhlp_, Process32First, "");
	DynamicGetProcAddress(m_hTlHlp, tlhlp_, Process32Next, "");
	DynamicGetProcAddress(m_hTlHlp, tlhlp_, Module32First, "");
	DynamicGetProcAddress(m_hTlHlp, tlhlp_, Module32Next, "");

	m_bToolHelp = true;
	return true;
}

bool CProcessApi::Load_NtApi()
{
	if (m_bNtApi)
		return true;
	if (!m_hNtApi)
		m_hNtApi = LoadLibrary (_T("ntdll.dll"));
	if (!m_hNtApi)
		return false; 

	DynamicGetProcAddress(m_hNtApi, ntapi_, ZwQuerySystemInformation, "");

	m_bNtApi = true;
	return true;
}

//--------------------------------------------------------------------------------------------
// Populates a tProcessesList with the help of ToolHelp API
// Returns an error code paeXXXX
DWORD CProcessApi::ProcessesPopulateToolHelp(tProcessesData *pd)
{
  // create a process snapshot
  HANDLE hSnap = tlhlp_CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnap == INVALID_HANDLE_VALUE)
    return paeNoSnap;

  BOOL bMore;
  tProcessInfo pi = {0};

  PROCESSENTRY32 pe32 = {sizeof(PROCESSENTRY32), 0};

  // clear the list
  pd->pl->RemoveAll ();

  // initialize position
  pd->Pos = 0;

  bMore = tlhlp_Process32First(hSnap, &pe32);
  while (bMore)
  {
    // convert from PROCESSENTRY32 to my unified tProcessInfo struct
    pi.pid = pe32.th32ProcessID;
    lstrcpy (pi.FileName, pe32.szExeFile);

    pd->pl->Add(pi);
    bMore = tlhlp_Process32Next(hSnap, &pe32);
  }

  CloseHandle(hSnap);
  return paeSuccess;
}


//--------------------------------------------------------------------------------------------
// Populates the list using PsApi functions
DWORD CProcessApi::ProcessesPopulatePsApi(tProcessesData *pd)
{
  DWORD nProcess, // number of processes returned
        nCount(4096); // maximum number of processes (defined by me)

  // Dynamic array for storing returned processes IDs
  DWORD *processes = new DWORD[nCount];

  // enum all processes
  if (!psapi_EnumProcesses(processes, nCount * sizeof(DWORD), &nProcess))
  {
    delete [] processes;
    return paeNoSnap;
  }

  // convert fron bytes count to items count
  nProcess /= 4;

  // walk in process list
  for (DWORD i = 0; i < nProcess; i++) {

	  if (processes[i] == 0)
		  // Idle
		  continue;

	// open process for querying only
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
	if (!hProcess)
		continue;

    // get the process's image name by getting first module
    DWORD nmod;
    HMODULE mod1;

	tProcessInfo pi = {0};
	pi.pid = processes [i];
	if (!psapi_EnumProcessModules (hProcess, &mod1, sizeof(mod1), &nmod))
		lstrcpy (pi.FileName, _T("-"));
	else
		psapi_GetModuleFileNameEx (hProcess, mod1, pi.FileName, sizeof(pi.FileName));
	pd->pl->Add (pi);
	    
	CloseHandle(hProcess);
  }

  // reposition list to 0
  pd->Pos = 0;
  delete [] processes;

  return paeSuccess;
}

DWORD CProcessApi::ProcessesPopulateNtApi (tProcessesData *pd)
{
	ULONG cbBuffer = 0x10000;
	char* pBuffer;
	NTSTATUS Status;
	do {
		pBuffer = new char [cbBuffer];
		if (pBuffer == NULL)
			return paeNoMem;
		Status = ntapi_ZwQuerySystemInformation (SystemProcessesAndThreadsInformation,
			pBuffer, cbBuffer, NULL);
		if (Status == STATUS_INFO_LENGTH_MISMATCH) {
			delete [] pBuffer;
			cbBuffer *= 2;
		} else
			if (!NT_SUCCESS(Status)) {
				delete [] pBuffer;
				return paeNoSnap;
			}
	} while (Status == STATUS_INFO_LENGTH_MISMATCH);

	PSYSTEM_PROCESSES pProcesses = (PSYSTEM_PROCESSES) pBuffer;
	for (;;) {
		if (pProcesses->ProcessName.Buffer != NULL) {
			tProcessInfo pi;
			pi.pid = pProcesses->ProcessId;
#ifdef UNICODE
			lstrcpy (pi.FileName, pProcesses->ProcessName.Buffer);
#else
			WideCharToMultiByte (CP_ACP, 0, pProcesses->ProcessName.Buffer, -1,
				pi.FileName, MAX_PATH, NULL, NULL);
#endif
			pd->pl->Add (pi);
		} // else
			// Idle

		if (pProcesses->NextEntryDelta == 0)
			break;

		// find the address of the next process structure
		pProcesses = (PSYSTEM_PROCESSES)(((LPBYTE)pProcesses) + pProcesses->NextEntryDelta);
	}
	pd->Pos = 0;
	delete [] pBuffer;
	return paeSuccess;
}

DWORD CProcessApi::ModulesPopulateNtApi(DWORD /* pid */, tModulesData* /* md */)
{
	return paeSuccess;
}	

//--------------------------------------------------------------------------------------------
// Returns the count in the processes list
DWORD CProcessApi::ProcessesCount(DWORD_PTR lid) const
{
  return (DWORD)(reinterpret_cast<tProcessesData *>(lid))->pl->GetCount();
}

//--------------------------------------------------------------------------------------------
// Returns the count in the modules list
DWORD CProcessApi::ModulesCount(DWORD_PTR lid) const
{
  return (DWORD)(reinterpret_cast<tModulesData *>(lid))->ml->GetCount();
}
