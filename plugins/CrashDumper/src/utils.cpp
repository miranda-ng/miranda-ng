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
	TIME_ZONE_INFORMATION tzInfo = {};
	uint32_t type = GetTimeZoneInformation(&tzInfo);

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

void GetISO8061Time(SYSTEMTIME *stLocal, LPTSTR lpszString, uint32_t dwSize)
{
	SYSTEMTIME loctime;
	if (stLocal == nullptr) {
		stLocal = &loctime;
		GetLocalTime(stLocal);
	}

	if (g_plugin.bClassicDates) {
		GetDateFormat(LOCALE_INVARIANT, 0, stLocal, L"d MMM yyyy", lpszString, dwSize);
		int dlen = (int)mir_wstrlen(lpszString);
		GetTimeFormat(LOCALE_INVARIANT, 0, stLocal, L" H:mm:ss", lpszString + dlen, dwSize - dlen);
	}
	else {
		int offset = GetTZOffset();

		// Build a string showing the date and time.
		mir_snwprintf(lpszString, dwSize, L"%d-%02d-%02d %02d:%02d:%02d%+03d%02d",
			stLocal->wYear, stLocal->wMonth, stLocal->wDay,
			stLocal->wHour, stLocal->wMinute, stLocal->wSecond,
			offset / 60, offset % 60);
	}
}

void GetLastWriteTime(FILETIME *ftime, LPTSTR lpszString, uint32_t dwSize)
{
	FILETIME ftLocal;
	SYSTEMTIME stLocal;

	// Convert the last-write time to local time.
	FileTimeToLocalFileTime(ftime, &ftLocal);
	FileTimeToSystemTime(&ftLocal, &stLocal);

	GetISO8061Time(&stLocal, lpszString, dwSize);
}

void GetLastWriteTime(LPCTSTR fileName, LPTSTR lpszString, uint32_t dwSize)
{
	WIN32_FIND_DATA FindFileData;

	HANDLE hFind = FindFirstFile(fileName, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) return;
	FindClose(hFind);

	GetLastWriteTime(&FindFileData.ftLastWriteTime, lpszString, dwSize);
}

const PLUGININFOEX* GetMirInfo(HMODULE hModule)
{
	CMPluginBase &pPlugin = GetPluginByInstance(hModule);
	return (pPlugin.getInst() == hModule) ? &pPlugin.getInfo() : nullptr;
}

void GetInternetExplorerVersion(CMStringW &buffer)
{
	HKEY hKey;
	DWORD size;

	wchar_t ieVersion[1024] = {};
	wchar_t ieBuild[512] = {};
	wchar_t iVer[64] = {};

	if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Internet Explorer", 0, KEY_QUERY_VALUE, &hKey)) {
		size = _countof(ieBuild);
		if (RegQueryValueEx(hKey, L"Build", nullptr, nullptr, (LPBYTE)ieBuild, &size) != ERROR_SUCCESS)
			ieBuild[0] = 0;

		size = _countof(ieVersion);
		if (RegQueryValueEx(hKey, L"Version", nullptr, nullptr, (LPBYTE)ieVersion, &size) != ERROR_SUCCESS)
			ieVersion[0] = 0;

		size = _countof(iVer);
		if (RegQueryValueEx(hKey, L"IVer", nullptr, nullptr, (LPBYTE)iVer, &size) != ERROR_SUCCESS)
			iVer[0] = 0;

		RegCloseKey(hKey);
	}

	buffer.Append(L"Internet Explorer: ");
	if (ieVersion[0] == 0) {
		if (iVer[0] == 0)
			buffer.Append(L"<not installed>");
		else if (mir_wstrcmp(iVer, L"100") == 0)
			buffer.Append(L"1.0");
		else if (mir_wstrcmp(iVer, L"101") == 0)
			buffer.Append(L"NT");
		else if (mir_wstrcmp(iVer, L"102") == 0)
			buffer.Append(L"2.0");
		else if (mir_wstrcmp(iVer, L"103") == 0)
			buffer.Append(L"3.0");
	}
	else buffer.Append(ieVersion);

	if (ieBuild[0] != 0)
		buffer.AppendFormat(L" (build %s)", ieBuild);
}

void TrimMultiSpaces(wchar_t *str)
{
	wchar_t *src = str, *dest = str;
	bool trimst = false;

	for (;;) {
		if (*src == ' ') {
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

void GetProcessorString(CMStringW &buffer)
{
	wchar_t cpuIdent[512] = {};
	wchar_t cpuName[512] = {};

	HKEY hKey;
	if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Hardware\\Description\\System\\CentralProcessor\\0", 0, KEY_QUERY_VALUE, &hKey)) {
		DWORD size = _countof(cpuName);
		if (RegQueryValueEx(hKey, L"ProcessorNameString", nullptr, nullptr, (LPBYTE)cpuName, &size) != ERROR_SUCCESS)
			mir_wstrcpy(cpuName, L"Unknown");

		size = _countof(cpuIdent);
		if (RegQueryValueEx(hKey, L"Identifier", nullptr, nullptr, (LPBYTE)cpuIdent, &size) != ERROR_SUCCESS)
			if (RegQueryValueEx(hKey, L"VendorIdentifier", nullptr, nullptr, (LPBYTE)cpuIdent, &size) != ERROR_SUCCESS)
				mir_wstrcpy(cpuIdent, L"Unknown");

		RegCloseKey(hKey);
	}
	TrimMultiSpaces(cpuName);
	buffer.AppendFormat(L"CPU: %s [%s]", cpuName, cpuIdent);

	if (IsProcessorFeaturePresent(PF_NX_ENABLED))
		buffer.Append(L" [DEP Enabled]");

	SYSTEM_INFO si = {};
	GetSystemInfo(&si);

	if (si.dwNumberOfProcessors > 1)
		buffer.AppendFormat(L" [%u CPUs]", si.dwNumberOfProcessors);
}

void GetFreeMemoryString(CMStringW &buffer)
{
	unsigned ram;
	MEMORYSTATUSEX ms = {};
	ms.dwLength = sizeof(ms);
	GlobalMemoryStatusEx(&ms);
	ram = (unsigned int)((ms.ullTotalPhys / (1024 * 1024)) + 1);
	buffer.AppendFormat(L"Installed RAM: %u MBytes", ram);
}

void GetFreeDiskString(LPCTSTR dirname, CMStringW &buffer)
{
	ULARGE_INTEGER tnb, tfb, fs = {};
	GetDiskFreeSpaceEx(dirname, &fs, &tnb, &tfb);
	fs.QuadPart /= (1024 * 1024);

	buffer.AppendFormat(L"Free disk space on Miranda partition: %u MBytes", fs.LowPart);
}

void ReadableExceptionInfo(PEXCEPTION_RECORD excrec, CMStringW& buffer)
{
	buffer.Append(L"Exception: ");

	switch (excrec->ExceptionCode) {
	case EXCEPTION_BREAKPOINT:
		buffer.Append(L"User Defined Breakpoint");
		break;

	case EXCEPTION_ACCESS_VIOLATION:
		buffer.Append(L"Access Violation");
		break;

	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		buffer.Append(L"Array Bounds Exceeded");
		break;

	case EXCEPTION_DATATYPE_MISALIGNMENT:
		buffer.Append(L"Datatype Misalignment");
		break;

	case EXCEPTION_FLT_DENORMAL_OPERAND:
		buffer.Append(L"Floating Point denormlized operand");
		break;

	case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		buffer.Append(L"Floating Point divide by 0");
		break;

	case EXCEPTION_FLT_INEXACT_RESULT:
		buffer.Append(L"Floating Point inexact result");
		break;

	case EXCEPTION_FLT_INVALID_OPERATION:
		buffer.Append(L"Floating Point invalid operation");
		break;

	case EXCEPTION_FLT_OVERFLOW:
		buffer.Append(L"Floating Point overflow");
		break;

	case EXCEPTION_FLT_STACK_CHECK:
		buffer.Append(L"Floating Point stack overflow/underflow");
		break;

	case EXCEPTION_FLT_UNDERFLOW:
		buffer.Append(L"Floating Point underflow");
		break;

	case EXCEPTION_ILLEGAL_INSTRUCTION:
		buffer.Append(L"Invalid instruction executed");
		break;

	case EXCEPTION_IN_PAGE_ERROR:
		buffer.Append(L"Access to the not present page");
		break;

	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		buffer.Append(L"Integer divide by zero");
		break;

	case EXCEPTION_INT_OVERFLOW:
		buffer.Append(L"Integer overflow");
		break;

	case EXCEPTION_PRIV_INSTRUCTION:
		buffer.Append(L"Priveleged instruction executed");
		break;

	case EXCEPTION_STACK_OVERFLOW:
		buffer.Append(L"Stack overflow");
		break;

	case 0xe06d7363:
		buffer.Append(L"Unhandled C++ software exception");
		break;

	default:
		buffer.AppendFormat(L"%x", excrec->ExceptionCode);
		break;
	}

	buffer.AppendFormat(L" at address %p.", excrec->ExceptionAddress);

	if (excrec->ExceptionCode == EXCEPTION_ACCESS_VIOLATION || excrec->ExceptionCode == EXCEPTION_IN_PAGE_ERROR) {
		switch (excrec->ExceptionInformation[0]) {
		case 0:
			buffer.AppendFormat(L" Reading from address %p.", (LPVOID)excrec->ExceptionInformation[1]);
			break;

		case 1:
			buffer.AppendFormat(L" Writing to address %p.", (LPVOID)excrec->ExceptionInformation[1]);
			break;

		case 8:
			buffer.AppendFormat(L" DEP at address %p.", (LPVOID)excrec->ExceptionInformation[1]);
			break;
		}
	}
}

void GetAdminString(CMStringW &buffer)
{
	BOOL b;
	__try {
		SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
		PSID AdministratorsGroup;

		b = AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
			DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup);
		if (b) {
			if (!CheckTokenMembership(nullptr, AdministratorsGroup, &b))
				b = FALSE;
			FreeSid(AdministratorsGroup);
		}
		else b = GetLastError() == ERROR_CALL_NOT_IMPLEMENTED;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		b = TRUE;
	}

	buffer.AppendFormat(L"Administrator privileges: %s", b ? L"Yes" : L"No");
}

void GetLanguageString(CMStringW &buffer)
{
	wchar_t name1[256], name2[256], name3[256], name4[256];

	GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SENGLANGUAGE, name1, 256);
	GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SENGLANGUAGE, name2, 256);

	GetLocaleInfo(MAKELCID(GetUserDefaultUILanguage(), SORT_DEFAULT), LOCALE_SENGLANGUAGE, name3, 256);
	GetLocaleInfo(MAKELCID(GetSystemDefaultUILanguage(), SORT_DEFAULT), LOCALE_SENGLANGUAGE, name4, 256);

	buffer.AppendFormat(L"OS Languages: (UI | Locale (User/System)) : %s/%s | %s/%s", name3, name4, name1, name2);
}

void GetLanguagePackString(CMStringW &buffer)
{
	buffer.Append(L"Language pack: ");
	if (packlcid != LOCALE_USER_DEFAULT) {
		wchar_t lang[MAX_PATH], ctry[MAX_PATH];
		if (GetLocaleInfo(packlcid, LOCALE_SENGLANGUAGE, lang, MAX_PATH)) {
			if (GetLocaleInfo(packlcid, LOCALE_SISO3166CTRYNAME, ctry, MAX_PATH))
				buffer.AppendFormat(L"%s (%s) [%04x]", lang, ctry, packlcid);
			else
				buffer.Append(lang);
		}
		else buffer.Append(L"Locale id invalid");
	}
	else buffer.Append(L"No language pack installed");
}

void GetWow64String(CMStringW &buffer)
{
	BOOL wow64 = 0;
	if (!IsWow64Process(GetCurrentProcess(), &wow64))
		wow64 = 0;

	if (wow64)
		buffer.Append(L" [running inside WOW64]");
}

void GetVersionInfo(HMODULE hLib, CMStringW& buffer)
{
	HRSRC hVersion = FindResource(hLib, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
	if (hVersion != nullptr) {
		HGLOBAL hGlobal = LoadResource(hLib, hVersion);
		if (hGlobal != nullptr) {
			LPVOID versionInfo = LockResource(hGlobal);
			if (versionInfo != nullptr) {
				int vl = *(unsigned short*)versionInfo;
				unsigned *res = (unsigned*)versionInfo;
				while (*res != 0xfeef04bd && ((char*)res - (char*)versionInfo) < vl) ++res;

				if (((char*)res - (char*)versionInfo) < vl) {
					VS_FIXEDFILEINFO *vsInfo = (VS_FIXEDFILEINFO*)res;
					buffer.AppendFormat(L" v.%u.%u.%u.%u",
						HIWORD(vsInfo->dwFileVersionMS), LOWORD(vsInfo->dwFileVersionMS),
						HIWORD(vsInfo->dwFileVersionLS), LOWORD(vsInfo->dwFileVersionLS));
				}
			}
			FreeResource(hGlobal);
		}
	}
}
