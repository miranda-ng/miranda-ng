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

#include "Common.h"

BOOL DlgDld;
int  Number = 0;
TCHAR tszDialogMsg[2048] = {0};
FILEINFO *pFileInfo = NULL;
HANDLE CheckThread = NULL, hNetlibUser = NULL;
POPUP_OPTIONS PopupOptions = {0};
aPopups PopupsList[POPUPS];

/////////////////////////////////////////////////////////////////////////////////////
// we don't use Icon_Register here because it should work under Miranda IM too

struct
{
	char *szIconName;
	char *szDescr;
	int   IconID;
}
static iconList[] =
{
	{ "check_update", LPGEN("Check for plugin updates"),    IDI_MENU },
	{ "btn_ok",			LPGEN("'Yes' Button"),              IDI_OK },
	{ "btn_cancel",   LPGEN("'No' Button"),               IDI_CANCEL }
};

void IcoLibInit()
{
	TCHAR destfile[MAX_PATH];
	GetModuleFileName(hInst, destfile, MAX_PATH);

	SKINICONDESC sid = { sizeof(sid) };
	sid.flags = SIDF_PATH_TCHAR;
	sid.cx = sid.cy = 16;
	sid.ptszDefaultFile = destfile;
	sid.pszSection = MODULEA;

	for (int i = 0; i < SIZEOF(iconList); i++) {
		sid.pszName = iconList[i].szIconName;
		sid.pszDescription = iconList[i].szDescr;
		sid.iDefaultIndex = -iconList[i].IconID;
		Skin_AddIcon(&sid);
	}
}

BOOL NetlibInit()
{
	NETLIBUSER nlu = {0};
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_TCHAR;	// | NUF_HTTPGATEWAY;
	nlu.ptszDescriptiveName = TranslateT("Plugin Updater HTTP connection");
	nlu.szSettingsModule = MODNAME;
	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	return hNetlibUser != NULL;
}

void NetlibUnInit()
{
	Netlib_CloseHandle(hNetlibUser);
	hNetlibUser = NULL;
}

void InitPopupList()
{
	int index = 0;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = DBGetContactSettingDword(NULL, MODNAME, "Popups0Bg", COLOR_BG_FIRSTDEFAULT);
	PopupsList[index].colorText = DBGetContactSettingDword(NULL, MODNAME, "Popups0Tx", COLOR_TX_DEFAULT);

	index = 1;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = DBGetContactSettingDword(NULL, MODNAME, "Popups1Bg", COLOR_BG_SECONDDEFAULT);
	PopupsList[index].colorText = DBGetContactSettingDword(NULL, MODNAME, "Popups1Tx", COLOR_TX_DEFAULT);

	index = 2;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = DBGetContactSettingDword(NULL, MODNAME, "Popups2Bg", COLOR_BG_FIRSTDEFAULT);
	PopupsList[index].colorText = DBGetContactSettingDword(NULL, MODNAME, "Popups2Tx", COLOR_TX_DEFAULT);

	index = 3;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = DBGetContactSettingDword(NULL, MODNAME, "Popups3Bg", COLOR_BG_SECONDDEFAULT);
	PopupsList[index].colorText = DBGetContactSettingDword(NULL, MODNAME, "Popups3Tx", COLOR_TX_DEFAULT);
}

void LoadOptions()
{
	PopupOptions.DefColors = DBGetContactSettingByte(NULL, MODNAME, "DefColors", DEFAULT_COLORS);
	PopupOptions.LeftClickAction= DBGetContactSettingByte(NULL, MODNAME, "LeftClickAction", DEFAULT_POPUP_LCLICK);
	PopupOptions.RightClickAction = DBGetContactSettingByte(NULL, MODNAME, "RightClickAction", DEFAULT_POPUP_RCLICK);
	PopupOptions.Timeout = DBGetContactSettingDword(NULL, MODNAME, "Timeout", DEFAULT_TIMEOUT_VALUE);
	
	opts.bUpdateOnStartup = DBGetContactSettingByte(NULL, MODNAME, "UpdateOnStartup", DEFAULT_UPDATEONSTARTUP);
	opts.bOnlyOnceADay = DBGetContactSettingByte(NULL, MODNAME, "OnlyOnceADay", DEFAULT_ONLYONCEADAY);
	opts.bUpdateOnPeriod = DBGetContactSettingByte(NULL, MODNAME, "UpdateOnPeriod", DEFAULT_UPDATEONPERIOD);
	opts.Period = DBGetContactSettingDword(NULL, MODNAME, "Period", DEFAULT_PERIOD);
	opts.bPeriodMeasure = DBGetContactSettingByte(NULL, MODNAME, "PeriodMeasure", DEFAULT_PERIODMEASURE);
	opts.bUpdateIcons = DBGetContactSettingByte(NULL, MODNAME, "UpdateIcons", DEFAULT_UPDATEICONS);
}

BOOL DownloadFile(LPCTSTR tszURL, LPCTSTR tszLocal)
{
	DWORD dwBytes;

	NETLIBHTTPREQUEST nlhr = {0};
	#if MIRANDA_VER < 0x0A00
		nlhr.cbSize = NETLIBHTTPREQUEST_V1_SIZE;
	#else
		nlhr.cbSize = sizeof(nlhr);
	#endif
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11;
	char *szUrl = mir_t2a(tszURL);
	nlhr.szUrl = szUrl;
	nlhr.headersCount = 4;
	nlhr.headers=(NETLIBHTTPHEADER*)mir_alloc(sizeof(NETLIBHTTPHEADER)*nlhr.headersCount);
	nlhr.headers[0].szName   = "User-Agent";
	nlhr.headers[0].szValue = "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)";
	nlhr.headers[1].szName  = "Connection";
	nlhr.headers[1].szValue = "close";
	nlhr.headers[2].szName  = "Cache-Control";
	nlhr.headers[2].szValue = "no-cache";
	nlhr.headers[3].szName  = "Pragma";
	nlhr.headers[3].szValue = "no-cache";

	bool ret = false;
	NETLIBHTTPREQUEST *pReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUser,(LPARAM)&nlhr);
	if (pReply) {
		if ((200 == pReply->resultCode) && (pReply->dataLength > 0)) {
			HANDLE hFile = CreateFile(tszLocal, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile != INVALID_HANDLE_VALUE) {
				// write the downloaded file firectly
				WriteFile(hFile, pReply->pData, (DWORD)pReply->dataLength, &dwBytes, NULL);
				CloseHandle(hFile);
				ret = true;
			}
			else {
				// try to write it via PU stub
				TCHAR tszTempFile[MAX_PATH];
				mir_sntprintf(tszTempFile, SIZEOF(tszTempFile), _T("%s\\pulocal.tmp"), tszTempPath);
				hFile = CreateFile(tszTempFile, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE) {
					WriteFile(hFile, pReply->pData, (DWORD)pReply->dataLength, &dwBytes, NULL);
					CloseHandle(hFile);
					SafeMoveFile(tszTempFile, tszLocal);
					ret = true;
				}
			}
		}
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)pReply);
	}

	mir_free(szUrl);
	mir_free(nlhr.headers);

	DlgDld = ret;
	return ret;
}

void __stdcall RestartMe(void*)
{
	CallService(MS_SYSTEM_RESTART, db_get_b(NULL,MODNAME,"RestartCurrentProfile",1) ? 1 : 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL AllowUpdateOnStartup()
{
	if (opts.bOnlyOnceADay) {
		time_t now = time(NULL);
		time_t was = DBGetContactSettingDword(NULL, MODNAME, "LastUpdate", 0);

		if ((now - was) < 86400)
			return FALSE;
	}
	return TRUE;
}

LONG PeriodToMilliseconds(const int period, BYTE& periodMeasure)
{
	LONG result = period * 1000;
	switch(periodMeasure) {
	case 1:
		// day
		result *= 60 * 60 * 24;
		break;

	default:
		// hour
		if (periodMeasure != 0)
			periodMeasure = 0;
		result *= 60 * 60;
		break;
	}
	return result;
}

void CALLBACK TimerAPCProc(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
	DoCheck(1);
}

void InitTimer()
{
	CancelWaitableTimer(Timer);
	if (opts.bUpdateOnPeriod) {
		LONG interval = PeriodToMilliseconds(opts.Period, opts.bPeriodMeasure);

		_int64 qwDueTime = -10000i64 * interval;

		LARGE_INTEGER li = {0};
		li.LowPart = (DWORD) ( qwDueTime & 0xFFFFFFFF );
		li.HighPart = (LONG) ( qwDueTime >> 32 );

		SetWaitableTimer(Timer, &li, interval, TimerAPCProc, NULL, 0);
	}
}

void strdel(TCHAR *parBuffer, int len )
{
	TCHAR* p;
	for (p = parBuffer+len; *p != 0; p++)
		p[ -len ] = *p;

	p[ -len ] = '\0';
}

#if MIRANDA_VER < 0x0A00
char* rtrim(char *str)
{
	if (str == NULL)
		return NULL;

	char *p = strchr(str, 0);
	while (--p >= str) {
		switch (*p) {
		case ' ': case '\t': case '\n': case '\r':
			*p = 0; break;
		default:
			return str;
		}
	}
	return str;
}
#endif

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
