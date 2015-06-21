/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "stdafx.h"

static HANDLE hPipe = NULL;

BOOL IsRunAsAdmin()
{
	BOOL bIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = NULL;

	// Allocate and initialize a SID of the administrators group.
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup)) {
		dwError = GetLastError();
		goto Cleanup;
	}

	// Determine whether the SID of administrators group is bEnabled in
	// the primary access token of the process.
	if (!CheckTokenMembership(NULL, pAdministratorsGroup, &bIsRunAsAdmin)) {
		dwError = GetLastError();
		goto Cleanup;
	}

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (pAdministratorsGroup) {
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}

	return bIsRunAsAdmin;
}

bool PrepareEscalation()
{
	// First try to create a file near Miranda32.exe
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, _countof(szPath));
	TCHAR *ext = _tcsrchr(szPath, '.');
	if (ext != NULL)
		*ext = '\0';
	_tcscat(szPath, _T(".test"));
	HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		// we are admins or UAC is disable, cool
		CloseHandle(hFile);
		DeleteFile(szPath);
		return true;
	}
	
	// Check the current process's "run as administrator" status.
	if (IsRunAsAdmin())
		return true;

	// Elevate the process. Create a pipe for a stub first
	TCHAR tszPipeName[MAX_PATH];
	_sntprintf(tszPipeName, _countof(tszPipeName), _T("\\\\.\\pipe\\Miranda_Pu_%d"), GetCurrentProcessId());
	hPipe = CreateNamedPipe(tszPipeName, PIPE_ACCESS_DUPLEX, PIPE_READMODE_BYTE | PIPE_WAIT, 1, 1024, 1024, NMPWAIT_USE_DEFAULT_WAIT, NULL);
	if (hPipe != INVALID_HANDLE_VALUE) {
		TCHAR cmdLine[100], *p;
		GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath));
		if ((p = _tcsrchr(szPath, '\\')) != 0)
			_tcscpy(p + 1, _T("pu_stub.exe"));
		_sntprintf(cmdLine, _countof(cmdLine), _T("%d"), GetCurrentProcessId());

		// Launch a stub
		SHELLEXECUTEINFO sei = { 0 };
		sei.cbSize = sizeof(sei);
		sei.lpVerb = L"runas";
		sei.lpFile = szPath;
		sei.lpParameters = cmdLine;
		sei.hwnd = NULL;
		sei.nShow = SW_NORMAL;
		if (ShellExecuteEx(&sei)) {
			ConnectNamedPipe(hPipe, NULL);
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

int TransactPipe(int opcode, const TCHAR *p1, const TCHAR *p2)
{
	BYTE buf[1024];
	DWORD l1 = lstrlen(p1), l2 = lstrlen(p2);
	if (l1 > MAX_PATH || l2 > MAX_PATH)
		return 0;

	*(DWORD*)buf = opcode;
	TCHAR *dst = (TCHAR*)&buf[sizeof(DWORD)];
	lstrcpy(dst, p1);
	dst += l1 + 1;
	if (p2) {
		lstrcpy(dst, p2);
		dst += l2 + 1;
	}
	else *dst++ = 0;

	DWORD dwBytes = 0, dwError;
	if (WriteFile(hPipe, buf, (DWORD)((BYTE*)dst - buf), &dwBytes, NULL) == 0)
		return 0;

	dwError = 0;
	if (ReadFile(hPipe, &dwError, sizeof(DWORD), &dwBytes, NULL) == 0) return 0;
	if (dwBytes != sizeof(DWORD)) return 0;

	return dwError == ERROR_SUCCESS;
}

int SafeCopyFile(const TCHAR *pSrc, const TCHAR *pDst)
{
	if (hPipe == NULL)
		return CopyFile(pSrc, pDst, FALSE);

	return TransactPipe(1, pSrc, pDst);
}

int SafeMoveFile(const TCHAR *pSrc, const TCHAR *pDst)
{
	if (hPipe == NULL) {
		DeleteFile(pDst);
		if (MoveFile(pSrc, pDst) == 0) // use copy on error
			CopyFile(pSrc, pDst, FALSE);
		DeleteFile(pSrc);
	}

	return TransactPipe(2, pSrc, pDst);
}

int SafeDeleteFile(const TCHAR *pFile)
{
	if (hPipe == NULL)
		return DeleteFile(pFile);

	return TransactPipe(3, pFile, NULL);
}

int SafeCreateDirectory(const TCHAR *pFolder)
{
	if (hPipe == NULL)
		return CreateDirectory(pFolder, NULL);

	return TransactPipe(4, pFolder, NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool TryDeleteFile(const TCHAR *ptszFileName)
{
	for (int i = 0; i < 5; i++) {
		if (SafeDeleteFile(ptszFileName))
			return true;

		switch (GetLastError()) {
		case ERROR_ACCESS_DENIED:
		case ERROR_SHARING_VIOLATION:
		case ERROR_LOCK_VIOLATION:
			break;

		default:
			return false;
		}

		Sleep(200);
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

static TCHAR *arDlls[] = 
{
	_T("mir_app.dll"), _T("mir_core.dll"), _T("pcre16.dll"), _T("zlib.dll"), _T("libtox.dll"),
	_T("libs\\mir_app.dll"), _T("libs\\mir_core.dll"), _T("libs\\libjson.dll"), _T("libs\\pcre16.dll"), _T("libs\\zlib.dll")
};

bool CheckDlls()
{
	bool bInit = false;

	for (int i = 0; i < _countof(arDlls); i++) {
		// if dll is missing - skip it
		if (_taccess(arDlls[i], 0) != 0)
			continue;

		// there's smth to delete. init UAC
		if (!bInit) {
			// failed? then we need UAC
			if (!PrepareEscalation())
				return false;

			bInit = true;
		}

		if (!TryDeleteFile(arDlls[i]))
			return false;
	}

	return true;
}
