/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2012 Miranda ICQ/IM project, 
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

pfnMyMonitorFromPoint MyMonitorFromPoint;
pfnMyMonitorFromRect MyMonitorFromRect;
pfnMyMonitorFromWindow MyMonitorFromWindow;
pfnMyGetMonitorInfo MyGetMonitorInfo;

typedef DWORD (WINAPI *pfnMsgWaitForMultipleObjectsEx)(DWORD, CONST HANDLE*, DWORD, DWORD, DWORD);
pfnMsgWaitForMultipleObjectsEx msgWaitForMultipleObjectsEx;

pfnSHAutoComplete shAutoComplete;
pfnSHGetSpecialFolderPathA shGetSpecialFolderPathA;
pfnSHGetSpecialFolderPathW shGetSpecialFolderPathW;

pfnOpenInputDesktop openInputDesktop;
pfnCloseDesktop closeDesktop;

pfnAnimateWindow animateWindow;
pfnSetLayeredWindowAttributes setLayeredWindowAttributes;

pfnOpenThemeData openThemeData;
pfnIsThemeBackgroundPartiallyTransparent isThemeBackgroundPartiallyTransparent;
pfnDrawThemeParentBackground drawThemeParentBackground;
pfnDrawThemeBackground drawThemeBackground;
pfnDrawThemeText drawThemeText;
pfnDrawThemeTextEx drawThemeTextEx;
pfnGetThemeBackgroundContentRect getThemeBackgroundContentRect;
pfnGetThemeFont getThemeFont;
pfnCloseThemeData closeThemeData;
pfnEnableThemeDialogTexture enableThemeDialogTexture;
pfnSetWindowTheme setWindowTheme;
pfnSetWindowThemeAttribute setWindowThemeAttribute;
pfnIsThemeActive isThemeActive;
pfnBufferedPaintInit bufferedPaintInit;
pfnBufferedPaintUninit bufferedPaintUninit;
pfnBeginBufferedPaint beginBufferedPaint;
pfnEndBufferedPaint endBufferedPaint;
pfnGetBufferedPaintBits getBufferedPaintBits;

pfnDwmExtendFrameIntoClientArea dwmExtendFrameIntoClientArea;
pfnDwmIsCompositionEnabled dwmIsCompositionEnabled;

LPFN_GETADDRINFO MyGetaddrinfo;
LPFN_FREEADDRINFO MyFreeaddrinfo;
LPFN_WSASTRINGTOADDRESSA MyWSAStringToAddress;
LPFN_WSAADDRESSTOSTRINGA MyWSAAddressToString;

ITaskbarList3 * pTaskbarInterface;

HANDLE hOkToExitEvent, hModulesLoadedEvent;
HANDLE hShutdownEvent, hPreShutdownEvent;
static HANDLE hWaitObjects[MAXIMUM_WAIT_OBJECTS-1];
static char *pszWaitServices[MAXIMUM_WAIT_OBJECTS-1];
static int waitObjectCount = 0;
HANDLE hMirandaShutdown, hThreadQueueEmpty;
HINSTANCE hInst;
int hLangpack = 0;

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
// forkthread - starts a new thread

static INT_PTR ForkThreadService(WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)forkthread((pThreadFunc)wParam, 0, (void*)lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// forkthreadex - starts a new thread with the extended info and returns the thread id

static INT_PTR ForkThreadServiceEx(WPARAM wParam, LPARAM lParam)
{
	FORK_THREADEX_PARAMS* params = (FORK_THREADEX_PARAMS*)lParam;
	if (params == NULL)
		return 0;

	UINT threadID;
	return forkthreadex(NULL, params->iStackSize, params->pFunc, (void*)wParam, params->arg, params->threadID ? params->threadID : &threadID);
}

/////////////////////////////////////////////////////////////////////////////////////////

typedef LONG (WINAPI *pNtQIT)(HANDLE, LONG, PVOID, ULONG, PULONG);
#define ThreadQuerySetWin32StartAddress 9

void* GetCurrentThreadEntryPoint()
{
	LONG  ntStatus;
	HANDLE hDupHandle, hCurrentProcess;
	DWORD_PTR dwStartAddress;

	pNtQIT NtQueryInformationThread = (pNtQIT)GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "NtQueryInformationThread");
	if (NtQueryInformationThread == NULL) return 0;

	hCurrentProcess = GetCurrentProcess();
	if ( !DuplicateHandle(hCurrentProcess, GetCurrentThread(), hCurrentProcess, &hDupHandle, THREAD_QUERY_INFORMATION, FALSE, 0)) {
		SetLastError(ERROR_ACCESS_DENIED);
		return NULL;
	}
	ntStatus = NtQueryInformationThread(hDupHandle, ThreadQuerySetWin32StartAddress, &dwStartAddress, sizeof(DWORD_PTR), NULL);
	CloseHandle(hDupHandle);

	if (ntStatus != ERROR_SUCCESS) return 0;
	return (void*)dwStartAddress;
}

INT_PTR MirandaIsTerminated(WPARAM, LPARAM)
{
	return WaitForSingleObject(hMirandaShutdown, 0) == WAIT_OBJECT_0;
}

static void __cdecl compactHeapsThread(void*)
{
	while ( !Miranda_Terminated())
	{
		HANDLE hHeaps[256];
		DWORD hc;
		SleepEx((1000*60)*5, TRUE); // every 5 minutes
		hc=GetProcessHeaps(255, (PHANDLE)&hHeaps);
		if (hc != 0 && hc < 256) {
			DWORD j;
			for (j=0;j<hc;j++) HeapCompact(hHeaps[j], 0);
		}
	} //while
}

void (*SetIdleCallback) (void)=NULL;

static INT_PTR SystemSetIdleCallback(WPARAM, LPARAM lParam)
{
	if (lParam && SetIdleCallback == NULL) {
		SetIdleCallback=(void (*)(void))lParam;
		return 1;
	}
	return 0;
}

static DWORD dwEventTime=0;
void checkIdle(MSG * msg)
{
	switch(msg->message) {
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
	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
		SendDlgItemMessage(hwnd, IDC_PROGRESSBAR, PBM_SETRANGE, 0, MAKELPARAM(0, MIRANDA_PROCESS_WAIT_STEPS));
		SendDlgItemMessage(hwnd, IDC_PROGRESSBAR, PBM_SETSTEP, 1, 0);
		SetTimer(hwnd, 1, MIRANDA_PROCESS_WAIT_RESOLUTION, NULL);
		break;

	case WM_TIMER:
		if ( SendDlgItemMessage(hwnd, IDC_PROGRESSBAR, PBM_STEPIT, 0, 0) == MIRANDA_PROCESS_WAIT_STEPS)
			EndDialog(hwnd, 0);
		if ( WaitForSingleObject((HANDLE)GetWindowLongPtr(hwnd, GWLP_USERDATA), 1) != WAIT_TIMEOUT)
		{
			SendDlgItemMessage(hwnd, IDC_PROGRESSBAR, PBM_SETPOS, MIRANDA_PROCESS_WAIT_STEPS, 0);
			EndDialog(hwnd, 0);
		}
		break;

	case WM_COMMAND:
		if ( HIWORD(wParam) == IDCANCEL)
		{
			SendDlgItemMessage(hwnd, IDC_PROGRESSBAR, PBM_SETPOS, MIRANDA_PROCESS_WAIT_STEPS, 0);
			EndDialog(hwnd, 0);
		}
		break;
	}
	return FALSE;
}

void ParseCommandLine()
{
	char* cmdline = GetCommandLineA();
	char* p = strstr(cmdline, "/restart:");
	if (p) {
		HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, atol(p+9));
		if (hProcess) {
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_WAITRESTART), NULL, WaitForProcessDlgProc, (LPARAM)hProcess);
			CloseHandle(hProcess);
		}	
	}	
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	DWORD myPid=0;
	int messageloop=1;
	HMODULE hUser32, hThemeAPI, hDwmApi, hShFolder = NULL;
	int result = 0;

	hInst = hInstance;

	setlocale(LC_ALL, "");

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	hUser32 = GetModuleHandleA("user32");
	openInputDesktop = (pfnOpenInputDesktop)GetProcAddress (hUser32, "OpenInputDesktop");
	closeDesktop = (pfnCloseDesktop)GetProcAddress (hUser32, "CloseDesktop");
	msgWaitForMultipleObjectsEx = (pfnMsgWaitForMultipleObjectsEx)GetProcAddress(hUser32, "MsgWaitForMultipleObjectsEx");
	animateWindow =(pfnAnimateWindow)GetProcAddress(hUser32, "AnimateWindow");
	setLayeredWindowAttributes =(pfnSetLayeredWindowAttributes)GetProcAddress(hUser32, "SetLayeredWindowAttributes");

	MyMonitorFromPoint = (pfnMyMonitorFromPoint)GetProcAddress(hUser32, "MonitorFromPoint");
	MyMonitorFromRect = (pfnMyMonitorFromRect)GetProcAddress(hUser32, "MonitorFromRect");
	MyMonitorFromWindow = (pfnMyMonitorFromWindow)GetProcAddress(hUser32, "MonitorFromWindow");
	MyGetMonitorInfo = (pfnMyGetMonitorInfo)GetProcAddress(hUser32, "GetMonitorInfoW");

	hShFolder = GetModuleHandleA("shell32");
	shGetSpecialFolderPathA = (pfnSHGetSpecialFolderPathA)GetProcAddress(hShFolder, "SHGetSpecialFolderPathA");
	shGetSpecialFolderPathW = (pfnSHGetSpecialFolderPathW)GetProcAddress(hShFolder, "SHGetSpecialFolderPathW");
	if (shGetSpecialFolderPathA == NULL) {
		hShFolder = LoadLibraryA("ShFolder.dll");
		shGetSpecialFolderPathA = (pfnSHGetSpecialFolderPathA)GetProcAddress(hShFolder, "SHGetSpecialFolderPathA");
		shGetSpecialFolderPathW = (pfnSHGetSpecialFolderPathW)GetProcAddress(hShFolder, "SHGetSpecialFolderPathW");
	}

	shAutoComplete = (pfnSHAutoComplete)GetProcAddress(GetModuleHandleA("shlwapi"), "SHAutoComplete");

	if ( IsWinVerXPPlus()) {
		hThemeAPI = LoadLibraryA("uxtheme.dll");
		if (hThemeAPI) {
			openThemeData = (pfnOpenThemeData)GetProcAddress(hThemeAPI, "OpenThemeData");
			isThemeBackgroundPartiallyTransparent = (pfnIsThemeBackgroundPartiallyTransparent)GetProcAddress(hThemeAPI, "IsThemeBackgroundPartiallyTransparent");
			drawThemeParentBackground  = (pfnDrawThemeParentBackground)GetProcAddress(hThemeAPI, "DrawThemeParentBackground");
			drawThemeBackground = (pfnDrawThemeBackground)GetProcAddress(hThemeAPI, "DrawThemeBackground");
			drawThemeText = (pfnDrawThemeText)GetProcAddress(hThemeAPI, "DrawThemeText");
			drawThemeTextEx = (pfnDrawThemeTextEx)GetProcAddress(hThemeAPI, "DrawThemeTextEx");
			getThemeBackgroundContentRect = (pfnGetThemeBackgroundContentRect)GetProcAddress(hThemeAPI , "GetThemeBackgroundContentRect");
			getThemeFont = (pfnGetThemeFont)GetProcAddress(hThemeAPI, "GetThemeFont");
			closeThemeData  = (pfnCloseThemeData)GetProcAddress(hThemeAPI, "CloseThemeData");
			enableThemeDialogTexture = (pfnEnableThemeDialogTexture)GetProcAddress(hThemeAPI, "EnableThemeDialogTexture");
			setWindowTheme = (pfnSetWindowTheme)GetProcAddress(hThemeAPI, "SetWindowTheme");
			setWindowThemeAttribute = (pfnSetWindowThemeAttribute)GetProcAddress(hThemeAPI, "SetWindowThemeAttribute");
			isThemeActive = (pfnIsThemeActive)GetProcAddress(hThemeAPI, "IsThemeActive");
			bufferedPaintInit = (pfnBufferedPaintInit)GetProcAddress(hThemeAPI, "BufferedPaintInit");
			bufferedPaintUninit = (pfnBufferedPaintUninit)GetProcAddress(hThemeAPI, "BufferedPaintUninit");
			beginBufferedPaint = (pfnBeginBufferedPaint)GetProcAddress(hThemeAPI, "BeginBufferedPaint");
			endBufferedPaint = (pfnEndBufferedPaint)GetProcAddress(hThemeAPI, "EndBufferedPaint");
			getBufferedPaintBits = (pfnGetBufferedPaintBits)GetProcAddress(hThemeAPI, "GetBufferedPaintBits");
		}
	}

	if ( IsWinVerVistaPlus()) {
		hDwmApi = LoadLibraryA("dwmapi.dll");
		if (hDwmApi) {
			dwmExtendFrameIntoClientArea = (pfnDwmExtendFrameIntoClientArea)GetProcAddress(hDwmApi, "DwmExtendFrameIntoClientArea");
			dwmIsCompositionEnabled = (pfnDwmIsCompositionEnabled)GetProcAddress(hDwmApi, "DwmIsCompositionEnabled");
		}
	}

	HMODULE hWinSock = GetModuleHandleA("ws2_32");
	MyGetaddrinfo = (LPFN_GETADDRINFO)GetProcAddress(hWinSock, "getaddrinfo");
	MyFreeaddrinfo = (LPFN_FREEADDRINFO)GetProcAddress(hWinSock, "freeaddrinfo");
	MyWSAStringToAddress = (LPFN_WSASTRINGTOADDRESSA)GetProcAddress(hWinSock, "WSAStringToAddressA");
	MyWSAAddressToString = (LPFN_WSAADDRESSTOSTRINGA)GetProcAddress(hWinSock, "WSAAddressToStringA");

	if (bufferedPaintInit)
		bufferedPaintInit();

	OleInitialize(NULL);

	if ( IsWinVer7Plus())
		CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList3, (void**)&pTaskbarInterface);

	if ( LoadDefaultModules()) {
		NotifyEventHooks(hShutdownEvent, 0, 0);
		UnloadDefaultModules();

		result = 1;
		goto exit;
	}
	NotifyEventHooks(hModulesLoadedEvent, 0, 0);

	// ensure that the kernel hooks the SystemShutdownProc() after all plugins
	HookEvent(ME_SYSTEM_SHUTDOWN, SystemShutdownProc);

	forkthread(compactHeapsThread, 0, NULL);
	CreateServiceFunction(MS_SYSTEM_SETIDLECALLBACK, SystemSetIdleCallback);
	CreateServiceFunction(MS_SYSTEM_GETIDLE, SystemGetIdle);
	dwEventTime=GetTickCount();
	myPid=GetCurrentProcessId();
	while (messageloop) {
		MSG msg;
		DWORD rc;
		BOOL dying=FALSE;
		rc = MsgWaitForMultipleObjectsEx(waitObjectCount, hWaitObjects, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE);
		if (rc >= WAIT_OBJECT_0 && rc < WAIT_OBJECT_0 + waitObjectCount) {
			rc -= WAIT_OBJECT_0;
			CallService(pszWaitServices[rc], (WPARAM) hWaitObjects[rc], 0);
		}
		//
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message != WM_QUIT) {
				HWND h=GetForegroundWindow();
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
			else if ( !dying) {
				dying++;
				SetEvent(hMirandaShutdown);
				NotifyEventHooks(hPreShutdownEvent, 0, 0);

				// this spins and processes the msg loop, objects and APC.
				UnwindThreadWait();
				NotifyEventHooks(hShutdownEvent, 0, 0);
				// if the hooks generated any messages, it'll get processed before the second WM_QUIT
				PostQuitMessage(0);
			}
			else if (dying)
				messageloop=0;
		}
	}

exit:
	UnloadNewPluginsModule();
	CloseHandle(hMirandaShutdown);
	CloseHandle(hThreadQueueEmpty);

	if (pTaskbarInterface)
		pTaskbarInterface->Release();

	OleUninitialize();

	if (bufferedPaintUninit) bufferedPaintUninit();

	if (hDwmApi) FreeLibrary(hDwmApi);
	if (hThemeAPI) FreeLibrary(hThemeAPI);
	if (hShFolder) FreeLibrary(hShFolder);

	return result;
}

static INT_PTR OkToExit(WPARAM, LPARAM)
{
	return NotifyEventHooks(hOkToExitEvent, 0, 0) == 0;
}

static INT_PTR GetMirandaVersion(WPARAM, LPARAM)
{
	TCHAR filename[MAX_PATH];
	DWORD unused;
	DWORD verInfoSize;
	UINT blockSize;
	PVOID pVerInfo;
	VS_FIXEDFILEINFO *vsffi;
	DWORD ver;

	GetModuleFileName(NULL, filename, SIZEOF(filename));
	verInfoSize=GetFileVersionInfoSize(filename, &unused);
	pVerInfo=mir_alloc(verInfoSize);
	GetFileVersionInfo(filename, 0, verInfoSize, pVerInfo);
	VerQueryValue(pVerInfo, _T("\\"), (PVOID*)&vsffi, &blockSize);
	ver=(((vsffi->dwProductVersionMS>>16)&0xFF)<<24)|
		((vsffi->dwProductVersionMS&0xFF)<<16)|
		(((vsffi->dwProductVersionLS>>16)&0xFF)<<8)|
		(vsffi->dwProductVersionLS&0xFF);
	mir_free(pVerInfo);
	return (INT_PTR)ver;
}

static INT_PTR GetMirandaVersionText(WPARAM wParam, LPARAM lParam)
{
	TCHAR filename[MAX_PATH], *productVersion;
	DWORD unused;
	DWORD verInfoSize;
	UINT blockSize;
	PVOID pVerInfo;

	GetModuleFileName(NULL, filename, SIZEOF(filename));
	verInfoSize=GetFileVersionInfoSize(filename, &unused);
	pVerInfo=mir_alloc(verInfoSize);
	GetFileVersionInfo(filename, 0, verInfoSize, pVerInfo);
	VerQueryValue(pVerInfo, _T("\\StringFileInfo\\000004b0\\ProductVersion"), (LPVOID*)&productVersion, &blockSize);
	#if defined(_WIN64)
		mir_snprintf((char*)lParam, wParam, "%S x64 Unicode", productVersion);
	#else
		mir_snprintf((char*)lParam, wParam, "%S Unicode", productVersion);
	#endif
	mir_free(pVerInfo);
	return 0;
}

INT_PTR WaitOnHandle(WPARAM wParam, LPARAM lParam)
{
	if (waitObjectCount >= MAXIMUM_WAIT_OBJECTS-1)
		return 1;
	
	hWaitObjects[waitObjectCount] = (HANDLE)wParam;
	pszWaitServices[waitObjectCount] = (char*)lParam;
	waitObjectCount++;
	return 0;
}

static INT_PTR RemoveWait(WPARAM wParam, LPARAM)
{
	int i;

	for (i=0;i<waitObjectCount;i++)
		if (hWaitObjects[i] == (HANDLE)wParam) 
			break;

	if (i == waitObjectCount)
		return 1;

	waitObjectCount--;
	MoveMemory(&hWaitObjects[i], &hWaitObjects[i+1], sizeof(HANDLE)*(waitObjectCount-i));
	MoveMemory(&pszWaitServices[i], &pszWaitServices[i+1], sizeof(char*)*(waitObjectCount-i));
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

struct MM_INTERFACE
{
	size_t cbSize;
	void* (*mmi_malloc) (size_t);
	void* (*mmi_realloc) (void*, size_t);
	void  (*mmi_free) (void*);

	void*    (*mmi_calloc) (size_t);
	char*    (*mmi_strdup) (const char *src);
	wchar_t* (*mmi_wstrdup) (const wchar_t *src);
	int      (*mir_snprintf) (char *buffer, size_t count, const char* fmt, ...);
	int      (*mir_sntprintf) (TCHAR *buffer, size_t count, const TCHAR* fmt, ...);
	int      (*mir_vsnprintf) (char *buffer, size_t count, const char* fmt, va_list va);
	int      (*mir_vsntprintf) (TCHAR *buffer, size_t count, const TCHAR* fmt, va_list va);

	wchar_t* (*mir_a2u_cp) (const char* src, int codepage);
	wchar_t* (*mir_a2u)(const char* src);
	char*    (*mir_u2a_cp)(const wchar_t* src, int codepage);
	char*    (*mir_u2a)(const wchar_t* src);
};

#define MS_SYSTEM_GET_MMI  "Miranda/System/GetMMI"

INT_PTR GetMemoryManagerInterface(WPARAM, LPARAM lParam)
{
	struct MM_INTERFACE *mmi = (struct MM_INTERFACE*) lParam;
	if (mmi == NULL)
		return 1;

	mmi->mmi_malloc = mir_alloc;
	mmi->mmi_realloc = mir_realloc;
	mmi->mmi_free = mir_free;

	switch(mmi->cbSize) {
	case sizeof(struct MM_INTERFACE):
		mmi->mir_snprintf = mir_snprintf;
		mmi->mir_sntprintf = mir_sntprintf;
		mmi->mir_vsnprintf = mir_vsnprintf;
		mmi->mir_vsntprintf = mir_vsntprintf;
		mmi->mir_a2u_cp = mir_a2u_cp;
		mmi->mir_a2u = mir_a2u;
		mmi->mir_u2a_cp = mir_u2a_cp;
		mmi->mir_u2a = mir_u2a;
		// fall through

	case MMI_SIZE_V2:
		mmi->mmi_calloc = mir_calloc;
		mmi->mmi_strdup = mir_strdup;
		mmi->mmi_wstrdup = mir_wstrdup;
		// fall through

	case MMI_SIZE_V1:
		break;

	default:
#if defined(_DEBUG)
		DebugBreak();
#endif
		return 1;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

struct LIST_INTERFACE
{
	size_t    cbSize;

	SortedList* (*List_Create)(int, int);
	void        (*List_Destroy)(SortedList*);

	void* (*List_Find)(SortedList*, void*);
	int   (*List_GetIndex)(SortedList*, void*, int*);
	int   (*List_Insert)(SortedList*, void*, int);
	int   (*List_Remove)(SortedList*, int);
	int   (*List_IndexOf)(SortedList*, void*);

	int   (*List_InsertPtr)(SortedList* list, void* p);
	int   (*List_RemovePtr)(SortedList* list, void* p);

	void  (*List_Copy)(SortedList* src, SortedList* dst, size_t);
	void  (*List_ObjCopy)(SortedList* src, SortedList* dst, size_t);
};

#define MS_SYSTEM_GET_LI  "Miranda/System/GetLI"

INT_PTR GetListInterface(WPARAM, LPARAM lParam)
{
	struct LIST_INTERFACE *li = (struct LIST_INTERFACE*) lParam;
	if (li == NULL)
		return 1;

	li->List_Copy      = List_Copy;
	li->List_ObjCopy   = List_ObjCopy;
	li->List_InsertPtr = List_InsertPtr;
	li->List_RemovePtr = List_RemovePtr;
	li->List_Create    = List_Create;
	li->List_Destroy   = List_Destroy;
	li->List_Find      = List_Find;
	li->List_GetIndex  = List_GetIndex;
	li->List_Insert    = List_Insert;
	li->List_Remove    = List_Remove;
	li->List_IndexOf   = List_IndexOf;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

struct UTF8_INTERFACE
{
	size_t cbSize;

	// decodes utf8 and places the result back into the same buffer.
	// if the second parameter is present, the additional wchar_t* string gets allocated,
	// and filled with the decoded utf8 content without any information loss.
	// this string should be freed using mir_free()
	char* (*utf8_decode)(char* str, wchar_t** ucs2);
	char* (*utf8_decodecp)(char* str, int codepage, wchar_t** ucs2);

	// encodes an ANSI string into a utf8 format using the current langpack code page,
	// or CP_ACP, if lanpack is missing
	// the resulting string should be freed using mir_free
	char* (*utf8_encode)(const char* src);
	char* (*utf8_encodecp)(const char* src, int codepage);

	// encodes an WCHAR string into a utf8 format
	// the resulting string should be freed using mir_free
	char* (*utf8_encodeW)(const wchar_t* src);

	// decodes utf8 and returns the result as wchar_t* that should be freed using mir_free()
	// the input buffer remains unchanged
	wchar_t* (*utf8_decodeW)(const char* str);

	// returns the predicted length of the utf-8 string
	int (*utf8_lenW)(const wchar_t* src);
};

#define MS_SYSTEM_GET_UTFI  "Miranda/System/GetUTFI"

INT_PTR GetUtfInterface(WPARAM, LPARAM lParam)
{
	struct UTF8_INTERFACE *utfi = (struct UTF8_INTERFACE*) lParam;
	if (utfi == NULL)
		return 1;

	if (utfi->cbSize != sizeof(UTF8_INTERFACE))
		return 1;

	utfi->utf8_decode   = Utf8Decode;
	utfi->utf8_decodecp = Utf8DecodeCP;
	utfi->utf8_encode   = Utf8Encode;
	utfi->utf8_encodecp = Utf8EncodeCP;
	utfi->utf8_encodeW  = Utf8EncodeW;
	utfi->utf8_decodeW = Utf8DecodeW;
	utfi->utf8_lenW = Ucs2toUtf8Len;
	return 0;
}

int LoadSystemModule(void)
{
	hMirandaShutdown = CreateEvent(NULL, TRUE, FALSE, NULL);
	hThreadQueueEmpty = CreateEvent(NULL, TRUE, TRUE, NULL);

	hShutdownEvent = CreateHookableEvent(ME_SYSTEM_SHUTDOWN);
	hPreShutdownEvent = CreateHookableEvent(ME_SYSTEM_PRESHUTDOWN);
	hModulesLoadedEvent = CreateHookableEvent(ME_SYSTEM_MODULESLOADED);
	hOkToExitEvent = CreateHookableEvent(ME_SYSTEM_OKTOEXIT);

	CreateServiceFunction(MS_SYSTEM_FORK_THREAD, ForkThreadService);
	CreateServiceFunction(MS_SYSTEM_FORK_THREAD_EX, ForkThreadServiceEx);
	CreateServiceFunction(MS_SYSTEM_THREAD_PUSH, UnwindThreadPush);
	CreateServiceFunction(MS_SYSTEM_THREAD_POP, UnwindThreadPop);
	CreateServiceFunction(MS_SYSTEM_TERMINATED, MirandaIsTerminated);
	CreateServiceFunction(MS_SYSTEM_OKTOEXIT, OkToExit);
	CreateServiceFunction(MS_SYSTEM_GETVERSION, GetMirandaVersion);
	CreateServiceFunction(MS_SYSTEM_GETVERSIONTEXT, GetMirandaVersionText);
	CreateServiceFunction(MS_SYSTEM_WAITONHANDLE, WaitOnHandle);
	CreateServiceFunction(MS_SYSTEM_REMOVEWAIT, RemoveWait);
	CreateServiceFunction(MS_SYSTEM_GET_LI, GetListInterface);
	CreateServiceFunction(MS_SYSTEM_GET_MMI, GetMemoryManagerInterface);
	CreateServiceFunction(MS_SYSTEM_GET_UTFI, GetUtfInterface);
	CreateServiceFunction(MS_SYSTEM_GETEXCEPTFILTER, srvGetExceptionFilter);
	CreateServiceFunction(MS_SYSTEM_SETEXCEPTFILTER, srvSetExceptionFilter);
	return 0;
}
