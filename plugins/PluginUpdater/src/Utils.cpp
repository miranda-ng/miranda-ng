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
HANDLE hCheckThread = NULL, hListThread = NULL, hNetlibUser = NULL;
POPUP_OPTIONS PopupOptions = {0};
aPopups PopupsList[POPUPS];
extern DWORD g_mirandaVersion;

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
	{ "check_update", LPGEN("Check for updates"),           IDI_MENU },
	{ "btn_ok",	      LPGEN("'Yes' Button"),                IDI_OK },
	{ "btn_cancel",   LPGEN("'No' Button"),                 IDI_CANCEL },
	{ "info",         LPGEN("Plugin info"),                 IDI_INFO },
	{ "plg_list",     LPGEN("Plugin list"),                 IDI_PLGLIST },
};

void IcoLibInit()
{
	TCHAR destfile[MAX_PATH];
	GetModuleFileName(hInst, destfile, MAX_PATH);

	SKINICONDESC sid = { sizeof(sid) };
	sid.flags = SIDF_PATH_TCHAR;
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
	PopupsList[index].colorBack = db_get_dw(NULL, MODNAME, "Popups0Bg", COLOR_BG_FIRSTDEFAULT);
	PopupsList[index].colorText = db_get_dw(NULL, MODNAME, "Popups0Tx", COLOR_TX_DEFAULT);

	index = 1;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = db_get_dw(NULL, MODNAME, "Popups1Bg", COLOR_BG_SECONDDEFAULT);
	PopupsList[index].colorText = db_get_dw(NULL, MODNAME, "Popups1Tx", COLOR_TX_DEFAULT);

	index = 2;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = db_get_dw(NULL, MODNAME, "Popups2Bg", COLOR_BG_FIRSTDEFAULT);
	PopupsList[index].colorText = db_get_dw(NULL, MODNAME, "Popups2Tx", COLOR_TX_DEFAULT);

	index = 3;
	PopupsList[index].ID = index;
	PopupsList[index].Icon = SKINICON_OTHER_MIRANDA;
	PopupsList[index].colorBack = db_get_dw(NULL, MODNAME, "Popups3Bg", COLOR_BG_SECONDDEFAULT);
	PopupsList[index].colorText = db_get_dw(NULL, MODNAME, "Popups3Tx", COLOR_TX_DEFAULT);
}

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
	opts.bForceRedownload = db_get_b(NULL, MODNAME, "ForceRedownload", 0);
	opts.bSilentMode = db_get_b(NULL, MODNAME, "SilentMode", 0);
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

TCHAR* GetDefaultUrl()
{
	#if MIRANDA_VER < 0x0A00
		return mir_tstrdup(_T("http://miranda-ng.org/distr/deprecated/0.94.9"));
	#else
		TCHAR *result = db_get_tsa(NULL, MODNAME, "UpdateURL");
		if (result == NULL) { // URL is not set
			db_set_ts(NULL, MODNAME, "UpdateURL", _T(DEFAULT_UPDATE_URL));
			result = mir_tstrdup( _T(DEFAULT_UPDATE_URL));
		}
		return result;
	#endif
}

int CompareHashes(const ServListEntry *p1, const ServListEntry *p2)
{
	return _tcsicmp(p1->m_name, p2->m_name);
}

bool ParseHashes(const TCHAR *ptszUrl, ptrT& baseUrl, SERVLIST& arHashes)
{
	REPLACEVARSARRAY vars[2];
	vars[0].lptzKey = _T("platform");
	#ifdef _WIN64
		vars[0].lptzValue = _T("64");
	#else
		vars[0].lptzValue = _T("32");
	#endif
	vars[1].lptzKey = vars[1].lptzValue = 0;

	REPLACEVARSDATA dat = { sizeof(REPLACEVARSDATA) };
	dat.dwFlags = RVF_TCHAR;
	dat.variables = vars;
	baseUrl = (TCHAR*)CallService(MS_UTILS_REPLACEVARS, (WPARAM)ptszUrl, (LPARAM)&dat);

	// Download version info
	if (!opts.bSilent)
		ShowPopup(NULL, TranslateT("Plugin Updater"), TranslateT("Checking new updates..."), 2, 0, true);

	FILEURL pFileUrl;
	mir_sntprintf(pFileUrl.tszDownloadURL, SIZEOF(pFileUrl.tszDownloadURL), _T("%s/hashes.zip"), baseUrl);
	mir_sntprintf(pFileUrl.tszDiskPath, SIZEOF(pFileUrl.tszDiskPath), _T("%s\\hashes.zip"), tszTempPath);
	pFileUrl.CRCsum = 0;

	HANDLE nlc;
	bool ret = DownloadFile(&pFileUrl, nlc);
	Netlib_CloseHandle(nlc);

	if (!ret) {
		if(!opts.bSilent)
			ShowPopup(0, LPGENT("Plugin Updater"), LPGENT("An error occurred while checking new updates."), 1, 0);
		return false;
	}

	if(!unzip(pFileUrl.tszDiskPath, tszTempPath, NULL,true))
		return false;
	
	DeleteFile(pFileUrl.tszDiskPath);

	TCHAR tszTmpIni[MAX_PATH] = {0};
	mir_sntprintf(tszTmpIni, SIZEOF(tszTmpIni), _T("%s\\hashes.txt"), tszTempPath);
	FILE *fp = _tfopen(tszTmpIni, _T("r"));
	if (!fp)
		return false;

	char str[200];
	while(fgets(str, SIZEOF(str), fp) != NULL) {
		rtrim(str);
		char *p = strchr(str, ' ');
		if (p == NULL)
			continue;

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
	fclose(fp);
	DeleteFile(tszTmpIni);
	return true;
}


bool DownloadFile(FILEURL *pFileURL, HANDLE &nlc)
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
	if (g_mirandaVersion >= PLUGIN_MAKE_VERSION(0, 9, 0, 0))
		nlhr.flags |= NLHRF_PERSISTENT;
	nlhr.nlc = nlc;
	char *szUrl = mir_t2a(pFileURL->tszDownloadURL);
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
	for (int i = 0; !ret && i < MAX_RETRIES; i++) {
		Netlib_LogfT(hNetlibUser,_T("Downloading file %s to %s (attempt %d)"),pFileURL->tszDownloadURL,pFileURL->tszDiskPath, i+1);
		NETLIBHTTPREQUEST *pReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUser, (LPARAM)&nlhr);
		if (pReply) {
			nlc = pReply->nlc;
			if ((200 == pReply->resultCode) && (pReply->dataLength > 0)) {
				HANDLE hFile = CreateFile(pFileURL->tszDiskPath, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE) {
					// write the downloaded file directly
					WriteFile(hFile, pReply->pData, (DWORD)pReply->dataLength, &dwBytes, NULL);
					CloseHandle(hFile);
					if (pFileURL->CRCsum) {
						InitCrcTable();
						int crc = Get_CRC((unsigned char*)pReply->pData, (long)dwBytes);
						if (crc == pFileURL->CRCsum)
							ret = true;
						else
							Netlib_LogfT(hNetlibUser,_T("crc check failed for %s"),pFileURL->tszDownloadURL);
					}
					else
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
						SafeMoveFile(tszTempFile, pFileURL->tszDiskPath);
						if (pFileURL->CRCsum) {
							InitCrcTable();
							int crc = Get_CRC((unsigned char*)pReply->pData, (long)dwBytes);
							if (crc == pFileURL->CRCsum)
								ret = true;
							else
								Netlib_LogfT(hNetlibUser,_T("crc check failed for %s"),pFileURL->tszDownloadURL);
						}
						else
							ret = true;
					}
				}
			}
			else
				Netlib_LogfT(hNetlibUser,_T("Downloading file %s failed with error %d"),pFileURL->tszDownloadURL,pReply->resultCode);
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)pReply);
		}
		else {
			nlc = NULL;
		}
	}
	if(!ret)
		Netlib_LogfT(hNetlibUser,_T("Downloading file %s failed, giving up"),pFileURL->tszDownloadURL);

	mir_free(szUrl);
	mir_free(nlhr.headers);

	DlgDld = ret;
	return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL AllowUpdateOnStartup()
{
	if (opts.bOnlyOnceADay) {
		time_t now = time(NULL);
		time_t was = db_get_dw(NULL, MODNAME, "LastUpdate", 0);

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
	DoCheck(true);
}

void InitTimer(int type)
{
	CancelWaitableTimer(Timer);
	if (opts.bUpdateOnPeriod) {
		LONG interval = PeriodToMilliseconds(opts.Period, opts.bPeriodMeasure);

		switch (type) {
		case 0: // default, plan next check relative to last check
		{
			time_t now = time(NULL);
			time_t was = db_get_dw(NULL, MODNAME, "LastUpdate", 0);

			interval -= (now - was) * 1000;
			if (interval <= 0)
				interval = 1000; // no last update or too far in the past -> do it now
			break;
		}
		case 1: // options changed, use set interval from now
			break;
		case 2: // failed last check, check again in two hours
			interval = 1000 * 60 * 60 * 2;
			break;
		}

		LARGE_INTEGER li = {0};
		li.QuadPart = -1 * (interval * 10000LL);
		SetWaitableTimer(Timer, &li, 0, TimerAPCProc, NULL, 0);
	}
}

void strdel(TCHAR *parBuffer, int len)
{
	TCHAR* p;
	for (p = parBuffer + len; *p != 0; p++)
		p[-len] = *p;

	p[-len] = '\0';
}

#if MIRANDA_VER < 0x0A00
static char szHexTable[] = "0123456789abcdef";

char* bin2hex(const void *pData, size_t len, char *dest)
{
	const BYTE *p = (const BYTE*)pData;
	char *d = dest;

	for (size_t i=0; i < len; i++, p++) {
		*d++ = szHexTable[*p >> 4];
		*d++ = szHexTable[*p & 0x0F];
	}
	*d = 0;

	return dest;
}

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

void CreatePathToFileT(TCHAR* tszFilePath)
{
	TCHAR* pszLastBackslash = _tcsrchr(tszFilePath, '\\');
	if (pszLastBackslash == NULL)
		return;

	*pszLastBackslash = '\0';
	CreateDirectoryTreeT(tszFilePath);
	*pszLastBackslash = '\\';
}

void __stdcall RestartMe(void*)
{
	TCHAR mirandaPath[MAX_PATH], cmdLine[MAX_PATH];
	GetModuleFileName(NULL, mirandaPath, SIZEOF(mirandaPath));

	TCHAR *profilename = Utils_ReplaceVarsT(_T("%miranda_profilename%"));
	mir_sntprintf(cmdLine, SIZEOF(cmdLine), _T("\"%s\" /restart:%d /profile=%s"), mirandaPath, GetCurrentProcessId(), profilename);
	mir_free(profilename);

	CallService("CloseAction", 0, 0);

	PROCESS_INFORMATION pi;
	STARTUPINFO si = { sizeof(si) };
	CreateProcess(mirandaPath, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
}

#else

void __stdcall RestartMe(void*)
{
	CallService(MS_SYSTEM_RESTART, db_get_b(NULL,MODNAME,"RestartCurrentProfile",1) ? 1 : 0, 0);
}

#endif

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

/////////////////////////////////////////////////////////////////////////////////////////

char *StrToLower(char *str)
{
	for (int i = 0; str[i]; i++)
		str[i] = tolower(str[i]);

	return str;
}
