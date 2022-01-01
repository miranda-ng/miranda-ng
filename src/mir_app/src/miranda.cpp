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

#include "stdafx.h"
#include "clc.h"

#if defined(VLD_ENABLED)
#include "msapi\vld.h"
#endif

#pragma comment(lib, "version.lib")

int LoadDefaultModules(void);
void UnloadNewPluginsModule(void);
void UnloadDefaultModules(void);

typedef HRESULT(STDAPICALLTYPE* pfnBufferedPaintInit)(void);
pfnBufferedPaintInit bufferedPaintInit;

typedef HRESULT(STDAPICALLTYPE* pfnBufferedPaintUninit)(void);
pfnBufferedPaintUninit bufferedPaintUninit;

typedef HANDLE(STDAPICALLTYPE* pfnBeginBufferedPaint)(HDC, RECT*, BP_BUFFERFORMAT, BP_PAINTPARAMS*, HDC*);
pfnBeginBufferedPaint beginBufferedPaint;

typedef HRESULT(STDAPICALLTYPE* pfnEndBufferedPaint)(HANDLE, BOOL);
pfnEndBufferedPaint endBufferedPaint;

HANDLE hOkToExitEvent, hModulesLoadedEvent;
HANDLE hShutdownEvent, hPreShutdownEvent;
uint32_t hMainThreadId;
bool g_bModulesLoadedFired = false, g_bMirandaTerminated = false;
int g_iIconX, g_iIconY, g_iIconSX, g_iIconSY;

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

HBITMAP ConvertIconToBitmap(HIMAGELIST hIml, int iconId)
{
	BITMAPINFO bmi = { 0 };
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biWidth = g_iIconSX;
	bmi.bmiHeader.biHeight = g_iIconSY;

	HDC hdc = CreateCompatibleDC(nullptr);
	HBITMAP hbmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, nullptr, nullptr, 0);
	HBITMAP hbmpOld = (HBITMAP)SelectObject(hdc, hbmp);

	BLENDFUNCTION bfAlpha = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	BP_PAINTPARAMS paintParams = { 0 };
	paintParams.cbSize = sizeof(paintParams);
	paintParams.dwFlags = BPPF_ERASE;
	paintParams.pBlendFunction = &bfAlpha;

	HDC hdcBuffer;
	RECT rcIcon = { 0, 0, g_iIconSX, g_iIconSY };
	HANDLE hPaintBuffer = beginBufferedPaint(hdc, &rcIcon, BPBF_DIB, &paintParams, &hdcBuffer);
	if (hPaintBuffer) {
		ImageList_Draw(hIml, iconId, hdc, 0, 0, ILD_TRANSPARENT);
		endBufferedPaint(hPaintBuffer, TRUE);
	}

	SelectObject(hdc, hbmpOld);
	DeleteDC(hdc);

	return hbmp;
}

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	"Miranda NG",
	MIRANDA_VERSION_DWORD,
	"",
	"Miranda NG team",
	LEGAL_COPYRIGHT,
	"https://miranda-ng.org",
	UNICODE_AWARE,
	MIID_LAST
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(nullptr, pluginInfoEx)
{}

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
	union {
		MWaitableStub m_pFunc;
		MWaitableStubEx m_pFuncEx;
	};
	void *m_pInfo;

	bool m_bOwnsEvent;
};

static OBJLIST<MWaitableObject> arWaitableObjects(1, HandleKeySortT);

MIR_APP_DLL(void) Miranda_WaitOnHandle(MWaitableStub pFunc, HANDLE hEvent)
{
	arWaitableObjects.insert(new MWaitableObject(pFunc, hEvent));
}

MIR_APP_DLL(void) Miranda_WaitOnHandleEx(MWaitableStubEx pFunc, void *pInfo)
{
	arWaitableObjects.insert(new MWaitableObject(pFunc, pInfo));
}

/////////////////////////////////////////////////////////////////////////////////////////
// dll entry point

BOOL WINAPI DllMain(HINSTANCE hinstDLL, uint32_t dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH) {
		g_plugin.setInst(hinstDLL);
		RegisterModule(&g_plugin);

		g_iIconX = GetSystemMetrics(SM_CXICON);
		g_iIconY = GetSystemMetrics(SM_CYICON);
		g_iIconSX = GetSystemMetrics(SM_CXSMICON);
		g_iIconSY = GetSystemMetrics(SM_CYSMICON);

		#if defined(VLD_ENABLED)
			VLDSetOptions(VLD_OPT_SKIP_HEAPFREE_LEAKS, -1, -1);
			VLDSetReportOptions(VLD_OPT_REPORT_TO_FILE, L"C:\\temp\\vld.txt");
		#endif
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

typedef LONG(WINAPI *pNtQIT)(HANDLE, LONG, PVOID, ULONG, PULONG);
#define ThreadQuerySetWin32StartAddress 9

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

void (*SetIdleCallback)(void) = nullptr;

MIR_APP_DLL(void) Miranda_SetIdleCallback(void(__cdecl *pfnCallback)(void))
{
	SetIdleCallback = pfnCallback;
}

static uint32_t dwEventTime = 0;
void checkIdle(MSG * msg)
{
	switch (msg->message) {
	case WM_MOUSEACTIVATE:
	case WM_MOUSEMOVE:
	case WM_CHAR:
		dwEventTime = GetTickCount();
	}
}

MIR_APP_DLL(uint32_t) Miranda_GetIdle()
{
	return dwEventTime;
}

static int SystemShutdownProc(WPARAM, LPARAM)
{
	UnloadDefaultModules();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

#define MIRANDA_PROCESS_WAIT_TIMEOUT        60000
#define MIRANDA_PROCESS_WAIT_RESOLUTION     1000
#define MIRANDA_PROCESS_WAIT_STEPS          (MIRANDA_PROCESS_WAIT_TIMEOUT/MIRANDA_PROCESS_WAIT_RESOLUTION)

class CWaitRestartDlg : public CDlgBase
{
	HANDLE m_hProcess;
	CTimer m_timer;
	CCtrlProgress m_progress;
	CCtrlButton m_cancel;

protected:
	bool OnInitDialog() override
	{
		m_progress.SetRange(MIRANDA_PROCESS_WAIT_STEPS);
		m_progress.SetStep(1);
		m_timer.Start(MIRANDA_PROCESS_WAIT_RESOLUTION);
		return true;
	}

	void Timer_OnEvent(CTimer*)
	{
		if (m_progress.Move() == MIRANDA_PROCESS_WAIT_STEPS)
			EndModal(0);
		if (WaitForSingleObject(m_hProcess, 1) != WAIT_TIMEOUT) {
			m_progress.SetPosition(MIRANDA_PROCESS_WAIT_STEPS);
			EndModal(0);
		}
	}

	void Cancel_OnClick(CCtrlBase*)
	{
		m_progress.SetPosition(MIRANDA_PROCESS_WAIT_STEPS);
		EndModal(1);
	}

public:
	CWaitRestartDlg(HANDLE hProcess)
		: CDlgBase(g_plugin, IDD_WAITRESTART), m_timer(this, 1),
		m_progress(this, IDC_PROGRESSBAR), m_cancel(this, IDCANCEL)
	{
		m_autoClose = 0;
		m_hProcess = hProcess;
		m_timer.OnEvent = Callback(this, &CWaitRestartDlg::Timer_OnEvent);
		m_cancel.OnClick = Callback(this, &CWaitRestartDlg::Cancel_OnClick);
	}
};

INT_PTR CheckRestart()
{
	LPCTSTR tszPID = CmdLine_GetOption(L"restart");
	if (tszPID) {
		HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, _wtol(tszPID));
		if (hProcess) {
			INT_PTR result = CWaitRestartDlg(hProcess).DoModal();
			CloseHandle(hProcess);
			return result;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

class MSystemWindow : public CDlgBase
{

public:
	MSystemWindow() :
		CDlgBase(g_plugin, IDD_EMPTY)
	{
	}
};

static MSystemWindow *g_pSystemWindow;

/////////////////////////////////////////////////////////////////////////////////////////

static void crtErrorHandler(const wchar_t*, const wchar_t*, const wchar_t*, unsigned, uintptr_t)
{}

static uint32_t myWait()
{
	HANDLE *hWaitObjects = (HANDLE*)_alloca(arWaitableObjects.getCount() * sizeof(HANDLE));
	for (int i = 0; i < arWaitableObjects.getCount(); i++)
		hWaitObjects[i] = arWaitableObjects[i].m_hEvent;

	return MsgWaitForMultipleObjectsEx(arWaitableObjects.getCount(), hWaitObjects, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE);
}

int WINAPI mir_main(LPTSTR cmdLine)
{
	hMainThreadId = GetCurrentThreadId();

	_set_invalid_parameter_handler(&crtErrorHandler);
#ifdef _DEBUG
	_CrtSetReportMode(_CRT_ASSERT, 0);
#endif

	CmdLine_Parse(cmdLine);
	setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "C");

#ifdef _DEBUG
	if (CmdLine_GetOption(L"memdebug"))
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	HMODULE hThemeAPI;
	if (IsWinVerVistaPlus()) {
		hThemeAPI = LoadLibrary(L"uxtheme.dll");
		if (hThemeAPI) {
			bufferedPaintInit = (pfnBufferedPaintInit)GetProcAddress(hThemeAPI, "BufferedPaintInit");
			bufferedPaintUninit = (pfnBufferedPaintUninit)GetProcAddress(hThemeAPI, "BufferedPaintUninit");
			beginBufferedPaint = (pfnBeginBufferedPaint)GetProcAddress(hThemeAPI, "BeginBufferedPaint");
			endBufferedPaint = (pfnEndBufferedPaint)GetProcAddress(hThemeAPI, "EndBufferedPaint");
		}
	}
	else hThemeAPI = nullptr;

	if (bufferedPaintInit)
		bufferedPaintInit();

	OleInitialize(nullptr);

	g_pSystemWindow = new MSystemWindow();
	g_pSystemWindow->Create();

	int result = 0;
	if (LoadDefaultModules()) {
		g_bMirandaTerminated = true;
		NotifyEventHooks(hPreShutdownEvent, 0, 0);
		NotifyEventHooks(hShutdownEvent, 0, 0);
		UnloadDefaultModules();

		result = 1;
	}
	else {
		InitPathVar();

		if (plugin_service == nullptr) {
			NotifyEventHooks(hModulesLoadedEvent, 0, 0);
			g_bModulesLoadedFired = true;
		}

		// ensure that the kernel hooks the SystemShutdownProc() after all plugins
		HookEvent(ME_SYSTEM_SHUTDOWN, SystemShutdownProc);

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
					if (SetIdleCallback != nullptr)
						SetIdleCallback();
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

	UnloadNewPluginsModule();
	UnloadCoreModule();

	if (hThemeAPI)
		FreeLibrary(hThemeAPI);

	UninitTray();

	delete g_pSystemWindow;

	OleUninitialize();

	if (bufferedPaintUninit)
		bufferedPaintUninit();
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_APP_DLL(bool) Miranda_IsTerminated()
{
	return g_bMirandaTerminated;
}

MIR_APP_DLL(bool) Miranda_OkToExit()
{
	return NotifyEventHooks(hOkToExitEvent, 0, 0) == 0;
}

MIR_APP_DLL(void) Miranda_Close()
{
	while (!Miranda_OkToExit()) {
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		Sleep(0);
	}

	DestroyWindow(g_clistApi.hwndContactList);
}

/////////////////////////////////////////////////////////////////////////////////////////
// version functions

MIR_APP_DLL(uint32_t) Miranda_GetVersion()
{
	wchar_t filename[MAX_PATH];
	GetModuleFileName(g_plugin.getInst(), filename, _countof(filename));

	DWORD unused, verInfoSize = GetFileVersionInfoSize(filename, &unused);
	PVOID pVerInfo = _alloca(verInfoSize);
	GetFileVersionInfo(filename, 0, verInfoSize, pVerInfo);

	UINT blockSize;
	VS_FIXEDFILEINFO *vsffi;
	VerQueryValue(pVerInfo, L"\\", (PVOID*)&vsffi, &blockSize);
	return (((vsffi->dwProductVersionMS >> 16) & 0xFF) << 24) |
		((vsffi->dwProductVersionMS & 0xFF) << 16) |
		(((vsffi->dwProductVersionLS >> 16) & 0xFF) << 8) |
		(vsffi->dwProductVersionLS & 0xFF);
}

MIR_APP_DLL(void) Miranda_GetFileVersion(MFileVersion *pVer)
{
	wchar_t filename[MAX_PATH];
	GetModuleFileName(g_plugin.getInst(), filename, _countof(filename));

	DWORD unused, verInfoSize = GetFileVersionInfoSize(filename, &unused);
	PVOID pVerInfo = _alloca(verInfoSize);
	GetFileVersionInfo(filename, 0, verInfoSize, pVerInfo);

	UINT blockSize;
	VS_FIXEDFILEINFO *vsffi;
	VerQueryValue(pVerInfo, L"\\", (PVOID*)&vsffi, &blockSize);

	(*pVer)[0] = HIWORD(vsffi->dwProductVersionMS);
	(*pVer)[1] = LOWORD(vsffi->dwProductVersionMS);
	(*pVer)[2] = HIWORD(vsffi->dwProductVersionLS);
	(*pVer)[3] = LOWORD(vsffi->dwProductVersionLS);
}

MIR_APP_DLL(void) Miranda_GetVersionText(char *pDest, size_t cbSize)
{
	wchar_t filename[MAX_PATH], *productVersion;
	GetModuleFileName(g_plugin.getInst(), filename, _countof(filename));

	DWORD unused, verInfoSize = GetFileVersionInfoSize(filename, &unused);
	PVOID pVerInfo = _alloca(verInfoSize);
	GetFileVersionInfo(filename, 0, verInfoSize, pVerInfo);

	UINT blockSize;
	VerQueryValue(pVerInfo, L"\\StringFileInfo\\000004b0\\ProductVersion", (LPVOID*)&productVersion, &blockSize);
	strncpy_s(pDest, cbSize, _T2A(productVersion), _TRUNCATE);
#if defined(_WIN64)
	strcat_s(pDest, cbSize, " x64");
#endif
}

MIR_APP_DLL(CDlgBase *) Miranda_GetSystemWindow()
{
	return g_pSystemWindow;
}

///////////////////////////////////////////////////////////////////////////////

extern HANDLE hOptionsInitEvent;

int LoadSystemModule(void)
{
	hShutdownEvent = CreateHookableEvent(ME_SYSTEM_SHUTDOWN);
	hPreShutdownEvent = CreateHookableEvent(ME_SYSTEM_PRESHUTDOWN);
	hModulesLoadedEvent = CreateHookableEvent(ME_SYSTEM_MODULESLOADED);
	hOkToExitEvent = CreateHookableEvent(ME_SYSTEM_OKTOEXIT);
	hOptionsInitEvent = CreateHookableEvent(ME_OPT_INITIALISE);
	return 0;
}
