/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project, 
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
#include <m_plugins.h>

// list of hooks

static int compareHooks(const THook* p1, const THook* p2)
{
	return strcmp(p1->name, p2->name);
}

static LIST<THook> hooks(50, compareHooks);

struct THookToMainThreadItem
{
	THook* hook;
	HANDLE hDoneEvent;
	WPARAM wParam;
	LPARAM lParam;
	int result;
};

// list of services

struct TService
{
	DWORD nameHash;
	HINSTANCE hOwner;
	union {
		MIRANDASERVICE pfnService;
		MIRANDASERVICEPARAM pfnServiceParam;
		MIRANDASERVICEOBJ pfnServiceObj;
		MIRANDASERVICEOBJPARAM pfnServiceObjParam;
	};
	int flags;
	LPARAM lParam;
	void* object;
	char name[1];
};

LIST<TService> services(100, NumericKeySortT);

typedef struct
{
	HANDLE hDoneEvent;
	WPARAM wParam;
	LPARAM lParam;
	int result;
	const char *name;
}
	TServiceToMainThreadItem;

// other static variables
static BOOL bServiceMode = FALSE;
static CRITICAL_SECTION csHooks, csServices;
static DWORD  mainThreadId;
static int    hookId = 1;
static HANDLE hMainThread;
static HANDLE hMissingService;
static THook *pLastHook = NULL;

/////////////////////////////////////////////////////////////////////////////////////////

static int QueueMainThread(PAPCFUNC pFunc, void* pParam, HANDLE hDoneEvent)
{
	int result = QueueUserAPC(pFunc, hMainThread, (ULONG_PTR)pParam);
	PostMessage(hAPCWindow, WM_NULL, 0, 0); // let this get processed in its own time
	if (hDoneEvent) {
		WaitForSingleObject(hDoneEvent, INFINITE);
		CloseHandle(hDoneEvent);
	}
	return result;
}

///////////////////////////////////////////////////////////////////////////////
// HOOKS

MIR_CORE_DLL(HANDLE) CreateHookableEvent(const char *name)
{
	if (name == NULL)
		return NULL;

	EnterCriticalSection(&csHooks);
	int idx;
	if ((idx = hooks.getIndex((THook*)name)) != -1) {
		LeaveCriticalSection(&csHooks);
		return NULL;
	}

	THook* newItem = (THook*)mir_alloc(sizeof(THook));
	strncpy(newItem->name, name, sizeof(newItem->name)); newItem->name[ MAXMODULELABELLENGTH-1 ] = 0;
	newItem->id = hookId++;
	newItem->subscriberCount = 0;
	newItem->subscriber = NULL;
	newItem->pfnHook = NULL;
	newItem->secretSignature = HOOK_SECRET_SIGNATURE;
	InitializeCriticalSection(&newItem->csHook);
	hooks.insert(newItem);

	LeaveCriticalSection(&csHooks);
	return (HANDLE)newItem;
}

MIR_CORE_DLL(int) DestroyHookableEvent(HANDLE hEvent)
{
	EnterCriticalSection(&csHooks);
	if (pLastHook == (THook*)hEvent)
		pLastHook = NULL;

	int idx;
	if ((idx = hooks.getIndex((THook*)hEvent)) == -1) {
      LeaveCriticalSection(&csHooks);
		return 1;
	}
	
	THook* p = hooks[idx];
	p->secretSignature = 0;
	if (p->subscriberCount) {
		mir_free(p->subscriber);
		p->subscriber = NULL;
		p->subscriberCount = 0;
	}
	hooks.remove(idx);
	DeleteCriticalSection(&p->csHook);
	mir_free(p);

	LeaveCriticalSection(&csHooks);
	return 0;
}

MIR_CORE_DLL(int) SetHookDefaultForHookableEvent(HANDLE hEvent, MIRANDAHOOK pfnHook)
{
	THook* p = (THook*)hEvent;

	EnterCriticalSection(&csHooks);
	if (hooks.getIndex(p) != -1)
		p->pfnHook = pfnHook;
	LeaveCriticalSection(&csHooks);
	return 0;
}

MIR_CORE_DLL(int) CallPluginEventHook(HINSTANCE hInst, HANDLE hEvent, WPARAM wParam, LPARAM lParam)
{
	int returnVal = 0;
	THook* p = (THook*)hEvent;
	if (p == NULL)
		return -1;

	EnterCriticalSection(&p->csHook);
	for (int i = 0; i < p->subscriberCount; i++) {
		THookSubscriber* s = &p->subscriber[i];
		if (s->hOwner != hInst)
			continue;

		switch (s->type) {
			case 1:	returnVal = s->pfnHook(wParam, lParam);	break;
			case 2:	returnVal = s->pfnHookParam(wParam, lParam, s->lParam); break;
			case 3:	returnVal = s->pfnHookObj(s->object, wParam, lParam); break;
			case 4:	returnVal = s->pfnHookObjParam(s->object, wParam, lParam, s->lParam); break;
			case 5:	returnVal = SendMessage(s->hwnd, s->message, wParam, lParam); break;
			default: continue;
		}
		if (returnVal)
			break;
	}

	if (p->subscriberCount == 0 && p->pfnHook != 0)
		returnVal = p->pfnHook(wParam, lParam);

	LeaveCriticalSection(&p->csHook);
	return returnVal;
}

MIR_CORE_DLL(int) CallHookSubscribers(HANDLE hEvent, WPARAM wParam, LPARAM lParam)
{
	int returnVal = 0;
	THook* p = (THook*)hEvent;
	if (p == NULL)
		return -1;

	EnterCriticalSection(&p->csHook);

	// NOTE: We've got the critical section while all this lot are called. That's mostly safe, though.
	for (int i = 0; i < p->subscriberCount; i++) {
		THookSubscriber* s = &p->subscriber[i];
		switch (s->type) {
			case 1:	returnVal = s->pfnHook(wParam, lParam);	break;
			case 2:	returnVal = s->pfnHookParam(wParam, lParam, s->lParam); break;
			case 3:	returnVal = s->pfnHookObj(s->object, wParam, lParam); break;
			case 4:	returnVal = s->pfnHookObjParam(s->object, wParam, lParam, s->lParam); break;
			case 5:	returnVal = SendMessage(s->hwnd, s->message, wParam, lParam); break;
			default: continue;
		}
		if (returnVal)
			break;
	}

	// check for no hooks and call the default hook if any
	if (p->subscriberCount == 0 && p->pfnHook != 0)
		returnVal = p->pfnHook(wParam, lParam);

	LeaveCriticalSection(&p->csHook);
	return returnVal;
}

static bool checkHook(HANDLE hHook)
{
	THook* p = (THook*)hHook;
	if (p == NULL)
		return false;

	bool ret;
	__try
	{
		if (p->secretSignature != HOOK_SECRET_SIGNATURE)
			ret = false;
		else if (p->subscriberCount == 0 && p->pfnHook == NULL)
			ret = false;
		else
			ret = true;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		ret = false;
	}

	return ret;
}

static void CALLBACK HookToMainAPCFunc(ULONG_PTR dwParam)
{
	THookToMainThreadItem* item = (THookToMainThreadItem*)dwParam;
	item->result = CallHookSubscribers(item->hook, item->wParam, item->lParam);
	SetEvent(item->hDoneEvent);
}

MIR_CORE_DLL(int) NotifyEventHooks(HANDLE hEvent, WPARAM wParam, LPARAM lParam)
{
	if ( !checkHook(hEvent))
		return -1;

	if ( GetCurrentThreadId() == mainThreadId)
		return CallHookSubscribers(hEvent, wParam, lParam);

	mir_ptr<THookToMainThreadItem> item;
	item->hDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	item->hook = (THook*)hEvent;
	item->wParam = wParam;
	item->lParam = lParam;
	QueueMainThread(HookToMainAPCFunc, item, item->hDoneEvent);
	return item->result;
}

static HANDLE HookEventInt(int type, const char* name, MIRANDAHOOK hookProc, void* object, LPARAM lParam)
{
	EnterCriticalSection(&csHooks);
	int idx;
	if ((idx = hooks.getIndex((THook*)name)) == -1) {
		#ifdef _DEBUG
			OutputDebugStringA("Attempt to hook: \t");
			OutputDebugStringA(name);
			OutputDebugStringA("\n");
		#endif
		LeaveCriticalSection(&csHooks);
		return NULL;
	}

	THook* p = hooks[ idx ];
	p->subscriber = (THookSubscriber*)mir_realloc(p->subscriber, sizeof(THookSubscriber)*(p->subscriberCount+1));
	p->subscriber[ p->subscriberCount ].type = type;
	p->subscriber[ p->subscriberCount ].pfnHook = hookProc;
	p->subscriber[ p->subscriberCount ].object = object;
	p->subscriber[ p->subscriberCount ].lParam = lParam;
	p->subscriber[ p->subscriberCount ].hOwner = GetInstByAddress(hookProc);
	p->subscriberCount++;

	HANDLE ret = (HANDLE)((p->id << 16) | p->subscriberCount);
	LeaveCriticalSection(&csHooks);
	return ret;
}

MIR_CORE_DLL(HANDLE) HookEvent(const char* name, MIRANDAHOOK hookProc)
{
	return HookEventInt(1, name, hookProc, 0, 0);
}

MIR_CORE_DLL(HANDLE) HookEventParam(const char* name, MIRANDAHOOKPARAM hookProc, LPARAM lParam)
{
	return HookEventInt(2, name, (MIRANDAHOOK)hookProc, 0, lParam);
}

MIR_CORE_DLL(HANDLE) HookEventObj(const char* name, MIRANDAHOOKOBJ hookProc, void* object)
{
	return HookEventInt(3, name, (MIRANDAHOOK)hookProc, object, 0);
}

MIR_CORE_DLL(HANDLE) HookEventObjParam(const char* name, MIRANDAHOOKOBJPARAM hookProc, void* object, LPARAM lParam)
{
	return HookEventInt(4, name, (MIRANDAHOOK)hookProc, object, lParam);
}

MIR_CORE_DLL(HANDLE) HookEventMessage(const char* name, HWND hwnd, UINT message)
{
	EnterCriticalSection(&csHooks);

	int idx;
	if ((idx = hooks.getIndex((THook*)name)) == -1) {
		#ifdef _DEBUG
			MessageBoxA(NULL, "Attempt to hook non-existant event", name, MB_OK);
		#endif
		LeaveCriticalSection(&csHooks);
		return NULL;
	}

	THook* p = hooks[ idx ];
	p->subscriber = (THookSubscriber*)mir_realloc(p->subscriber, sizeof(THookSubscriber)*(p->subscriberCount+1));
	p->subscriber[ p->subscriberCount ].type = 5;
	p->subscriber[ p->subscriberCount ].hwnd = hwnd;
	p->subscriber[ p->subscriberCount ].message = message;
	p->subscriberCount++;

	HANDLE ret = (HANDLE)((p->id << 16) | p->subscriberCount);
	LeaveCriticalSection(&csHooks);
	return ret;
}

MIR_CORE_DLL(int) UnhookEvent(HANDLE hHook)
{
	if (hHook == NULL)
		return 0;

	int hookId = (int)hHook >> 16;
	int subscriberId = ((int)hHook & 0xFFFF) - 1;

	EnterCriticalSection(&csHooks);

	THook* p = NULL;
	for (int i = 0; i < hooks.getCount(); i++)
		if (hooks[i]->id == hookId) {
			p = hooks[i];
			break;
		}

	if (p == NULL) {
		LeaveCriticalSection(&csHooks);
		return 1;
	}

	if (subscriberId >= p->subscriberCount || subscriberId < 0) {
		LeaveCriticalSection(&csHooks);
		return 1;
	}

	p->subscriber[subscriberId].type    = 0;
	p->subscriber[subscriberId].pfnHook = NULL;
	p->subscriber[subscriberId].hOwner  = NULL;
	while (p->subscriberCount && p->subscriber[p->subscriberCount-1].type == 0)
		p->subscriberCount--;
	if (p->subscriberCount == 0) {
		if (p->subscriber) mir_free(p->subscriber);
		p->subscriber = NULL;
	}
	LeaveCriticalSection(&csHooks);
	return 0;
}

MIR_CORE_DLL(void) KillModuleEventHooks(HINSTANCE hInst)
{
	EnterCriticalSection(&csHooks);

	for (int i = hooks.getCount()-1; i >= 0; i--) {
		if (hooks[i]->subscriberCount == 0)
			continue;

		for (int j = hooks[i]->subscriberCount-1; j >= 0; j--) {
			if (hooks[i]->subscriber[j].hOwner != hInst)
				continue;

			char szModuleName[ MAX_PATH ];
			GetModuleFileNameA(hooks[i]->subscriber[j].hOwner, szModuleName, sizeof(szModuleName));
			UnhookEvent((HANDLE)((hooks[i]->id << 16) + j + 1));
			if (hooks[i]->subscriberCount == 0)
				break;
		}
	}

	LeaveCriticalSection(&csHooks);
}

MIR_CORE_DLL(void) KillObjectEventHooks(void* pObject)
{
	EnterCriticalSection(&csHooks);

	for (int i = hooks.getCount()-1; i >= 0; i--) {
		if (hooks[i]->subscriberCount == 0)
			continue;

		for (int j = hooks[i]->subscriberCount-1; j >= 0; j--) {
			if (hooks[i]->subscriber[j].object == pObject) {
				UnhookEvent((HANDLE)((hooks[i]->id << 16) + j + 1));
				if (hooks[i]->subscriberCount == 0)
					break;
	}	}	}

	LeaveCriticalSection(&csHooks);
}

/////////////////////SERVICES

static __inline TService* FindServiceByName(const char *name)
{
	unsigned hash = mir_hashstr(name);
	return services.find((TService*)&hash);
}

static HANDLE CreateServiceInt(int type, const char *name, MIRANDASERVICE serviceProc, void* object, LPARAM lParam)
{
	if (name == NULL)
		return NULL;

	TService tmp;
	tmp.nameHash = mir_hashstr(name);

	EnterCriticalSection(&csServices);

	if (services.getIndex(&tmp) != -1) {
		LeaveCriticalSection(&csServices);
		return NULL;
	}

	TService* p = (TService*)mir_alloc(sizeof(*p) + strlen(name));
	strcpy(p->name, name);
	p->nameHash   = tmp.nameHash;
	p->pfnService = serviceProc;
	p->hOwner     = GetInstByAddress(serviceProc);
	p->flags      = type;
	p->lParam     = lParam;
	p->object     = object;
	services.insert(p);

	LeaveCriticalSection(&csServices);
	return (HANDLE)tmp.nameHash;
}

MIR_CORE_DLL(HANDLE) CreateServiceFunction(const char *name, MIRANDASERVICE serviceProc)
{
	return CreateServiceInt(0, name, serviceProc, 0, 0);
}

MIR_CORE_DLL(HANDLE) CreateServiceFunctionParam(const char *name, MIRANDASERVICEPARAM serviceProc, LPARAM lParam)
{
	return CreateServiceInt(1, name, (MIRANDASERVICE)serviceProc, 0, lParam);
}

MIR_CORE_DLL(HANDLE) CreateServiceFunctionObj(const char *name, MIRANDASERVICEOBJ serviceProc, void* object)
{
	return CreateServiceInt(2, name, (MIRANDASERVICE)serviceProc, object, 0);
}

MIR_CORE_DLL(HANDLE) CreateServiceFunctionObjParam(const char *name, MIRANDASERVICEOBJPARAM serviceProc, void* object, LPARAM lParam)
{
	return CreateServiceInt(3, name, (MIRANDASERVICE)serviceProc, object, lParam);
}

MIR_CORE_DLL(int) DestroyServiceFunction(HANDLE hService)
{
	int idx;

	EnterCriticalSection(&csServices);
	if ((idx = services.getIndex((TService*)&hService)) != -1) {
		mir_free(services[idx]);
		services.remove(idx);
	}

	LeaveCriticalSection(&csServices);
	return 0;
}

MIR_CORE_DLL(int) ServiceExists(const char *name)
{
	if (name == NULL)
		return FALSE;

	EnterCriticalSection(&csServices);
	int ret = FindServiceByName(name) != NULL;
	LeaveCriticalSection(&csServices);
	return ret;
}

MIR_CORE_DLL(INT_PTR) CallService(const char *name, WPARAM wParam, LPARAM lParam)
{
	#ifdef _DEBUG
		if (name == NULL) {
			MessageBoxA(0, "Someone tried to CallService(NULL, ..) see stack trace for details", "", 0);
			DebugBreak();
			return CALLSERVICE_NOTFOUND;
		}
	#else
		if (name == NULL) return CALLSERVICE_NOTFOUND;
	#endif

	EnterCriticalSection(&csServices);
	TService *pService = FindServiceByName(name);
	if (pService == NULL) {
		LeaveCriticalSection(&csServices);
		#ifdef _DEBUG
			OutputDebugStringA("Missing service called: \t");
			OutputDebugStringA(name);
			OutputDebugStringA("\n");
		#endif

		return CALLSERVICE_NOTFOUND;
	}

	MIRANDASERVICE pfnService = pService->pfnService;
	int flags = pService->flags;
	LPARAM fnParam = pService->lParam;
	void* object = pService->object;
	LeaveCriticalSection(&csServices);
	switch(flags) {
		case 1:  return ((MIRANDASERVICEPARAM)pfnService)(wParam, lParam, fnParam);
		case 2:  return ((MIRANDASERVICEOBJ)pfnService)(object, wParam, lParam);
		case 3:  return ((MIRANDASERVICEOBJPARAM)pfnService)(object, wParam, lParam, fnParam);
		default: return pfnService(wParam, lParam);
}	}

static void CALLBACK CallServiceToMainAPCFunc(ULONG_PTR dwParam)
{
	TServiceToMainThreadItem *item = (TServiceToMainThreadItem*) dwParam;
	item->result = CallService(item->name, item->wParam, item->lParam);
	SetEvent(item->hDoneEvent);
}

MIR_CORE_DLL(INT_PTR) CallServiceSync(const char *name, WPARAM wParam, LPARAM lParam)
{
	if (name == NULL)
		return CALLSERVICE_NOTFOUND;
	
	// the service is looked up within the main thread, since the time it takes
	// for the APC queue to clear the service being called maybe removed.
	// even thou it may exists before the call, the critsec can't be locked between calls.
	if (GetCurrentThreadId() == mainThreadId)
		return CallService(name, wParam, lParam);

	mir_ptr<TServiceToMainThreadItem> item;
	item->wParam = wParam;
	item->lParam = lParam;
	item->name = name;
	item->hDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	QueueMainThread(CallServiceToMainAPCFunc, item, item->hDoneEvent);
	return item->result;
}

MIR_CORE_DLL(int) CallFunctionAsync(void (__stdcall *func)(void *), void *arg)
{
	QueueMainThread((PAPCFUNC)func, arg, 0);
	return 0;
}

MIR_CORE_DLL(void) KillModuleServices(HINSTANCE hInst)
{
	EnterCriticalSection(&csServices);

	for (int i = services.getCount()-1; i >= 0; i--) {
		if (services[i]->hOwner == hInst) {
			char szModuleName[ MAX_PATH ];
			GetModuleFileNameA(services[i]->hOwner, szModuleName, sizeof(szModuleName));
			DestroyServiceFunction((HANDLE)services[i]->nameHash);
		}
	}

	LeaveCriticalSection(&csServices);
}

MIR_CORE_DLL(void) KillObjectServices(void* pObject)
{
	EnterCriticalSection(&csServices);

	for (int i = services.getCount()-1; i >= 0; i--)
		if (services[i]->object == pObject)
			DestroyServiceFunction((HANDLE)services[i]->nameHash);

	LeaveCriticalSection(&csServices);
}

///////////////////////////////////////////////////////////////////////////////

int InitialiseModularEngine(void)
{
	InitializeCriticalSection(&csHooks);
	InitializeCriticalSection(&csServices);

	mainThreadId = GetCurrentThreadId();
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hMainThread, 0, FALSE, DUPLICATE_SAME_ACCESS);

	hMissingService = CreateHookableEvent(ME_SYSTEM_MISSINGSERVICE);
	return 0;
}

void DestroyModularEngine(void)
{
	EnterCriticalSection(&csHooks);

	for (int i=0; i < hooks.getCount(); i++) {
		THook* p = hooks[i];
		if (p->subscriberCount)
			mir_free(p->subscriber);
		DeleteCriticalSection(&p->csHook);
		mir_free(p);
	}
	hooks.destroy();

	LeaveCriticalSection(&csHooks);
	DeleteCriticalSection(&csHooks);

	EnterCriticalSection(&csServices);

	for (int j=0; j < services.getCount(); j++)
		mir_free(services[j]);
	services.destroy();

	LeaveCriticalSection(&csServices);
	DeleteCriticalSection(&csServices);
	CloseHandle(hMainThread);
}
