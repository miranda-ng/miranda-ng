#include "stdafx.h"

void pid2name(uint32_t procid, wchar_t *buffer, size_t bufLen)
{
	PROCESSENTRY32 ProcessStruct;
	ProcessStruct.dwSize = sizeof(PROCESSENTRY32);
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
		return;
	if (Process32First(hSnap, &ProcessStruct) == FALSE)
		return;

	do {
		if (ProcessStruct.th32ProcessID == procid) {
			wcsncpy_s(buffer, bufLen, ProcessStruct.szExeFile, _TRUNCATE);
			break;
		}
	} while (Process32Next(hSnap, &ProcessStruct));

	CloseHandle(hSnap);
}
