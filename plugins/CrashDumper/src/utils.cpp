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

#include "utils.h"

static HINSTANCE hKernel = GetModuleHandleA("kernel32.dll");

typedef BOOL(WINAPI *tGetProductInfo)(DWORD, DWORD, DWORD, DWORD, PDWORD);
tGetProductInfo pGetProductInfo = (tGetProductInfo)GetProcAddress(hKernel, "GetProductInfo");

void GetOSDisplayString(CMString &buffer)
{
	OSVERSIONINFOEX osvi = { 0 };
	SYSTEM_INFO si = { 0 };
	BOOL bOsVersionInfoEx;
	DWORD dwType = 0;

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi);
	if (!bOsVersionInfoEx) {
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO*)&osvi))
			return;
	}

	GetNativeSystemInfo(&si);

	if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId && osvi.dwMajorVersion > 4) {
		buffer.Append(TEXT("Operating System: Microsoft "));

		// Test for the specific product.
		if (osvi.dwMajorVersion == 6) {
			switch (osvi.dwMinorVersion) {
			case 0:
				if (osvi.wProductType == VER_NT_WORKSTATION)
					buffer.Append(TEXT("Windows Vista "));
				else
					buffer.Append(TEXT("Windows Server 2008 "));
				break;

			case 1:
				if (osvi.wProductType == VER_NT_WORKSTATION)
					buffer.Append(TEXT("Windows 7 "));
				else
					buffer.Append(TEXT("Windows Server 2008 R2 "));
				break;

			case 2:
				if (osvi.wProductType == VER_NT_WORKSTATION)
					buffer.Append(TEXT("Windows 8 "));
				else
					buffer.Append(TEXT("Windows Server 2012 "));
				break;

			default:
				if (osvi.wProductType == VER_NT_WORKSTATION)
					buffer.Append(TEXT("Windows 8.1 "));
				else
					buffer.Append(TEXT("Windows Server 2012 R2 "));
				break;
			}

			if (pGetProductInfo)
				pGetProductInfo(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);

			switch (dwType) {
			case PRODUCT_ULTIMATE:
				buffer.Append(TEXT("Ultimate Edition"));
				break;
			case PRODUCT_PROFESSIONAL:
				buffer.Append(TEXT("Professional Edition"));
				break;
			case PRODUCT_HOME_PREMIUM:
				buffer.Append(TEXT("Home Premium Edition"));
				break;
			case PRODUCT_HOME_BASIC:
				buffer.Append(TEXT("Home Basic Edition"));
				break;
			case PRODUCT_ENTERPRISE:
				buffer.Append(TEXT("Enterprise Edition"));
				break;
			case PRODUCT_BUSINESS:
				buffer.Append(TEXT("Business Edition"));
				break;
			case PRODUCT_STARTER:
				buffer.Append(TEXT("Starter Edition"));
				break;
			case PRODUCT_CLUSTER_SERVER:
				buffer.Append(TEXT("Cluster Server Edition"));
				break;
			case PRODUCT_DATACENTER_SERVER:
				buffer.Append(TEXT("Datacenter Edition"));
				break;
			case PRODUCT_DATACENTER_SERVER_CORE:
				buffer.Append(TEXT("Datacenter Edition (core installation)"));
				break;
			case PRODUCT_ENTERPRISE_SERVER:
				buffer.Append(TEXT("Enterprise Edition"));
				break;
			case PRODUCT_ENTERPRISE_SERVER_CORE:
				buffer.Append(TEXT("Enterprise Edition (core installation)"));
				break;
			case PRODUCT_ENTERPRISE_SERVER_IA64:
				buffer.Append(TEXT("Enterprise Edition for Itanium-based Systems"));
				break;
			case PRODUCT_SMALLBUSINESS_SERVER:
				buffer.Append(TEXT("Small Business Server"));
				break;
			case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
				buffer.Append(TEXT("Small Business Server Premium Edition"));
				break;
			case PRODUCT_STANDARD_SERVER:
				buffer.Append(TEXT("Standard Edition"));
				break;
			case PRODUCT_STANDARD_SERVER_CORE:
				buffer.Append(TEXT("Standard Edition (core installation)"));
				break;
			case PRODUCT_WEB_SERVER:
				buffer.Append(TEXT("Web Server Edition"));
				break;
			}

			if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
				buffer.Append(TEXT(", 64-bit"));
			else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
				buffer.Append(TEXT(", 32-bit"));
		}

		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
			if (GetSystemMetrics(SM_SERVERR2))
				buffer.Append(TEXT("Windows Server 2003 R2, "));
			else if (osvi.wSuiteMask == VER_SUITE_STORAGE_SERVER)
				buffer.Append(TEXT("Windows Storage Server 2003"));
			else if (osvi.wProductType == VER_NT_WORKSTATION &&
				si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
				buffer.Append(TEXT("Windows XP Professional x64 Edition"));
			else buffer.Append(TEXT("Windows Server 2003, "));

			// Test for the server type.
			if (osvi.wProductType != VER_NT_WORKSTATION) {
				if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) {
					if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
						buffer.Append(TEXT("Datacenter Edition for Itanium-based Systems"));
					else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						buffer.Append(TEXT("Enterprise Edition for Itanium-based Systems"));
				}
				else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
					if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
						buffer.Append(TEXT("Datacenter x64 Edition"));
					else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						buffer.Append(TEXT("Enterprise x64 Edition"));
					else buffer.Append(TEXT("Standard x64 Edition"));
				}
				else {
					if (osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
						buffer.Append(TEXT("Compute Cluster Edition"));
					else if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
						buffer.Append(TEXT("Datacenter Edition"));
					else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
						buffer.Append(TEXT("Enterprise Edition"));
					else if (osvi.wSuiteMask & VER_SUITE_BLADE)
						buffer.Append(TEXT("Web Edition"));
					else buffer.Append(TEXT("Standard Edition"));
				}
			}
		}

		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) {
			buffer.Append(TEXT("Windows XP "));
			if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
				buffer.Append(TEXT("Home Edition"));
			else
				buffer.Append(TEXT("Professional"));
		}

		if (osvi.szCSDVersion[0] != 0) {
			buffer.Append(TEXT(" "));
			buffer.Append(osvi.szCSDVersion);
		}

		buffer.AppendFormat(TEXT(" (build %d)"), osvi.dwBuildNumber);
	}
}

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
		int dlen = (int)_tcslen(lpszString);
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
		size = SIZEOF(ieBuild);
		if (RegQueryValueEx(hKey, TEXT("Build"), NULL, NULL, (LPBYTE)ieBuild, &size) != ERROR_SUCCESS)
			ieBuild[0] = 0;

		size = SIZEOF(ieVersion);
		if (RegQueryValueEx(hKey, TEXT("Version"), NULL, NULL, (LPBYTE)ieVersion, &size) != ERROR_SUCCESS)
			ieVersion[0] = 0;

		size = SIZEOF(iVer);
		if (RegQueryValueEx(hKey, TEXT("IVer"), NULL, NULL, (LPBYTE)iVer, &size) != ERROR_SUCCESS)
			iVer[0] = 0;

		RegCloseKey(hKey);
	}

	buffer.Append(TEXT("Internet Explorer: "));
	if (ieVersion[0] == 0) {
		if (iVer[0] == 0)
			buffer.Append(TEXT("<not installed>"));
		else if (_tcscmp(iVer, TEXT("100")) == 0)
			buffer.Append(TEXT("1.0"));
		else if (_tcscmp(iVer, TEXT("101")) == 0)
			buffer.Append(TEXT("NT"));
		else if (_tcscmp(iVer, TEXT("102")) == 0)
			buffer.Append(TEXT("2.0"));
		else if (_tcscmp(iVer, TEXT("103")) == 0)
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
		size = SIZEOF(cpuName);
		if (RegQueryValueEx(hKey, TEXT("ProcessorNameString"), NULL, NULL, (LPBYTE)cpuName, &size) != ERROR_SUCCESS)
			_tcscpy(cpuName, TEXT("Unknown"));

		size = SIZEOF(cpuIdent);
		if (RegQueryValueEx(hKey, TEXT("Identifier"), NULL, NULL, (LPBYTE)cpuIdent, &size) != ERROR_SUCCESS)
			if (RegQueryValueEx(hKey, TEXT("VendorIdentifier"), NULL, NULL, (LPBYTE)cpuIdent, &size) != ERROR_SUCCESS)
				_tcscpy(cpuIdent, TEXT("Unknown"));

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


bool CreateDirectoryTree(LPTSTR szDir)
{
	DWORD dwAttr = GetFileAttributes(szDir);
	if (dwAttr != INVALID_FILE_ATTRIBUTES && (dwAttr & FILE_ATTRIBUTE_DIRECTORY))
		return true;

	TCHAR* pszSlash = _tcsrchr(szDir, TEXT('\\'));
	if (pszSlash == NULL)
		return false;

	*pszSlash = 0;
	bool res = CreateDirectoryTree(szDir);
	*pszSlash = TEXT('\\');

	if (res) res = CreateDirectory(szDir, NULL) != 0;

	return res;
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

	OpenClipboard(NULL);
	EmptyClipboard();


	SetClipboardData(CF_UNICODETEXT, hData);
	CloseClipboard();
}

bool IsPluginEnabled(TCHAR* filename)
{
	char* fname;
	crsi_t2a(fname, filename);
	char* ext = strstr(_strlwr(fname), ".dll");
	bool res = ext && ext[4] == '\0' && db_get_b(NULL, "PluginDisable", fname, 0) == 0;
	return res;
}
