/*
Miranda Crash Dumper Plugin
Copyright (C) 2008 - 2012 Boris Krasnovskiy All Rights Reserved

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

static HINSTANCE hKernel = GetModuleHandleA("kernel32.dll");

int GetTZOffset(void)
{
	TIME_ZONE_INFORMATION tzInfo = { 0 };
	DWORD type = GetTimeZoneInformation(&tzInfo);

	int offset = 0;
	switch (type) {
	case TIME_ZONE_ID_DAYLIGHT:
		offset = -(tzInfo.Bias + tzInfo.DaylightBias);
		break;

	case TIME_ZONE_ID_STANDARD:
		offset = -(tzInfo.Bias + tzInfo.StandardBias);
		break;

	case TIME_ZONE_ID_UNKNOWN:
		offset = -tzInfo.Bias;
		break;
	}
	return offset;
}

void GetISO8061Time(SYSTEMTIME *stLocal, LPTSTR lpszString, DWORD dwSize)
{
	SYSTEMTIME loctime;
	if (stLocal == NULL) {
		stLocal = &loctime;
		GetLocalTime(stLocal);
	}

	if (clsdates) {
		GetDateFormat(LOCALE_INVARIANT, 0, stLocal, TEXT("d MMM yyyy"), lpszString, dwSize);
		int dlen = (int)mir_tstrlen(lpszString);
		GetTimeFormat(LOCALE_INVARIANT, 0, stLocal, TEXT(" H:mm:ss"), lpszString + dlen, dwSize - dlen);
	}
	else {
		int offset = GetTZOffset();

		// Build a string showing the date and time.
		mir_sntprintf(lpszString, dwSize, TEXT("%d-%02d-%02d %02d:%02d:%02d%+03d%02d"),
			stLocal->wYear, stLocal->wMonth, stLocal->wDay,
			stLocal->wHour, stLocal->wMinute, stLocal->wSecond,
			offset / 60, offset % 60);
	}
}

void GetLastWriteTime(FILETIME *ftime, LPTSTR lpszString, DWORD dwSize)
{
	FILETIME ftLocal;
	SYSTEMTIME stLocal;

	// Convert the last-write time to local time.
	FileTimeToLocalFileTime(ftime, &ftLocal);
	FileTimeToSystemTime(&ftLocal, &stLocal);

	GetISO8061Time(&stLocal, lpszString, dwSize);
}

void GetLastWriteTime(LPCTSTR fileName, LPTSTR lpszString, DWORD dwSize)
{
	WIN32_FIND_DATA FindFileData;

	HANDLE hFind = FindFirstFile(fileName, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) return;
	FindClose(hFind);

	GetLastWriteTime(&FindFileData.ftLastWriteTime, lpszString, dwSize);
}

typedef PLUGININFOEX * (__cdecl * Miranda_Plugin_Info) (DWORD mirandaVersion);

PLUGININFOEX* GetMirInfo(HMODULE hModule)
{
	Miranda_Plugin_Info bpi = (Miranda_Plugin_Info)GetProcAddress(hModule, "MirandaPluginInfoEx");
	if (bpi == NULL)
		return NULL;

	return bpi(mirandaVersion);
}

void GetInternetExplorerVersion(CMString &buffer)
{
	HKEY hKey;
	DWORD size;

	TCHAR ieVersion[1024] = { 0 };
	TCHAR ieBuild[512] = { 0 };
	TCHAR iVer[64] = { 0 };

	if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Internet Explorer"), 0, KEY_QUERY_VALUE, &hKey)) {
		size = _countof(ieBuild);
		if (RegQueryValueEx(hKey, TEXT("Build"), NULL, NULL, (LPBYTE)ieBuild, &size) != ERROR_SUCCESS)
			ieBuild[0] = 0;

		size = _countof(ieVersion);
		if (RegQueryValueEx(hKey, TEXT("Version"), NULL, NULL, (LPBYTE)ieVersion, &size) != ERROR_SUCCESS)
			ieVersion[0] = 0;

		size = _countof(iVer);
		if (RegQueryValueEx(hKey, TEXT("IVer"), NULL, NULL, (LPBYTE)iVer, &size) != ERROR_SUCCESS)
			iVer[0] = 0;

		RegCloseKey(hKey);
	}

	buffer.Append(TEXT("Internet Explorer: "));
	if (ieVersion[0] == 0) {
		if (iVer[0] == 0)
			buffer.Append(TEXT("<not installed>"));
		else if (mir_tstrcmp(iVer, TEXT("100")) == 0)
			buffer.Append(TEXT("1.0"));
		else if (mir_tstrcmp(iVer, TEXT("101")) == 0)
			buffer.Append(TEXT("NT"));
		else if (mir_tstrcmp(iVer, TEXT("102")) == 0)
			buffer.Append(TEXT("2.0"));
		else if (mir_tstrcmp(iVer, TEXT("103")) == 0)
			buffer.Append(TEXT("3.0"));
	}
	else buffer.Append(ieVersion);

	if (ieBuild[0] != 0)
		buffer.AppendFormat(TEXT(" (build %s)"), ieBuild);
}

void TrimMultiSpaces(TCHAR *str)
{
	TCHAR *src = str, *dest = str;
	bool trimst = false;

	for (;;) {
		if (*src == TEXT(' ')) {
			if (!trimst) {
				trimst = true;
				*dest++ = *src;
			}
		}
		else {
			trimst = false;
			*dest++ = *src;
		}
		if (*src++ == 0) break;
	}
}

void GetProcessorString(CMString &buffer)
{
	HKEY hKey;
	DWORD size;

	TCHAR cpuIdent[512] = { 0 };
	TCHAR cpuName[512] = { 0 };

	if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Hardware\\Description\\System\\CentralProcessor\\0"), 0, KEY_QUERY_VALUE, &hKey)) {
		size = _countof(cpuName);
		if (RegQueryValueEx(hKey, TEXT("ProcessorNameString"), NULL, NULL, (LPBYTE)cpuName, &size) != ERROR_SUCCESS)
			mir_tstrcpy(cpuName, TEXT("Unknown"));

		size = _countof(cpuIdent);
		if (RegQueryValueEx(hKey, TEXT("Identifier"), NULL, NULL, (LPBYTE)cpuIdent, &size) != ERROR_SUCCESS)
			if (RegQueryValueEx(hKey, TEXT("VendorIdentifier"), NULL, NULL, (LPBYTE)cpuIdent, &size) != ERROR_SUCCESS)
				mir_tstrcpy(cpuIdent, TEXT("Unknown"));

		RegCloseKey(hKey);
	}
	TrimMultiSpaces(cpuName);
	buffer.AppendFormat(TEXT("CPU: %s [%s]"), cpuName, cpuIdent);

	if (IsProcessorFeaturePresent(PF_NX_ENABLED))
		buffer.Append(TEXT(" [DEP Enabled]"));

	SYSTEM_INFO si = { 0 };
	GetSystemInfo(&si);

	if (si.dwNumberOfProcessors > 1)
		buffer.AppendFormat(TEXT(" [%u CPUs]"), si.dwNumberOfProcessors);
}

void GetFreeMemoryString(CMString &buffer)
{
	unsigned ram;
	MEMORYSTATUSEX ms = { 0 };
	ms.dwLength = sizeof(ms);
	GlobalMemoryStatusEx(&ms);
	ram = (unsigned int)((ms.ullTotalPhys / (1024 * 1024)) + 1);
	buffer.AppendFormat(TEXT("Installed RAM: %u MBytes"), ram);
}

void GetFreeDiskString(LPCTSTR dirname, CMString &buffer)
{
	ULARGE_INTEGER tnb, tfb, fs = { 0 };
	GetDiskFreeSpaceEx(dirname, &fs, &tnb, &tfb);
	fs.QuadPart /= (1024 * 1024);

	buffer.AppendFormat(TEXT("Free disk space on Miranda partition: %u MBytes"), fs.LowPart);
}

void ReadableExceptionInfo(PEXCEPTION_RECORD excrec, CMString& buffer)
{
	buffer.Append(TEXT("Exception: "));

	switch (excrec->ExceptionCode) {
	case EXCEPTION_BREAKPOINT:
		buffer.Append(TEXT("User Defined Breakpoint"));
		break;

	case EXCEPTION_ACCESS_VIOLATION:
		buffer.Append(TEXT("Access Violation"));
		break;

	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		buffer.Append(TEXT("Array Bounds Exceeded"));
		break;

	case EXCEPTION_DATATYPE_MISALIGNMENT:
		buffer.Append(TEXT("Datatype Misalignment"));
		break;

	case EXCEPTION_FLT_DENORMAL_OPERAND:
		buffer.Append(TEXT("Floating Point denormlized operand"));
		break;

	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		buffer.Append(TEXT("Floating Point divide by 0"));
		break;

	case EXCEPTION_FLT_INEXACT_RESULT:
		buffer.Append(TEXT("Floating Point inexact result"));
		break;

	case EXCEPTION_FLT_INVALID_OPERATION:
		buffer.Append(TEXT("Floating Point invalid operation"));
		break;

	case EXCEPTION_FLT_OVERFLOW:
		buffer.Append(TEXT("Floating Point overflow"));
		break;

	case EXCEPTION_FLT_STACK_CHECK:
		buffer.Append(TEXT("Floating Point stack overflow/underflow"));
		break;

	case EXCEPTION_FLT_UNDERFLOW:
		buffer.Append(TEXT("Floating Point underflow"));
		break;

	case EXCEPTION_ILLEGAL_INSTRUCTION:
		buffer.Append(TEXT("Invalid instruction executed"));
		break;

	case EXCEPTION_IN_PAGE_ERROR:
		buffer.Append(TEXT("Access to the not present page"));
		break;

	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		buffer.Append(TEXT("Integer divide by zero"));
		break;

	case EXCEPTION_INT_OVERFLOW:
		buffer.Append(TEXT("Integer overflow"));
		break;

	case EXCEPTION_PRIV_INSTRUCTION:
		buffer.Append(TEXT("Priveleged instruction executed"));
		break;

	case EXCEPTION_STACK_OVERFLOW:
		buffer.Append(TEXT("Stack overflow"));
		break;

	case 0xe06d7363:
		buffer.Append(TEXT("Unhandled C++ software exception"));
		break;

	default:
		buffer.AppendFormat(TEXT("%x"), excrec->ExceptionCode);
		break;
	}

	buffer.AppendFormat(TEXT(" at address %p."), excrec->ExceptionAddress);

	if (excrec->ExceptionCode == EXCEPTION_ACCESS_VIOLATION || excrec->ExceptionCode == EXCEPTION_IN_PAGE_ERROR) {
		switch (excrec->ExceptionInformation[0]) {
		case 0:
			buffer.AppendFormat(TEXT(" Reading from address %p."), (LPVOID)excrec->ExceptionInformation[1]);
			break;

		case 1:
			buffer.AppendFormat(TEXT(" Writing to address %p."), (LPVOID)excrec->ExceptionInformation[1]);
			break;

		case 8:
			buffer.AppendFormat(TEXT(" DEP at address %p."), (LPVOID)excrec->ExceptionInformation[1]);
			break;
		}
	}
}

void GetAdminString(CMString &buffer)
{
	BOOL b;
	__try {
		SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
		PSID AdministratorsGroup;

		b = AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
			DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup);
		if (b) {
			if (!CheckTokenMembership(NULL, AdministratorsGroup, &b))
				b = FALSE;
			FreeSid(AdministratorsGroup);
		}
		else b = GetLastError() == ERROR_CALL_NOT_IMPLEMENTED;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		b = TRUE;
	}

	buffer.AppendFormat(TEXT("Administrator privileges: %s"), b ? TEXT("Yes") : TEXT("No"));
}

void GetLanguageString(CMString &buffer)
{
	TCHAR name1[256], name2[256], name3[256], name4[256];

	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SENGLANGUAGE, name1, 256);
	GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SENGLANGUAGE, name2, 256);

	GetLocaleInfo(MAKELCID(GetUserDefaultUILanguage(), SORT_DEFAULT), LOCALE_SENGLANGUAGE, name3, 256);
	GetLocaleInfo(MAKELCID(GetSystemDefaultUILanguage(), SORT_DEFAULT), LOCALE_SENGLANGUAGE, name4, 256);

	buffer.AppendFormat(TEXT("OS Languages: (UI | Locale (User/System)) : %s/%s | %s/%s"), name3, name4, name1, name2);
}

void GetLanguagePackString(CMString &buffer)
{
	buffer.Append(TEXT("Language pack: "));
	if (packlcid != LOCALE_USER_DEFAULT) {
		TCHAR lang[MAX_PATH], ctry[MAX_PATH];
		if (GetLocaleInfo(packlcid, LOCALE_SENGLANGUAGE, lang, MAX_PATH)) {
			if (GetLocaleInfo(packlcid, LOCALE_SISO3166CTRYNAME, ctry, MAX_PATH))
				buffer.AppendFormat(TEXT("%s (%s) [%04x]"), lang, ctry, packlcid);
			else
				buffer.Append(lang);
		}
		else
			buffer.Append(TEXT("Locale id invalid"));
	}
	else
		buffer.Append(TEXT("No language pack installed"));
}

void GetWow64String(CMString &buffer)
{
	BOOL wow64 = 0;
	if (!IsWow64Process(GetCurrentProcess(), &wow64))
		wow64 = 0;

	if (wow64)
		buffer.Append(TEXT(" [running inside WOW64]"));
}

void GetVersionInfo(HMODULE hLib, CMString& buffer)
{
	HRSRC hVersion = FindResource(hLib, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
	if (hVersion != NULL) {
		HGLOBAL hGlobal = LoadResource(hLib, hVersion);
		if (hGlobal != NULL) {
			LPVOID versionInfo = LockResource(hGlobal);
			if (versionInfo != NULL) {
				int vl = *(unsigned short*)versionInfo;
				unsigned *res = (unsigned*)versionInfo;
				while (*res != 0xfeef04bd && ((char*)res - (char*)versionInfo) < vl) ++res;

				if (((char*)res - (char*)versionInfo) < vl) {
					VS_FIXEDFILEINFO *vsInfo = (VS_FIXEDFILEINFO*)res;
					buffer.AppendFormat(TEXT(" v.%u.%u.%u.%u"),
						HIWORD(vsInfo->dwFileVersionMS), LOWORD(vsInfo->dwFileVersionMS),
						HIWORD(vsInfo->dwFileVersionLS), LOWORD(vsInfo->dwFileVersionLS));
				}
			}
			FreeResource(hGlobal);
		}
	}
}

void StoreStringToClip(CMString& buffer)
{
	int bufLen = (buffer.GetLength() + 1) * sizeof(TCHAR);
	HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, bufLen);
	LPSTR buf = (LPSTR)GlobalLock(hData);

	memcpy(buf, buffer.c_str(), bufLen);

	GlobalUnlock(hData);

	if(OpenClipboard(NULL)) {
		EmptyClipboard();


		SetClipboardData(CF_UNICODETEXT, hData);
		CloseClipboard();
	}
}

bool IsPluginEnabled(TCHAR* filename)
{
	char* fname;
	crsi_t2a(fname, filename);
	char* ext = strstr(_strlwr(fname), ".dll");
	bool res = ext && ext[4] == '\0' && db_get_b(NULL, "PluginDisable", fname, 0) == 0;
	return res;
}
