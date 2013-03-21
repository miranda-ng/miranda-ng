/*

WinPopup Protocol plugin for Miranda IM.

Copyright (C) 2004-2006 Nikolay Raspopov

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

typedef LONG NTSTATUS;
typedef LONG KPRIORITY;
typedef LONG SYSTEM_INFORMATION_CLASS;

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
#define SystemProcessesAndThreadsInformation 5

typedef struct _CLIENT_ID {
	DWORD UniqueProcess;
	DWORD UniqueThread;
} CLIENT_ID;

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
} UNICODE_STRING;

typedef struct _VM_COUNTERS {
	SIZE_T PeakVirtualSize;
	SIZE_T VirtualSize;
	ULONG PageFaultCount;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	SIZE_T QuotaPeakPagedPoolUsage;
	SIZE_T QuotaPagedPoolUsage;
	SIZE_T QuotaPeakNonPagedPoolUsage;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
} VM_COUNTERS;

typedef struct _SYSTEM_THREADS {
	LARGE_INTEGER KernelTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER CreateTime;
	ULONG WaitTime;
	PVOID StartAddress;
	CLIENT_ID ClientId;
	KPRIORITY Priority;
	KPRIORITY BasePriority;
	ULONG ContextSwitchCount;
	LONG State;
	LONG WaitReason;
} SYSTEM_THREADS, * PSYSTEM_THREADS;

typedef struct _SYSTEM_PROCESSES {
	ULONG NextEntryDelta;
	ULONG ThreadCount;
	ULONG Reserved1[6];
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER KernelTime;
	UNICODE_STRING ProcessName;
	KPRIORITY BasePriority;
	ULONG ProcessId;
	ULONG InheritedFromProcessId;
	ULONG HandleCount;
	ULONG Reserved2[2];
	VM_COUNTERS VmCounters;
#if _WIN32_WINNT >= 0x500
	IO_COUNTERS IoCounters;
#endif
	SYSTEM_THREADS Threads[1];
} SYSTEM_PROCESSES, * PSYSTEM_PROCESSES;

//--------------------------------------------------------------------------------------------
// CProcessApi class - written by Elias Bachaalany.
// Check the implementation file for more information
//

class CProcessApi
{
public:
	typedef struct tProcessInfo
	{
		DWORD pid;
		TCHAR FileName[MAX_PATH];
	};

	typedef struct tModuleInfo
	{
		LPVOID ImageBase;
		DWORD  ImageSize;
		TCHAR  FileName[MAX_PATH];
	};

private:
	typedef CAtlArray <tProcessInfo> tProcessesList;
	typedef CAtlArray <tModuleInfo> tModulesList;

	typedef struct tProcessesData
	{
		DWORD Pos;
		tProcessesList *pl;
	};

	typedef struct tModulesData
	{
		DWORD Pos;
		tModulesList *ml;
	};

	// PSAPI.DLL functions prototype
	typedef BOOL (WINAPI *t_psapi_EnumProcesses)(
		DWORD *lpidProcess,  // array of process identifiers
		DWORD cb,            // size of array
		DWORD *cbNeeded      // number of bytes returned
	);

	typedef BOOL (WINAPI *t_psapi_EnumProcessModules)(
		HANDLE hProcess,      // handle to process
		HMODULE *lphModule,   // array of module handles
		DWORD cb,             // size of array
		LPDWORD lpcbNeeded    // number of bytes required
	);

	typedef DWORD (WINAPI *t_psapi_GetModuleFileNameEx)(
		HANDLE hProcess,    // handle to process
		HMODULE hModule,    // handle to module
		LPTSTR lpFilename,  // path buffer
		DWORD nSize         // maximum characters to retrieve
	);

	typedef BOOL (WINAPI *t_psapi_GetModuleInformation)(
		HANDLE hProcess,
		HMODULE hModule,
		LPMODULEINFO lpmodinfo,
		DWORD cb
	);

	// functions instances
	t_psapi_GetModuleFileNameEx       psapi_GetModuleFileNameEx;
	t_psapi_EnumProcessModules        psapi_EnumProcessModules;
	t_psapi_EnumProcesses             psapi_EnumProcesses;
	t_psapi_GetModuleInformation      psapi_GetModuleInformation;

	// TOOLHELP functions prototype
	typedef HANDLE (WINAPI *t_tlhlp_CreateToolhelp32Snapshot)(
		DWORD dwFlags,
		DWORD th32ProcessID
	);

	typedef BOOL (WINAPI *t_tlhlp_Process32First)(
		HANDLE hSnapshot,
		LPPROCESSENTRY32 lppe
	);

	typedef BOOL (WINAPI *t_tlhlp_Process32Next)(
		HANDLE hSnapshot,
		LPPROCESSENTRY32 lppe
	);

	typedef BOOL (WINAPI *t_tlhlp_Module32First)(
		HANDLE hSnapshot,
		LPMODULEENTRY32 lpme
	);

	typedef BOOL (WINAPI *t_tlhlp_Module32Next)(
		HANDLE hSnapshot,
		LPMODULEENTRY32 lpme
	);

	// functions instances
	t_tlhlp_CreateToolhelp32Snapshot	tlhlp_CreateToolhelp32Snapshot;
	t_tlhlp_Process32First				tlhlp_Process32First;
	t_tlhlp_Process32Next				tlhlp_Process32Next; 
	t_tlhlp_Module32First				tlhlp_Module32First;
	t_tlhlp_Module32Next				tlhlp_Module32Next;

	// NTDLL.DLL functions prototype
	typedef NTSTATUS (NTAPI *t_ntapi_ZwQuerySystemInformation)(
		SYSTEM_INFORMATION_CLASS SystemInformationClass,
		PVOID SystemInformation,
		ULONG SystemInformationLength,
		PULONG ReturnLength
	);

	// functions instances
	t_ntapi_ZwQuerySystemInformation	ntapi_ZwQuerySystemInformation;

	// Private member variables
	HMODULE   m_hPsApi;
	HMODULE   m_hTlHlp;
	HMODULE   m_hNtApi;

	bool      m_bPsApi;
	bool      m_bToolHelp;
	bool      m_bNtApi;

	bool Load_PsApi();
	bool Load_TlHlp();
	bool Load_NtApi();

	DWORD ProcessesPopulatePsApi(tProcessesData *pd);
	DWORD ProcessesPopulateToolHelp(tProcessesData *pd);
	DWORD ProcessesPopulateNtApi(tProcessesData *pd);

	DWORD ModulesPopulatePsApi(DWORD pid, tModulesData *md);
	DWORD ModulesPopulateToolHelp(DWORD pid, tModulesData *md);
	DWORD ModulesPopulateNtApi(DWORD pid, tModulesData *md);

public:
	// CProcessApi error enum
	enum
	{
		paeSuccess = 0,                     // No error
		paeNoApi,                           // No process API helper dll found
		paeNoEntryPoint,                    // One needed entrypoint not found in helper dll
		paeNoMem,                           // Not enough memory
		paeNoSnap,                          // Could not get a snapshot
		paeNoMore,                          // List contains no more items
		paeOutOfBounds,                     // Tried to access list w/ an invalid index
		paeYYY
	};

	DWORD LastError; // Holds the last error

	CProcessApi();
	~CProcessApi();

	bool Init (bool bNtApiFirst = false);

	DWORD ProcessesGetList();
	bool  ProcessesWalk(DWORD_PTR lid, tProcessInfo *pi, DWORD Pos = -1);
	DWORD ProcessesCount(DWORD_PTR lid) const;
	void  ProcessesFreeList(DWORD_PTR lid);

	DWORD ModulesGetList(DWORD ProcessID);
	bool  ModulesWalk(DWORD_PTR lid, tModuleInfo *mi, DWORD Pos = -1);
	DWORD ModulesCount(DWORD_PTR lid) const;
	void  ModulesFreeList(DWORD_PTR lid);
};

// Получение PID запущенного процесса по имени его выполнимого модуля
DWORD GetProcessId (LPCTSTR name);
