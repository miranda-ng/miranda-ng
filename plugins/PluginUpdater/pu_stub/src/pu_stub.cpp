
#include "stdafx.h"

#include "..\..\build\appstub\appstub.cpp"
#pragma comment(lib, "delayimp.lib")

void log(const wchar_t *tszFormat, ...)
{
	#if defined(_DEBUG)
	FILE *out = fopen("c:\\temp\\pu.log", "a");
	if (out) {
		va_list params;
		va_start(params, tszFormat);
		vfwprintf(out, tszFormat, params);
		va_end(params);
		fputc('\n', out);
		fclose(out);
	}
	#endif
}

int CreateDirectoryTreeW(const wchar_t* szDir)
{
	wchar_t szTestDir[MAX_PATH];
	lstrcpynW(szTestDir, szDir, MAX_PATH);

	DWORD dwAttributes = GetFileAttributesW(szTestDir);
	if (dwAttributes != INVALID_FILE_ATTRIBUTES && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return 0;

	wchar_t *pszLastBackslash = wcsrchr(szTestDir, '\\');
	if (pszLastBackslash == nullptr)
		return 0;

	*pszLastBackslash = '\0';
	CreateDirectoryTreeW(szTestDir);
	*pszLastBackslash = '\\';
	return (CreateDirectoryW(szTestDir, nullptr) == 0) ? GetLastError() : 0;
}

void CreatePathToFileW(wchar_t *wszFilePath)
{
	wchar_t* pszLastBackslash = wcsrchr(wszFilePath, '\\');
	if (pszLastBackslash == nullptr)
		return;

	*pszLastBackslash = '\0';
	CreateDirectoryTreeW(wszFilePath);
	*pszLastBackslash = '\\';
}

int APIENTRY wWinMain(HINSTANCE /*hInstance*/, HINSTANCE, LPTSTR lpCmdLine, int)
{
	DWORD dwError;

	wchar_t tszPipeName[MAX_PATH];
	#if _MSC_VER < 1400
	swprintf(tszPipeName, L"\\\\.\\pipe\\Miranda_Pu_%s", lpCmdLine);
	#else
	swprintf_s(tszPipeName, L"\\\\.\\pipe\\Miranda_Pu_%s", lpCmdLine);
	#endif
	log(L"Opening pipe %s...", tszPipeName);
	HANDLE hPipe = CreateFile(tszPipeName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hPipe == INVALID_HANDLE_VALUE) {
		dwError = GetLastError();
		log(L"Failed to open a pipe: error %d", dwError);
		return dwError;
	}

	log(L"Entering the reading cycle...");

	BYTE szReadBuffer[1024] = { 0 };
	DWORD dwBytes;
	while (ReadFile(hPipe, szReadBuffer, sizeof(szReadBuffer), &dwBytes, nullptr)) {
		DWORD dwAction = *(DWORD*)szReadBuffer;
		wchar_t *ptszFile1 = (wchar_t*)(szReadBuffer + sizeof(DWORD));
		wchar_t *ptszFile2 = ptszFile1 + wcslen(ptszFile1) + 1;
		log(L"Received command: %d <%s> <%s>", dwAction, ptszFile1, ptszFile2);
		switch (dwAction) {
		case 1:  // copy
			dwError = CopyFile(ptszFile1, ptszFile2, FALSE);
			break;

		case 2: // move
			DeleteFile(ptszFile2);
			if (MoveFile(ptszFile1, ptszFile2) == 0) // use copy on error
				dwError = CopyFile(ptszFile1, ptszFile2, FALSE);
			else
				dwError = 0;
			DeleteFile(ptszFile1);
			break;

		case 3: // erase
			dwError = DeleteFile(ptszFile1);
			break;

		case 4: // create dir														  
			dwError = CreateDirectoryTreeW(ptszFile1);
			break;

		case 5: // create path to file
			CreatePathToFileW(ptszFile1);
			dwError = 0;
			break;

		default:
			dwError = ERROR_UNKNOWN_FEATURE;
		}

		WriteFile(hPipe, &dwError, sizeof(DWORD), &dwBytes, nullptr);
	}

	dwError = GetLastError();
	log(L"Pipe is closed (%d), exiting", dwError);
	CloseHandle(hPipe);
	return 0;
}
