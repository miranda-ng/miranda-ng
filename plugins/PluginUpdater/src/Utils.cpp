/*
Copyright (C) 2010 Mataes

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

HNETLIBUSER hNetlibUser = nullptr;
HANDLE hPipe = nullptr;

/////////////////////////////////////////////////////////////////////////////////////

IconItem iconList[] =
{
	{ LPGEN("Check for updates"),"check_update", IDI_MENU },
	{ LPGEN("Plugin info"), "info", IDI_INFO },
	{ LPGEN("Component list"),"plg_list", IDI_PLGLIST }
};

void InitIcoLib()
{
	g_plugin.registerIcon(MODULEA, iconList);
}

void InitNetlib()
{
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szDescriptiveName.w = TranslateT("Plugin Updater HTTP connections");
	nlu.szSettingsModule = MODULENAME;
	hNetlibUser = Netlib_RegisterUser(&nlu);
}

void UnloadNetlib()
{
	Netlib_CloseHandle(hNetlibUser);
	hNetlibUser = nullptr;
}

int CompareHashes(const ServListEntry *p1, const ServListEntry *p2)
{
	return _wcsicmp(p1->m_name, p2->m_name);
}

bool ParseHashes(const wchar_t *ptszUrl, ptrW &baseUrl, SERVLIST &arHashes)
{
	REPLACEVARSARRAY vars[2];
	vars[0].key.w = L"platform";
	#ifdef _WIN64
		vars[0].value.w = L"64";
	#else
		vars[0].value.w = L"32";
	#endif
	vars[1].key.w = vars[1].value.w = nullptr;

	baseUrl = Utils_ReplaceVarsW(ptszUrl, 0, vars);

	// Download version info
	FILEURL pFileUrl;
	mir_snwprintf(pFileUrl.tszDownloadURL, L"%s/hashes.zip", baseUrl.get());
	mir_snwprintf(pFileUrl.tszDiskPath, L"%s\\hashes.zip", g_tszTempPath);
	pFileUrl.CRCsum = 0;

	HNETLIBCONN nlc = nullptr;
	bool ret = DownloadFile(&pFileUrl, nlc);
	Netlib_CloseHandle(nlc);

	if (!ret) {
		Netlib_LogfW(hNetlibUser, L"Downloading list of available updates from %s failed", baseUrl.get());
		ShowPopup(TranslateT("Plugin Updater"), TranslateT("An error occurred while checking for new updates."), POPUP_TYPE_ERROR);
		Skin_PlaySound("updatefailed");
		return false;
	}

	if (!unzip(pFileUrl.tszDiskPath, g_tszTempPath, nullptr, true)) {
		Netlib_LogfW(hNetlibUser, L"Unzipping list of available updates from %s failed", baseUrl.get());
		ShowPopup(TranslateT("Plugin Updater"), TranslateT("An error occurred while checking for new updates."), POPUP_TYPE_ERROR);
		Skin_PlaySound("updatefailed");
		return false;
	}

	DeleteFile(pFileUrl.tszDiskPath);

	wchar_t tszTmpIni[MAX_PATH];
	mir_snwprintf(tszTmpIni, L"%s\\hashes.txt", g_tszTempPath);
	FILE *fp = _wfopen(tszTmpIni, L"r");
	if (!fp) {
		Netlib_LogfW(hNetlibUser, L"Opening %s failed", g_tszTempPath);
		ShowPopup(TranslateT("Plugin Updater"), TranslateT("An error occurred while checking for new updates."), POPUP_TYPE_ERROR);
		return false;
	}

	bool bDoNotSwitchToStable = false;
	char str[200];
	while (fgets(str, _countof(str), fp) != nullptr) {
		rtrim(str);
		// Do not allow the user to switch back to stable
		if (!strcmp(str, "DoNotSwitchToStable")) {
			bDoNotSwitchToStable = true;
		}
		else if (str[0] != ';') { // ';' marks a comment
			Netlib_Logf(hNetlibUser, "Update: %s", str);
			char *p = strchr(str, ' ');
			if (p != nullptr) {
				*p++ = 0;
				_strlwr(p);

				int dwCrc32;
				char *p1 = strchr(p, ' ');
				if (p1 == nullptr)
					dwCrc32 = 0;
				else {
					*p1++ = 0;
					sscanf(p1, "%08x", &dwCrc32);
				}
				arHashes.insert(new ServListEntry(str, p, dwCrc32));
			}
		}
	}
	fclose(fp);
	DeleteFile(tszTmpIni);

	if (bDoNotSwitchToStable) {
		g_plugin.setByte(DB_SETTING_DONT_SWITCH_TO_STABLE, 1);
		// Reset setting if needed
		int UpdateMode = g_plugin.getByte(DB_SETTING_UPDATE_MODE, UPDATE_MODE_STABLE);
		if (UpdateMode == UPDATE_MODE_STABLE)
			g_plugin.setByte(DB_SETTING_UPDATE_MODE, UPDATE_MODE_TRUNK);
	}
	else
		g_plugin.setByte(DB_SETTING_DONT_SWITCH_TO_STABLE, 0);

	return true;
}


bool DownloadFile(FILEURL *pFileURL, HNETLIBCONN &nlc)
{
	char szMirVer[100];
	Miranda_GetVersionText(szMirVer, _countof(szMirVer));
	if (auto *p = strchr(szMirVer, '('))
		*p = 0;
	rtrim(szMirVer);

	char szOsVer[100];
	OS_GetShortString(szOsVer, _countof(szOsVer));

	CMStringA szUserAgent("Miranda NG/");
	szUserAgent.Append(szMirVer);
	szUserAgent.AppendFormat(" (%s", szOsVer);
	#ifdef _WIN64
		szUserAgent.Append("; Win64; x64");
	#endif
	szUserAgent.Append(")");

	NETLIBHTTPHEADER headers[4] = {
		{ "User-Agent", szUserAgent.GetBuffer() },
		{ "Connection", "close" },
		{ "Cache-Control", "no-cache" },
		{ "Pragma", "no-cache" }
	};

	ptrA szUrl(mir_u2a(pFileURL->tszDownloadURL));

	NETLIBHTTPREQUEST nlhr = {};
	nlhr.cbSize = sizeof(nlhr);
	nlhr.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11 | NLHRF_PERSISTENT;
	nlhr.requestType = REQUEST_GET;
	nlhr.nlc = nlc;
	nlhr.szUrl = szUrl;
	nlhr.headersCount = _countof(headers);
	nlhr.headers = headers;

	for (int i = 0; i < MAX_RETRIES; i++) {
		Netlib_LogfW(hNetlibUser, L"Downloading file %s to %s (attempt %d)", pFileURL->tszDownloadURL, pFileURL->tszDiskPath, i + 1);
		NLHR_PTR pReply(Netlib_HttpTransaction(hNetlibUser, &nlhr));
		if (pReply) {
			nlc = pReply->nlc;
			if ((200 == pReply->resultCode) && (pReply->dataLength > 0)) {
				// Check CRC sum
				if (pFileURL->CRCsum) {
					int crc = crc32(0, (unsigned char *)pReply->pData, pReply->dataLength);
					if (crc != pFileURL->CRCsum) {
						// crc check failed, try again
						Netlib_LogfW(hNetlibUser, L"crc check failed for file %s", pFileURL->tszDiskPath);
						continue;
					}
				}

				DWORD dwBytes;
				HANDLE hFile = CreateFile(pFileURL->tszDiskPath, GENERIC_READ | GENERIC_WRITE, NULL, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
				if (hFile != INVALID_HANDLE_VALUE) {
					// write the downloaded file directly
					WriteFile(hFile, pReply->pData, (DWORD)pReply->dataLength, &dwBytes, nullptr);
					CloseHandle(hFile);
				}
				else {
					// try to write it via PU stub
					wchar_t tszTempFile[MAX_PATH];
					mir_snwprintf(tszTempFile, L"%s\\pulocal.tmp", g_tszTempPath);
					hFile = CreateFile(tszTempFile, GENERIC_READ | GENERIC_WRITE, NULL, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
					if (hFile != INVALID_HANDLE_VALUE) {
						WriteFile(hFile, pReply->pData, (DWORD)pReply->dataLength, &dwBytes, nullptr);
						CloseHandle(hFile);
						SafeMoveFile(tszTempFile, pFileURL->tszDiskPath);
					}
				}
				return true;
			}
			Netlib_LogfW(hNetlibUser, L"Downloading file %s failed with error %d", pFileURL->tszDownloadURL, pReply->resultCode);
		}
		else {
			Netlib_LogfW(hNetlibUser, L"Downloading file %s failed, host is propably temporary down.", pFileURL->tszDownloadURL);
			nlc = nullptr;
		}
	}

	Netlib_LogfW(hNetlibUser, L"Downloading file %s failed, giving up", pFileURL->tszDownloadURL);
	return false;
}

void __stdcall OpenPluginOptions(void*)
{
	g_plugin.openOptions(nullptr, L"Plugins");
}

//   FUNCTION: IsRunAsAdmin()
//
//   PURPOSE: The function checks whether the current process is run as
//   administrator. In other words, it dictates whether the primary access
//   token of the process belongs to user account that is a member of the
//   local Administrators group and it is elevated.
//
//   RETURN VALUE: Returns TRUE if the primary access token of the process
//   belongs to user account that is a member of the local Administrators
//   group and it is elevated. Returns FALSE if the token does not.
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception which
//   contains the Win32 error code of the failure.
//
//   EXAMPLE CALL:
//     try
//     {
//         if (IsRunAsAdmin())
//             wprintf (L"Process is run as administrator\n");
//         else
//             wprintf (L"Process is not run as administrator\n");
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"IsRunAsAdmin failed w/err %lu\n", dwError);
//     }
//
BOOL IsRunAsAdmin()
{
	BOOL fIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = nullptr;

	// Allocate and initialize a SID of the administrators group.
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup)) {
		dwError = GetLastError();
		goto Cleanup;
	}

	// Determine whether the SID of administrators group is bEnabled in
	// the primary access token of the process.
	if (!CheckTokenMembership(nullptr, pAdministratorsGroup, &fIsRunAsAdmin)) {
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

	return fIsRunAsAdmin;
}

//
//   FUNCTION: IsProcessElevated()
//
//   PURPOSE: The function gets the elevation information of the current
//   process. It dictates whether the process is elevated or not. Token
//   elevation is only available on Windows Vista and newer operating
//   systems, thus IsProcessElevated throws a C++ exception if it is called
//   on systems prior to Windows Vista. It is not appropriate to use this
//   function to determine whether a process is run as administartor.
//
//   RETURN VALUE: Returns TRUE if the process is elevated. Returns FALSE if
//   it is not.
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception
//   which contains the Win32 error code of the failure. For example, if
//   IsProcessElevated is called on systems prior to Windows Vista, the error
//   code will be ERROR_INVALID_PARAMETER.
//
//   NOTE: TOKEN_INFORMATION_CLASS provides TokenElevationType to check the
//   elevation type (TokenElevationTypeDefault / TokenElevationTypeLimited /
//   TokenElevationTypeFull) of the process. It is different from
//   TokenElevation in that, when UAC is turned off, elevation type always
//   returns TokenElevationTypeDefault even though the process is elevated
//   (Integrity Level == High). In other words, it is not safe to say if the
//   process is elevated based on elevation type. Instead, we should use
//   TokenElevation.
//
//   EXAMPLE CALL:
//     try
//     {
//         if (IsProcessElevated())
//             wprintf (L"Process is elevated\n");
//         else
//             wprintf (L"Process is not elevated\n");
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"IsProcessElevated failed w/err %lu\n", dwError);
//     }
//
BOOL IsProcessElevated()
{
	BOOL fIsElevated = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	HANDLE hToken = nullptr;

	// Open the primary access token of the process with TOKEN_QUERY.
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		dwError = GetLastError();
		goto Cleanup;
	}

	// Retrieve token elevation information.
	TOKEN_ELEVATION elevation;
	DWORD dwSize;
	if (!GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize)) {
		// When the process is run on operating systems prior to Windows
		// Vista, GetTokenInformation returns FALSE with the
		// ERROR_INVALID_PARAMETER error code because TokenElevation is
		// not supported on those operating systems.
		dwError = GetLastError();
		goto Cleanup;
	}

	fIsElevated = elevation.TokenIsElevated;

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (hToken) {
		CloseHandle(hToken);
		hToken = nullptr;
	}

	// Throw the error if something failed in the function.
	if (ERROR_SUCCESS != dwError) {
		throw dwError;
	}

	return fIsElevated;
}

bool PrepareEscalation()
{
	// First try to create a file near Miranda32.exe
	wchar_t szPath[MAX_PATH];
	GetModuleFileName(nullptr, szPath, _countof(szPath));
	wchar_t *ext = wcsrchr(szPath, '.');
	if (ext != nullptr)
		*ext = '\0';
	wcscat(szPath, L".test");
	HANDLE hFile = CreateFile(szPath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile != INVALID_HANDLE_VALUE) {
		// we are admins or UAC is disable, cool
		CloseHandle(hFile);
		DeleteFile(szPath);
		return true;
	}
	else if (IsRunAsAdmin()) {
		// Check the current process's "run as administrator" status.
		return true;
	}
	else {
		// Elevate the process. Create a pipe for a stub first
		wchar_t tszPipeName[MAX_PATH];
		mir_snwprintf(tszPipeName, L"\\\\.\\pipe\\Miranda_Pu_%d", GetCurrentProcessId());
		hPipe = CreateNamedPipe(tszPipeName, PIPE_ACCESS_DUPLEX, PIPE_READMODE_BYTE | PIPE_WAIT, 1, 1024, 1024, NMPWAIT_USE_DEFAULT_WAIT, nullptr);
		if (hPipe == INVALID_HANDLE_VALUE) {
			hPipe = nullptr;
		}
		else {
			wchar_t cmdLine[100], *p;
			GetModuleFileName(nullptr, szPath, ARRAYSIZE(szPath));
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
				if (hPipe != nullptr)
					ConnectNamedPipe(hPipe, nullptr);
				return true;
			}

			DWORD dwError = GetLastError();
			if (dwError == ERROR_CANCELLED) {
				// The user refused to allow privileges elevation.
				// Do nothing ...
			}
		}
		return false;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

int TransactPipe(int opcode, const wchar_t *p1, const wchar_t *p2)
{
	BYTE buf[1024];
	DWORD l1 = lstrlen(p1), l2 = lstrlen(p2);
	if (l1 > MAX_PATH || l2 > MAX_PATH)
		return 0;

	*(DWORD *)buf = opcode;
	wchar_t *dst = (wchar_t *)&buf[sizeof(DWORD)];
	lstrcpy(dst, p1);
	dst += l1 + 1;
	if (p2) {
		lstrcpy(dst, p2);
		dst += l2 + 1;
	}
	else *dst++ = 0;

	DWORD dwBytes = 0, dwError;
	if (WriteFile(hPipe, buf, (DWORD)((BYTE *)dst - buf), &dwBytes, nullptr) == 0)
		return 0;

	dwError = 0;
	if (ReadFile(hPipe, &dwError, sizeof(DWORD), &dwBytes, nullptr) == 0) return 0;
	if (dwBytes != sizeof(DWORD)) return 0;

	return dwError == ERROR_SUCCESS;
}

int SafeCopyFile(const wchar_t *pSrc, const wchar_t *pDst)
{
	if (hPipe == nullptr)
		return CopyFile(pSrc, pDst, FALSE);

	return TransactPipe(1, pSrc, pDst);
}

int SafeMoveFile(const wchar_t *pSrc, const wchar_t *pDst)
{
	if (hPipe == nullptr) {
		DeleteFile(pDst);
		if (MoveFile(pSrc, pDst) == 0) // use copy on error
			CopyFile(pSrc, pDst, FALSE);
		DeleteFile(pSrc);
	}

	return TransactPipe(2, pSrc, pDst);
}

int SafeDeleteFile(const wchar_t *pFile)
{
	if (hPipe == nullptr)
		return DeleteFile(pFile);

	return TransactPipe(3, pFile, nullptr);
}

int SafeCreateDirectory(const wchar_t *pFolder)
{
	if (hPipe == nullptr)
		return CreateDirectoryTreeW(pFolder);

	return TransactPipe(4, pFolder, nullptr);
}

int SafeCreateFilePath(const wchar_t *pFolder)
{
	if (hPipe == nullptr) {
		CreatePathToFileW(pFolder);
		return 0;
	}

	return TransactPipe(5, pFolder, nullptr);
}

void BackupFile(wchar_t *ptszSrcFileName, wchar_t *ptszBackFileName)
{
	SafeCreateFilePath(ptszBackFileName);
	SafeMoveFile(ptszSrcFileName, ptszBackFileName);
}

/////////////////////////////////////////////////////////////////////////////////////////

char *StrToLower(char *str)
{
	for (int i = 0; str[i]; i++)
		str[i] = tolower(str[i]);

	return str;
}
