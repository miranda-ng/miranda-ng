/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

/////////////////////////////////////////////////////////////////////////////////////////
// pu_stub.exe interface

#include "stdafx.h"

static HANDLE g_hPipe = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////
// are we running with admin priviledges?

static bool IsRunAsAdmin()
{
	BOOL bIsRunAsAdmin = false;
	uint32_t dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = nullptr;

	// Allocate and initialize a SID of the administrators group.
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup)) {
		dwError = GetLastError();
		goto Cleanup;
	}

	// Determine whether the SID of administrators group is bEnabled in
	// the primary access token of the process.
	if (!CheckTokenMembership(nullptr, pAdministratorsGroup, &bIsRunAsAdmin)) {
		dwError = GetLastError();
		goto Cleanup;
	}

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (pAdministratorsGroup) {
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = nullptr;
	}

	// Throw the error if something failed in the function.
	if (ERROR_SUCCESS != dwError) {
		throw dwError;
	}

	return bIsRunAsAdmin != 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Checks if we're working via pu_stub or not

MIR_APP_DLL(bool) PU::IsDirect()
{
	return g_hPipe == nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Checks if Miranda's folder is writeable

MIR_APP_DLL(bool) PU::IsMirandaFolderWritable()
{
	if (!IsWinVerVistaPlus())
		return true;

	wchar_t wszPath[MAX_PATH];
	GetModuleFileNameW(nullptr, wszPath, _countof(wszPath));
	wchar_t *ext = wcsrchr(wszPath, '.');
	if (ext != nullptr)
		*ext = '\0';
	wcscat(wszPath, L".test");
	HANDLE hFile = CreateFileW(wszPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	CloseHandle(hFile);
	DeleteFileW(wszPath);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Checks if a process has enough rights to write into Miranda's folder

MIR_APP_DLL(bool) PU::IsProcessElevated()
{
	bool bIsElevated = false;
	HANDLE hToken = nullptr;

	// Open the primary access token of the process with TOKEN_QUERY.
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		goto Cleanup;

	// Retrieve token elevation information.
	TOKEN_ELEVATION elevation;
	DWORD dwSize;
	if (!GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize)) {
		// When the process is run on operating systems prior to Windows
		// Vista, GetTokenInformation returns FALSE with the
		// ERROR_INVALID_PARAMETER error code because TokenElevation is
		// not supported on those operating systems.
		goto Cleanup;
	}

	bIsElevated = elevation.TokenIsElevated != 0;

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (hToken)
		CloseHandle(hToken);

	return bIsElevated;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Launches pu_stub.exe with elevated priviledges if needed

MIR_APP_DLL(bool) PU::PrepareEscalation(const wchar_t *pwszFile)
{
	CMStringW wszFilePath;
	// First try to create a file near Miranda32.exe
	if (pwszFile == nullptr) {
		wchar_t szPath[MAX_PATH];
		GetModuleFileName(nullptr, szPath, _countof(szPath));
		wchar_t *ext = wcsrchr(szPath, '.');
		if (ext != nullptr)
			*ext = '\0';
		wszFilePath = szPath;
	}
	else wszFilePath = pwszFile;

	wszFilePath.Append(L".test");

	HANDLE hFile = CreateFileW(wszFilePath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE) {
		// we are admins or UAC is disable, cool
		CloseHandle(hFile);
		DeleteFileW(wszFilePath);
		return true;
	}

	// Check the current process's "run as administrator" status.
	if (IsRunAsAdmin())
		return true;

	// if pipe already opened?
	if (g_hPipe != nullptr)
		return true;

	// Elevate the process. Create a pipe for a stub first
	wchar_t wzPipeName[MAX_PATH];
	mir_snwprintf(wzPipeName, L"\\\\.\\pipe\\Miranda_Pu_%d", GetCurrentProcessId());
	g_hPipe = CreateNamedPipe(wzPipeName, PIPE_ACCESS_DUPLEX, PIPE_READMODE_BYTE | PIPE_WAIT, 1, 1024, 1024, NMPWAIT_USE_DEFAULT_WAIT, nullptr);
	if (g_hPipe == INVALID_HANDLE_VALUE) {
		g_hPipe = nullptr;
	}
	else {
		wchar_t cmdLine[100], *p;
		wchar_t szPath[MAX_PATH];
		GetModuleFileName(nullptr, szPath, _countof(szPath));
		if ((p = wcsrchr(szPath, '\\')) != nullptr)
			wcscpy(p + 1, L"pu_stub.exe");
		mir_snwprintf(cmdLine, L"%d", GetCurrentProcessId());

		// Launch a stub
		SHELLEXECUTEINFO sei = { sizeof(sei) };
		sei.lpVerb = L"runas";
		sei.lpFile = szPath;
		sei.lpParameters = cmdLine;
		sei.hwnd = nullptr;
		sei.nShow = SW_NORMAL;
		if (ShellExecuteEx(&sei)) {
			if (g_hPipe != nullptr)
				ConnectNamedPipe(g_hPipe, nullptr);
			return true;
		}

		uint32_t dwError = GetLastError();
		if (dwError == ERROR_CANCELLED) {
			// The user refused to allow privileges elevation.
			// Do nothing ...
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int TransactPipe(int opcode, const wchar_t *p1, const wchar_t *p2)
{
	uint8_t buf[1024];
	uint32_t l1 = lstrlen(p1), l2 = lstrlen(p2);
	if (l1 > MAX_PATH || l2 > MAX_PATH)
		return ERROR_BAD_ARGUMENTS;

	*(uint32_t *)buf = opcode;
	wchar_t *dst = (wchar_t *)&buf[sizeof(uint32_t)];
	lstrcpy(dst, p1);
	dst += l1 + 1;
	if (p2) {
		lstrcpy(dst, p2);
		dst += l2 + 1;
	}
	else *dst++ = 0;

	DWORD dwBytes = 0, dwError;
	if (!WriteFile(g_hPipe, buf, (uint32_t)((uint8_t *)dst - buf), &dwBytes, nullptr))
		return GetLastError();

	dwError = 0;
	if (!ReadFile(g_hPipe, &dwError, sizeof(uint32_t), &dwBytes, nullptr))
		return GetLastError();
	if (dwBytes != sizeof(uint32_t))
		return ERROR_BAD_ARGUMENTS;

	return dwError;
}

MIR_APP_DLL(int) PU::SafeCopyFile(const wchar_t *pSrc, const wchar_t *pDst)
{
	if (g_hPipe == nullptr)
		return CopyFileW(pSrc, pDst, FALSE);

	return TransactPipe(1, pSrc, pDst);
}

MIR_APP_DLL(int) PU::SafeMoveFile(const wchar_t *pSrc, const wchar_t *pDst)
{
	if (g_hPipe == nullptr) {
		if (!DeleteFileW(pDst)) {
			uint32_t dwError = GetLastError();
			if (dwError != ERROR_ACCESS_DENIED && dwError != ERROR_FILE_NOT_FOUND)
				return dwError;
		}

		if (!MoveFileW(pSrc, pDst)) { // use copy on error
			switch (uint32_t dwError = GetLastError()) {
			case ERROR_ALREADY_EXISTS:
			case ERROR_FILE_NOT_FOUND:
				return 0; // this file was included into many archives, so Miranda tries to move it again & again

			case ERROR_ACCESS_DENIED:
			case ERROR_SHARING_VIOLATION:
			case ERROR_LOCK_VIOLATION:
				// use copy routine if a move operation isn't available
				// for example, when files are on different disks
				if (!CopyFileW(pSrc, pDst, FALSE))
					return GetLastError();

				if (!DeleteFileW(pSrc))
					return GetLastError();
				break;

			default:
				return dwError;
			}
		}

		return ERROR_SUCCESS;
	}

	return TransactPipe(2, pSrc, pDst);
}

MIR_APP_DLL(int) PU::SafeDeleteFile(const wchar_t *pwszFile)
{
	if (g_hPipe == nullptr)
		return DeleteFileW(pwszFile);

	return TransactPipe(3, pwszFile, nullptr);
}

MIR_APP_DLL(int) PU::SafeRecycleBin(const wchar_t *pwszFile)
{
	if (g_hPipe == nullptr) {
		CMStringW tmpPath(pwszFile);
		tmpPath.AppendChar(0);

		SHFILEOPSTRUCT shfo = {};
		shfo.wFunc = FO_DELETE;
		shfo.pFrom = tmpPath;
		shfo.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT | FOF_ALLOWUNDO;
		return SHFileOperation(&shfo);
	}

	return TransactPipe(7, pwszFile, nullptr);
}

MIR_APP_DLL(int) PU::SafeCreateDirectory(const wchar_t *pwszFolder)
{
	if (g_hPipe == nullptr)
		return CreateDirectoryTreeW(pwszFolder);

	return TransactPipe(4, pwszFolder, nullptr);
}

MIR_APP_DLL(int) PU::SafeDeleteDirectory(const wchar_t *pwszDirName)
{
	if (g_hPipe == nullptr)
		return DeleteDirectoryTreeW(pwszDirName);

	return TransactPipe(6, pwszDirName, nullptr);
}

MIR_APP_DLL(int) PU::SafeCreateFilePath(const wchar_t *pwszFolder)
{
	if (g_hPipe == nullptr) {
		CreatePathToFileW(pwszFolder);
		return 0;
	}

	return TransactPipe(5, pwszFolder, nullptr);
}
