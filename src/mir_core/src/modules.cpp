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
static mir_cs csHooks, csServices;
static DWORD  mainThreadId;
static int    hookId = 1;

/////////////////////////////////////////////////////////////////////////////////////////

__forceinline HANDLE getThreadEvent()
{
	HANDLE pData = (HANDLE)TlsGetValue(mir_tls);
	if (pData == NULL) {
		pData = CreateEvent(NULL, FALSE, FALSE, NULL);
		TlsSetValue(mir_tls, pData);
	}
	return pData;
}

static int QueueMainThread(PAPCFUNC pFunc, void* pParam, HANDLE hDoneEvent)
{
	int result = PostMessage(hAPCWindow, WM_USER+1, (WPARAM)pFunc, (LPARAM)pParam); // let this get processed in its own time
	if (hDoneEvent)
		WaitForSingleObject(hDoneEvent, INFINITE);

	return result;
}

///////////////////////////////////////////////////////////////////////////////
// HOOKS

MIR_CORE_DLL(HANDLE) CreateHookableEvent(const char *name)
{
	if (name == NULL)
		return NULL;

	mir_cslock lck(csHooks);

	int idx;
	if ((idx = hooks.getIndex((THook*)name)) != -1)
		return hooks[idx];

	THook* newItem = (THook*)mir_alloc(sizeof(THook));
	strncpy(newItem->name, name, sizeof(newItem->name)); newItem->name[ MAXMODULELABELLENGTH-1 ] = 0;
	newItem->id = hookId++;
	newItem->subscriberCount = 0;
	newItem->subscriber = NULL;
	newItem->pfnHook = NULL;
	newItem->secretSignature = HOOK_SECRET_SIGNATURE;
	InitializeCriticalSection(&newItem->csHook);
	hooks.insert(newItem);
	return (HANDLE)newItem;
}

MIR_CORE_DLL(int) DestroyHookableEvent(HANDLE hEvent)
{
	if (hEvent == NULL)
		return 1;

	mir_cslock lck(csHooks);

	int idx;
	if ((idx = hooks.getIndex((THook*)hEvent)) == -1)
		return 1;

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
	return 0;
}

MIR_CORE_DLL(int) SetHookDefaultForHookableEvent(HANDLE hEvent, MIRANDAHOOK pfnHook)
{
	THook* p = (THook*)hEvent;

	mir_cslock lck(csHooks);
	if (hooks.getIndex(p) != -1)
		p->pfnHook = pfnHook;
	return 0;
}

MIR_CORE_DLL(int) CallPluginEventHook(HINSTANCE hInst, HANDLE hEvent, WPARAM wParam, LPARAM lParam)
{
	THook* p = (THook*)hEvent;
	if (p == NULL || hInst == NULL)
		return -1;

	mir_cslock lck(p->csHook);
	for (int i = 0; i < p->subscriberCount; i++) {
		THookSubscriber* s = &p->subscriber[i];
		if (s->hOwner != hInst)
			continue;

		int returnVal;
		switch (s->type) {
			case 1:	returnVal = s->pfnHook(wParam, lParam);	break;
			case 2:	returnVal = s->pfnHookParam(wParam, lParam, s->lParam); break;
			case 3:	returnVal = s->pfnHookObj(s->object, wParam, lParam); break;
			case 4:	returnVal = s->pfnHookObjParam(s->object, wParam, lParam, s->lParam); break;
			case 5:	returnVal = SendMessage(s->hwnd, s->message, wParam, lParam); break;
			default: continue;
		}
		if (returnVal)
			return returnVal;
	}

	if (p->subscriberCount == 0 && p->pfnHook != 0)
		return p->pfnHook(wParam, lParam);

	return 0;
}

static int CallHookSubscribers(THook* p, WPARAM wParam, LPARAM lParam)
{
	if (p == NULL)
		return -1;

	mir_cslock lck(p->csHook);

	// NOTE: We've got the critical section while all this lot are called. That's mostly safe, though.
	for (int i = 0; i < p->subscriberCount; i++) {
		THookSubscriber* s = &p->subscriber[i];

		int returnVal;
		switch (s->type) {
			case 1:	returnVal = s->pfnHook(wParam, lParam);	break;
			case 2:	returnVal = s->pfnHookParam(wParam, lParam, s->lParam); break;
			case 3:	returnVal = s->pfnHookObj(s->object, wParam, lParam); break;
			case 4:	returnVal = s->pfnHookObjParam(s->object, wParam, lParam, s->lParam); break;
			case 5:	returnVal = SendMessage(s->hwnd, s->message, wParam, lParam); break;
			default: continue;
		}
		if (returnVal)
			return returnVal;
	}

	// call the default hook if any
	if (p->pfnHook != 0)
		return p->pfnHook(wParam, lParam);

	return 0;
}

enum { hookOk, hookEmpty, hookInvalid };

__forceinline int checkHook(THook* p)
{
	if (p == NULL)
		return hookInvalid;

	int ret;
	__try
	{
		if (p->secretSignature != HOOK_SECRET_SIGNATURE)
			ret = hookInvalid;
		else if (p->subscriberCount == 0 && p->pfnHook == NULL)
			ret = hookEmpty;
		else
			ret = hookOk;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		ret = hookInvalid;
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
	switch ( checkHook((THook*)hEvent)) {
		case hookInvalid: return -1;
		case hookEmpty: return 0;
	}

	if ( GetCurrentThreadId() == mainThreadId)
		return CallHookSubscribers((THook*)hEvent, wParam, lParam);

	THookToMainThreadItem item;
	item.hDoneEvent = getThreadEvent();
	item.hook = (THook*)hEvent;
	item.wParam = wParam;
	item.lParam = lParam;
	QueueMainThread(HookToMainAPCFunc, &item, item.hDoneEvent);
	return item.result;
}

MIR_CORE_DLL(int) NotifyFastHook(HANDLE hEvent, WPARAM wParam, LPARAM lParam)
{
	switch ( checkHook((THook*)hEvent)) {
		case hookInvalid: return -1;
		case hookEmpty: return 0;
	}

	return CallHookSubscribers((THook*)hEvent, wParam, lParam);
}

extern "C" MIR_CORE_DLL(int) GetSubscribersCount(THook* pHook)
{
	switch ( checkHook(pHook)) {
		case hookInvalid:
		case hookEmpty: return 0;
	}
	return pHook->subscriberCount;
}

static HANDLE HookEventInt(int type, const char *name, MIRANDAHOOK hookProc, void* object, LPARAM lParam)
{
	mir_cslock lck(csHooks);

	int idx;
	if ((idx = hooks.getIndex((THook*)name)) == -1)
		return NULL;

	THook* p = hooks[ idx ];
	p->subscriber = (THookSubscriber*)mir_realloc(p->subscriber, sizeof(THookSubscriber)*(p->subscriberCount+1));
	p->subscriber[ p->subscriberCount ].type = type;
	p->subscriber[ p->subscriberCount ].pfnHook = hookProc;
	p->subscriber[ p->subscriberCount ].object = object;
	p->subscriber[ p->subscriberCount ].lParam = lParam;
	p->subscriber[ p->subscriberCount ].hOwner = GetInstByAddress(hookProc);
	p->subscriberCount++;

	return (HANDLE)((p->id << 16) | p->subscriberCount);
}

MIR_CORE_DLL(HANDLE) HookEvent(const char *name, MIRANDAHOOK hookProc)
{
	return HookEventInt(1, name, hookProc, 0, 0);
}

MIR_CORE_DLL(HANDLE) HookEventParam(const char *name, MIRANDAHOOKPARAM hookProc, LPARAM lParam)
{
	return HookEventInt(2, name, (MIRANDAHOOK)hookProc, 0, lParam);
}

MIR_CORE_DLL(HANDLE) HookEventObj(const char *name, MIRANDAHOOKOBJ hookProc, void* object)
{
	return HookEventInt(3, name, (MIRANDAHOOK)hookProc, object, 0);
}

MIR_CORE_DLL(HANDLE) HookEventObjParam(const char *name, MIRANDAHOOKOBJPARAM hookProc, void* object, LPARAM lParam)
{
	return HookEventInt(4, name, (MIRANDAHOOK)hookProc, object, lParam);
}

MIR_CORE_DLL(HANDLE) HookEventMessage(const char *name, HWND hwnd, UINT message)
{
	mir_cslock lck(csHooks);

	int idx;
	if ((idx = hooks.getIndex((THook*)name)) == -1)
		return NULL;

	THook* p = hooks[ idx ];
	p->subscriber = (THookSubscriber*)mir_realloc(p->subscriber, sizeof(THookSubscriber)*(p->subscriberCount+1));
	p->subscriber[ p->subscriberCount ].type = 5;
	p->subscriber[ p->subscriberCount ].hwnd = hwnd;
	p->subscriber[ p->subscriberCount ].message = message;
	p->subscriberCount++;
	return (HANDLE)((p->id << 16) | p->subscriberCount);
}

MIR_CORE_DLL(int) UnhookEvent(HANDLE hHook)
{
	if (hHook == NULL)
		return 0;

	int hookId = (int)hHook >> 16;
	int subscriberId = ((int)hHook & 0xFFFF) - 1;

	mir_cslock lck(csHooks);

	THook* p = NULL;
	for (int i = 0; i < hooks.getCount(); i++)
		if (hooks[i]->id == hookId) {
			p = hooks[i];
			break;
		}

	if (p == NULL)
		return 1;

	if (subscriberId >= p->subscriberCount || subscriberId < 0)
		return 1;

	p->subscriber[subscriberId].type    = 0;
	p->subscriber[subscriberId].pfnHook = NULL;
	p->subscriber[subscriberId].hOwner  = NULL;
	while (p->subscriberCount && p->subscriber[p->subscriberCount-1].type == 0)
		p->subscriberCount--;
	if (p->subscriberCount == 0) {
		mir_free(p->subscriber);
		p->subscriber = NULL;
	}
	return 0;
}

MIR_CORE_DLL(void) KillModuleEventHooks(HINSTANCE hInst)
{
	mir_cslock lck(csHooks);

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
}

MIR_CORE_DLL(void) KillObjectEventHooks(void* pObject)
{
	mir_cslock lck(csHooks);

	for (int i = hooks.getCount()-1; i >= 0; i--) {
		if (hooks[i]->subscriberCount == 0)
			continue;

		for (int j = hooks[i]->subscriberCount-1; j >= 0; j--) {
			if (hooks[i]->subscriber[j].object == pObject) {
				UnhookEvent((HANDLE)((hooks[i]->id << 16) + j + 1));
				if (hooks[i]->subscriberCount == 0)
					break;
			}
		}
	}
}

static void DestroyHooks()
{
	mir_cslock lck(csHooks);

	for (int i=0; i < hooks.getCount(); i++) {
		THook* p = hooks[i];
		if (p->subscriberCount)
			mir_free(p->subscriber);
		DeleteCriticalSection(&p->csHook);
		mir_free(p);
	}
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

	mir_cslock lck(csServices);

	if (services.getIndex(&tmp) != -1)
		return NULL;

	TService* p = (TService*)mir_alloc(sizeof(*p) + strlen(name));
	strcpy(p->name, name);
	p->nameHash   = tmp.nameHash;
	p->pfnService = serviceProc;
	p->hOwner     = GetInstByAddress(serviceProc);
	p->flags      = type;
	p->lParam     = lParam;
	p->object     = object;
	services.insert(p);

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

MIR_CORE_DLL(HANDLE) CreateProtoServiceFunction(const char *szModule, const char *szService, MIRANDASERVICE serviceProc)
{
	char str[MAXMODULELABELLENGTH * 2];
	strncpy_s(str, szModule, _TRUNCATE);
	strncat_s(str, szService, _TRUNCATE);
	return CreateServiceFunction(str, serviceProc);
}

MIR_CORE_DLL(int) DestroyServiceFunction(HANDLE hService)
{
	mir_cslock lck(csServices);

	int idx;
	if ((idx = services.getIndex((TService*)&hService)) != -1) {
		mir_free(services[idx]);
		services.remove(idx);
	}

	return 0;
}

MIR_CORE_DLL(int) ServiceExists(const char *name)
{
	if (name == NULL)
		return FALSE;

	mir_cslock lck(csServices);
	return FindServiceByName(name) != NULL;
}

MIR_CORE_DLL(INT_PTR) CallService(const char *name, WPARAM wParam, LPARAM lParam)
{
	if (name == NULL)
		return CALLSERVICE_NOTFOUND;

	TService *pService;
	{
		mir_cslock lck(csServices);
		if ((pService = FindServiceByName(name)) == NULL)
			return CALLSERVICE_NOTFOUND;
	}

	MIRANDASERVICE pfnService = pService->pfnService;
	int flags = pService->flags;
	LPARAM fnParam = pService->lParam;
	void* object = pService->object;
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

	TServiceToMainThreadItem item;
	item.wParam = wParam;
	item.lParam = lParam;
	item.name = name;
	item.hDoneEvent = getThreadEvent();
	QueueMainThread(CallServiceToMainAPCFunc, &item, item.hDoneEvent);
	return item.result;
}

MIR_CORE_DLL(int) CallFunctionAsync(void (__stdcall *func)(void *), void *arg)
{
	if (GetCurrentThreadId() == mainThreadId)
		func(arg);
	else
		QueueMainThread((PAPCFUNC)func, arg, 0);
	return 0;
}

MIR_CORE_DLL(void) KillModuleServices(HINSTANCE hInst)
{
	mir_cslock lck(csServices);

	for (int i = services.getCount()-1; i >= 0; i--) {
		if (services[i]->hOwner == hInst) {
			char szModuleName[ MAX_PATH ];
			GetModuleFileNameA(services[i]->hOwner, szModuleName, sizeof(szModuleName));
			DestroyServiceFunction((HANDLE)services[i]->nameHash);
		}
	}
}

MIR_CORE_DLL(void) KillObjectServices(void* pObject)
{
	mir_cslock lck(csServices);

	for (int i = services.getCount()-1; i >= 0; i--)
		if (services[i]->object == pObject)
			DestroyServiceFunction((HANDLE)services[i]->nameHash);
}

static void DestroyServices()
{
	mir_cslock lck(csServices);

	for (int j=0; j < services.getCount(); j++)
		mir_free(services[j]);
}

///////////////////////////////////////////////////////////////////////////////

static int sttComparePlugins(const HINSTANCE__* p1, const HINSTANCE__* p2)
{
	if (p1 == p2)
		return 0;

	return (p1 < p2) ? -1 : 1;
}

LIST<HINSTANCE__> pluginListAddr(10, sttComparePlugins);

MIR_CORE_DLL(void) RegisterModule(HINSTANCE hInst)
{
	pluginListAddr.insert(hInst);
}

MIR_CORE_DLL(void) UnregisterModule(HINSTANCE hInst)
{
	pluginListAddr.remove(hInst);
}

MIR_CORE_DLL(HINSTANCE) GetInstByAddress(void* codePtr)
{
	if (pluginListAddr.getCount() == 0)
		return NULL;

	int idx;
	List_GetIndex((SortedList*)&pluginListAddr, codePtr, &idx);
	if (idx > 0)
		idx--;

	HINSTANCE result = pluginListAddr[idx];
	if (result < hInst && codePtr > hInst)
		result = hInst;
	else if (idx == 0 && codePtr < (void*)result)
		result = NULL;

	return result;
}

///////////////////////////////////////////////////////////////////////////////

int InitialiseModularEngine(void)
{
	mainThreadId = GetCurrentThreadId();
	return 0;
}

void DestroyModularEngine(void)
{
	DestroyHooks();
	DestroyServices();
}
