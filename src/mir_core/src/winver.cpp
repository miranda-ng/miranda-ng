/*
Copyright (C) 2012-14 Miranda NG team (http://miranda-ng.org)

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

#include "commonheaders.h"

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
		GetClassName(hWnd, tszClassName, SIZEOF(tszClassName));
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
	BOOL bLocked = (!GetUserObjectInformation(hDesk, UOI_NAME, tszName, SIZEOF(tszName), &cbName) || mir_tstrcmpi(tszName, _T("default")) != 0);
	CloseDesktop(hDesk);
	return bLocked;
}

MIR_CORE_DLL(BOOL) IsScreenSaverRunning(void)
{
	BOOL rc = FALSE;
	SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &rc, FALSE);
	return rc != 0;
}
