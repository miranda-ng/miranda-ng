/*
Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "../stdafx.h"

#ifndef _WIN32_WINNT_WIN8
#define _WIN32_WINNT_WIN8     0x0602 // Windows 8
#endif

#ifndef _WIN32_WINNT_WINBLUE
#define _WIN32_WINNT_WINBLUE  0x0603 // Windows 8.1
#endif

#ifndef _WIN32_WINNT_WIN10
#define _WIN32_WINNT_WIN10    0x0A00 // Windows 10
#endif

static int dwWinVer;

void InitWinver()
{
	DWORD dwVer = LOWORD(GetVersion());
	dwWinVer = MAKEWORD(HIBYTE(dwVer), LOBYTE(dwVer));
}

MIR_CORE_DLL(BOOL) IsWinVerVistaPlus()
{
	return dwWinVer >= _WIN32_WINNT_VISTA;
}

MIR_CORE_DLL(BOOL) IsWinVer7Plus()
{
	return dwWinVer >= _WIN32_WINNT_WIN7;
}

MIR_CORE_DLL(BOOL) IsWinVer8Plus()
{
	return dwWinVer >= _WIN32_WINNT_WIN8;
}

MIR_CORE_DLL(BOOL) IsWinVer81Plus()
{
	return dwWinVer >= _WIN32_WINNT_WINBLUE;
}

MIR_CORE_DLL(BOOL) IsWinVer10Plus()
{
	return dwWinVer >= _WIN32_WINNT_WIN10;
}

MIR_CORE_DLL(BOOL) IsFullScreen()
{
	RECT rcScreen = { 0 };

	rcScreen.right = GetSystemMetrics(SM_CXSCREEN);
	rcScreen.bottom = GetSystemMetrics(SM_CYSCREEN);

	HMONITOR hMon = MonitorFromWindow(GetForegroundWindow(), MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
	if (GetMonitorInfo(hMon, &mi))
		rcScreen = mi.rcMonitor;

	HWND hWndDesktop = GetDesktopWindow();
	HWND hWndShell = GetShellWindow();

	// check foregroundwindow
	HWND hWnd = GetForegroundWindow();
	if (hWnd && hWnd != hWndDesktop && hWnd != hWndShell) {
		wchar_t tszClassName[128] = L"";
		GetClassName(hWnd, tszClassName, _countof(tszClassName));
		if (wcscmp(tszClassName, L"WorkerW")) {
			RECT rect, rectw, recti;
			GetWindowRect(hWnd, &rectw);

			GetClientRect(hWnd, &rect);
			ClientToScreen(hWnd, (LPPOINT)&rect);
			ClientToScreen(hWnd, (LPPOINT)&rect.right);

			if (EqualRect(&rect, &rectw) && IntersectRect(&recti, &rect, &rcScreen) && EqualRect(&recti, &rcScreen))
				return true;
		}
	}

	return false;
}

MIR_CORE_DLL(BOOL) IsWorkstationLocked(void)
{
	HDESK hDesk = OpenInputDesktop(0, FALSE, DESKTOP_SWITCHDESKTOP);
	if (hDesk == nullptr)
		return true;

	wchar_t tszName[100];
	DWORD cbName;
	BOOL bLocked = (!GetUserObjectInformation(hDesk, UOI_NAME, tszName, _countof(tszName), &cbName) || mir_wstrcmpi(tszName, L"default") != 0);
	CloseDesktop(hDesk);
	return bLocked;
}

MIR_CORE_DLL(BOOL) IsTerminalDisconnected(void)
{
	PVOID pBuffer = nullptr;
	DWORD pBytesReturned = 0;
	BOOL result = FALSE;

	if (WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, WTS_CURRENT_SESSION, WTSConnectState, (LPTSTR *)&pBuffer, &pBytesReturned))
		if (*(PDWORD)pBuffer == WTSDisconnected)
			result = TRUE;

	if (pBuffer)
		WTSFreeMemory(pBuffer);
	return result;
}

MIR_CORE_DLL(BOOL) IsScreenSaverRunning(void)
{
	BOOL rc = FALSE;
	SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &rc, FALSE);
	return rc != 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(BOOL) OS_GetShortString(char *buf, size_t bufSize)
{
	if (buf == nullptr || bufSize == 0)
		return false;

	mir_snprintf(buf, bufSize, "Windows NT %d.%d", HIBYTE(dwWinVer), LOBYTE(dwWinVer));
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRODUCT_CORE_N // Win8
#define PRODUCT_CORE_SINGLELANGUAGE               0x00000064
#define PRODUCT_PROFESSIONAL_WMC                  0x00000067
#endif

typedef BOOL(WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
typedef LPCSTR(WINAPI *WGV)(void);

MIR_CORE_DLL(BOOL) OS_GetDisplayString(char *buf, size_t bufSize)
{
	if (buf == nullptr || bufSize == 0)
		return 0;

	buf[0] = 0;

	OSVERSIONINFOEX osvi = { 0 };
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	BOOL bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi);
	if (!bOsVersionInfoEx) {
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO*)&osvi))
			return false;
	}

	if (VER_PLATFORM_WIN32_NT != osvi.dwPlatformId || osvi.dwMajorVersion <= 4)
		return false;

	SYSTEM_INFO sysInfo = { 0 };
	GetNativeSystemInfo(&sysInfo);

	CMStringA ret("Microsoft ");

	// Test for the specific product.
	if (osvi.dwMajorVersion >= 6) {
		if (osvi.dwMajorVersion == 10) {
			if (osvi.wProductType == VER_NT_WORKSTATION)
				ret.Append("Windows 10 ");
			else
				ret.Append("Windows Server 10 ");
		}
		else switch (osvi.dwMinorVersion) {
		case 0:
			if (osvi.wProductType == VER_NT_WORKSTATION)
				ret.Append("Windows Vista ");
			else
				ret.Append("Windows Server 2008 ");
			break;

		case 1:
			if (osvi.wProductType == VER_NT_WORKSTATION)
				ret.Append("Windows 7 ");
			else
				ret.Append("Windows Server 2008 R2 ");
			break;

		case 2:
			if (osvi.wProductType == VER_NT_WORKSTATION)
				ret.Append("Windows 8 ");
			else
				ret.Append("Windows Server 2012 ");
			break;

		case 3:
			if (osvi.wProductType == VER_NT_WORKSTATION)
				ret.Append("Windows 8.1 ");
			else
				ret.Append("Windows Server 2012 R2 ");
			break;
		}

		DWORD dwType = 0;
		HMODULE hKernel = GetModuleHandle(L"kernel32.dll");
		PGPI pGPI = (PGPI)GetProcAddress(hKernel, "GetProductInfo");
		if (pGPI != nullptr)
			pGPI(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);

		switch (dwType) {
		case PRODUCT_ULTIMATE:
			ret.Append("Ultimate Edition");
			break;
		case PRODUCT_PROFESSIONAL:
			ret.Append("Professional Edition");
			break;
		case PRODUCT_PROFESSIONAL_WMC:
			ret.Append("Professional Edition with Media Center");
			break;
		case PRODUCT_HOME_PREMIUM:
			ret.Append("Home Premium Edition");
			break;
		case PRODUCT_HOME_BASIC:
			ret.Append("Home Basic Edition");
			break;
		case PRODUCT_ENTERPRISE:
			ret.Append("Enterprise Edition");
			break;
		case PRODUCT_BUSINESS:
			ret.Append("Business Edition");
			break;
		case PRODUCT_STARTER:
			ret.Append("Starter Edition");
			break;
		case PRODUCT_CLUSTER_SERVER:
			ret.Append("Cluster Server Edition");
			break;
		case PRODUCT_DATACENTER_SERVER:
			ret.Append("Datacenter Edition");
			break;
		case PRODUCT_DATACENTER_SERVER_CORE:
			ret.Append("Datacenter Edition (core installation)");
			break;
		case PRODUCT_ENTERPRISE_SERVER:
			ret.Append("Enterprise Edition");
			break;
		case PRODUCT_ENTERPRISE_SERVER_CORE:
			ret.Append("Enterprise Edition (core installation)");
			break;
		case PRODUCT_ENTERPRISE_SERVER_IA64:
			ret.Append("Enterprise Edition for Itanium-based Systems");
			break;
		case PRODUCT_SMALLBUSINESS_SERVER:
			ret.Append("Small Business Server");
			break;
		case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
			ret.Append("Small Business Server Premium Edition");
			break;
		case PRODUCT_STANDARD_SERVER:
			ret.Append("Standard Edition");
			break;
		case PRODUCT_STANDARD_SERVER_CORE:
			ret.Append("Standard Edition (core installation)");
			break;
		case PRODUCT_WEB_SERVER:
			ret.Append("Web Server Edition");
			break;
		case PRODUCT_CORE_SINGLELANGUAGE:
			ret.Append("Home Single Language");
			break;
		}
		if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
			ret.Append(", 64-bit");
		else if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
			ret.Append(", 32-bit");
	}

	if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
		if (GetSystemMetrics(SM_SERVERR2))
			ret.Append("Windows Server 2003 R2, ");
		else if (osvi.wSuiteMask == VER_SUITE_STORAGE_SERVER)
			ret.Append("Windows Storage Server 2003");
		else if (osvi.wSuiteMask == VER_SUITE_WH_SERVER)
			ret.Append("Windows Home Server");
		else if (osvi.wProductType == VER_NT_WORKSTATION && sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
			ret.Append("Windows XP Professional x64 Edition");
		else
			ret.Append("Windows Server 2003, ");

		// Test for the server type.
		if (osvi.wProductType != VER_NT_WORKSTATION) {
			if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) {
				if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
					ret.Append("Datacenter Edition for Itanium-based Systems");
				else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
					ret.Append("Enterprise Edition for Itanium-based Systems");
			}
			else if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
				if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
					ret.Append("Datacenter x64 Edition");
				else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
					ret.Append("Enterprise x64 Edition");
				else ret.Append("Standard x64 Edition");
			}
			else {
				if (osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
					ret.Append("Compute Cluster Edition");
				else if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
					ret.Append("Datacenter Edition");
				else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
					ret.Append("Enterprise Edition");
				else if (osvi.wSuiteMask & VER_SUITE_BLADE)
					ret.Append("Web Edition");
				else ret.Append("Standard Edition");
			}
		}
	}

	if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) {
		ret.Append("Windows XP ");
		if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
			ret.Append("Home Edition");
		else ret.Append("Professional");
	}

	if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
		ret.Append("Windows 2000 ");

		if (osvi.wProductType == VER_NT_WORKSTATION)
			ret.Append("Professional");
		else {
			if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
				ret.Append("Datacenter Server");
			else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
				ret.Append("Advanced Server");
			else ret.Append("Server");
		}
	}

	// Include service pack (if any) and build number.
	if (mir_wstrlen(osvi.szCSDVersion) > 0) {
		ret.Append(" ");
		ret.Append(_T2A(osvi.szCSDVersion));
	}

	ret.AppendFormat(" (build %d)", osvi.dwBuildNumber);

	HMODULE hNtDll = GetModuleHandleA("ntdll.dll");
	if (WGV wine_get_version = (WGV)GetProcAddress(hNtDll, "wine_get_version"))
	{
		ret.AppendFormat(" (Wine %s)", wine_get_version());
	}

	mir_strncpy(buf, ret, bufSize);
	return true;
}
