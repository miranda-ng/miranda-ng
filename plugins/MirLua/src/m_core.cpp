#include "stdafx.h"

static int core_CreateHookableEvent(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	HANDLE res = CreateHookableEvent(name);
	if (res == NULL)
	{
		lua_pushnil(L);

		return 1;
	}
	lua_pushlightuserdata(L, res);

	return 1;
}

int HookEventObjParam(void *obj, WPARAM wParam, LPARAM lParam, LPARAM param)
{
	lua_State *L = lua_newthread(*(CMLua*)obj);

	int ref = param;
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

	if (wParam)
		lua_pushlightuserdata(L, (void*)wParam);
	else
		lua_pushnil(L);

	if (lParam)
		lua_pushlightuserdata(L, (void*)lParam);
	else
		lua_pushnil(L);

	luaM_pcall(L, 2, 1);

	return lua_tointeger(L, -1);
}

static int core_HookEvent(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_pushvalue(L, 2);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	HANDLE hEvent = HookEventObjParam(name, HookEventObjParam, g_mLua, ref);
	if (hEvent == NULL)
	{
		luaL_unref(L, LUA_REGISTRYINDEX, ref);
		lua_pushnil(L);

		return 1;
	}

	return 1;
}

static int core_UnhookEvent(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	HANDLE hEvent = lua_touserdata(L, 1);

	int res = UnhookEvent(hEvent);
	lua_pushboolean(L, !res);

	return 1;
}

static int core_NotifyEventHooks(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	HANDLE hEvent = lua_touserdata(L, 1);
	WPARAM wParam = (WPARAM)luaM_tomparam(L, 2);
	LPARAM lParam = (LPARAM)luaM_tomparam(L, 3);

	int res = NotifyEventHooks(hEvent, wParam, lParam);
	lua_pushboolean(L, res != -1);

	return 1;
}

static int core_DestroyHookableEvent(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	HANDLE hEvent = lua_touserdata(L, 1);

	int res = DestroyHookableEvent(hEvent);
	lua_pushboolean(L, !res);

	return 1;
}

/***********************************************/

INT_PTR CreateServiceFunctionObjParam(void *obj, WPARAM wParam, LPARAM lParam, LPARAM param)
{
	lua_State *L = lua_newthread(*(CMLua*)obj);

	int ref = param;
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

	lua_pushlightuserdata(L, (void*)wParam);
	lua_pushlightuserdata(L, (void*)lParam);
	luaM_pcall(L, 2, 1);

	INT_PTR res = lua_tointeger(L, 1);
	lua_pushinteger(L, res);

	return res;
}

static int core_CreateServiceFunction(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_pushvalue(L, 2);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	HANDLE hService = CreateServiceFunctionObjParam(name, CreateServiceFunctionObjParam, g_mLua, ref);
	if (!hService)
	{
		luaL_unref(L, LUA_REGISTRYINDEX, ref);
		lua_pushnil(L);
		return 1;
	}

	return 1;
}

static int core_CallService(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	WPARAM wParam = (WPARAM)luaM_tomparam(L, 2);
	LPARAM lParam = (LPARAM)luaM_tomparam(L, 3);

	INT_PTR res = CallService(name, wParam, lParam);
	lua_pushinteger(L, res);

	return 1;
}

static int core_ServiceExists(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	int res = ServiceExists(name);
	lua_pushboolean(L, res);

	return 1;
}

static int core_DestroyServiceFunction(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	HANDLE hService = lua_touserdata(L, 1);

	DestroyServiceFunction(hService);

	return 0;
}

/***********************************************/

static int core_IsPluginLoaded(lua_State *L)
{
	const char *value = lua_tostring(L, 1);

	MUUID uuid = { 0 };
	bool res = UuidFromStringA((RPC_CSTR)value, (UUID*)&uuid) == RPC_S_OK;
	if (res)
		res = IsPluginLoaded(uuid) > 0;
	lua_pushboolean(L, res);

	return 1;
}

static int core_Utf8DecodeA(lua_State *L)
{
	return luaM_toansi(L);
}

static int core_Utf8DecodeW(lua_State *L)
{
	return luaM_toucs2(L);
}

static int core_Free(lua_State *L)
{
	if (lua_islightuserdata(L, 1))
	{
		void *ptr = lua_touserdata(L, 1);
		mir_free(ptr);
	}

	return 0;
}

static int core_Translate(lua_State *L)
{
	char *what = (char*)luaL_checkstring(L, 1);

	ptrT value(mir_utf8decodeT(what));
	lua_pushstring(L, T2Utf(TranslateW_LP(value, hLangpack)));

	return 1;
}

static int core_Parse(lua_State *L)
{
	char *what = (char*)luaL_checkstring(L, 1);

	ptrT value(mir_utf8decodeT(what));
	lua_pushstring(L, T2Utf(VARST(value)));

	return 1;
}

static int core_GetFullPath(lua_State *L)
{
	TCHAR path[MAX_PATH];
	GetModuleFileName(NULL, path, MAX_PATH);
	
	lua_pushstring(L, ptrA(mir_utf8encodeT(path)));

	return 1;
}

/***********************************************/

struct core_ForkThreadParam
{
	lua_State *L;
	int threadRef;
	int functionRef;
	HANDLE hThread;
};

std::map<HANDLE, core_ForkThreadParam*> lstThreads;

void DestroyThread(core_ForkThreadParam *ftp)
{
	luaL_unref(ftp->L, LUA_REGISTRYINDEX, ftp->functionRef);
	luaL_unref(ftp->L, LUA_REGISTRYINDEX, ftp->threadRef);
	lstThreads.erase(ftp->hThread);

	delete ftp;
}

void __cdecl ThreadFunc(void *p)
{
	core_ForkThreadParam *ftp = (core_ForkThreadParam*)p;

	lua_rawgeti(ftp->L, LUA_REGISTRYINDEX, ftp->functionRef);
	luaM_pcall(ftp->L, 0, 1);
	DestroyThread(ftp);
}

static int core_ForkThread(lua_State *L)
{
	core_ForkThreadParam *p = new core_ForkThreadParam();

	p->L = lua_newthread(L);
	p->threadRef = luaL_ref(L, LUA_REGISTRYINDEX);
	lua_pushvalue(L, 1);
	p->functionRef = luaL_ref(L, LUA_REGISTRYINDEX);

	p->hThread = mir_forkthread(ThreadFunc, p);
	lstThreads[p->hThread] = p;
	lua_pushlightuserdata(L, p->hThread);

	return 1;
}

static int core_TerminateThread(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	HANDLE hThread = (HANDLE)lua_touserdata(L, 1);

	auto it = lstThreads.find(hThread);
	if (it != lstThreads.end())
	{
		DestroyThread(it->second);
		lua_pushboolean(L, TerminateThread(hThread, 0));
	}
	else lua_pushboolean(L, 0);

	return 1;
}

/***********************************************/

luaL_Reg coreApi[] =
{
	{ "CreateHookableEvent", core_CreateHookableEvent },
	// potentially unsefe for use in scripts
	//{ "DestroyHookableEvent", core_DestroyHookableEvent },

	{ "NotifyEventHooks", core_NotifyEventHooks },

	{ "HookEvent", core_HookEvent },
	{ "UnhookEvent", core_UnhookEvent },

	{ "CreateServiceFunction", core_CreateServiceFunction },
	{ "DestroyServiceFunction", core_DestroyServiceFunction },

	{ "ServiceExists", core_ServiceExists },
	{ "CallService", core_CallService },

	{ "IsPluginLoaded", core_IsPluginLoaded },

	{ "Utf8DecodeA", core_Utf8DecodeA },
	{ "Utf8DecodeW", core_Utf8DecodeW },
	
	{ "Free", core_Free },

	{ "Translate", core_Translate },

	{ "Parse", core_Parse },

	{ "GetFullPath", core_GetFullPath },

	{ "ForkThread", core_ForkThread },
	{ "TerminateThread", core_TerminateThread },

	{ "Version", NULL },

	{ "NULL", NULL },
	{ "INVALID_HANDLE_VALUE", NULL },
	{ "CALLSERVICE_NOTFOUND", NULL },

	{ NULL, NULL }
};

/***********************************************/

LUAMOD_API int luaopen_m_core(lua_State *L)
{
	luaL_newlib(L, coreApi);
	lua_pushlightuserdata(L, NULL);
	lua_setfield(L, -2, "NULL");
	lua_pushlightuserdata(L, INVALID_HANDLE_VALUE);
	lua_setfield(L, -2, "INVALID_HANDLE_VALUE");
	lua_pushinteger(L, CALLSERVICE_NOTFOUND);
	lua_setfield(L, -2, "CALLSERVICE_NOTFOUND");

	char version[128];
	CallService(MS_SYSTEM_GETVERSIONTEXT, (WPARAM)_countof(version), (LPARAM)version);
	lua_pushstring(L, ptrA(mir_utf8encode(version)));
	lua_setfield(L, -2, "Version");

	// set copy to global variable m
	lua_pushvalue(L, -1);
	lua_setglobal(L, "m");

	return 1;
}