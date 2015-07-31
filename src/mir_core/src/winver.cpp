/*
Copyright (C) 2012-15 Miranda NG team (http://miranda-ng.org)

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

#include "stdafx.h"

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
		TCHAR tszClassName[128] = _T("");
		GetClassName(hWnd, tszClassName, _countof(tszClassName));
		if (_tcscmp(tszClassName, _T("WorkerW"))) {
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
	if (hDesk == NULL)
		return true;

	TCHAR tszName[100];
	DWORD cbName;
	BOOL bLocked = (!GetUserObjectInformation(hDesk, UOI_NAME, tszName, _countof(tszName), &cbName) || mir_tstrcmpi(tszName, _T("default")) != 0);
	CloseDesktop(hDesk);
	return bLocked;
}

MIR_CORE_DLL(BOOL) IsScreenSaverRunning(void)
{
	BOOL rc = FALSE;
	SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &rc, FALSE);
	return rc != 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef VER_SUITE_WH_SERVER
#define VER_SUITE_WH_SERVER                     0x00008000
#endif

#ifndef PRODUCT_ULTIMATE
#define PRODUCT_UNDEFINED                         0x00000000
#define PRODUCT_ULTIMATE                          0x00000001
#define PRODUCT_HOME_BASIC                        0x00000002
#define PRODUCT_HOME_PREMIUM                      0x00000003
#define PRODUCT_ENTERPRISE                        0x00000004
#define PRODUCT_HOME_BASIC_N                      0x00000005
#define PRODUCT_BUSINESS                          0x00000006
#define PRODUCT_STANDARD_SERVER                   0x00000007
#define PRODUCT_DATACENTER_SERVER                 0x00000008
#define PRODUCT_SMALLBUSINESS_SERVER              0x00000009
#define PRODUCT_ENTERPRISE_SERVER                 0x0000000A
#define PRODUCT_STARTER                           0x0000000B
#define PRODUCT_DATACENTER_SERVER_CORE            0x0000000C
#define PRODUCT_STANDARD_SERVER_CORE              0x0000000D
#define PRODUCT_ENTERPRISE_SERVER_CORE            0x0000000E
#define PRODUCT_ENTERPRISE_SERVER_IA64            0x0000000F
#define PRODUCT_BUSINESS_N                        0x00000010
#define PRODUCT_WEB_SERVER                        0x00000011
#define PRODUCT_CLUSTER_SERVER                    0x00000012
#define PRODUCT_HOME_SERVER                       0x00000013
#define PRODUCT_STORAGE_EXPRESS_SERVER            0x00000014
#define PRODUCT_STORAGE_STANDARD_SERVER           0x00000015
#define PRODUCT_STORAGE_WORKGROUP_SERVER          0x00000016
#define PRODUCT_STORAGE_ENTERPRISE_SERVER         0x00000017
#define PRODUCT_SERVER_FOR_SMALLBUSINESS          0x00000018
#define PRODUCT_SMALLBUSINESS_SERVER_PREMIUM      0x00000019
#define PRODUCT_HOME_PREMIUM_N                    0x0000001a
#define PRODUCT_ENTERPRISE_N                      0x0000001b
#define PRODUCT_ULTIMATE_N                        0x0000001c
#define PRODUCT_WEB_SERVER_CORE                   0x0000001d
#define PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT  0x0000001e
#define PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY    0x0000001f
#define PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING   0x00000020
#define PRODUCT_SERVER_FOUNDATION                 0x00000021
#define PRODUCT_HOME_PREMIUM_SERVER               0x00000022
#define PRODUCT_SERVER_FOR_SMALLBUSINESS_V        0x00000023
#define PRODUCT_STANDARD_SERVER_V                 0x00000024
#define PRODUCT_DATACENTER_SERVER_V               0x00000025
#define PRODUCT_ENTERPRISE_SERVER_V               0x00000026
#define PRODUCT_DATACENTER_SERVER_CORE_V          0x00000027
#define PRODUCT_STANDARD_SERVER_CORE_V            0x00000028
#define PRODUCT_ENTERPRISE_SERVER_CORE_V          0x00000029
#define PRODUCT_HYPERV                            0x0000002a
#define PRODUCT_STORAGE_EXPRESS_SERVER_CORE       0x0000002b
#define PRODUCT_STORAGE_STANDARD_SERVER_CORE      0x0000002c
#define PRODUCT_STORAGE_WORKGROUP_SERVER_CORE     0x0000002d
#define PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE    0x0000002e
#define PRODUCT_STARTER_N                         0x0000002f
#define PRODUCT_PROFESSIONAL                      0x00000030
#define PRODUCT_PROFESSIONAL_N                    0x00000031
#define PRODUCT_SB_SOLUTION_SERVER                0x00000032
#define PRODUCT_SERVER_FOR_SB_SOLUTIONS           0x00000033
#define PRODUCT_STANDARD_SERVER_SOLUTIONS         0x00000034
#define PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE    0x00000035
#define PRODUCT_SB_SOLUTION_SERVER_EM             0x00000036
#define PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM        0x00000037
#define PRODUCT_SOLUTION_EMBEDDEDSERVER           0x00000038
#define PRODUCT_SOLUTION_EMBEDDEDSERVER_CORE      0x00000039
#define PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT     0x0000003B
#define PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL     0x0000003C
#define PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC  0x0000003D
#define PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC  0x0000003E
#define PRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE 0x0000003f
#define PRODUCT_CLUSTER_SERVER_V                  0x00000040
#define PRODUCT_EMBEDDED                          0x00000041
#define PRODUCT_STARTER_E                         0x00000042
#define PRODUCT_HOME_BASIC_E                      0x00000043
#define PRODUCT_HOME_PREMIUM_E                    0x00000044
#define PRODUCT_PROFESSIONAL_E                    0x00000045
#define PRODUCT_ENTERPRISE_E                      0x00000046
#define PRODUCT_ULTIMATE_E                        0x00000047
#endif
#ifndef PRODUCT_CORE_N // Win8
#define PRODUCT_CORE_N                            0x00000062
#define PRODUCT_CORE_COUNTRYSPECIFIC              0x00000063
#define PRODUCT_CORE_SINGLELANGUAGE               0x00000064
#define PRODUCT_CORE                              0x00000065
#define PRODUCT_PROFESSIONAL_WMC                  0x00000067
#define PRODUCT_UNLICENSED                        0xABCDABCD
#endif

typedef BOOL(WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

MIR_CORE_DLL(BOOL) GetOSDisplayString(TCHAR *buf, size_t bufSize)
{
	if (buf == NULL || bufSize == 0)
		return 0;

	buf[0] = 0;

	OSVERSIONINFOEX osvi = { 0 };
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	BOOL bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi);
	if (!bOsVersionInfoEx) {
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO*)&osvi))
			return FALSE;
	}

	if (VER_PLATFORM_WIN32_NT != osvi.dwPlatformId || osvi.dwMajorVersion <= 4)
		return FALSE;

	SYSTEM_INFO sysInfo = { 0 };
	GetNativeSystemInfo(&sysInfo);

	CMString ret(_T("Microsoft "));

	// Test for the specific product.
	if (osvi.dwMajorVersion >= 6) {
		if (osvi.dwMajorVersion == 10) {
			if (osvi.wProductType == VER_NT_WORKSTATION)
				ret.Append(_T("Windows 10 "));
			else
				ret.Append(_T("Windows Server 10 "));
		}
		else switch (osvi.dwMinorVersion) {
		case 0:
			if (osvi.wProductType == VER_NT_WORKSTATION)
				ret.Append(_T("Windows Vista "));
			else
				ret.Append(_T("Windows Server 2008 "));
			break;

		case 1:
			if (osvi.wProductType == VER_NT_WORKSTATION)
				ret.Append(_T("Windows 7 "));
			else
				ret.Append(_T("Windows Server 2008 R2 "));
			break;

		case 2:
			if (osvi.wProductType == VER_NT_WORKSTATION)
				ret.Append(_T("Windows 8 "));
			else
				ret.Append(_T("Windows Server 2012 "));
			break;

		case 3:
			if (osvi.wProductType == VER_NT_WORKSTATION)
				ret.Append(_T("Windows 8.1 "));
			else
				ret.Append(_T("Windows Server 2012 R2 "));
			break;
		}

		DWORD dwType = 0;
		HMODULE hKernel = GetModuleHandle(_T("kernel32.dll"));
		PGPI pGPI = (PGPI)GetProcAddress(hKernel, "GetProductInfo");
		if (pGPI != NULL)
			pGPI(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);

		switch (dwType) {
		case PRODUCT_ULTIMATE:
			ret.Append(_T("Ultimate Edition"));
			break;
		case PRODUCT_PROFESSIONAL:
			ret.Append(_T("Professional Edition"));
			break;
		case PRODUCT_PROFESSIONAL_WMC:
			ret.Append(_T("Professional Edition with Media Center"));
			break;
		case PRODUCT_HOME_PREMIUM:
			ret.Append(_T("Home Premium Edition"));
			break;
		case PRODUCT_HOME_BASIC:
			ret.Append(_T("Home Basic Edition"));
			break;
		case PRODUCT_ENTERPRISE:
			ret.Append(_T("Enterprise Edition"));
			break;
		case PRODUCT_BUSINESS:
			ret.Append(_T("Business Edition"));
			break;
		case PRODUCT_STARTER:
			ret.Append(_T("Starter Edition"));
			break;
		case PRODUCT_CLUSTER_SERVER:
			ret.Append(_T("Cluster Server Edition"));
			break;
		case PRODUCT_DATACENTER_SERVER:
			ret.Append(_T("Datacenter Edition"));
			break;
		case PRODUCT_DATACENTER_SERVER_CORE:
			ret.Append(_T("Datacenter Edition (core installation)"));
			break;
		case PRODUCT_ENTERPRISE_SERVER:
			ret.Append(_T("Enterprise Edition"));
			break;
		case PRODUCT_ENTERPRISE_SERVER_CORE:
			ret.Append(_T("Enterprise Edition (core installation)"));
			break;
		case PRODUCT_ENTERPRISE_SERVER_IA64:
			ret.Append(_T("Enterprise Edition for Itanium-based Systems"));
			break;
		case PRODUCT_SMALLBUSINESS_SERVER:
			ret.Append(_T("Small Business Server"));
			break;
		case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
			ret.Append(_T("Small Business Server Premium Edition"));
			break;
		case PRODUCT_STANDARD_SERVER:
			ret.Append(_T("Standard Edition"));
			break;
		case PRODUCT_STANDARD_SERVER_CORE:
			ret.Append(_T("Standard Edition (core installation)"));
			break;
		case PRODUCT_WEB_SERVER:
			ret.Append(_T("Web Server Edition"));
			break;
		}
		if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
			ret.Append(_T(", 64-bit"));
		else if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
			ret.Append(_T(", 32-bit"));
	}

	if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) {
		if (GetSystemMetrics(SM_SERVERR2))
			ret.Append(_T("Windows Server 2003 R2, "));
		else if (osvi.wSuiteMask == VER_SUITE_STORAGE_SERVER)
			ret.Append(_T("Windows Storage Server 2003"));
		else if (osvi.wSuiteMask == VER_SUITE_WH_SERVER)
			ret.Append(_T("Windows Home Server"));
		else if (osvi.wProductType == VER_NT_WORKSTATION && sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
			ret.Append(_T("Windows XP Professional x64 Edition"));
		else
			ret.Append(_T("Windows Server 2003, "));

		// Test for the server type.
		if (osvi.wProductType != VER_NT_WORKSTATION) {
			if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) {
				if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
					ret.Append(_T("Datacenter Edition for Itanium-based Systems"));
				else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
					ret.Append(_T("Enterprise Edition for Itanium-based Systems"));
			}
			else if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
				if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
					ret.Append(_T("Datacenter x64 Edition"));
				else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
					ret.Append(_T("Enterprise x64 Edition"));
				else ret.Append(_T("Standard x64 Edition"));
			}
			else {
				if (osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER)
					ret.Append(_T("Compute Cluster Edition"));
				else if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
					ret.Append(_T("Datacenter Edition"));
				else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
					ret.Append(_T("Enterprise Edition"));
				else if (osvi.wSuiteMask & VER_SUITE_BLADE)
					ret.Append(_T("Web Edition"));
				else ret.Append(_T("Standard Edition"));
			}
		}
	}

	if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) {
		ret.Append(_T("Windows XP "));
		if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
			ret.Append(_T("Home Edition"));
		else ret.Append(_T("Professional"));
	}

	if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) {
		ret.Append(_T("Windows 2000 "));

		if (osvi.wProductType == VER_NT_WORKSTATION)
			ret.Append(_T("Professional"));
		else {
			if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
				ret.Append(_T("Datacenter Server"));
			else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
				ret.Append(_T("Advanced Server"));
			else ret.Append(_T("Server"));
		}
	}

	// Include service pack (if any) and build number.

	if (mir_tstrlen(osvi.szCSDVersion) > 0) {
		ret.Append(_T(" "));
		ret.Append(osvi.szCSDVersion);
	}

	ret.AppendFormat(_T(" (build %d)"), osvi.dwBuildNumber);
	mir_tstrncpy(buf, ret, bufSize);
	return TRUE;
}
