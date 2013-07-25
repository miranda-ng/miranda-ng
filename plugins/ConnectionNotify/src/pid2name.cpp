#include "ConnectionNotify.h"

void pid2name(DWORD procid,TCHAR* buffer)
{
    HANDLE hSnap = INVALID_HANDLE_VALUE;
    HANDLE hProcess = INVALID_HANDLE_VALUE;
    PROCESSENTRY32 ProcessStruct;
    ProcessStruct.dwSize = sizeof(PROCESSENTRY32);
    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnap == INVALID_HANDLE_VALUE)
        return;
    if(Process32First(hSnap, &ProcessStruct) == FALSE)
        return;
    do
    {
		if(ProcessStruct.th32ProcessID==procid)
		{
			_stprintf(buffer,_T("%s"),ProcessStruct.szExeFile); //!!!!!!!!!!!!
            break;
        }
    }
    while( Process32Next( hSnap, &ProcessStruct ) );
    
	CloseHandle( hSnap );
}

/*
#define STATUS_SUCCESS               ((NTSTATUS)0x00000000L)
#define STATUS_INFO_LENGTH_MISMATCH  ((NTSTATUS)0xC0000004L)

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemProcessInformation = 5
} SYSTEM_INFORMATION_CLASS;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING;

typedef LONG KPRIORITY; // Thread priority

typedef struct _SYSTEM_PROCESS_INFORMATION_DETAILD {
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    LARGE_INTEGER SpareLi1;
    LARGE_INTEGER SpareLi2;
    LARGE_INTEGER SpareLi3;
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    UNICODE_STRING ImageName;
    KPRIORITY BasePriority;
    HANDLE UniqueProcessId;
    ULONG InheritedFromUniqueProcessId;
    ULONG HandleCount;
    BYTE Reserved4[4];
    PVOID Reserved5[11];
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivatePageCount;
    LARGE_INTEGER Reserved6[6];
} SYSTEM_PROCESS_INFORMATION_DETAILD, *PSYSTEM_PROCESS_INFORMATION_DETAILD;

typedef NTSTATUS (WINAPI *PFN_NT_QUERY_SYSTEM_INFORMATION)(
  IN       SYSTEM_INFORMATION_CLASS SystemInformationClass,
  IN OUT   PVOID SystemInformation,
  IN       ULONG SystemInformationLength,
  OUT OPTIONAL  PULONG ReturnLength
);

void pid2name(DWORD procid,TCHAR* buffer)
{
	size_t bufferSize = 102400;
    PSYSTEM_PROCESS_INFORMATION_DETAILD pspid=(PSYSTEM_PROCESS_INFORMATION_DETAILD) malloc (bufferSize);
    ULONG ReturnLength;
    PFN_NT_QUERY_SYSTEM_INFORMATION pfnNtQuerySystemInformation = (PFN_NT_QUERY_SYSTEM_INFORMATION)GetProcAddress (GetModuleHandle(TEXT("ntdll.dll")), "NtQuerySystemInformation");
    NTSTATUS status;

    while (TRUE) 
	{
        status = pfnNtQuerySystemInformation (SystemProcessInformation, (PVOID)pspid, bufferSize, &ReturnLength);
        if (status == STATUS_SUCCESS)
            break;
        else if (status != STATUS_INFO_LENGTH_MISMATCH) 
		{ // 0xC0000004L
            //_tprintf (TEXT("ERROR 0x%X\n"), status);
			goto error_block;
        }

        bufferSize *= 2;
        pspid = (PSYSTEM_PROCESS_INFORMATION_DETAILD) realloc ((PVOID)pspid, bufferSize);
    }

    for (;;pspid=(PSYSTEM_PROCESS_INFORMATION_DETAILD)(pspid->NextEntryOffset + (PBYTE)pspid)) {
		if (pspid->UniqueProcessId == (HANDLE)procid)
		{
			_stprintf(buffer,"%ls",pspid->ImageName.Buffer);
			break;
		}
        //_tprintf (TEXT("ProcessId: %d, ImageFileName: %ls\n"), pspid->UniqueProcessId,(pspid->ImageName.Length && pspid->ImageName.Buffer)? pspid->ImageName.Buffer: L"");
        if (pspid->NextEntryOffset == 0) break;
    }
error_block:
if (pspid!=NULL)
{
	free(pspid);
	pspid=NULL;
}
return;
}
*/