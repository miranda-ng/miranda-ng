#include "utils.h"

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
	exchndlr = NULL;
	exchndlrv = NULL;
}

int myDebugFilter(unsigned int code, PEXCEPTION_POINTERS ep)
{
	if (code == VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND) ||
		code == VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND)) {
		PDelayLoadInfo dlld = (PDelayLoadInfo)ep->ExceptionRecord->ExceptionInformation[0];

		char str[256];
		int off = mir_snprintf(str, SIZEOF(str), "dbghelp.dll v.5.0 or later required to provide a crash report\n");
		off += mir_snprintf(str + off, SIZEOF(str) - off, "Missing Module: %s ", dlld->szDll);

		if (dlld->dlp.fImportByName)
			mir_snprintf(str + off, SIZEOF(str) - off, "Function: %s ", dlld->dlp.szProcName);
		else
			mir_snprintf(str + off, SIZEOF(str) - off, "Ordinal: %x ", dlld->dlp.dwOrdinal);

		MessageBoxA(NULL, str, "Miranda Crash Dumper", MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}


void myfilterWorker(PEXCEPTION_POINTERS exc_ptr, bool notify)
{
	TCHAR path[MAX_PATH];
	SYSTEMTIME st;
	HANDLE hDumpFile = NULL;

	GetLocalTime(&st);
	CreateDirectoryTree(CrashLogFolder);

	__try {
		if (dtsubfldr) {
			mir_sntprintf(path, SIZEOF(path), TEXT("%s\\%02d.%02d.%02d"), CrashLogFolder, st.wYear, st.wMonth, st.wDay);
			CreateDirectory(path, NULL);
			mir_sntprintf(path, SIZEOF(path), TEXT("%s\\%02d.%02d.%02d\\crash%02d%02d%02d%02d%02d%02d.mdmp"), CrashLogFolder,
				st.wYear, st.wMonth, st.wDay, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		}
		else
			mir_sntprintf(path, SIZEOF(path), TEXT("%s\\crash%02d%02d%02d%02d%02d%02d.mdmp"), CrashLogFolder,
			st.wYear, st.wMonth, st.wDay, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

		hDumpFile = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hDumpFile != INVALID_HANDLE_VALUE)
			CreateMiniDump(hDumpFile, exc_ptr);
		else if (GetLastError() != ERROR_ALREADY_EXISTS)
			MessageBox(NULL, TranslateT("Crash Report write location is not available"),
			TEXT("Miranda Crash Dumper"), MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_TOPMOST);

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}

	bool empty = GetFileSize(hDumpFile, NULL) == 0;
	CloseHandle(hDumpFile);
	if (empty) DeleteFile(path);

	__try {
		if (dtsubfldr) {
			mir_sntprintf(path, SIZEOF(path), TEXT("%s\\%02d.%02d.%02d"), CrashLogFolder, st.wYear, st.wMonth, st.wDay);
			CreateDirectory(path, NULL);
			mir_sntprintf(path, SIZEOF(path), TEXT("%s\\%02d.%02d.%02d\\crash%02d%02d%02d%02d%02d%02d.txt"), CrashLogFolder,
				st.wYear, st.wMonth, st.wDay, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		}
		else
			mir_sntprintf(path, SIZEOF(path), TEXT("%s\\crash%02d%02d%02d%02d%02d%02d.txt"), CrashLogFolder,
			st.wYear, st.wMonth, st.wDay, st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

		hDumpFile = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

		mir_sntprintf(path, SIZEOF(path), TranslateT("Miranda crashed. Crash report stored in the folder:\n %s\n\n Would you like store it in the clipboard as well?"), CrashLogFolder);

		if (hDumpFile != INVALID_HANDLE_VALUE)
			CreateCrashReport(hDumpFile, exc_ptr, notify ? path : NULL);
	}
	__except (myDebugFilter(GetExceptionCode(), GetExceptionInformation())) {}

	bool empty1 = GetFileSize(hDumpFile, NULL) == 0;
	CloseHandle(hDumpFile);
	if (empty1) DeleteFile(path);
}

LONG WINAPI myfilter(PEXCEPTION_POINTERS exc_ptr)
{
	if (exc_ptr == lastptr) return EXCEPTION_EXECUTE_HANDLER;
	lastptr = exc_ptr;

	myfilterWorker(exc_ptr, true);

	return exchndlr ? ((LPTOP_LEVEL_EXCEPTION_FILTER)exchndlr)(exc_ptr) : EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI myfilterv(PEXCEPTION_POINTERS exc_ptr)
{
	if (0xC0000000L <= exc_ptr->ExceptionRecord->ExceptionCode && 0xC0000500L >= exc_ptr->ExceptionRecord->ExceptionCode) {
		if (exc_ptr == lastptr) return EXCEPTION_EXECUTE_HANDLER;
		lastptr = exc_ptr;

		myfilterWorker(exc_ptr, true);
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

DWORD MirandaThreadFilter(DWORD code, EXCEPTION_POINTERS* info)
{
	if (info != lastptr) {
		lastptr = info;
		myfilterWorker(info, true);
	}
	return threadfltr(code, info);
}

void InitExceptionHandler(void)
{
	threadfltr = Miranda_SetExceptFilter(MirandaThreadFilter);
	SetExceptionHandler();
}

void DestroyExceptionHandler(void)
{
	Miranda_SetExceptFilter(threadfltr);
	RemoveExceptionHandler();
}
