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
HANDLE hShutdownEvent, hPreShutdownEvent;
uint32_t mir_tls = 0;

static bool g_bMirandaTerminated = false;
bool g_bEnableDpiAware = false;

MIR_CORE_DLL(bool) Miranda_IsTerminated()
{
	return g_bMirandaTerminated;
}

MIR_CORE_DLL(void) Miranda_SetTerminated(void)
{
	g_bMirandaTerminated = true;

	NotifyEventHooks(hPreShutdownEvent, 0, 0);
	NotifyEventHooks(hShutdownEvent, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

struct MWaitableObject
{
	MWaitableObject(MWaitableStub pFunc, HANDLE hEvent) :
		m_bOwnsEvent(false),
		m_hEvent(hEvent),
		m_pFunc(pFunc),
		m_pInfo(INVALID_HANDLE_VALUE)
	{
		if (hEvent == nullptr) {
			m_hEvent = CreateEvent(nullptr, TRUE, TRUE, nullptr);
			m_bOwnsEvent = true;
		}
	}

	MWaitableObject(MWaitableStubEx pFunc, void *pInfo) :
		m_bOwnsEvent(true),
		m_hEvent(CreateEvent(nullptr, TRUE, TRUE, nullptr)),
		m_pFuncEx(pFunc),
		m_pInfo(pInfo)
	{}

	~MWaitableObject()
	{
		if (m_bOwnsEvent)
			::CloseHandle(m_hEvent);
	}

	HANDLE m_hEvent;
	union
	{
		MWaitableStub m_pFunc;
		MWaitableStubEx m_pFuncEx;
	};
	void *m_pInfo;

	bool m_bOwnsEvent;
};

static OBJLIST<MWaitableObject> arWaitableObjects(1, HandleKeySortT);

MIR_CORE_DLL(void) Miranda_WaitOnHandle(MWaitableStub pFunc, HANDLE hEvent)
{
	arWaitableObjects.insert(new MWaitableObject(pFunc, hEvent));
}

MIR_CORE_DLL(void) Miranda_WaitOnHandleEx(MWaitableStubEx pFunc, void *pInfo)
{
	arWaitableObjects.insert(new MWaitableObject(pFunc, pInfo));
}

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

	wchar_t wszIniPath[MAX_PATH];
	PathToAbsoluteW(L"mirandaboot.ini", wszIniPath);
	if (GetPrivateProfileIntW(L"Interface", L"DpiAware", 0, wszIniPath) == 1)
		g_bEnableDpiAware = true;

	CreateServiceFunction(MS_SYSTEM_RESTART, RestartMiranda);

	hShutdownEvent = CreateHookableEvent(ME_SYSTEM_SHUTDOWN);
	hPreShutdownEvent = CreateHookableEvent(ME_SYSTEM_PRESHUTDOWN);

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
// Message loop

static HMODULE hThemeAPI;

typedef HRESULT(STDAPICALLTYPE *pfnBufferedPaintInit)(void);
pfnBufferedPaintInit bufferedPaintInit;

typedef HRESULT(STDAPICALLTYPE *pfnBufferedPaintUninit)(void);
pfnBufferedPaintUninit bufferedPaintUninit;

static void crtErrorHandler(const wchar_t *, const wchar_t *, const wchar_t *, unsigned, uintptr_t)
{}

static void __cdecl compactHeapsThread(void*)
{
	Thread_SetName("compactHeapsThread");

	while (!Miranda_IsTerminated()) {
		SleepEx((1000 * 60) * 5, TRUE); // every 5 minutes

		HANDLE hHeaps[256];
		uint32_t hc = GetProcessHeaps(255, (PHANDLE)&hHeaps);
		if (hc != 0 && hc < 256) {
			__try {
				for (uint32_t j = 0; j < hc; j++)
					HeapCompact(hHeaps[j], 0);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{}
		}
	}
}

MIR_CORE_DLL(void) BeginMessageLoop()
{
	_set_invalid_parameter_handler(&crtErrorHandler);

	#ifdef _DEBUG
		_CrtSetReportMode(_CRT_ASSERT, 0);
	#endif

	#ifdef _DEBUG
		if (CmdLine_GetOption(L"memdebug"))
			_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	hThemeAPI = LoadLibrary(L"uxtheme.dll");
	if (hThemeAPI) {
		bufferedPaintInit = (pfnBufferedPaintInit)GetProcAddress(hThemeAPI, "BufferedPaintInit");
		bufferedPaintUninit = (pfnBufferedPaintUninit)GetProcAddress(hThemeAPI, "BufferedPaintUninit");
	}

	if (g_bEnableDpiAware) {
		typedef BOOL (WINAPI *pfnSetProcessDPIAware_t)(void);
		auto *pFunc = (pfnSetProcessDPIAware_t)GetProcAddress(GetModuleHandleW(L"user32"), "SetProcessDPIAware");
		if (pFunc != nullptr)
			pFunc();
	}

	if (bufferedPaintInit)
		bufferedPaintInit();

	OleInitialize(nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////

static uint32_t dwEventTime = 0;

void checkIdle(MSG *msg)
{
	switch (msg->message) {
	case WM_MOUSEACTIVATE:
	case WM_MOUSEMOVE:
	case WM_CHAR:
		dwEventTime = GetTickCount();
	}
}

MIR_CORE_DLL(uint32_t) Miranda_GetIdle()
{
	return dwEventTime;
}

static uint32_t myWait()
{
	HANDLE *hWaitObjects = (HANDLE *)_alloca(arWaitableObjects.getCount() * sizeof(HANDLE));
	for (int i = 0; i < arWaitableObjects.getCount(); i++)
		hWaitObjects[i] = arWaitableObjects[i].m_hEvent;

	return MsgWaitForMultipleObjectsEx(arWaitableObjects.getCount(), hWaitObjects, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE);
}

MIR_CORE_DLL(void) EnterMessageLoop()
{
	mir_forkthread(compactHeapsThread);
	dwEventTime = GetTickCount();
	uint32_t myPid = GetCurrentProcessId();

	bool messageloop = true;
	while (messageloop) {
		MSG msg;
		BOOL dying = FALSE;
		uint32_t rc = myWait();
		if (rc < WAIT_OBJECT_0 + arWaitableObjects.getCount()) {
			auto &pWait = arWaitableObjects[rc - WAIT_OBJECT_0];
			if (pWait.m_pInfo == INVALID_HANDLE_VALUE)
				(*pWait.m_pFunc)();
			else
				(*pWait.m_pFuncEx)(pWait.m_pInfo);

			if (pWait.m_bOwnsEvent)
				arWaitableObjects.remove(&pWait);
		}

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message != WM_QUIT) {
				HWND h = GetForegroundWindow();
				DWORD pid = 0;
				checkIdle(&msg);
				if (h != nullptr && GetWindowThreadProcessId(h, &pid) && pid == myPid && GetClassLongPtr(h, GCW_ATOM) == 32770)
					if (h != nullptr && IsDialogMessage(h, &msg)) /* Wine fix. */
						continue;

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else if (!dying) {
				dying++;
				g_bMirandaTerminated = true;
				NotifyEventHooks(hPreShutdownEvent, 0, 0);

				// this spins and processes the msg loop, objects and APC.
				Thread_Wait();
				NotifyEventHooks(hShutdownEvent, 0, 0);
				// if the hooks generated any messages, it'll get processed before the second WM_QUIT
				PostQuitMessage(0);
			}
			else if (dying)
				messageloop = false;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(void) LeaveMessageLoop()
{
	OleUninitialize();

	if (bufferedPaintUninit) {
		bufferedPaintUninit();
		FreeLibrary(hThemeAPI);
	}
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
