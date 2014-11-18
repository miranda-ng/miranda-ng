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

#include <m_netlib.h>

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
		DWORD rc = MsgWaitForMultipleObjectsEx(1, &hEvent, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE);
		if (rc == WAIT_OBJECT_0 + 1) {
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (IsDialogMessage(msg.hwnd, &msg)) continue;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else if (rc == WAIT_OBJECT_0) { // got object
			return 1;
		}
		else if (rc == WAIT_ABANDONED_0 || rc == WAIT_FAILED)
			return 0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// exception handling

static DWORD __cdecl sttDefaultFilter(DWORD, EXCEPTION_POINTERS*)
{
	return EXCEPTION_EXECUTE_HANDLER;
}

pfnExceptionFilter pMirandaExceptFilter = sttDefaultFilter;

MIR_CORE_DLL(pfnExceptionFilter) GetExceptionFilter()
{
	return pMirandaExceptFilter;
}

MIR_CORE_DLL(pfnExceptionFilter) SetExceptionFilter(pfnExceptionFilter pMirandaExceptFilter)
{
	pfnExceptionFilter oldOne = pMirandaExceptFilter;
	if (pMirandaExceptFilter != 0)
		pMirandaExceptFilter = pMirandaExceptFilter;
	return oldOne;
}

/////////////////////////////////////////////////////////////////////////////////////////
// thread support functions

struct THREAD_WAIT_ENTRY
{
	DWORD dwThreadId;	// valid if hThread isn't signalled
	HANDLE hThread;
	HINSTANCE hOwner;
	void *pObject;
};

static LIST<THREAD_WAIT_ENTRY> threads(10, NumericKeySortT);

struct FORK_ARG
{
	HANDLE hEvent;
	union
	{
		pThreadFunc threadcode;
		pThreadFuncEx threadcodeex;
	};
	void *arg, *owner;
};

/////////////////////////////////////////////////////////////////////////////////////////
// forkthread - starts a new thread

void __cdecl forkthread_r(void *arg)
{
	FORK_ARG *fa = (FORK_ARG*)arg;
	pThreadFunc callercode = fa->threadcode;
	void *cookie = fa->arg;
	Thread_Push((HINSTANCE)callercode);
	SetEvent(fa->hEvent);

	callercode(cookie);

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	Thread_Pop();
}

MIR_CORE_DLL(UINT_PTR) forkthread(void(__cdecl *threadcode)(void*), unsigned long stacksize, void *arg)
{
	FORK_ARG fa;
	fa.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	fa.threadcode = threadcode;
	fa.arg = arg;
	UINT_PTR rc = _beginthread(forkthread_r, stacksize, &fa);
	if ((UINT_PTR)-1L != rc)
		WaitForSingleObject(fa.hEvent, INFINITE);

	CloseHandle(fa.hEvent);
	return rc;
}

/////////////////////////////////////////////////////////////////////////////////////////
// forkthreadex - starts a new thread with the extended info and returns the thread id

unsigned __stdcall forkthreadex_r(void * arg)
{
	struct FORK_ARG *fa = (struct FORK_ARG *)arg;
	pThreadFuncEx threadcode = fa->threadcodeex;
	pThreadFuncOwner threadcodeex = (pThreadFuncOwner)fa->threadcodeex;
	void *cookie = fa->arg;
	void *owner = fa->owner;
	unsigned long rc = 0;

	Thread_Push((HINSTANCE)threadcode, fa->owner);
	SetEvent(fa->hEvent);
	if (owner)
		rc = threadcodeex(owner, cookie);
	else
		rc = threadcode(cookie);

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	Thread_Pop();
	return rc;
}

MIR_CORE_DLL(UINT_PTR) forkthreadex(
	void *sec,
	unsigned stacksize,
	unsigned(__stdcall *threadcode)(void*),
	void *owner,
	void *arg,
	unsigned *thraddr)
{
	struct FORK_ARG fa = { 0 };
	fa.threadcodeex = threadcode;
	fa.arg = arg;
	fa.owner = owner;
	fa.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	UINT_PTR rc = _beginthreadex(sec, stacksize, forkthreadex_r, (void *)&fa, 0, thraddr);
	if (rc)
		WaitForSingleObject(fa.hEvent, INFINITE);

	CloseHandle(fa.hEvent);
	return rc;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(void) KillObjectThreads(void* owner)
{
	if (owner == NULL)
		return;

	WaitForSingleObject(hStackMutex, INFINITE);

	HANDLE *threadPool = (HANDLE*)alloca(threads.getCount()*sizeof(HANDLE));
	int threadCount = 0;

	for (int j = threads.getCount(); j--;) {
		THREAD_WAIT_ENTRY *p = threads[j];
		if (p->pObject == owner)
			threadPool[threadCount++] = p->hThread;
	}
	ReleaseMutex(hStackMutex);

	// is there anything to kill?
	if (threadCount > 0) {
		if (WaitForMultipleObjects(threadCount, threadPool, TRUE, 5000) == WAIT_TIMEOUT) {
			// forcibly kill all remaining threads after 5 secs
			WaitForSingleObject(hStackMutex, INFINITE);
			for (int j = threads.getCount() - 1; j >= 0; j--) {
				THREAD_WAIT_ENTRY* p = threads[j];
				if (p->pObject == owner) {
					char szModuleName[MAX_PATH];
					GetModuleFileNameA(p->hOwner, szModuleName, sizeof(szModuleName));
					Netlib_Logf(0, "Killing objec thread %s:%p", szModuleName, p->dwThreadId);
					TerminateThread(p->hThread, 9999);
					CloseHandle(p->hThread);
					threads.remove(j);
					mir_free(p);
				}
			}
			ReleaseMutex(hStackMutex);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static void CALLBACK KillAllThreads(HWND, UINT, UINT_PTR, DWORD)
{
	if (MirandaWaitForMutex(hStackMutex)) {
		for (int j = 0; j < threads.getCount(); j++) {
			THREAD_WAIT_ENTRY *p = threads[j];
			char szModuleName[MAX_PATH];
			GetModuleFileNameA(p->hOwner, szModuleName, sizeof(szModuleName));
			Netlib_Logf(0, "Killing thread %s:%p", szModuleName, p->dwThreadId);
			TerminateThread(p->hThread, 9999);
			CloseHandle(p->hThread);
			mir_free(p);
		}

		threads.destroy();

		ReleaseMutex(hStackMutex);
		SetEvent(hThreadQueueEmpty);
	}
}

MIR_CORE_DLL(void) Thread_Wait(void)
{
	// acquire the list and wake up any alertable threads
	if (MirandaWaitForMutex(hStackMutex)) {
		for (int j = 0; j < threads.getCount(); j++)
			QueueUserAPC(DummyAPCFunc, threads[j]->hThread, 0);
		ReleaseMutex(hStackMutex);
	}

	// give all unclosed threads 5 seconds to close
	SetTimer(NULL, 0, 5000, KillAllThreads);

	// wait til the thread list is empty
	MirandaWaitForMutex(hThreadQueueEmpty);
}

/////////////////////////////////////////////////////////////////////////////////////////

typedef LONG (WINAPI *pNtQIT)(HANDLE, LONG, PVOID, ULONG, PULONG);
#define ThreadQuerySetWin32StartAddress 9

static void* GetCurrentThreadEntryPoint()
{
	pNtQIT NtQueryInformationThread = (pNtQIT)GetProcAddress(GetModuleHandle(_T("ntdll.dll")), "NtQueryInformationThread");
	if (NtQueryInformationThread == NULL) return 0;

	HANDLE hDupHandle, hCurrentProcess = GetCurrentProcess();
	if (!DuplicateHandle(hCurrentProcess, GetCurrentThread(), hCurrentProcess, &hDupHandle, THREAD_QUERY_INFORMATION, FALSE, 0)) {
		SetLastError(ERROR_ACCESS_DENIED);
		return NULL;
	}
	
	DWORD_PTR dwStartAddress;
	LONG ntStatus = NtQueryInformationThread(hDupHandle, ThreadQuerySetWin32StartAddress, &dwStartAddress, sizeof(DWORD_PTR), NULL);
	CloseHandle(hDupHandle);

	return (ntStatus != ERROR_SUCCESS) ? NULL : (void*)dwStartAddress;
}

MIR_CORE_DLL(INT_PTR) Thread_Push(HINSTANCE hInst, void* pOwner)
{
	ResetEvent(hThreadQueueEmpty); // thread list is not empty
	if (WaitForSingleObject(hStackMutex, INFINITE) == WAIT_OBJECT_0) {
		THREAD_WAIT_ENTRY* p = (THREAD_WAIT_ENTRY*)mir_calloc(sizeof(THREAD_WAIT_ENTRY));

		DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &p->hThread, 0, FALSE, DUPLICATE_SAME_ACCESS);
		p->dwThreadId = GetCurrentThreadId();
		p->pObject = pOwner;
		if (pluginListAddr.getIndex(hInst) != -1)
			p->hOwner = hInst;
		else
			p->hOwner = GetInstByAddress((hInst != NULL) ? (PVOID)hInst : GetCurrentThreadEntryPoint());

		threads.insert(p);

		ReleaseMutex(hStackMutex);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(INT_PTR) Thread_Pop()
{
	if (WaitForSingleObject(hStackMutex, INFINITE) == WAIT_OBJECT_0) {
		DWORD dwThreadId = GetCurrentThreadId();
		for (int j = 0; j < threads.getCount(); j++) {
			THREAD_WAIT_ENTRY *p = threads[j];
			if (p->dwThreadId == dwThreadId) {
				SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
				CloseHandle(p->hThread);
				threads.remove(j);
				mir_free(p);

				if (!threads.getCount()) {
					threads.destroy();
					ReleaseMutex(hStackMutex);
					SetEvent(hThreadQueueEmpty); // thread list is empty now
					return 0;
				}

				ReleaseMutex(hStackMutex);
				return 0;
			}
		}
		ReleaseMutex(hStackMutex);
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

const DWORD MS_VC_EXCEPTION=0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

MIR_CORE_DLL(void) Thread_SetName(const char *szThreadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = GetCurrentThreadId();
	info.dwFlags = 0;

	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{}
}
