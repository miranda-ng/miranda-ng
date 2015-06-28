#include "stdafx.h"

static LIST<void> Hooks(1, PtrKeySortT);
static LIST<void> Events(1, PtrKeySortT);
static LIST<void> Services(1, PtrKeySortT);

struct HandleRefParam
{
	HANDLE h;
	int ref;
	lua_State *L;
	HandleRefParam(HANDLE h) : L(NULL), h(h), ref(0) { }
	HandleRefParam(lua_State *L, HANDLE h, int ref = 0) : L(L), h(h), ref(ref) { }
};
static LIST<void> HookRefs(1, HandleKeySortT);
static LIST<void> ServiceRefs(1, HandleKeySortT);

void CMLua::KillModuleEventHooks()
{
	while (Hooks.getCount())
	{
		HANDLE hHook = Hooks[0];
		Hooks.remove(0);
		UnhookEvent(hHook);
	}

	while (Events.getCount())
	{
		HANDLE hEvent = Events[0];
		Events.remove(hEvent);
		DestroyHookableEvent(hEvent);
	}

	while (HookRefs.getCount())
	{
		HandleRefParam *param = (HandleRefParam*)HookRefs[0];
		if (param != NULL)
		{
			luaL_unref(param->L, LUA_REGISTRYINDEX, param->ref);
			HookRefs.remove(0);
			delete param;
		}
	}
}

void CMLua::KillModuleServices()
{
	while (Services.getCount())
	{
		HANDLE hService = Services[0];
		Services.remove(0);
		DestroyServiceFunction(hService);
	}

	while (ServiceRefs.getCount())
	{
		HandleRefParam *param = (HandleRefParam*)ServiceRefs[0];
		if (param != NULL)
		{
			luaL_unref(param->L, LUA_REGISTRYINDEX, param->ref);
			ServiceRefs.remove(0);
			delete param;
		}
	}
}

static int lua_CreateHookableEvent(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	HANDLE res = ::CreateHookableEvent(name);
	lua_pushlightuserdata(L, res);

	Events.insert(res);

	return 1;
}

static int lua_DestroyHookableEvent(lua_State *L)
{
	HANDLE hEvent = (HANDLE)lua_touserdata(L, 1);

	Events.remove(hEvent);

	int res = ::DestroyHookableEvent(hEvent);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_NotifyEventHooks(lua_State *L)
{
	HANDLE hEvent = (HANDLE)lua_touserdata(L, 1);
	WPARAM wParam = luaM_towparam(L, 2);
	LPARAM lParam = luaM_tolparam(L, 3);

	int res = ::NotifyEventHooks(hEvent, wParam, lParam);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_HookEvent(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	if (!lua_isfunction(L, 2))
	{
		lua_pushlightuserdata(L, NULL);
		return 1;
	}

	lua_pushvalue(L, 2);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	HANDLE res = ::HookEventObjParam(name, CMLua::HookEventObjParam, L, ref);
	lua_pushlightuserdata(L, res);

	Hooks.insert(res);
	HookRefs.insert(new HandleRefParam(L, res, ref));

	return 1;
}

static int lua_UnhookEvent(lua_State *L)
{
	HANDLE hEvent = (HANDLE)lua_touserdata(L, 1);

	Hooks.remove(hEvent);

	HandleRefParam *param = (HandleRefParam*)HookRefs.find(hEvent);
	if (param != NULL)
	{
		luaL_unref(param->L, LUA_REGISTRYINDEX, param->ref);
		HookRefs.remove(param);
		delete param;
	}

	int res = ::UnhookEvent(hEvent);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_OnModulesLoaded(lua_State *L)
{
	if (!lua_isfunction(L, 1))
	{
		lua_pushlightuserdata(L, NULL);
		return 1;
	}
	
	lua_pushvalue(L, 1);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);
	
	HANDLE res = ::HookEventObjParam(ME_SYSTEM_MODULESLOADED, CMLua::HookEventObjParam, L, ref);
	lua_pushlightuserdata(L, res);

	Hooks.insert(res);
	HookRefs.insert(new HandleRefParam(L, res, ref));

	return 1;
}

static int lua_OnPreShutdown(lua_State *L)
{
	if (!lua_isfunction(L, 1))
	{
		lua_pushlightuserdata(L, NULL);
		return 1;
	}

	lua_pushvalue(L, 1);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	HANDLE res = ::HookEventObjParam(ME_SYSTEM_PRESHUTDOWN, CMLua::HookEventObjParam, L, ref);
	lua_pushlightuserdata(L, res);

	Hooks.insert(res);
	HookRefs.insert(new HandleRefParam(L, res, ref));

	return 1;
}

static INT_PTR ServiceFunctionObjParam(void *obj, WPARAM wParam, LPARAM lParam, LPARAM param)
{
	lua_State *L = (lua_State*)obj;

	int ref = param;
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

	lua_pushnumber(L, wParam);
	lua_pushnumber(L, lParam);
	if (lua_pcall(L, 2, 1, 0))
		printf("%s\n", lua_tostring(L, -1));

	INT_PTR res = (INT_PTR)lua_tointeger(L, 1);
	lua_pushinteger(L, res);

	return res;
}

static int lua_CreateServiceFunction(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	if (!lua_isfunction(L, 2))
	{
		lua_pushlightuserdata(L, NULL);
		return 1;
	}

	lua_pushvalue(L, 2);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	HANDLE res = ::CreateServiceFunctionObjParam(name, ServiceFunctionObjParam, L, ref);
	lua_pushlightuserdata(L, res);

	Services.insert(res);
	ServiceRefs.insert(new HandleRefParam(L, res, ref));

	return 1;
}

static int lua_DestroyServiceFunction(lua_State *L)
{
	HANDLE hService = (HANDLE)lua_touserdata(L, 1);

	Services.remove(hService);

	HandleRefParam *param = (HandleRefParam*)ServiceRefs.find(hService);
	if (param != NULL)
	{
		luaL_unref(param->L, LUA_REGISTRYINDEX, param->ref);
		ServiceRefs.remove(param);
		delete param;
	}

	int res = ::DestroyServiceFunction(hService);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_ServiceExists(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	int res = ::ServiceExists(name);
	lua_pushboolean(L, res);

	return 1;
}

static int lua_CallService(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	WPARAM wParam = luaM_towparam(L, 2);
	LPARAM lParam = luaM_tolparam(L, 3);

	INT_PTR res = ::CallService(name, wParam, lParam);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_Translate(lua_State *L)
{
	char *what = (char*)luaL_checkstring(L, 1);

	ptrT value(mir_utf8decodeT(what));
	lua_pushstring(L, T2Utf(TranslateW_LP(value, hScriptsLangpack)));

	return 1;
}

static int lua_ReplaceVariables(lua_State *L)
{
	char *what = (char*)luaL_checkstring(L, 1);

	ptrT value(mir_utf8decodeT(what));
	lua_pushstring(L, T2Utf(VARST(value)));

	return 1;
}

luaL_Reg coreApi[] =
{
	{ "CreateHookableEvent", lua_CreateHookableEvent },
	{ "DestroyHookableEvent", lua_DestroyHookableEvent },

	{ "NotifyEventHooks", lua_NotifyEventHooks },

	{ "HookEvent", lua_HookEvent },
	{ "UnhookEvent", lua_UnhookEvent },
	{ "OnModulesLoaded", lua_OnModulesLoaded },
	{ "OnPreShutdown", lua_OnPreShutdown },

	{ "CreateServiceFunction", lua_CreateServiceFunction },
	{ "DestroyServiceFunction", lua_DestroyServiceFunction },

	{ "ServiceExists", lua_ServiceExists },
	{ "CallService", lua_CallService },

	{ "Translate", lua_Translate },
	{ "ReplaceVariables", lua_ReplaceVariables },

	{ "NULL", NULL },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m(lua_State *L)
{
	luaL_newlib(L, coreApi);
	lua_pushlightuserdata(L, NULL);
	lua_setfield(L, -2, "NULL");
	lua_setglobal(L, "m");

	return 1;
}