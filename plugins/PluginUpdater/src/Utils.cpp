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

HANDLE hNetlibUser = NULL, hPipe = NULL;

/////////////////////////////////////////////////////////////////////////////////////
void LoadOptions()
{
	PopupOptions.DefColors = db_get_b(NULL, MODNAME, "DefColors", DEFAULT_COLORS);
	PopupOptions.LeftClickAction= db_get_b(NULL, MODNAME, "LeftClickAction", DEFAULT_POPUP_LCLICK);
	PopupOptions.RightClickAction = db_get_b(NULL, MODNAME, "RightClickAction", DEFAULT_POPUP_RCLICK);
	PopupOptions.Timeout = db_get_dw(NULL, MODNAME, "Timeout", DEFAULT_TIMEOUT_VALUE);

	opts.bUpdateOnStartup = db_get_b(NULL, MODNAME, "UpdateOnStartup", DEFAULT_UPDATEONSTARTUP);
	opts.bOnlyOnceADay = db_get_b(NULL, MODNAME, "OnlyOnceADay", DEFAULT_ONLYONCEADAY);
	opts.bUpdateOnPeriod = db_get_b(NULL, MODNAME, "UpdateOnPeriod", DEFAULT_UPDATEONPERIOD);
	opts.Period = db_get_dw(NULL, MODNAME, "Period", DEFAULT_PERIOD);
	opts.bPeriodMeasure = db_get_b(NULL, MODNAME, "PeriodMeasure", DEFAULT_PERIODMEASURE);
	opts.bForceRedownload = db_get_b(NULL, MODNAME, DB_SETTING_REDOWNLOAD, 0);
	opts.bSilentMode = db_get_b(NULL, MODNAME, "SilentMode", 0);
	opts.bBackup = db_get_b(NULL, MODNAME, "Backup", 0);
}

#if MIRANDA_VER >= 0x0A00
IconItemT iconList[] =
{
	{ LPGENT("Check for updates"),"check_update", IDI_MENU },
	{ LPGENT("Plugin info"), "info", IDI_INFO },
	{ LPGENT("Component list"),"plg_list", IDI_PLGLIST }
};

void InitIcoLib()
{
	Icon_RegisterT(hInst,MODULE,iconList, _countof(iconList));
}
#endif

void InitNetlib()
{
	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_TCHAR;	// | NUF_HTTPGATEWAY;
	nlu.ptszDescriptiveName = TranslateT("Plugin Updater HTTP connections");
	nlu.szSettingsModule = MODNAME;
	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
}

void UnloadNetlib()
{
	Netlib_CloseHandle(hNetlibUser);
	hNetlibUser = NULL;
}

ULONG crc32_table[256];
ULONG ulPolynomial = 0x04c11db7;

//////////////////////////////////////////////////////////////
// Reflection is a requirement for the official CRC-32 standard.
// You can create CRCs without it, but they won't conform to the standard.
//////////////////////////////////////////////////////////////////////////

ULONG Reflect(ULONG ref, char ch)
{
	// Used only by Init_CRC32_Table()
	ULONG value(0);

	// Swap bit 0 for bit 7
	// bit 1 for bit 6, etc.
	for(int i = 1; i < (ch + 1); i++)
	{
		if(ref & 1)
			value |= 1 << (ch - i);
		ref >>= 1;
	}
	return value;
}

void InitCrcTable()
{
	// 256 values representing ASCII character codes.
	for(int i = 0; i <= 0xFF; i++)
	{
		crc32_table[i] = Reflect(i, 8) << 24;
		for (int j = 0; j < 8; j++)
			crc32_table[i] = (crc32_table[i] << 1) ^ (crc32_table[i] & (1 << 31) ? ulPolynomial : 0);
		crc32_table[i] = Reflect(crc32_table[i], 32);
	}
}

int Get_CRC(unsigned char* buffer, ULONG bufsize)
{
	ULONG  crc(0xffffffff);
	int len;
	len = bufsize;
	// Save the text in the buffer.

	// Perform the algorithm on each character
	// in the string, using the lookup table values.

	for(int i = 0; i < len; i++)
		crc = (crc >> 8) ^ crc32_table[(crc & 0xFF) ^ buffer[i]];

	// Exclusive OR the result with the beginning value.
	return crc^0xffffffff;
}

int CompareHashes(const ServListEntry *p1, const ServListEntry *p2)
{
	return _tcsicmp(p1->m_name, p2->m_name);
}

bool ParseHashes(const TCHAR *ptszUrl, ptrT &baseUrl, SERVLIST &arHashes)
{
	REPLACEVARSARRAY vars[2];
#if MIRANDA_VER >=0x0A00
	vars[0].key.t = _T("platform");
#ifdef _WIN64
	vars[0].value.t = _T("64");
#else
	vars[0].value.t = _T("32");
#endif
	vars[1].key.t = vars[1].value.t = 0;
#else
	vars[0].lptzKey = _T("platform");
#ifdef _WIN64
	vars[0].lptzValue = _T("64");
#else
	vars[0].lptzValue = _T("32");
#endif
	vars[1].lptzKey = vars[1].lptzValue = 0;
#endif
	baseUrl = Utils_ReplaceVarsT(ptszUrl, 0, vars);

	// Download version info
	FILEURL pFileUrl;
	mir_sntprintf(pFileUrl.tszDownloadURL, _countof(pFileUrl.tszDownloadURL), _T("%s/hashes.zip"), baseUrl);
	mir_sntprintf(pFileUrl.tszDiskPath, _countof(pFileUrl.tszDiskPath), _T("%s\\hashes.zip"), tszTempPath);
	pFileUrl.CRCsum = 0;

	HANDLE nlc;
	bool ret = DownloadFile(&pFileUrl, nlc);
	Netlib_CloseHandle(nlc);

	if (!ret) {
		Netlib_LogfT(hNetlibUser,_T("Downloading list of available updates from %s failed"),baseUrl);
		ShowPopup(TranslateT("Plugin Updater"), TranslateT("An error occurred while checking for new updates."), POPUP_TYPE_ERROR);
		SkinPlaySound("updatefailed");
		return false;
	}

	if(!unzip(pFileUrl.tszDiskPath, tszTempPath, NULL,true)) {
		Netlib_LogfT(hNetlibUser,_T("Unzipping list of available updates from %s failed"),baseUrl);
		ShowPopup(TranslateT("Plugin Updater"), TranslateT("An error occurred while checking for new updates."), POPUP_TYPE_ERROR);
		SkinPlaySound("updatefailed");
		return false;
	}
	
	DeleteFile(pFileUrl.tszDiskPath);

	TCHAR tszTmpIni[MAX_PATH] = {0};
	mir_sntprintf(tszTmpIni, _countof(tszTmpIni), _T("%s\\hashes.txt"), tszTempPath);
	FILE *fp = _tfopen(tszTmpIni, _T("r"));
	if (!fp) {
		Netlib_LogfT(hNetlibUser,_T("Opening %s failed"), tszTempPath);
		ShowPopup(TranslateT("Plugin Updater"), TranslateT("An error occurred while checking for new updates."), POPUP_TYPE_ERROR);
		return false;
	}

	bool bDoNotSwitchToStable = false;
	char str[200];
	while(fgets(str, _countof(str), fp) != NULL) {
		rtrim(str);
		// Do not allow the user to switch back to stable
		if (!strcmp(str, "DoNotSwitchToStable")) {
			bDoNotSwitchToStable = true;
		}
		else if (str[0] != ';') { // ';' marks a comment
			Netlib_Logf(hNetlibUser, "Update: %s", str);
			char *p = strchr(str, ' ');
			if (p != NULL) {
				*p++ = 0;
				_strlwr(p);

				int dwCrc32;
				char *p1 = strchr(p, ' ');
				if (p1 == NULL)
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
		db_set_b(NULL, MODNAME, DB_SETTING_DONT_SWITCH_TO_STABLE, 1);
		// Reset setting if needed
		int UpdateMode = db_get_b(NULL, MODNAME, DB_SETTING_UPDATE_MODE, UPDATE_MODE_STABLE);
		if (UpdateMode == UPDATE_MODE_STABLE)
			db_set_b(NULL, MODNAME, DB_SETTING_UPDATE_MODE, UPDATE_MODE_TRUNK);
	}
	else
		db_set_b(NULL, MODNAME, DB_SETTING_DONT_SWITCH_TO_STABLE, 0);

	return true;
}


bool DownloadFile(FILEURL *pFileURL, HANDLE &nlc)
{
	NETLIBHTTPREQUEST nlhr = {0};
#if MIRANDA_VER < 0x0A00
	nlhr.cbSize = NETLIBHTTPREQUEST_V1_SIZE;
	nlhr.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11;
	if (g_mirandaVersion >= PLUGIN_MAKE_VERSION(0, 9, 0, 0))
		nlhr.flags |= NLHRF_PERSISTENT;
#else
	nlhr.cbSize = sizeof(nlhr);
	nlhr.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11 | NLHRF_PERSISTENT;
#endif
	nlhr.requestType = REQUEST_GET;
	nlhr.nlc = nlc;
	char *szUrl = mir_t2a(pFileURL->tszDownloadURL);
	nlhr.szUrl = szUrl;
	nlhr.headersCount = 4;
	nlhr.headers=(NETLIBHTTPHEADER*)mir_alloc(sizeof(NETLIBHTTPHEADER)*nlhr.headersCount);
	nlhr.headers[0].szName   = "User-Agent";
	nlhr.headers[0].szValue = NETLIB_USER_AGENT;
	nlhr.headers[1].szName  = "Connection";
	nlhr.headers[1].szValue = "close";
	nlhr.headers[2].szName  = "Cache-Control";
	nlhr.headers[2].szValue = "no-cache";
	nlhr.headers[3].szName  = "Pragma";
	nlhr.headers[3].szValue = "no-cache";

	bool ret = false;
	for (int i = 0; !ret && i < MAX_RETRIES; i++) {
		Netlib_LogfT(hNetlibUser,_T("Downloading file %s to %s (attempt %d)"),pFileURL->tszDownloadURL,pFileURL->tszDiskPath, i+1);
		NETLIBHTTPREQUEST *pReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUser, (LPARAM)&nlhr);
		if (pReply) {
			nlc = pReply->nlc;
			if ((200 == pReply->resultCode) && (pReply->dataLength > 0)) {
				// Check CRC sum
				if (pFileURL->CRCsum) {
					InitCrcTable();
					int crc = Get_CRC((unsigned char*)pReply->pData, pReply->dataLength);
					if (crc != pFileURL->CRCsum) {
						// crc check failed, try again
						Netlib_LogfT(hNetlibUser,_T("crc check failed for file %s"),pFileURL->tszDiskPath);
						CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)pReply);
						continue;
					}
				}

				HANDLE hFile = CreateFile(pFileURL->tszDiskPath, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE) {
					DWORD dwBytes;
					// write the downloaded file directly
					WriteFile(hFile, pReply->pData, (DWORD)pReply->dataLength, &dwBytes, NULL);
					CloseHandle(hFile);
				}
				else {
					// try to write it via PU stub
					TCHAR tszTempFile[MAX_PATH];
					mir_sntprintf(tszTempFile, _countof(tszTempFile), _T("%s\\pulocal.tmp"), tszTempPath);
					hFile = CreateFile(tszTempFile, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					if (hFile != INVALID_HANDLE_VALUE) {
						DWORD dwBytes;
						WriteFile(hFile, pReply->pData, (DWORD)pReply->dataLength, &dwBytes, NULL);
						CloseHandle(hFile);
						SafeMoveFile(tszTempFile, pFileURL->tszDiskPath);
					}
				}
				ret = true;
			}
			else
				Netlib_LogfT(hNetlibUser,_T("Downloading file %s failed with error %d"),pFileURL->tszDownloadURL,pReply->resultCode);
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)pReply);
		}
		else {
			Netlib_LogfT(hNetlibUser,_T("Downloading file %s failed, host is propably temporary down."),pFileURL->tszDownloadURL);
			nlc = NULL;
		}
	}
	if(!ret)
		Netlib_LogfT(hNetlibUser,_T("Downloading file %s failed, giving up"),pFileURL->tszDownloadURL);

	mir_free(szUrl);
	mir_free(nlhr.headers);

	return ret;
}

void strdel(TCHAR *parBuffer, int len)
{
	TCHAR* p;
	for (p = parBuffer + len; *p != 0; p++)
		p[-len] = *p;

	p[-len] = '\0';
}

void __stdcall OpenPluginOptions(void*)
{
	OPENOPTIONSDIALOG ood = {0};
	ood.cbSize = sizeof(ood);
	ood.pszPage = "Plugins";
	Options_Open(&ood);
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
	PSID pAdministratorsGroup = NULL;

	// Allocate and initialize a SID of the administrators group.
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pAdministratorsGroup))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	// Determine whether the SID of administrators group is bEnabled in
	// the primary access token of the process.
	if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (pAdministratorsGroup)
	{
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}

	// Throw the error if something failed in the function.
	if (ERROR_SUCCESS != dwError)
	{
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
	HANDLE hToken = NULL;

	// Open the primary access token of the process with TOKEN_QUERY.
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	// Retrieve token elevation information.
	TOKEN_ELEVATION elevation;
	DWORD dwSize;
	if (!GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize))
	{
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
	if (hToken)
	{
		CloseHandle(hToken);
		hToken = NULL;
	}

	// Throw the error if something failed in the function.
	if (ERROR_SUCCESS != dwError)
	{
		throw dwError;
	}

	return fIsElevated;
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
	else if (IsRunAsAdmin()) {
		// Check the current process's "run as administrator" status.
		return true;
	}
	else {
		// Elevate the process. Create a pipe for a stub first
		TCHAR tszPipeName[MAX_PATH];
		mir_sntprintf(tszPipeName, _countof(tszPipeName), _T("\\\\.\\pipe\\Miranda_Pu_%d"), GetCurrentProcessId());
		hPipe = CreateNamedPipe(tszPipeName, PIPE_ACCESS_DUPLEX, PIPE_READMODE_BYTE | PIPE_WAIT, 1, 1024, 1024, NMPWAIT_USE_DEFAULT_WAIT, NULL);
		if (hPipe == INVALID_HANDLE_VALUE) {
			hPipe = NULL;
		}
		else {
			TCHAR cmdLine[100], *p;
			GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath));
			if ((p = _tcsrchr(szPath, '\\')) != 0)
				_tcscpy(p+1, _T("pu_stub.exe"));
			mir_sntprintf(cmdLine, _countof(cmdLine), _T("%d"), GetCurrentProcessId());

			// Launch a stub
			SHELLEXECUTEINFO sei = { sizeof(sei) };
			sei.lpVerb = L"runas";
			sei.lpFile = szPath;
			sei.lpParameters = cmdLine;
			sei.hwnd = NULL;
			sei.nShow = SW_NORMAL;
			if (ShellExecuteEx(&sei)) {
				if (hPipe != NULL)
					ConnectNamedPipe(hPipe, NULL);
				return true;
			}

			DWORD dwError = GetLastError();
			if (dwError == ERROR_CANCELLED)
			{
				// The user refused to allow privileges elevation.
				// Do nothing ...
			}
		}
		return false;
	}
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
	dst += l1+1;
	if (p2) {
		lstrcpy(dst, p2);
		dst += l2+1;
	}
	else *dst++ = 0;

	DWORD dwBytes = 0, dwError;
	if ( WriteFile(hPipe, buf, (DWORD)((BYTE*)dst - buf), &dwBytes, NULL) == 0)
		return 0;

	dwError = 0;
	if ( ReadFile(hPipe, &dwError, sizeof(DWORD), &dwBytes, NULL) == 0) return 0;
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
		if ( MoveFile(pSrc, pDst) == 0) // use copy on error
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
		return CreateDirectoryTreeT(pFolder);

	return TransactPipe(4, pFolder, NULL);
}

int SafeCreateFilePath(TCHAR *pFolder)
{
	if (hPipe == NULL) {
		CreatePathToFileT(pFolder);
		return 0;
	}

	return TransactPipe(5, pFolder, NULL);
}

void BackupFile(TCHAR *ptszSrcFileName, TCHAR *ptszBackFileName)
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
