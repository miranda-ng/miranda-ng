#include "hdr/modern_commonheaders.h"
#include "hdr/modern_sync.h"

typedef struct tagSYNCCALLITEM
{
	WPARAM  wParam;
	LPARAM  lParam;
	int     nResult;
	HANDLE  hDoneEvent;
	PSYNCCALLBACKPROC pfnProc;    
} SYNCCALLITEM;
static void CALLBACK _SyncCallerUserAPCProc(ULONG_PTR dwParam)
{
	SYNCCALLITEM* item = (SYNCCALLITEM*) dwParam;
	item->nResult = item->pfnProc(item->wParam, item->lParam);
	SetEvent(item->hDoneEvent);
}
static INT_PTR SyncCaller(WPARAM proc, LPARAM lParam)
{
	typedef int (*P0PARAMFUNC)();
	typedef int (*P1PARAMFUNC)(WPARAM);
	typedef int (*P2PARAMFUNC)(WPARAM, LPARAM);
	typedef int (*P3PARAMFUNC)(WPARAM, LPARAM, LPARAM);
	typedef int (*P4PARAMFUNC)(WPARAM, LPARAM, LPARAM, LPARAM);

	LPARAM * params=(LPARAM *)lParam;
	int count=params[0];
	switch (count)
	{
	case 0:			
		{
			P0PARAMFUNC pfnProc=(P0PARAMFUNC)proc;
			return pfnProc();
		}
	case 1:
		{
			P1PARAMFUNC pfnProc=(P1PARAMFUNC)proc;
			return pfnProc((WPARAM)params[1]);
		}
	case 2:
		{
			P2PARAMFUNC pfnProc=(P2PARAMFUNC)proc;
			return pfnProc((WPARAM)params[1],(LPARAM)params[2]);
		}
	case 3:
		{
			P3PARAMFUNC pfnProc=(P3PARAMFUNC)proc;
			return pfnProc((WPARAM)params[1],(LPARAM)params[2], (LPARAM)params[3]);
		}
	case 4:
		{
			P4PARAMFUNC pfnProc=(P4PARAMFUNC)proc;
			return pfnProc((WPARAM)params[1],(LPARAM)params[2], (LPARAM)params[3], (LPARAM)params[4]);
		}
	}
	return 0;
}


int SyncCall(void * vproc, int count, ... )
{
	LPARAM params[5];
	va_list va;
	int i;
	params[0]=(LPARAM)count;
	va_start(va, count);
	for (i=0; i<count && i<SIZEOF(params)-1; i++)
	{
		params[i+1]=va_arg(va,LPARAM);
	}
	va_end(va);
	return SyncCallProxy(SyncCaller, (WPARAM)vproc, (LPARAM) params);
}

int SyncCallProxy(PSYNCCALLBACKPROC pfnProc, WPARAM wParam, LPARAM lParam, CRITICAL_SECTION * cs /* = NULL */)
{  
	SYNCCALLITEM item={0};
	
	int nReturn = 0;

	if ( cs != NULL )
	{
		if ( !fnTryEnterCriticalSection ) // for poor OSes like Win98
		{
			EnterCriticalSection( cs );
			int result = pfnProc( wParam, lParam );
			LeaveCriticalSection( cs );
			return result;
		}

		if ( fnTryEnterCriticalSection( cs ))
		{   //simple call (Fastest)
			int result=pfnProc(wParam,lParam);
			LeaveCriticalSection( cs );
			return result;
		}
		else
		{	//Window SendMessage Call(Middle)
			if ( SyncCallWinProcProxy( pfnProc, wParam, lParam, nReturn ) == S_OK)
				return nReturn;
		}
	}
	if ( SyncCallAPCProxy( pfnProc, wParam, lParam, nReturn ) == S_OK)
		return nReturn;
	
	return NULL;
}

HRESULT SyncCallWinProcProxy( PSYNCCALLBACKPROC pfnProc, WPARAM wParam, LPARAM lParam, int& nReturn )
{
	nReturn = 0;
	if (pcli->hwndContactList == NULL )
		return E_FAIL;
	
	SYNCCALLITEM item={0};
	item.wParam = wParam;
	item.lParam = lParam;
	item.pfnProc = pfnProc;
	nReturn = SendMessage(pcli->hwndContactList, UM_SYNCCALL, (WPARAM)&item,0);
	
	return S_OK;
}

HRESULT SyncCallAPCProxy( PSYNCCALLBACKPROC pfnProc, WPARAM wParam, LPARAM lParam, int& hReturn )
{
	hReturn = 0;

	if (g_hMainThread==NULL || pfnProc==NULL) 
		return E_FAIL;

	SYNCCALLITEM item={0};

	if (GetCurrentThreadId()!=g_dwMainThreadID)
	{
		item.wParam = wParam;
		item.lParam = lParam;
		item.pfnProc = pfnProc;
		item.hDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		QueueUserAPC(_SyncCallerUserAPCProc, g_hMainThread, (ULONG_PTR) &item);	
		PostMessage(pcli->hwndContactList,WM_NULL,0,0); // let this get processed in its own time
		
		WaitForSingleObject(item.hDoneEvent, INFINITE);
		CloseHandle(item.hDoneEvent);

		hReturn = item.nResult;

		return S_OK;
	}
	/* else */

	hReturn = pfnProc(wParam, lParam);
	return S_OK;
}





LRESULT SyncOnWndProcCall( WPARAM wParam )
{
	SYNCCALLITEM *psci=(SYNCCALLITEM *)wParam;
	if (psci)
		return psci->pfnProc( psci->wParam, psci->lParam );
	return 0;
}

int DoCall( PSYNCCALLBACKPROC pfnProc, WPARAM wParam, LPARAM lParam )
{
	return SyncCallProxy( pfnProc, 0, lParam );
}

