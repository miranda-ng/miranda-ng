/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "../stdafx.h"

HWND hAPCWindow = nullptr;

int  InitPathUtils(void);
void RecalculateTime(void);

void CheckLogs();
void InitLogs();
void UninitLogs();

void InitColourPicker();
void InitHyperlink();
void InitTimeZones();
void InitWinver();

HINSTANCE g_hInst = nullptr;

HCURSOR g_hCursorNS, g_hCursorWE;
HANDLE hThreadQueueEmpty;
uint32_t mir_tls = 0;

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR RestartMiranda(WPARAM wParam, LPARAM lParam)
{
	wchar_t mirandaPath[MAX_PATH], cmdLine[MAX_PATH];
	if (lParam)
		wcsncpy_s(mirandaPath, (const wchar_t*)lParam, _TRUNCATE);
	else
		GetModuleFileName(nullptr, mirandaPath, _countof(mirandaPath));

	if (wParam) {
		VARSW profilename(L"%miranda_profilename%");
		mir_snwprintf(cmdLine, L"\"%s\" /restart:%d /profile=%s", mirandaPath, GetCurrentProcessId(), (wchar_t*)profilename);
	}
	else mir_snwprintf(cmdLine, L"\"%s\" /restart:%d", mirandaPath, GetCurrentProcessId());

	CallService("CloseAction", 0, 0);

	PROCESS_INFORMATION pi;
	STARTUPINFO startupInfo = { 0 };
	startupInfo.cb = sizeof(startupInfo);
	CreateProcess(mirandaPath, cmdLine, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startupInfo, &pi);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// module init

static LRESULT CALLBACK APCWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_USER+1) {
		PAPCFUNC pFunc = (PAPCFUNC)wParam;
		pFunc((ULONG_PTR)lParam);
		return 0;
	}

	if (msg == WM_TIMER)
		CheckLogs();

	if (msg == WM_TIMECHANGE)
		RecalculateTime();

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void LoadCoreModule(void)
{
	INITCOMMONCONTROLSEX icce = {0};
	icce.dwSize = sizeof(icce);
	icce.dwICC = ICC_WIN95_CLASSES | ICC_USEREX_CLASSES;
	InitCommonControlsEx(&icce);

	hAPCWindow = CreateWindowEx(0, L"ComboLBox", nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
	SetClassLongPtr(hAPCWindow, GCL_STYLE, GetClassLongPtr(hAPCWindow, GCL_STYLE) | CS_DROPSHADOW);
	DestroyWindow(hAPCWindow);

	hAPCWindow = CreateWindowEx(0, L"STATIC", nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
	SetWindowLongPtr(hAPCWindow, GWLP_WNDPROC, (LONG_PTR)APCWndProc);
	SetTimer(hAPCWindow, 1, 1000, nullptr);
	hThreadQueueEmpty = CreateEvent(nullptr, TRUE, TRUE, nullptr);

	InitWinver();
	InitPathUtils();
	InitLogs();
	InitColourPicker();
	InitHyperlink();
	InitTimeZones();
	InitialiseModularEngine();

	CreateServiceFunction(MS_SYSTEM_RESTART, RestartMiranda);

	pfnRtlGenRandom = (PGENRANDOM)GetProcAddress(GetModuleHandleA("advapi32"), "SystemFunction036");
}

MIR_CORE_DLL(void) UnloadCoreModule(void)
{
	DestroyWindow(hAPCWindow);
	CloseHandle(hThreadQueueEmpty);
	TlsFree(mir_tls);

	DestroyModularEngine();
	UninitLogs();
	UnloadLangPackModule();
}

/////////////////////////////////////////////////////////////////////////////////////////
// entry point

BOOL WINAPI DllMain(HINSTANCE hinstDLL, uint32_t fdwReason, LPVOID)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		g_hInst = hinstDLL;
		mir_tls = TlsAlloc();
		LoadCoreModule();
	}
	else if (fdwReason == DLL_THREAD_DETACH) {
		HANDLE hEvent = TlsGetValue(mir_tls);
		if (hEvent)
			CloseHandle(hEvent);
	}
	return TRUE;
}
