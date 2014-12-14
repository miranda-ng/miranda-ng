#include "hdr/modern_commonheaders.h"
#include "hdr/modern_sync.h"

static INT_PTR SyncCaller(WPARAM proc, LPARAM lParam)
{
	typedef int(*P0PARAMFUNC)();
	typedef int(*P1PARAMFUNC)(WPARAM);
	typedef int(*P2PARAMFUNC)(WPARAM, LPARAM);
	typedef int(*P3PARAMFUNC)(WPARAM, LPARAM, LPARAM);
	typedef int(*P4PARAMFUNC)(WPARAM, LPARAM, LPARAM, LPARAM);

	LPARAM * params = (LPARAM *)lParam;
	int count = params[0];
	switch (count) {
	case 0:
		return ((P0PARAMFUNC)proc)();

	case 1:
		return ((P1PARAMFUNC)proc)((WPARAM)params[1]);

	case 2:
		return ((P2PARAMFUNC)proc)((WPARAM)params[1], (LPARAM)params[2]);

	case 3:
		return ((P3PARAMFUNC)proc)((WPARAM)params[1], (LPARAM)params[2], (LPARAM)params[3]);

	case 4:
		return ((P4PARAMFUNC)proc)((WPARAM)params[1], (LPARAM)params[2], (LPARAM)params[3], (LPARAM)params[4]);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct SYNCCALLITEM
{
	WPARAM  wParam;
	LPARAM  lParam;
	int     nResult;
	HANDLE  hDoneEvent;
	PSYNCCALLBACKPROC pfnProc;
};

static void CALLBACK _SyncCallerUserAPCProc(void* param)
{
	SYNCCALLITEM* item = (SYNCCALLITEM*)param;
	item->nResult = item->pfnProc(item->wParam, item->lParam);
	SetEvent(item->hDoneEvent);
}

static int SyncCallAPCProxy(PSYNCCALLBACKPROC pfnProc, WPARAM wParam, LPARAM lParam)
{
	if (pfnProc == NULL)
		return 0;

	if (GetCurrentThreadId() == g_dwMainThreadID)
		return pfnProc(wParam, lParam);

	SYNCCALLITEM item;
	item.wParam = wParam;
	item.lParam = lParam;
	item.pfnProc = pfnProc;
	item.nResult = 0;
	item.hDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	CallFunctionAsync(_SyncCallerUserAPCProc, &item);

	WaitForSingleObject(item.hDoneEvent, INFINITE);
	CloseHandle(item.hDoneEvent);
	return item.nResult;
}

/////////////////////////////////////////////////////////////////////////////////////////

int DoCall(PSYNCCALLBACKPROC pfnProc, WPARAM, LPARAM lParam)
{
	return SyncCallAPCProxy(pfnProc, 0, lParam);
}

int SyncCall(void * vproc, int count, ...)
{
	LPARAM params[5];
	va_list va;
	int i;
	params[0] = (LPARAM)count;
	va_start(va, count);
	for (i = 0; i < count && i < SIZEOF(params) - 1; i++)
		params[i + 1] = va_arg(va, LPARAM);

	va_end(va);
	return SyncCallAPCProxy(SyncCaller, (WPARAM)vproc, (LPARAM)params);
}
