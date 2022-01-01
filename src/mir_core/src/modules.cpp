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
	uint32_t nameHash;
	HINSTANCE hOwner;
	union
	{
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
static uint32_t  mainThreadId;
static int    sttHookId = 1;

/////////////////////////////////////////////////////////////////////////////////////////

__forceinline HANDLE getThreadEvent()
{
	HANDLE pData = (HANDLE)TlsGetValue(mir_tls);
	if (pData == nullptr) {
		pData = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		TlsSetValue(mir_tls, pData);
	}
	return pData;
}

static int QueueMainThread(PAPCFUNC pFunc, void* pParam, HANDLE hDoneEvent)
{
	int result = PostMessage(hAPCWindow, WM_USER + 1, (WPARAM)pFunc, (LPARAM)pParam); // let this get processed in its own time
	if (hDoneEvent)
		WaitForSingleObject(hDoneEvent, INFINITE);

	return result;
}

///////////////////////////////////////////////////////////////////////////////
// HOOKS

MIR_CORE_DLL(HANDLE) CreateHookableEvent(const char *name)
{
	if (name == nullptr)
		return nullptr;

	mir_cslock lck(csHooks);

	int idx;
	if ((idx = hooks.getIndex((THook*)name)) != -1)
		return hooks[idx];

	THook *newItem = new THook();
	strncpy(newItem->name, name, sizeof(newItem->name)); newItem->name[MAXMODULELABELLENGTH - 1] = 0;
	newItem->id = sttHookId++;
	hooks.insert(newItem);
	return (HANDLE)newItem;
}

MIR_CORE_DLL(int) DestroyHookableEvent(HANDLE hEvent)
{
	if (hEvent == nullptr)
		return 1;

	mir_cslock lck(csHooks);

	int idx;
	if ((idx = hooks.getIndex((THook*)hEvent)) == -1)
		return 1;

	THook *p = hooks[idx];
	p->secretSignature = 0;
	if (p->subscriberCount) {
		mir_free(p->subscriber);
		p->subscriber = nullptr;
		p->subscriberCount = 0;
	}
	hooks.remove(idx);
	delete p;
	return 0;
}

MIR_CORE_DLL(int) SetHookDefaultForHookableEvent(HANDLE hEvent, MIRANDAHOOK pfnHook)
{
	THook *p = (THook*)hEvent;

	mir_cslock lck(csHooks);
	if (hooks.getIndex(p) != -1)
		p->pfnHook = pfnHook;
	return 0;
}

MIR_CORE_DLL(int) CallPluginEventHook(HINSTANCE hInst, HANDLE hEvent, WPARAM wParam, LPARAM lParam)
{
	THook *p = (THook*)hEvent;
	if (p == nullptr || hInst == nullptr)
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

	if (p->subscriberCount == 0 && p->pfnHook != nullptr)
		return p->pfnHook(wParam, lParam);

	return 0;
}

MIR_CORE_DLL(int) CallObjectEventHook(void *pObject, HANDLE hEvent, WPARAM wParam, LPARAM lParam)
{
	THook *p = (THook*)hEvent;
	if (p == nullptr || pObject == nullptr)
		return -1;

	mir_cslock lck(p->csHook);
	for (int i = 0; i < p->subscriberCount; i++) {
		THookSubscriber* s = &p->subscriber[i];
		if (s->object != pObject)
			continue;

		int returnVal;
		switch (s->type) {
		case 3:	returnVal = s->pfnHookObj(s->object, wParam, lParam); break;
		case 4:	returnVal = s->pfnHookObjParam(s->object, wParam, lParam, s->lParam); break;
		default: continue;
		}
		if (returnVal)
			return returnVal;
	}

	if (p->subscriberCount == 0 && p->pfnHook != nullptr)
		return p->pfnHook(wParam, lParam);

	return 0;
}

static int CallHookSubscribers(THook *p, WPARAM wParam, LPARAM lParam)
{
	if (p == nullptr)
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
	if (p->pfnHook != nullptr)
		return p->pfnHook(wParam, lParam);

	return 0;
}

enum { hookOk, hookEmpty, hookInvalid };

int checkHook(THook *p)
{
	if (p == nullptr)
		return hookInvalid;

	int ret;
	__try {
		if (p->secretSignature != HOOK_SECRET_SIGNATURE)
			ret = hookInvalid;
		else if (p->subscriberCount == 0 && p->pfnHook == nullptr)
			ret = hookEmpty;
		else
			ret = hookOk;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
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
	switch (checkHook((THook*)hEvent)) {
	case hookInvalid: return -1;
	case hookEmpty: return 0;
	}

	if (GetCurrentThreadId() == mainThreadId)
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
	switch (checkHook((THook*)hEvent)) {
	case hookInvalid: return -1;
	case hookEmpty: return 0;
	}

	return CallHookSubscribers((THook*)hEvent, wParam, lParam);
}

extern "C" MIR_CORE_DLL(int) GetSubscribersCount(THook* pHook)
{
	switch (checkHook(pHook)) {
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
		return nullptr;

	THook *p = hooks[idx];
	p->subscriber = (THookSubscriber*)mir_realloc(p->subscriber, sizeof(THookSubscriber)*(p->subscriberCount + 1));

	THookSubscriber &s = p->subscriber[p->subscriberCount];
	s.type = type;
	s.pfnHook = hookProc;
	s.object = object;
	s.lParam = lParam;
	s.hOwner = GetInstByAddress(hookProc);
	p->subscriberCount++;

	return (HANDLE)((p->id << 16) | p->subscriberCount);
}

MIR_CORE_DLL(HANDLE) HookEvent(const char *name, MIRANDAHOOK hookProc)
{
	return HookEventInt(1, name, hookProc, nullptr, 0);
}

MIR_CORE_DLL(HANDLE) HookEventParam(const char *name, MIRANDAHOOKPARAM hookProc, LPARAM lParam)
{
	return HookEventInt(2, name, (MIRANDAHOOK)hookProc, nullptr, lParam);
}

MIR_CORE_DLL(HANDLE) HookEventObj(const char *name, MIRANDAHOOKOBJ hookProc, void* object)
{
	return HookEventInt(3, name, (MIRANDAHOOK)hookProc, object, 0);
}

MIR_CORE_DLL(HANDLE) HookEventObjParam(const char *name, MIRANDAHOOKOBJPARAM hookProc, void* object, LPARAM lParam)
{
	return HookEventInt(4, name, (MIRANDAHOOK)hookProc, object, lParam);
}

MIR_CORE_DLL(HANDLE) HookTemporaryEvent(const char *name, MIRANDAHOOK hookProc)
{
	mir_cslockfull lck(csHooks);

	int idx;
	if ((idx = hooks.getIndex((THook*)name)) == -1) {
		lck.unlock();
		hookProc(0, 0);
		return nullptr;
	}

	THook *p = hooks[idx];
	p->subscriber = (THookSubscriber*)mir_realloc(p->subscriber, sizeof(THookSubscriber)*(p->subscriberCount + 1));

	THookSubscriber &s = p->subscriber[p->subscriberCount];
	memset(&s, 0, sizeof(THookSubscriber));
	s.type = 1;
	s.pfnHook = hookProc;
	s.hOwner = GetInstByAddress(hookProc);

	p->subscriberCount++;
	return (HANDLE)((p->id << 16) | p->subscriberCount);
}

MIR_CORE_DLL(HANDLE) HookEventMessage(const char *name, HWND hwnd, UINT message)
{
	mir_cslock lck(csHooks);

	int idx;
	if ((idx = hooks.getIndex((THook*)name)) == -1)
		return nullptr;

	THook *p = hooks[idx];
	p->subscriber = (THookSubscriber*)mir_realloc(p->subscriber, sizeof(THookSubscriber)*(p->subscriberCount + 1));
	p->subscriber[p->subscriberCount].type = 5;
	p->subscriber[p->subscriberCount].hwnd = hwnd;
	p->subscriber[p->subscriberCount].message = message;
	p->subscriberCount++;
	return (HANDLE)((p->id << 16) | p->subscriberCount);
}

MIR_CORE_DLL(int) UnhookEvent(HANDLE hHook)
{
	if (hHook == nullptr)
		return 0;

	int hookId = (INT_PTR)hHook >> 16;
	int subscriberId = ((INT_PTR)hHook & 0xFFFF) - 1;

	mir_cslock lck(csHooks);

	THook *p = nullptr;
	for (auto &it : hooks)
		if (it->id == hookId) {
			p = it;
			break;
		}

	if (p == nullptr)
		return 1;

	if (subscriberId >= p->subscriberCount || subscriberId < 0)
		return 1;

	p->subscriber[subscriberId].type = 0;
	p->subscriber[subscriberId].pfnHook = nullptr;
	p->subscriber[subscriberId].hOwner = nullptr;
	while (p->subscriberCount && p->subscriber[p->subscriberCount - 1].type == 0)
		p->subscriberCount--;
	if (p->subscriberCount == 0) {
		mir_free(p->subscriber);
		p->subscriber = nullptr;
	}
	return 0;
}

MIR_CORE_DLL(void) KillModuleEventHooks(HINSTANCE hInst)
{
	mir_cslock lck(csHooks);

	for (auto &it : hooks.rev_iter()) {
		if (it->subscriberCount == 0)
			continue;

		for (int j = it->subscriberCount - 1; j >= 0; j--) {
			if (it->subscriber[j].hOwner != hInst)
				continue;

			char szModuleName[MAX_PATH];
			GetModuleFileNameA(it->subscriber[j].hOwner, szModuleName, sizeof(szModuleName));
			UnhookEvent((HANDLE)((it->id << 16) + j + 1));
			if (it->subscriberCount == 0)
				break;
		}
	}
}

MIR_CORE_DLL(void) KillObjectEventHooks(void* pObject)
{
	mir_cslock lck(csHooks);

	for (auto &it : hooks.rev_iter()) {
		if (it->subscriberCount == 0)
			continue;

		for (int j = it->subscriberCount - 1; j >= 0; j--) {
			if (it->subscriber[j].object == pObject) {
				UnhookEvent((HANDLE)((it->id << 16) + j + 1));
				if (it->subscriberCount == 0)
					break;
			}
		}
	}
}

static void DestroyHooks()
{
	mir_cslock lck(csHooks);

	for (auto &it : hooks) {
		if (it->subscriberCount)
			mir_free(it->subscriber);
		delete it;
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
	if (name == nullptr)
		return nullptr;

	TService tmp;
	tmp.nameHash = mir_hashstr(name);

	mir_cslock lck(csServices);

	if (services.getIndex(&tmp) != -1)
		return nullptr;

	TService* p = (TService*)mir_alloc(sizeof(*p) + strlen(name));
	strcpy(p->name, name);
	p->nameHash = tmp.nameHash;
	p->pfnService = serviceProc;
	p->hOwner = GetInstByAddress(serviceProc);
	p->flags = type;
	p->lParam = lParam;
	p->object = object;
	services.insert(p);

	return (HANDLE)tmp.nameHash;
}

MIR_CORE_DLL(HANDLE) CreateServiceFunction(const char *name, MIRANDASERVICE serviceProc)
{
	return CreateServiceInt(0, name, serviceProc, nullptr, 0);
}

MIR_CORE_DLL(HANDLE) CreateServiceFunctionParam(const char *name, MIRANDASERVICEPARAM serviceProc, LPARAM lParam)
{
	return CreateServiceInt(1, name, (MIRANDASERVICE)serviceProc, nullptr, lParam);
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

	int idx = services.getIndex((TService*)&hService);
	if (idx != -1) {
		mir_free(services[idx]);
		services.remove(idx);
	}

	return 0;
}

MIR_CORE_DLL(bool) ServiceExists(const char *name)
{
	if (name == nullptr)
		return FALSE;

	mir_cslock lck(csServices);
	return FindServiceByName(name) != nullptr;
}

MIR_CORE_DLL(INT_PTR) CallService(const char *name, WPARAM wParam, LPARAM lParam)
{
	if (name == nullptr)
		return CALLSERVICE_NOTFOUND;

	TService *pService;
	{
		mir_cslock lck(csServices);
		if ((pService = FindServiceByName(name)) == nullptr)
			return CALLSERVICE_NOTFOUND;
	}

	MIRANDASERVICE pfnService = pService->pfnService;
	int flags = pService->flags;
	LPARAM fnParam = pService->lParam;
	void* object = pService->object;
	switch (flags) {
	case 1:  return ((MIRANDASERVICEPARAM)pfnService)(wParam, lParam, fnParam);
	case 2:  return ((MIRANDASERVICEOBJ)pfnService)(object, wParam, lParam);
	case 3:  return ((MIRANDASERVICEOBJPARAM)pfnService)(object, wParam, lParam, fnParam);
	default: return pfnService(wParam, lParam);
	}
}

static void CALLBACK CallServiceToMainAPCFunc(ULONG_PTR dwParam)
{
	TServiceToMainThreadItem *item = (TServiceToMainThreadItem*)dwParam;
	item->result = CallService(item->name, item->wParam, item->lParam);
	SetEvent(item->hDoneEvent);
}

MIR_CORE_DLL(INT_PTR) CallServiceSync(const char *name, WPARAM wParam, LPARAM lParam)
{
	if (name == nullptr)
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

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(int) CallFunctionAsync(void(__stdcall *func)(void *), void *arg)
{
	if (GetCurrentThreadId() == mainThreadId)
		func(arg);
	else
		QueueMainThread((PAPCFUNC)func, arg, nullptr);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct TSyncCallParam
{
	INT_PTR(__stdcall *func)(void *);
	void *arg;
	HANDLE hDoneEvent;
	INT_PTR result;
};

static void CALLBACK CallFuncToMainAPCFunc(ULONG_PTR dwParam)
{
	TSyncCallParam *item = (TSyncCallParam*)dwParam;
	item->result = (*item->func)(item->arg);
	SetEvent(item->hDoneEvent);
}

MIR_CORE_DLL(INT_PTR) CallFunctionSync(INT_PTR(__stdcall *func)(void *), void *arg)
{
	if (GetCurrentThreadId() == mainThreadId)
		return func(arg);

	TSyncCallParam param = { func, arg, getThreadEvent() };
	QueueMainThread(CallFuncToMainAPCFunc, &param, param.hDoneEvent);
	return param.result;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(void) KillModuleServices(HINSTANCE hInst)
{
	mir_cslock lck(csServices);

	for (auto &it : services.rev_iter()) {
		if (it->hOwner == hInst) {
			char szModuleName[MAX_PATH];
			GetModuleFileNameA(it->hOwner, szModuleName, sizeof(szModuleName));
			DestroyServiceFunction((HANDLE)it->nameHash);
		}
	}
}

MIR_CORE_DLL(void) KillObjectServices(void* pObject)
{
	mir_cslock lck(csServices);

	for (auto &it : services.rev_iter())
		if (it->object == pObject)
			DestroyServiceFunction((HANDLE)it->nameHash);
}

static void DestroyServices()
{
	mir_cslock lck(csServices);

	for (auto &it : services)
		mir_free(it);
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
