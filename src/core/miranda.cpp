/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#define MMI_SIZE_V1 (4*sizeof(void*))
#define MMI_SIZE_V2 (7*sizeof(void*))

int LoadDefaultModules(void);
void UnloadNewPluginsModule(void);
void UnloadDefaultModules(void);

pfnDrawThemeTextEx drawThemeTextEx;
pfnSetWindowThemeAttribute setWindowThemeAttribute;
pfnBufferedPaintInit bufferedPaintInit;
pfnBufferedPaintUninit bufferedPaintUninit;
pfnBeginBufferedPaint beginBufferedPaint;
pfnEndBufferedPaint endBufferedPaint;
pfnGetBufferedPaintBits getBufferedPaintBits;

pfnDwmExtendFrameIntoClientArea dwmExtendFrameIntoClientArea;
pfnDwmIsCompositionEnabled dwmIsCompositionEnabled;

ITaskbarList3 * pTaskbarInterface;

HANDLE hOkToExitEvent, hModulesLoadedEvent;
HANDLE hShutdownEvent, hPreShutdownEvent;
static HANDLE hWaitObjects[MAXIMUM_WAIT_OBJECTS-1];
static char *pszWaitServices[MAXIMUM_WAIT_OBJECTS-1];
static int waitObjectCount = 0;
HANDLE hMirandaShutdown;
HINSTANCE hInst;
DWORD hMainThreadId;
int hLangpack = 0;
bool bModulesLoadedFired = false;

/////////////////////////////////////////////////////////////////////////////////////////
// exception handling

static INT_PTR srvGetExceptionFilter(WPARAM, LPARAM)
{
	return (INT_PTR)GetExceptionFilter();
}

static INT_PTR srvSetExceptionFilter(WPARAM, LPARAM lParam)
{
	return (INT_PTR)SetExceptionFilter((pfnExceptionFilter)lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

typedef LONG(WINAPI *pNtQIT)(HANDLE, LONG, PVOID, ULONG, PULONG);
#define ThreadQuerySetWin32StartAddress 9

INT_PTR MirandaIsTerminated(WPARAM, LPARAM)
{
	return WaitForSingleObject(hMirandaShutdown, 0) == WAIT_OBJECT_0;
}

static void __cdecl compactHeapsThread(void*)
{
	Thread_SetName("compactHeapsThread");

	while (!Miranda_Terminated()) {
		HANDLE hHeaps[256];
		DWORD hc;
		SleepEx((1000 * 60) * 5, TRUE); // every 5 minutes
		hc = GetProcessHeaps(255, (PHANDLE)&hHeaps);
		if (hc != 0 && hc < 256) {
			DWORD j;
			for (j = 0; j < hc; j++)
				HeapCompact(hHeaps[j], 0);
		}
	} //while
}

void(*SetIdleCallback) (void) = NULL;

static INT_PTR SystemSetIdleCallback(WPARAM, LPARAM lParam)
{
	if (lParam && SetIdleCallback == NULL) {
		SetIdleCallback = (void(*)(void))lParam;
		return 1;
	}
	return 0;
}

static DWORD dwEventTime = 0;
void checkIdle(MSG * msg)
{
	switch (msg->message) {
	case WM_MOUSEACTIVATE:
	case WM_MOUSEMOVE:
	case WM_CHAR:
		dwEventTime = GetTickCount();
	}
}

static INT_PTR SystemGetIdle(WPARAM, LPARAM lParam)
{
	if (lParam) *(DWORD*)lParam = dwEventTime;
	return 0;
}

static int SystemShutdownProc(WPARAM, LPARAM)
{
	UnloadDefaultModules();
	return 0;
}

#define MIRANDA_PROCESS_WAIT_TIMEOUT        60000
#define MIRANDA_PROCESS_WAIT_RESOLUTION     1000
#define MIRANDA_PROCESS_WAIT_STEPS          (MIRANDA_PROCESS_WAIT_TIMEOUT/MIRANDA_PROCESS_WAIT_RESOLUTION)
static INT_PTR CALLBACK WaitForProcessDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
		SendDlgItemMessage(hwnd, IDC_PROGRESSBAR, PBM_SETRANGE, 0, MAKELPARAM(0, MIRANDA_PROCESS_WAIT_STEPS));
		SendDlgItemMessage(hwnd, IDC_PROGRESSBAR, PBM_SETSTEP, 1, 0);
		SetTimer(hwnd, 1, MIRANDA_PROCESS_WAIT_RESOLUTION, NULL);
		break;

	case WM_TIMER:
		if (SendDlgItemMessage(hwnd, IDC_PROGRESSBAR, PBM_STEPIT, 0, 0) == MIRANDA_PROCESS_WAIT_STEPS)
			EndDialog(hwnd, 0);
		if (WaitForSingleObject((HANDLE)GetWindowLongPtr(hwnd, GWLP_USERDATA), 1) != WAIT_TIMEOUT) {
			SendDlgItemMessage(hwnd, IDC_PROGRESSBAR, PBM_SETPOS, MIRANDA_PROCESS_WAIT_STEPS, 0);
			EndDialog(hwnd, 0);
		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL) {
			SendDlgItemMessage(hwnd, IDC_PROGRESSBAR, PBM_SETPOS, MIRANDA_PROCESS_WAIT_STEPS, 0);
			EndDialog(hwnd, 1);
		}
		break;
	}
	return FALSE;
}

INT_PTR CheckRestart()
{
	LPCTSTR tszPID = CmdLine_GetOption(_T("restart"));
	if (tszPID) {
		HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, _ttol(tszPID));
		if (hProcess) {
			INT_PTR result = DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_WAITRESTART), NULL, WaitForProcessDlgProc, (LPARAM)hProcess);
			CloseHandle(hProcess);
			return result;
		}
	}
	return 0;
}

static void crtErrorHandler(const wchar_t*, const wchar_t*, const wchar_t*, unsigned, uintptr_t)
{}


int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR cmdLine, int)
{
	hInst = hInstance;
	hMainThreadId = GetCurrentThreadId();

	_set_invalid_parameter_handler(&crtErrorHandler);
#ifdef _DEBUG
	_CrtSetReportMode(_CRT_ASSERT, 0);
#endif

	CmdLine_Parse(cmdLine);
	setlocale(LC_ALL, "");

#ifdef _DEBUG
	if (CmdLine_GetOption(_T("memdebug")))
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	HMODULE hDwmApi, hThemeAPI;
	if (IsWinVerVistaPlus()) {
		hDwmApi = LoadLibrary(_T("dwmapi.dll"));
		if (hDwmApi) {
			dwmExtendFrameIntoClientArea = (pfnDwmExtendFrameIntoClientArea)GetProcAddress(hDwmApi, "DwmExtendFrameIntoClientArea");
			dwmIsCompositionEnabled = (pfnDwmIsCompositionEnabled)GetProcAddress(hDwmApi, "DwmIsCompositionEnabled");
		}
		hThemeAPI = LoadLibrary(_T("uxtheme.dll"));
		if (hThemeAPI) {
			drawThemeTextEx = (pfnDrawThemeTextEx)GetProcAddress(hThemeAPI, "DrawThemeTextEx");
			setWindowThemeAttribute = (pfnSetWindowThemeAttribute)GetProcAddress(hThemeAPI, "SetWindowThemeAttribute");
			bufferedPaintInit = (pfnBufferedPaintInit)GetProcAddress(hThemeAPI, "BufferedPaintInit");
			bufferedPaintUninit = (pfnBufferedPaintUninit)GetProcAddress(hThemeAPI, "BufferedPaintUninit");
			beginBufferedPaint = (pfnBeginBufferedPaint)GetProcAddress(hThemeAPI, "BeginBufferedPaint");
			endBufferedPaint = (pfnEndBufferedPaint)GetProcAddress(hThemeAPI, "EndBufferedPaint");
			getBufferedPaintBits = (pfnGetBufferedPaintBits)GetProcAddress(hThemeAPI, "GetBufferedPaintBits");
		}
	}
	else hDwmApi = hThemeAPI = 0;

	if (bufferedPaintInit)
		bufferedPaintInit();

	OleInitialize(NULL);

	if (IsWinVer7Plus())
		CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList3, (void**)&pTaskbarInterface);

	int result = 0;
	if (LoadDefaultModules()) {
		SetEvent(hMirandaShutdown);
		NotifyEventHooks(hPreShutdownEvent, 0, 0);
		NotifyEventHooks(hShutdownEvent, 0, 0);
		UnloadDefaultModules();

		result = 1;
	}
	else {
		InitPathVar();
		NotifyEventHooks(hModulesLoadedEvent, 0, 0);
		bModulesLoadedFired = true;

		// ensure that the kernel hooks the SystemShutdownProc() after all plugins
		HookEvent(ME_SYSTEM_SHUTDOWN, SystemShutdownProc);

		forkthread(compactHeapsThread, 0, NULL);
		CreateServiceFunction(MS_SYSTEM_SETIDLECALLBACK, SystemSetIdleCallback);
		CreateServiceFunction(MS_SYSTEM_GETIDLE, SystemGetIdle);
		dwEventTime = GetTickCount();
		DWORD myPid = GetCurrentProcessId();

		bool messageloop = true;
		while (messageloop) {
			MSG msg;
			BOOL dying = FALSE;
			DWORD rc = MsgWaitForMultipleObjectsEx(waitObjectCount, hWaitObjects, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE);
			if (rc < WAIT_OBJECT_0 + waitObjectCount) {
				rc -= WAIT_OBJECT_0;
				CallService(pszWaitServices[rc], (WPARAM)hWaitObjects[rc], 0);
			}
			//
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message != WM_QUIT) {
					HWND h = GetForegroundWindow();
					DWORD pid = 0;
					checkIdle(&msg);
					if (h != NULL && GetWindowThreadProcessId(h, &pid) && pid == myPid && GetClassLongPtr(h, GCW_ATOM) == 32770)
						if (IsDialogMessage(h, &msg))
							continue;

					TranslateMessage(&msg);
					DispatchMessage(&msg);
					if (SetIdleCallback != NULL)
						SetIdleCallback();
				}
				else if (!dying) {
					dying++;
					SetEvent(hMirandaShutdown);
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
	CloseHandle(hMirandaShutdown);
	FreeLibrary(hDwmApi);
	FreeLibrary(hThemeAPI);

	if (pTaskbarInterface)
		pTaskbarInterface->Release();

	OleUninitialize();

	if (bufferedPaintUninit)
		bufferedPaintUninit();
	return result;
}

static INT_PTR OkToExit(WPARAM, LPARAM)
{
	return NotifyEventHooks(hOkToExitEvent, 0, 0) == 0;
}

static INT_PTR GetMirandaVersion(WPARAM, LPARAM)
{
	TCHAR filename[MAX_PATH];
	GetModuleFileName(NULL, filename, SIZEOF(filename));

	DWORD unused, verInfoSize = GetFileVersionInfoSize(filename, &unused);
	PVOID pVerInfo = _alloca(verInfoSize);
	GetFileVersionInfo(filename, 0, verInfoSize, pVerInfo);

	UINT blockSize;
	VS_FIXEDFILEINFO *vsffi;
	VerQueryValue(pVerInfo, _T("\\"), (PVOID*)&vsffi, &blockSize);
	DWORD ver = (((vsffi->dwProductVersionMS >> 16) & 0xFF) << 24) |
		((vsffi->dwProductVersionMS & 0xFF) << 16) |
		(((vsffi->dwProductVersionLS >> 16) & 0xFF) << 8) |
		(vsffi->dwProductVersionLS & 0xFF);
	return (INT_PTR)ver;
}

static INT_PTR GetMirandaFileVersion(WPARAM, LPARAM lParam)
{
	TCHAR filename[MAX_PATH];
	GetModuleFileName(NULL, filename, SIZEOF(filename));

	DWORD unused, verInfoSize = GetFileVersionInfoSize(filename, &unused);
	PVOID pVerInfo = _alloca(verInfoSize);
	GetFileVersionInfo(filename, 0, verInfoSize, pVerInfo);

	UINT blockSize;
	VS_FIXEDFILEINFO *vsffi;
	VerQueryValue(pVerInfo, _T("\\"), (PVOID*)&vsffi, &blockSize);

	WORD* p = (WORD*)lParam;
	p[0] = HIWORD(vsffi->dwProductVersionMS);
	p[1] = LOWORD(vsffi->dwProductVersionMS);
	p[2] = HIWORD(vsffi->dwProductVersionLS);
	p[3] = LOWORD(vsffi->dwProductVersionLS);
	return 0;
}

static INT_PTR GetMirandaVersionText(WPARAM wParam, LPARAM lParam)
{
	TCHAR filename[MAX_PATH], *productVersion;
	GetModuleFileName(NULL, filename, SIZEOF(filename));

	DWORD unused, verInfoSize = GetFileVersionInfoSize(filename, &unused);
	PVOID pVerInfo = _alloca(verInfoSize);
	GetFileVersionInfo(filename, 0, verInfoSize, pVerInfo);

	UINT blockSize;
	VerQueryValue(pVerInfo, _T("\\StringFileInfo\\000004b0\\ProductVersion"), (LPVOID*)&productVersion, &blockSize);
	strncpy((char*)lParam, _T2A(productVersion), wParam);
#if defined(_WIN64)
	strcat_s((char*)lParam, wParam, " x64");
#endif
	return 0;
}

INT_PTR WaitOnHandle(WPARAM wParam, LPARAM lParam)
{
	if (waitObjectCount >= MAXIMUM_WAIT_OBJECTS - 1)
		return 1;

	hWaitObjects[waitObjectCount] = (HANDLE)wParam;
	pszWaitServices[waitObjectCount] = (char*)lParam;
	waitObjectCount++;
	return 0;
}

static INT_PTR RemoveWait(WPARAM wParam, LPARAM)
{
	int i;

	for (i = 0; i < waitObjectCount; i++)
		if (hWaitObjects[i] == (HANDLE)wParam)
			break;

	if (i == waitObjectCount)
		return 1;

	waitObjectCount--;
	memmove(&hWaitObjects[i], &hWaitObjects[i + 1], sizeof(HANDLE)*(waitObjectCount - i));
	memmove(&pszWaitServices[i], &pszWaitServices[i + 1], sizeof(char*)*(waitObjectCount - i));
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

int LoadSystemModule(void)
{
	hMirandaShutdown = CreateEvent(NULL, TRUE, FALSE, NULL);

	hShutdownEvent = CreateHookableEvent(ME_SYSTEM_SHUTDOWN);
	hPreShutdownEvent = CreateHookableEvent(ME_SYSTEM_PRESHUTDOWN);
	hModulesLoadedEvent = CreateHookableEvent(ME_SYSTEM_MODULESLOADED);
	hOkToExitEvent = CreateHookableEvent(ME_SYSTEM_OKTOEXIT);

	CreateServiceFunction(MS_SYSTEM_TERMINATED, MirandaIsTerminated);
	CreateServiceFunction(MS_SYSTEM_OKTOEXIT, OkToExit);
	CreateServiceFunction(MS_SYSTEM_GETVERSION, GetMirandaVersion);
	CreateServiceFunction(MS_SYSTEM_GETFILEVERSION, GetMirandaFileVersion);
	CreateServiceFunction(MS_SYSTEM_GETVERSIONTEXT, GetMirandaVersionText);
	CreateServiceFunction(MS_SYSTEM_WAITONHANDLE, WaitOnHandle);
	CreateServiceFunction(MS_SYSTEM_REMOVEWAIT, RemoveWait);
	CreateServiceFunction(MS_SYSTEM_GETEXCEPTFILTER, srvGetExceptionFilter);
	CreateServiceFunction(MS_SYSTEM_SETEXCEPTFILTER, srvSetExceptionFilter);
	return 0;
}
