/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "commonheaders.h"

HWND hAPCWindow = NULL;

int  InitPathUtils(void);
void (*RecalculateTime)(void);

void CheckLogs();
void InitLogs();
void UninitLogs();

void InitWinver();
void InitMetaContacts();

int hLangpack = 0;
HINSTANCE hInst = 0;

HANDLE hStackMutex, hThreadQueueEmpty;
DWORD mir_tls = 0;
bool g_bDebugMode = false;

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

	if (msg == WM_TIMECHANGE && RecalculateTime)
		RecalculateTime();

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static void LoadCoreModule(void)
{
	INITCOMMONCONTROLSEX icce = {0};
	icce.dwSize = sizeof(icce);
	icce.dwICC = ICC_WIN95_CLASSES | ICC_USEREX_CLASSES;
	InitCommonControlsEx(&icce);

	hAPCWindow = CreateWindowEx(0, _T("ComboLBox"), NULL, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
	SetClassLongPtr(hAPCWindow, GCL_STYLE, GetClassLongPtr(hAPCWindow, GCL_STYLE) | CS_DROPSHADOW);
	DestroyWindow(hAPCWindow);
	hAPCWindow = NULL;

	hAPCWindow = CreateWindowEx(0, _T("STATIC"), NULL, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL);
	SetWindowLongPtr(hAPCWindow, GWLP_WNDPROC, (LONG_PTR)APCWndProc);
	SetTimer(hAPCWindow, 1, 1000, NULL);
	hStackMutex = CreateMutex(NULL, FALSE, NULL);
	hThreadQueueEmpty = CreateEvent(NULL, TRUE, TRUE, NULL);

	#ifdef _WIN64
		HMODULE mirInst = GetModuleHandleA("miranda64.exe");
	#else
		HMODULE mirInst = GetModuleHandleA("miranda32.exe");
	#endif
	RecalculateTime = (void (*)()) GetProcAddress(mirInst, "RecalculateTime");

	InitWinver();
	InitPathUtils();
	InitLogs();
	InitialiseModularEngine();
	InitProtocols();
	InitMetaContacts();
}

MIR_CORE_DLL(void) UnloadCoreModule(void)
{
	DestroyWindow(hAPCWindow);
	CloseHandle(hStackMutex);
	CloseHandle(hThreadQueueEmpty);
	TlsFree(mir_tls);

	UninitProtocols();
	DestroyModularEngine();
	UninitLogs();
	UnloadLangPackModule();
}

/////////////////////////////////////////////////////////////////////////////////////////
// entry point

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		hInst = hinstDLL;
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
