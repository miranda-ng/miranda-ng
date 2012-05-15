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

int InitPathUtils(void);
int InitialiseModularEngine(void);
int LoadDefaultModules(void);
void DestroyModularEngine(void);
void UnloadNewPluginsModule(void);
void UnloadDefaultModules(void);
void RecalculateTime(void);

HINSTANCE GetInstByAddress( void* codePtr );

pfnMyMonitorFromPoint MyMonitorFromPoint;
pfnMyMonitorFromRect MyMonitorFromRect;
pfnMyMonitorFromWindow MyMonitorFromWindow;
pfnMyGetMonitorInfo MyGetMonitorInfo;

typedef DWORD (WINAPI *pfnMsgWaitForMultipleObjectsEx)(DWORD,CONST HANDLE*,DWORD,DWORD,DWORD);
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

pfnGetaddrinfo MyGetaddrinfo;
pfnFreeaddrinfo MyFreeaddrinfo;

ITaskbarList3 * pTaskbarInterface;

static DWORD MsgWaitForMultipleObjectsExWorkaround(DWORD nCount, const HANDLE *pHandles,
	DWORD dwMsecs, DWORD dwWakeMask, DWORD dwFlags);

static HANDLE hOkToExitEvent,hModulesLoadedEvent;
static HANDLE hShutdownEvent,hPreShutdownEvent;
static HANDLE hWaitObjects[MAXIMUM_WAIT_OBJECTS-1];
static char *pszWaitServices[MAXIMUM_WAIT_OBJECTS-1];
static int waitObjectCount=0;
HANDLE hStackMutex,hMirandaShutdown,hThreadQueueEmpty;
HINSTANCE hMirandaInst;

/////////////////////////////////////////////////////////////////////////////////////////
// exception handling

static DWORD __cdecl sttDefaultFilter( DWORD, EXCEPTION_POINTERS* )
{
	return EXCEPTION_EXECUTE_HANDLER;
}

pfnExceptionFilter pMirandaExceptFilter = sttDefaultFilter;

static INT_PTR GetExceptionFilter( WPARAM, LPARAM )
{
	return ( INT_PTR )pMirandaExceptFilter;
}

static INT_PTR SetExceptionFilter( WPARAM, LPARAM lParam )
{
	pfnExceptionFilter oldOne = pMirandaExceptFilter;
	if ( lParam != 0 )
		pMirandaExceptFilter = ( pfnExceptionFilter )lParam;
	return ( INT_PTR )oldOne;
}

/////////////////////////////////////////////////////////////////////////////////////////
// thread support functions

typedef struct
{
	DWORD dwThreadId;	// valid if hThread isn't signalled
	HANDLE hThread;
	HINSTANCE hOwner;
	void* pObject;
	PVOID addr;
}
THREAD_WAIT_ENTRY;

static LIST<THREAD_WAIT_ENTRY> threads( 10, NumericKeySortT );

struct FORK_ARG {
	HANDLE hEvent;
	pThreadFunc threadcode;
	pThreadFuncEx threadcodeex;
	void *arg, *owner;
};

/////////////////////////////////////////////////////////////////////////////////////////
// forkthread - starts a new thread

void __cdecl forkthread_r(void * arg)
{
	struct FORK_ARG * fa = (struct FORK_ARG *) arg;
	void (*callercode)(void*)=fa->threadcode;
	void * cookie=fa->arg;
	CallService(MS_SYSTEM_THREAD_PUSH,0,(LPARAM)callercode);
	SetEvent(fa->hEvent);
	__try
	{
		callercode(cookie);
	}
	__except( pMirandaExceptFilter( GetExceptionCode(), GetExceptionInformation()))
	{
		Netlib_Logf( NULL, "Unhandled exception in thread %x", GetCurrentThreadId());
	}

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	CallService(MS_SYSTEM_THREAD_POP,0,0);
	return;
}

UINT_PTR forkthread (
	void (__cdecl *threadcode)(void*),
	unsigned long stacksize,
	void *arg
	)
{
	UINT_PTR rc;
	struct FORK_ARG fa;
	fa.hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
	fa.threadcode=threadcode;
	fa.arg=arg;
	rc=_beginthread(forkthread_r,stacksize,&fa);
	if ((UINT_PTR)-1L != rc)
		WaitForSingleObject(fa.hEvent,INFINITE);

	CloseHandle(fa.hEvent);
	return rc;
}

static INT_PTR ForkThreadService(WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR)forkthread(( pThreadFunc )wParam, 0, ( void* )lParam );
}

/////////////////////////////////////////////////////////////////////////////////////////
// forkthreadex - starts a new thread with the extended info and returns the thread id

unsigned __stdcall forkthreadex_r(void * arg)
{
	struct FORK_ARG *fa = (struct FORK_ARG *)arg;
	pThreadFuncEx threadcode = fa->threadcodeex;
	pThreadFuncOwner threadcodeex = ( pThreadFuncOwner )fa->threadcodeex;
	void *cookie = fa->arg;
	void *owner = fa->owner;
	unsigned long rc = 0;

	CallService(MS_SYSTEM_THREAD_PUSH, (WPARAM)fa->owner, (LPARAM)threadcode);
	SetEvent(fa->hEvent);
	__try
	{
		if ( owner )
			rc = threadcodeex( owner, cookie );
		else
			rc = threadcode( cookie );
	}
	__except( pMirandaExceptFilter( GetExceptionCode(), GetExceptionInformation()))
	{
		Netlib_Logf( NULL, "Unhandled exception in thread %x", GetCurrentThreadId());
	}

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	CallService(MS_SYSTEM_THREAD_POP,0,0);
	return rc;
}

UINT_PTR forkthreadex(
	void *sec,
	unsigned stacksize,
	unsigned (__stdcall *threadcode)(void*),
	void* owner,
	void *arg,
	unsigned *thraddr )
{
	UINT_PTR rc;
	struct FORK_ARG fa = { 0 };
	fa.threadcodeex = threadcode;
	fa.arg = arg;
	fa.owner = owner;
	fa.hEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	rc = _beginthreadex(sec,stacksize,forkthreadex_r,(void *)&fa,0,thraddr);
	if (rc)
		WaitForSingleObject(fa.hEvent,INFINITE);

	CloseHandle(fa.hEvent);
	return rc;
}

static INT_PTR ForkThreadServiceEx(WPARAM wParam, LPARAM lParam)
{
	FORK_THREADEX_PARAMS* params = (FORK_THREADEX_PARAMS*)lParam;
	if ( params == NULL )
		return 0;

	UINT threadID;
	return forkthreadex( NULL, params->iStackSize, params->pFunc, ( void* )wParam, params->arg, params->threadID ? params->threadID : &threadID );
}

/////////////////////////////////////////////////////////////////////////////////////////
// APC and mutex functions

static void __stdcall DummyAPCFunc(ULONG_PTR)
{
	/* called in the context of thread that cleared it's APC queue */
	return;
}

static int MirandaWaitForMutex(HANDLE hEvent)
{
	for (;;) {
		// will get WAIT_IO_COMPLETE for QueueUserAPC() which isnt a result
		DWORD rc = MsgWaitForMultipleObjectsExWorkaround(1, &hEvent, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE);
		if ( rc == WAIT_OBJECT_0 + 1 ) {
			MSG msg;
			while ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {
				if ( IsDialogMessage(msg.hwnd, &msg) ) continue;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		} else if ( rc==WAIT_OBJECT_0 ) {
			// got object
			return 1;
		} else if ( rc==WAIT_ABANDONED_0 || rc == WAIT_FAILED ) return 0;
	}
}

VOID CALLBACK KillAllThreads(HWND, UINT, UINT_PTR, DWORD)
{
	if ( MirandaWaitForMutex( hStackMutex )) {
		for ( int j=0; j < threads.getCount(); j++ ) {
			THREAD_WAIT_ENTRY* p = threads[j];
			char szModuleName[ MAX_PATH ];
			GetModuleFileNameA( p->hOwner, szModuleName, sizeof(szModuleName));
			Netlib_Logf( NULL, "Thread %p was abnormally terminated because module '%s' didn't release it. Entry point: %p",
				p->hThread, szModuleName, p->addr );
			TerminateThread( p->hThread, 9999 );
			CloseHandle(p->hThread);
			mir_free( p );
		}

		threads.destroy();

		ReleaseMutex( hStackMutex );
		SetEvent( hThreadQueueEmpty );
	}	
}

void KillObjectThreads( void* owner )
{
	if ( owner == NULL )
		return;

	WaitForSingleObject( hStackMutex, INFINITE );

	HANDLE* threadPool = ( HANDLE* )alloca( threads.getCount()*sizeof( HANDLE ));
	int threadCount = 0;

	for ( int j = threads.getCount(); j--; ) {
		THREAD_WAIT_ENTRY* p = threads[j];
		if ( p->pObject == owner )
			threadPool[ threadCount++ ] = p->hThread;
	}
	ReleaseMutex(hStackMutex); 

	// is there anything to kill?
	if ( threadCount > 0 ) {
		if ( WaitForMultipleObjects( threadCount, threadPool, TRUE, 5000 ) == WAIT_TIMEOUT ) {
			// forcibly kill all remaining threads after 5 secs
			WaitForSingleObject( hStackMutex, INFINITE );
			for ( int j = threads.getCount()-1; j >= 0; j-- ) {
				THREAD_WAIT_ENTRY* p = threads[j];
				if ( p->pObject == owner ) {
					TerminateThread( p->hThread, 9999 );
					CloseHandle( p->hThread );
					threads.remove( j );
					mir_free( p );
				}
			}
			ReleaseMutex(hStackMutex);
		}
	}
}

static void UnwindThreadWait(void)
{
	// acquire the list and wake up any alertable threads
	if ( MirandaWaitForMutex(hStackMutex) ) {
		int j;
		for ( j=0; j < threads.getCount(); j++ )
			QueueUserAPC(DummyAPCFunc,threads[j]->hThread, 0);
		ReleaseMutex(hStackMutex);
	}

	// give all unclosed threads 5 seconds to close
	SetTimer( NULL, 0, 5000, KillAllThreads );

	// wait til the thread list is empty
	MirandaWaitForMutex(hThreadQueueEmpty);
}

/////////////////////////////////////////////////////////////////////////////////////////

typedef LONG (WINAPI *pNtQIT)(HANDLE, LONG, PVOID, ULONG, PULONG);
#define ThreadQuerySetWin32StartAddress 9

void* GetCurrentThreadEntryPoint()
{
	LONG  ntStatus;
	HANDLE hDupHandle, hCurrentProcess;
	DWORD_PTR dwStartAddress;

	pNtQIT NtQueryInformationThread = (pNtQIT)GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "NtQueryInformationThread" );
	if(NtQueryInformationThread == NULL) return 0;

	hCurrentProcess = GetCurrentProcess();
	if(!DuplicateHandle(hCurrentProcess, GetCurrentThread(), hCurrentProcess, &hDupHandle, THREAD_QUERY_INFORMATION, FALSE, 0)){
		SetLastError(ERROR_ACCESS_DENIED);
		return NULL;
	}
	ntStatus = NtQueryInformationThread(hDupHandle, ThreadQuerySetWin32StartAddress, &dwStartAddress, sizeof(DWORD_PTR), NULL);
	CloseHandle(hDupHandle);

	if(ntStatus != ERROR_SUCCESS) return 0;
	return ( void* )dwStartAddress;
}

INT_PTR UnwindThreadPush(WPARAM wParam,LPARAM lParam)
{
	ResetEvent(hThreadQueueEmpty); // thread list is not empty
	if (WaitForSingleObject(hStackMutex, INFINITE) == WAIT_OBJECT_0)
	{
		THREAD_WAIT_ENTRY* p = (THREAD_WAIT_ENTRY*)mir_calloc(sizeof(THREAD_WAIT_ENTRY));

		DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &p->hThread, 0, FALSE, DUPLICATE_SAME_ACCESS);
		p->dwThreadId = GetCurrentThreadId();
		p->pObject = (void*)wParam;
		p->hOwner = GetInstByAddress((void*)lParam);
		p->addr = (void*)lParam;
		threads.insert(p);

		//Netlib_Logf( NULL, "*** pushing thread %x[%x] (%d)", hThread, GetCurrentThreadId(), threads.count );
		ReleaseMutex(hStackMutex);
	} //if
	return 0;
}

INT_PTR UnwindThreadPop(WPARAM,LPARAM)
{
	if (WaitForSingleObject(hStackMutex,INFINITE)==WAIT_OBJECT_0)
	{
		DWORD dwThreadId=GetCurrentThreadId();
		int j;
		//Netlib_Logf( NULL, "*** popping thread %x, %d threads left", dwThreadId, threads.count);
		for ( j=0; j < threads.getCount(); j++ ) {
			THREAD_WAIT_ENTRY* p = threads[j];
			if ( p->dwThreadId == dwThreadId ) {
				SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );
				CloseHandle( p->hThread );
				threads.remove( j );
				mir_free( p );

				if ( !threads.getCount()) {
					threads.destroy();
					ReleaseMutex( hStackMutex );
					SetEvent(hThreadQueueEmpty); // thread list is empty now
					return 0;
				} 

				ReleaseMutex(hStackMutex);
				return 0;
			} //if
		} //for
		ReleaseMutex(hStackMutex);
	} //if
	return 1;
}

INT_PTR MirandaIsTerminated(WPARAM, LPARAM)
{
	return WaitForSingleObject(hMirandaShutdown,0)==WAIT_OBJECT_0;
}

static void __cdecl compactHeapsThread(void*)
{
	while (!Miranda_Terminated())
	{
		HANDLE hHeaps[256];
		DWORD hc;
		SleepEx((1000*60)*5,TRUE); // every 5 minutes
		hc=GetProcessHeaps(255,(PHANDLE)&hHeaps);
		if (hc != 0 && hc < 256) {
			DWORD j;
			for (j=0;j<hc;j++) HeapCompact(hHeaps[j],0);
		}
	} //while
}

LRESULT CALLBACK APCWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg==WM_NULL) SleepEx(0,TRUE);
	if (msg == WM_TIMECHANGE) RecalculateTime();
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

HWND hAPCWindow=NULL;
void (*SetIdleCallback) (void)=NULL;

static INT_PTR SystemSetIdleCallback(WPARAM, LPARAM lParam)
{
	if (lParam && SetIdleCallback==NULL) {
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
	if ( lParam ) *(DWORD*)lParam = dwEventTime;
	return 0;
}

static DWORD MsgWaitForMultipleObjectsExWorkaround(DWORD nCount, const HANDLE *pHandles,
	DWORD dwMsecs, DWORD dwWakeMask, DWORD dwFlags)
{
	DWORD rc;
	if ( msgWaitForMultipleObjectsEx != NULL )
		return msgWaitForMultipleObjectsEx(nCount, pHandles, dwMsecs, dwWakeMask, dwFlags);
	rc=MsgWaitForMultipleObjects(nCount, pHandles, FALSE, 50, QS_ALLINPUT);
	if ( rc == WAIT_TIMEOUT ) rc=WaitForMultipleObjectsEx(nCount, pHandles, FALSE, 20, TRUE);
	return rc;
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
		TranslateDialogDefault( hwnd );
		SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);
		SendDlgItemMessage(hwnd, IDC_PROGRESSBAR, PBM_SETRANGE, 0, MAKELPARAM(0, MIRANDA_PROCESS_WAIT_STEPS));
		SendDlgItemMessage(hwnd, IDC_PROGRESSBAR, PBM_SETSTEP, 1, 0);
		SetTimer(hwnd, 1, MIRANDA_PROCESS_WAIT_RESOLUTION, NULL);
		break;

	case WM_TIMER:
		if (SendDlgItemMessage(hwnd, IDC_PROGRESSBAR, PBM_STEPIT, 0, 0) == MIRANDA_PROCESS_WAIT_STEPS)
			EndDialog(hwnd, 0);
		if (WaitForSingleObject((HANDLE)GetWindowLongPtr(hwnd, GWLP_USERDATA), 1) != WAIT_TIMEOUT)
		{
			SendDlgItemMessage(hwnd, IDC_PROGRESSBAR, PBM_SETPOS, MIRANDA_PROCESS_WAIT_STEPS, 0);
			EndDialog(hwnd, 0);
		}
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == IDCANCEL)
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
	char* p = strstr( cmdline, "/restart:" );
	if ( p ) {
		HANDLE hProcess = OpenProcess( SYNCHRONIZE, FALSE, atol( p+9 ));
		if ( hProcess ) {
			DialogBoxParam(hMirandaInst, MAKEINTRESOURCE(IDD_WAITRESTART), NULL, WaitForProcessDlgProc, (LPARAM)hProcess);
			CloseHandle( hProcess );
		}	
	}	
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int )
{
	DWORD myPid=0;
	int messageloop=1;
	HMODULE hUser32, hThemeAPI, hDwmApi, hShFolder = NULL;
	int result = 0;

	hMirandaInst = hInstance;

	setlocale(LC_ALL, "");

#ifdef _DEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
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
#ifdef _UNICODE
	MyGetMonitorInfo = (pfnMyGetMonitorInfo)GetProcAddress(hUser32, "GetMonitorInfoW");
#else
	MyGetMonitorInfo = (pfnMyGetMonitorInfo)GetProcAddress(hUser32, "GetMonitorInfoA");
#endif

	hShFolder = GetModuleHandleA("shell32");
	shGetSpecialFolderPathA = (pfnSHGetSpecialFolderPathA)GetProcAddress(hShFolder,"SHGetSpecialFolderPathA");
	shGetSpecialFolderPathW = (pfnSHGetSpecialFolderPathW)GetProcAddress(hShFolder,"SHGetSpecialFolderPathW");
	if (shGetSpecialFolderPathA == NULL)
	{
		hShFolder = LoadLibraryA("ShFolder.dll");
		shGetSpecialFolderPathA = (pfnSHGetSpecialFolderPathA)GetProcAddress(hShFolder,"SHGetSpecialFolderPathA");
		shGetSpecialFolderPathW = (pfnSHGetSpecialFolderPathW)GetProcAddress(hShFolder,"SHGetSpecialFolderPathW");
	}

	shAutoComplete = (pfnSHAutoComplete)GetProcAddress(GetModuleHandleA("shlwapi"),"SHAutoComplete");

	if (IsWinVerXPPlus())
	{
		hThemeAPI = LoadLibraryA("uxtheme.dll");
		if (hThemeAPI)
		{
			openThemeData = (pfnOpenThemeData)GetProcAddress(hThemeAPI, "OpenThemeData");
			isThemeBackgroundPartiallyTransparent = (pfnIsThemeBackgroundPartiallyTransparent)GetProcAddress(hThemeAPI, "IsThemeBackgroundPartiallyTransparent");
			drawThemeParentBackground  = (pfnDrawThemeParentBackground)GetProcAddress(hThemeAPI, "DrawThemeParentBackground");
			drawThemeBackground = (pfnDrawThemeBackground)GetProcAddress(hThemeAPI, "DrawThemeBackground");
			drawThemeText = (pfnDrawThemeText)GetProcAddress(hThemeAPI, "DrawThemeText");
			drawThemeTextEx = (pfnDrawThemeTextEx)GetProcAddress(hThemeAPI, "DrawThemeTextEx");
			getThemeBackgroundContentRect = (pfnGetThemeBackgroundContentRect)GetProcAddress(hThemeAPI ,"GetThemeBackgroundContentRect");
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

	if (IsWinVerVistaPlus())
	{
		hDwmApi = LoadLibraryA("dwmapi.dll");
		if (hDwmApi)
		{
			dwmExtendFrameIntoClientArea = (pfnDwmExtendFrameIntoClientArea)GetProcAddress(hDwmApi,"DwmExtendFrameIntoClientArea");
			dwmIsCompositionEnabled = (pfnDwmIsCompositionEnabled)GetProcAddress(hDwmApi,"DwmIsCompositionEnabled");
		}
	}

	HMODULE hWinSock = GetModuleHandleA("ws2_32");
	MyGetaddrinfo = (pfnGetaddrinfo)GetProcAddress(hWinSock, "getaddrinfo");
	MyFreeaddrinfo = (pfnFreeaddrinfo)GetProcAddress(hWinSock, "freeaddrinfo");

	if (bufferedPaintInit) bufferedPaintInit();

	OleInitialize(NULL);

	if (IsWinVer7Plus())
		CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList3, (void**)&pTaskbarInterface);

	InitialiseModularEngine();
//	ParseCommandLine();

	if (LoadDefaultModules()) {
		NotifyEventHooks(hShutdownEvent,0,0);
		UnloadDefaultModules();

		result = 1;
		goto exit;
	}
	NotifyEventHooks(hModulesLoadedEvent,0,0);

	// ensure that the kernel hooks the SystemShutdownProc() after all plugins
	HookEvent(ME_SYSTEM_SHUTDOWN,SystemShutdownProc);

	forkthread(compactHeapsThread,0,NULL);
	CreateServiceFunction(MS_SYSTEM_SETIDLECALLBACK,SystemSetIdleCallback);
	CreateServiceFunction(MS_SYSTEM_GETIDLE, SystemGetIdle);
	dwEventTime=GetTickCount();
	myPid=GetCurrentProcessId();
	for(;messageloop;) {
		MSG msg;
		DWORD rc;
		BOOL dying=FALSE;
		rc=MsgWaitForMultipleObjectsExWorkaround(waitObjectCount, hWaitObjects, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE);
		if ( rc >= WAIT_OBJECT_0 && rc < WAIT_OBJECT_0 + waitObjectCount) {
			rc -= WAIT_OBJECT_0;
			CallService(pszWaitServices[rc], (WPARAM) hWaitObjects[rc], 0);
		}
		//
		while ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) {
			if ( msg.message != WM_QUIT ) {
				HWND h=GetForegroundWindow();
				DWORD pid = 0;
				checkIdle(&msg);
				if ( h != NULL && GetWindowThreadProcessId(h, &pid) && pid == myPid && 
					GetClassLongPtr(h, GCW_ATOM)==32770 ) 
				{
					if ( IsDialogMessage(h, &msg) ) 
						continue;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if ( SetIdleCallback != NULL )
					SetIdleCallback();
			} else if ( !dying ) {
				dying++;
				SetEvent(hMirandaShutdown);
				NotifyEventHooks(hPreShutdownEvent, 0, 0);

				// this spins and processes the msg loop, objects and APC.
				UnwindThreadWait();
				NotifyEventHooks(hShutdownEvent, 0, 0);
				// if the hooks generated any messages, it'll get processed before the second WM_QUIT
				PostQuitMessage(0);
			} else if ( dying ) {
				messageloop=0;
			}
		} // while
	}

exit:
	UnloadNewPluginsModule();
	DestroyModularEngine();
	CloseHandle(hStackMutex);
	CloseHandle(hMirandaShutdown);
	CloseHandle(hThreadQueueEmpty);
	DestroyWindow(hAPCWindow);

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
	return NotifyEventHooks(hOkToExitEvent,0,0)==0;
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

	GetModuleFileName(NULL,filename,SIZEOF(filename));
	verInfoSize=GetFileVersionInfoSize(filename,&unused);
	pVerInfo=mir_alloc(verInfoSize);
	GetFileVersionInfo(filename,0,verInfoSize,pVerInfo);
	VerQueryValue(pVerInfo,_T("\\"),(PVOID*)&vsffi,&blockSize);
	ver=(((vsffi->dwProductVersionMS>>16)&0xFF)<<24)|
		((vsffi->dwProductVersionMS&0xFF)<<16)|
		(((vsffi->dwProductVersionLS>>16)&0xFF)<<8)|
		(vsffi->dwProductVersionLS&0xFF);
	mir_free(pVerInfo);
	return (INT_PTR)ver;
}

static INT_PTR GetMirandaVersionText(WPARAM wParam,LPARAM lParam)
{
	TCHAR filename[MAX_PATH], *productVersion;
	DWORD unused;
	DWORD verInfoSize;
	UINT blockSize;
	PVOID pVerInfo;

	GetModuleFileName(NULL,filename,SIZEOF(filename));
	verInfoSize=GetFileVersionInfoSize(filename,&unused);
	pVerInfo=mir_alloc(verInfoSize);
	GetFileVersionInfo(filename,0,verInfoSize,pVerInfo);
	VerQueryValue(pVerInfo,_T("\\StringFileInfo\\000004b0\\ProductVersion"),(LPVOID*)&productVersion,&blockSize);
#if defined( _WIN64 )
	mir_snprintf(( char* )lParam, wParam, "%S x64 Unicode", productVersion );
#elif defined( _UNICODE )
	mir_snprintf(( char* )lParam, wParam, "%S Unicode", productVersion );
#else
	lstrcpyn((char*)lParam,productVersion,wParam);
#endif
	mir_free(pVerInfo);
	return 0;
}

INT_PTR WaitOnHandle(WPARAM wParam,LPARAM lParam)
{
	if(waitObjectCount>=MAXIMUM_WAIT_OBJECTS-1) return 1;
	hWaitObjects[waitObjectCount]=(HANDLE)wParam;
	pszWaitServices[waitObjectCount]=(char*)lParam;
	waitObjectCount++;
	return 0;
}

static INT_PTR RemoveWait(WPARAM wParam, LPARAM)
{
	int i;

	for(i=0;i<waitObjectCount;i++)
		if(hWaitObjects[i]==(HANDLE)wParam) break;
	if(i==waitObjectCount) return 1;
	waitObjectCount--;
	MoveMemory(&hWaitObjects[i],&hWaitObjects[i+1],sizeof(HANDLE)*(waitObjectCount-i));
	MoveMemory(&pszWaitServices[i],&pszWaitServices[i+1],sizeof(char*)*(waitObjectCount-i));
	return 0;
}

INT_PTR GetMemoryManagerInterface(WPARAM, LPARAM lParam)
{
	struct MM_INTERFACE *mmi = (struct MM_INTERFACE*) lParam;
	if ( mmi == NULL )
		return 1;

	mmi->mmi_malloc = mir_alloc;
	mmi->mmi_realloc = mir_realloc;
	mmi->mmi_free = mir_free;

	switch( mmi->cbSize ) {
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
#if defined( _DEBUG )
		DebugBreak();
#endif
		return 1;
	}

	return 0;
}

INT_PTR GetListInterface(WPARAM, LPARAM lParam)
{
	struct LIST_INTERFACE *li = (struct LIST_INTERFACE*) lParam;
	if ( li == NULL )
		return 1;

	switch(li->cbSize) {
	case LIST_INTERFACE_V3_SIZE:
		li->List_Copy      = List_Copy;
		li->List_ObjCopy   = List_ObjCopy;

	case LIST_INTERFACE_V2_SIZE:
		li->List_InsertPtr = List_InsertPtr;
		li->List_RemovePtr = List_RemovePtr;

	case LIST_INTERFACE_V1_SIZE:
		li->List_Create   = List_Create;
		li->List_Destroy  = List_Destroy;
		li->List_Find     = List_Find;
		li->List_GetIndex = List_GetIndex;
		li->List_Insert   = List_Insert;
		li->List_Remove   = List_Remove;
		li->List_IndexOf  = List_IndexOf;
		return 0;
	}
	return 1;
}

INT_PTR GetUtfInterface(WPARAM, LPARAM lParam)
{
	struct UTF8_INTERFACE *utfi = (struct UTF8_INTERFACE*) lParam;
	if ( utfi == NULL )
		return 1;

	switch( utfi->cbSize ) {
	case UTF8_INTERFACE_SIZEOF_V1:
	case UTF8_INTERFACE_SIZEOF_V2:
	case sizeof( struct UTF8_INTERFACE ):
		break;

	default:
		return 1;
	}

	utfi->utf8_decode   = Utf8Decode;
	utfi->utf8_decodecp = Utf8DecodeCP;
	utfi->utf8_encode   = Utf8Encode;
	utfi->utf8_encodecp = Utf8EncodeCP;
	utfi->utf8_encodeW  = Utf8EncodeUcs2;
	if (utfi->cbSize > UTF8_INTERFACE_SIZEOF_V1)
		utfi->utf8_decodeW = Utf8DecodeUcs2;
	if (utfi->cbSize > UTF8_INTERFACE_SIZEOF_V2)
		utfi->utf8_lenW = Ucs2toUtf8Len;

	return 0;
}

int LoadSystemModule(void)
{
	INITCOMMONCONTROLSEX icce = {0};
	icce.dwSize = sizeof(icce);
	icce.dwICC = ICC_WIN95_CLASSES | ICC_USEREX_CLASSES;
	InitCommonControlsEx(&icce);

	if (IsWinVerXPPlus()) {
		hAPCWindow=CreateWindowEx(0,_T("ComboLBox"),NULL,0, 0,0,0,0, NULL,NULL,NULL,NULL);
		SetClassLongPtr(hAPCWindow, GCL_STYLE, GetClassLongPtr(hAPCWindow, GCL_STYLE) | CS_DROPSHADOW);
		DestroyWindow(hAPCWindow);
		hAPCWindow = NULL;
	}

	hAPCWindow=CreateWindowEx(0,_T("STATIC"),NULL,0, 0,0,0,0, NULL,NULL,NULL,NULL); // lame
	SetWindowLongPtr(hAPCWindow,GWLP_WNDPROC,(LONG_PTR)APCWndProc);
	hStackMutex=CreateMutex(NULL,FALSE,NULL);
	hMirandaShutdown=CreateEvent(NULL,TRUE,FALSE,NULL);
	hThreadQueueEmpty=CreateEvent(NULL,TRUE,TRUE,NULL);

	hShutdownEvent=CreateHookableEvent(ME_SYSTEM_SHUTDOWN);
	hPreShutdownEvent=CreateHookableEvent(ME_SYSTEM_PRESHUTDOWN);
	hModulesLoadedEvent=CreateHookableEvent(ME_SYSTEM_MODULESLOADED);
	hOkToExitEvent=CreateHookableEvent(ME_SYSTEM_OKTOEXIT);

	CreateServiceFunction(MS_SYSTEM_FORK_THREAD,ForkThreadService);
	CreateServiceFunction(MS_SYSTEM_FORK_THREAD_EX,ForkThreadServiceEx);
	CreateServiceFunction(MS_SYSTEM_THREAD_PUSH,UnwindThreadPush);
	CreateServiceFunction(MS_SYSTEM_THREAD_POP,UnwindThreadPop);
	CreateServiceFunction(MS_SYSTEM_TERMINATED,MirandaIsTerminated);
	CreateServiceFunction(MS_SYSTEM_OKTOEXIT,OkToExit);
	CreateServiceFunction(MS_SYSTEM_GETVERSION,GetMirandaVersion);
	CreateServiceFunction(MS_SYSTEM_GETVERSIONTEXT,GetMirandaVersionText);
	CreateServiceFunction(MS_SYSTEM_WAITONHANDLE,WaitOnHandle);
	CreateServiceFunction(MS_SYSTEM_REMOVEWAIT,RemoveWait);
	CreateServiceFunction(MS_SYSTEM_GET_LI,GetListInterface);
	CreateServiceFunction(MS_SYSTEM_GET_MMI,GetMemoryManagerInterface);
	CreateServiceFunction(MS_SYSTEM_GET_UTFI,GetUtfInterface);
	CreateServiceFunction(MS_SYSTEM_GETEXCEPTFILTER,GetExceptionFilter);
	CreateServiceFunction(MS_SYSTEM_SETEXCEPTFILTER,SetExceptionFilter);

	InitPathUtils();
	return 0;
}
