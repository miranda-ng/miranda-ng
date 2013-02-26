#define _CRT_SECURE_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <varargs.h>
#include <stdio.h>

// Global Variables:
HINSTANCE hInst;								// current instance

void log(const TCHAR *tszFormat, ...)
{
	#if defined(_DEBUG)
		FILE *out = fopen("c:\\temp\\pu.log", "a");
		if (out) {
			va_list params;
			va_start(params, tszFormat);
			_vftprintf(out, tszFormat, params);
			va_end(params);
			fputc('\n', out);
			fclose(out);
		}
	#endif
}

int CreateDirectoryTreeW(const WCHAR* szDir)
{
	DWORD  dwAttributes;
	WCHAR* pszLastBackslash, szTestDir[ MAX_PATH ];

	lstrcpynW(szTestDir, szDir, MAX_PATH);
	if ((dwAttributes = GetFileAttributesW(szTestDir)) != INVALID_FILE_ATTRIBUTES && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return 0;

	pszLastBackslash = wcsrchr(szTestDir, '\\');
	if (pszLastBackslash == NULL)
		return 0;

	*pszLastBackslash = '\0';
	CreateDirectoryTreeW(szTestDir);
	*pszLastBackslash = '\\';
	return (CreateDirectoryW(szTestDir, NULL) == 0) ? GetLastError() : 0;
}

void CreatePathToFileW(WCHAR* wszFilePath)
{
	WCHAR* pszLastBackslash = wcsrchr(wszFilePath, '\\');
	if (pszLastBackslash == NULL)
		return;

	*pszLastBackslash = '\0';
	CreateDirectoryTreeW(wszFilePath);
	*pszLastBackslash = '\\';
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpCmdLine, int)
{
	DWORD dwError;
	hInst = hInstance;

	TCHAR tszPipeName[MAX_PATH];
	#if _MSC_VER < 1400
		_stprintf(tszPipeName,  _T("\\\\.\\pipe\\Miranda_Pu_%s"), lpCmdLine);
	#else
		_stprintf_s(tszPipeName, MAX_PATH, _T("\\\\.\\pipe\\Miranda_Pu_%s"), lpCmdLine);
	#endif
	log( L"Opening pipe %s...", tszPipeName);
	HANDLE hPipe = CreateFile(tszPipeName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hPipe == INVALID_HANDLE_VALUE) {
		dwError = GetLastError();
		log( L"Failed to open a pipe: error %d", dwError);
		return dwError;
	}

	log( L"Entering the reading cycle...");

	BYTE szReadBuffer[1024];
	DWORD dwBytes;
	while ( ReadFile(hPipe, szReadBuffer, sizeof(szReadBuffer), &dwBytes, NULL)) {
		DWORD dwAction = *(DWORD*)szReadBuffer;
		TCHAR *ptszFile1 = (TCHAR*)(szReadBuffer + sizeof(DWORD));
		TCHAR *ptszFile2 = ptszFile1 + _tcslen(ptszFile1)+1;
		log( L"Received command: %d <%s> <%s>", dwAction, ptszFile1, ptszFile2);
		switch(dwAction) {
		case 1:  // copy
			dwError = CopyFile(ptszFile1, ptszFile2, FALSE);
			break;

		case 2: // move
			DeleteFile(ptszFile2);
			if ( MoveFile(ptszFile1, ptszFile2) == 0) // use copy on error
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

		WriteFile(hPipe, &dwError, sizeof(DWORD), &dwBytes, NULL);
	}

	dwError = GetLastError();
	log( L"Pipe is closed (%d), exiting", dwError);
	CloseHandle(hPipe);
	return 0;
}
