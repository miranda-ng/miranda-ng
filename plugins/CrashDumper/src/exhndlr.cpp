#include "stdafx.h"

#ifndef FACILITY_VISUALCPP
#define FACILITY_VISUALCPP ((LONG)0x6d)
#endif

static PVOID exchndlr, exchndlrv;
static pfnExceptionFilter  threadfltr;
static PEXCEPTION_POINTERS lastptr;

void SetExceptionHandler(void)
{
	exchndlr = SetUnhandledExceptionFilter(myfilter);
}

void RemoveExceptionHandler(void)
{
	if (exchndlrv)
		RemoveVectoredExceptionHandler(exchndlrv);
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)exchndlr);
	exchndlr = nullptr;
	exchndlrv = nullptr;
}

int myDebugFilter(unsigned int code, PEXCEPTION_POINTERS ep)
{
	if (code == VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND) ||
		code == VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND)) {
		PDelayLoadInfo dlld = (PDelayLoadInfo)ep->ExceptionRecord->ExceptionInformation[0];

		char str[256];
		int off = mir_snprintf(str, "dbghelp.dll v.5.0 or later required to provide a crash report\n");
		off += mir_snprintf(str + off, _countof(str) - off, "Missing Module: %s ", dlld->szDll);

		if (dlld->dlp.fImportByName)
			mir_snprintf(str + off, _countof(str) - off, "Function: %s ", dlld->dlp.szProcName);
		else
			mir_snprintf(str + off, _countof(str) - off, "Ordinal: %x ", dlld->dlp.dwOrdinal);

		MessageBoxA(nullptr, str, "Miranda Crash Dumper", MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}


void myfilterWorker(PEXCEPTION_POINTERS exc_ptr, bool notify)
{
	wchar_t path[MAX_PATH];
	HANDLE hDumpFile = nullptr;

	SYSTEMTIME st;
	GetLocalTime(&st);
	CreateDirectoryTreeW(CrashLogFolder);

	__try {
		if (g_plugin.bUseSubFolder) {
			mir_snwprintf(path, L"%s\\%02d.%02d.%02d", CrashLogFolder, st.wYear, st.wMonth, st.wDay);
			CreateDirectory(path, nullptr);
			mir_snwprintf(path, L"%s\\%02d.%02d.%02d\\crash%02d%02d%02d%02d%02d%02d.mdmp", CrashLogFolder,
				st.wYear, st.wMonth, st.wDay, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		}
		else
			mir_snwprintf(path, L"%s\\crash%02d%02d%02d%02d%02d%02d.mdmp", CrashLogFolder,
				st.wYear, st.wMonth, st.wDay, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

		hDumpFile = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hDumpFile != INVALID_HANDLE_VALUE)
			CreateMiniDump(hDumpFile, exc_ptr);
		else if (GetLastError() != ERROR_ALREADY_EXISTS)
			MessageBox(nullptr, TranslateT("Crash Report write location is not available"), L"Miranda Crash Dumper", MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}

	bool empty = GetFileSize(hDumpFile, nullptr) == 0;
	CloseHandle(hDumpFile);
	if (empty)
		DeleteFile(path);

	__try {
		if (g_plugin.bUseSubFolder) {
			mir_snwprintf(path, L"%s\\%02d.%02d.%02d", CrashLogFolder, st.wYear, st.wMonth, st.wDay);
			CreateDirectory(path, nullptr);
			mir_snwprintf(path, L"%s\\%02d.%02d.%02d\\crash%02d%02d%02d%02d%02d%02d.txt", CrashLogFolder,
				st.wYear, st.wMonth, st.wDay, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		}
		else 
			mir_snwprintf(path, L"%s\\crash%02d%02d%02d%02d%02d%02d.txt", CrashLogFolder,
				st.wYear, st.wMonth, st.wDay, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

		hDumpFile = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);

		mir_snwprintf(path, TranslateT("Miranda crashed. Crash report stored in the folder:\n %s\n\n Would you like store it in the clipboard as well?"), CrashLogFolder);

		if (hDumpFile != INVALID_HANDLE_VALUE)
			CreateCrashReport(hDumpFile, exc_ptr, notify ? path : nullptr);
	}
	__except (myDebugFilter(GetExceptionCode(), GetExceptionInformation())) {}

	empty = GetFileSize(hDumpFile, nullptr) == 0;
	CloseHandle(hDumpFile);
	if (empty)
		DeleteFile(path);
}

LONG WINAPI myfilter(PEXCEPTION_POINTERS exc_ptr)
{
	if (exc_ptr == lastptr)
		return EXCEPTION_EXECUTE_HANDLER;
	
	lastptr = exc_ptr;
	myfilterWorker(exc_ptr, true);
	return exchndlr ? ((LPTOP_LEVEL_EXCEPTION_FILTER)exchndlr)(exc_ptr) : EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI myfilterv(PEXCEPTION_POINTERS exc_ptr)
{
	if (0xC0000000L <= exc_ptr->ExceptionRecord->ExceptionCode && 0xC0000500L >= exc_ptr->ExceptionRecord->ExceptionCode) {
		if (exc_ptr == lastptr)
			return EXCEPTION_EXECUTE_HANDLER;
		
		lastptr = exc_ptr;
		myfilterWorker(exc_ptr, true);
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

uint32_t MirandaThreadFilter(uint32_t code, EXCEPTION_POINTERS* info)
{
	if (info != lastptr) {
		lastptr = info;
		myfilterWorker(info, true);
	}
	return threadfltr(code, info);
}

void InitExceptionHandler(void)
{
	threadfltr = SetExceptionFilter(MirandaThreadFilter);
	SetExceptionHandler();
}

void DestroyExceptionHandler(void)
{
	SetExceptionFilter(threadfltr);
	RemoveExceptionHandler();
}
