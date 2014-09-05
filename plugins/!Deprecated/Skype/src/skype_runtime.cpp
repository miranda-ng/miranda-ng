#include "skype.h"

#include "aes\aes.h" 

#include "..\..\..\..\skypekit\key.h"

char *CSkypeProto::LoadKeyPair()
{
	HRSRC hResource = FindResource(g_hInstance, MAKEINTRESOURCE(IDR_KEY), L"BIN");
	if (hResource == NULL)
		return NULL;

	HGLOBAL hLoadedResource = LoadResource(g_hInstance, hResource);
	if (hLoadedResource == NULL)
		return NULL;

	LPVOID pLockedResource = LockResource(hLoadedResource);
	if (pLockedResource == NULL)
		return NULL;

	int length = ::SizeofResource(g_hInstance, hResource);
	if (length == 0)
		return NULL;

	char* pData = (char*)_alloca(length + 1);
	::memcpy(pData, pLockedResource, length);
	pData[length] = 0;

	unsigned decodedLen;
	mir_ptr<BYTE> tmpD((BYTE*)::mir_base64_decode(pData, &decodedLen));
	BYTE *result = (BYTE*)::mir_alloc(decodedLen+17);

	aes_context ctx;
	char *key = (char*)::mir_base64_decode(MY_KEY, NULL);
	::aes_set_key(&ctx, (BYTE*)key, 128);
	::mir_free(key);

	for (unsigned i = 0; i < decodedLen; i += 16)
		aes_decrypt(&ctx, &tmpD[i], &result[i]);

	result[decodedLen] = 0;
	return (char *)result;
}

int CSkypeProto::StartSkypeRuntime(const wchar_t *profileName)
{
	STARTUPINFO cif = {0};
	cif.cb = sizeof(STARTUPINFO);
	cif.dwFlags = STARTF_USESHOWWINDOW;
	cif.wShowWindow = SW_HIDE;	

	wchar_t	fileName[MAX_PATH];
	::GetModuleFileName(g_hInstance, fileName, MAX_PATH);

	wchar_t *skypeKitPath = ::wcsrchr(fileName, '\\');
	if (skypeKitPath != NULL)
		*skypeKitPath = 0;
	::mir_snwprintf(fileName, SIZEOF(fileName), L"%s\\%s", fileName, L"SkypeKit.exe");

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	// todo: rework
	HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (::Process32First(snapshot, &entry) == TRUE)
	{
		while (::Process32Next(snapshot, &entry) == TRUE)
		{
			if (::wcsicmp(entry.szExeFile, L"SkypeKit.exe") == 0 || ::wcsicmp(entry.szExeFile, L"Skype.exe") == 0)
			{
				this->skypeKitPort += rand() % 1000 + 8963;
				break;
			}
		}
	}
	::CloseHandle(snapshot);

	wchar_t param[128];
	VARST dbPath( _T("%miranda_userdata%\\SkypeKit"));
	::mir_snwprintf(param, SIZEOF(param), L"-p -P %d -f \"%s\"", this->skypeKitPort, dbPath);
	int startingrt = ::CreateProcess(
		fileName, param,
		NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE,
		NULL, NULL, &cif, &this->skypeKitProcessInfo);

	return startingrt;
}

BOOL CSkypeProto::SafeTerminateProcess(HANDLE hProcess, UINT uExitCode)
{
	DWORD dwTID, dwCode, dwErr = 0;
	HANDLE hProcessDup = INVALID_HANDLE_VALUE;
	HANDLE hRT = NULL;
	HINSTANCE hKernel = ::GetModuleHandle(L"Kernel32");
	BOOL bSuccess = FALSE;

	BOOL bDup = ::DuplicateHandle(
		::GetCurrentProcess(),
		hProcess,
		GetCurrentProcess(),
		&hProcessDup,
		PROCESS_ALL_ACCESS,
		FALSE, 
		0);

	// Detect the special case where the process is 
	// already dead...
	if (::GetExitCodeProcess((bDup) ? hProcessDup : hProcess, &dwCode) && (dwCode == STILL_ACTIVE))
	{
		FARPROC pfnExitProc;

		pfnExitProc = GetProcAddress(hKernel, "ExitProcess");

		hRT = ::CreateRemoteThread(
			(bDup) ? hProcessDup : hProcess, 
			NULL, 
			0, 
			(LPTHREAD_START_ROUTINE)pfnExitProc,
			(PVOID)uExitCode, 0, &dwTID);

		if ( hRT == NULL )
			dwErr = GetLastError();
	}
	else
		dwErr = ERROR_PROCESS_ABORTED;

	if (hRT)
	{
		// Must wait process to terminate to 
		// guarantee that it has exited...
		::WaitForSingleObject((bDup) ? hProcessDup : hProcess, INFINITE);

		::CloseHandle(hRT);
		bSuccess = TRUE;
	}

	if ( bDup )
		::CloseHandle(hProcessDup);

	if ( !bSuccess )
		::SetLastError(dwErr);

	return bSuccess;
}

void CSkypeProto::StopSkypeRuntime()
{
	//DWORD dwExitCode = 0;
	//this->SafeTerminateProcess(this->skypeKitProcessInfo.hProcess, 0);
	//::PostThreadMessage(this->skypeKitProcessInfo.dwThreadId, WM_CLOSE, 0, 0);
	//::WaitForSingleObject(this->skypeKitProcessInfo.hProcess, 1500);

	DWORD dwExitCode = 0;
	::GetExitCodeProcess(this->skypeKitProcessInfo.hProcess, &dwExitCode);
	if (dwExitCode == STILL_ACTIVE)
		//::TerminateProcess(this->skypeKitProcessInfo.hProcess, 0); // Zero is the exit code
		this->SafeTerminateProcess(this->skypeKitProcessInfo.hProcess, 0);

	::CloseHandle(this->skypeKitProcessInfo.hThread);
	::CloseHandle(this->skypeKitProcessInfo.hProcess);
}
