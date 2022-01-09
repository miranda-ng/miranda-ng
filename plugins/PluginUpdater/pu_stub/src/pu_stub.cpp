
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
	wchar_t szTestDir[MAX_PATH+1];
	if (lstrcpynW(szTestDir, szDir, MAX_PATH) == nullptr)
		szTestDir[MAX_PATH] = 0;

	uint32_t dwAttributes = GetFileAttributesW(szTestDir);
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

int DeleteDirectoryTreeW(const wchar_t *pwszDirName)
{
	// file name shall be double sero ended
	wchar_t wszPath[MAX_PATH + 2];
	if (lstrcpynW(wszPath, pwszDirName, MAX_PATH) == nullptr)
		wszPath[MAX_PATH] = 0;
	wszPath[lstrlenW(wszPath) + 1] = 0;

	SHFILEOPSTRUCTW file_op = {
		NULL,
		FO_DELETE,
		wszPath,
		L"",
		FOF_NOCONFIRMATION |
		FOF_NOERRORUI |
		FOF_SILENT,
		false,
		0,
		L"" };
	return SHFileOperationW(&file_op);
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
	uint32_t dwError;

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

	uint8_t szReadBuffer[1024] = { 0 };
	DWORD dwBytes;
	while (ReadFile(hPipe, szReadBuffer, sizeof(szReadBuffer), &dwBytes, nullptr)) {
		uint32_t dwAction = *(uint32_t*)szReadBuffer;
		wchar_t *ptszFile1 = (wchar_t*)(szReadBuffer + sizeof(uint32_t));
		wchar_t *ptszFile2 = ptszFile1 + wcslen(ptszFile1) + 1;
		dwError = 0;
		log(L"Received command: %d <%s> <%s>", dwAction, ptszFile1, ptszFile2);
		switch (dwAction) {
		case 1:  // copy
			if (!CopyFileW(ptszFile1, ptszFile2, FALSE))
				dwError = GetLastError();
			break;

		case 2: // move
			if (!DeleteFileW(ptszFile2)) {
				uint32_t err = GetLastError();
				if (err != ERROR_ACCESS_DENIED && err != ERROR_FILE_NOT_FOUND) {
					dwError = err;
					break;
				}
			}
			
			if (!MoveFileW(ptszFile1, ptszFile2)) { // use copy on error
				switch (uint32_t err = GetLastError()) {
				case ERROR_ALREADY_EXISTS:
				case ERROR_FILE_NOT_FOUND:
					dwError = 0;
					break; // this file was included into many archives, so Miranda tries to move it again & again

				case ERROR_ACCESS_DENIED:
				case ERROR_SHARING_VIOLATION:
				case ERROR_LOCK_VIOLATION:
					// use copy routine if a move operation isn't available
					// for example, when files are on different disks
					if (!CopyFileW(ptszFile1, ptszFile2, FALSE))
						dwError = GetLastError();

					if (!DeleteFileW(ptszFile1))
						dwError = GetLastError();
					break;

				default:
					dwError = err;
					break;
				}
			}
			break;

		case 3: // erase
			if (!DeleteFileW(ptszFile1))
				dwError = GetLastError();
			break;

		case 4: // create dir														  
			dwError = CreateDirectoryTreeW(ptszFile1);
			break;

		case 5: // create path to file
			CreatePathToFileW(ptszFile1);
			dwError = 0;
			break;

		case 6: // delete folder recursively
			DeleteDirectoryTreeW(ptszFile1);
			dwError = 0;
			break;

		case 7:
			{
				wchar_t tmpPath[MAX_PATH+2];
				_snwprintf_s(tmpPath, _countof(tmpPath), L"%s%c%c", ptszFile1, 0, 0);

				SHFILEOPSTRUCTW shfo = {};
				shfo.wFunc = FO_DELETE;
				shfo.pFrom = tmpPath;
				shfo.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT | FOF_ALLOWUNDO;
				dwError = SHFileOperationW(&shfo);
			}
			break;

		default:
			dwError = ERROR_UNKNOWN_FEATURE;
		}

		WriteFile(hPipe, &dwError, sizeof(uint32_t), &dwBytes, nullptr);
	}

	dwError = GetLastError();
	log(L"Pipe is closed (%d), exiting", dwError);
	CloseHandle(hPipe);
	return 0;
}
