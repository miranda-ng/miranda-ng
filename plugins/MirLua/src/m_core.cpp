#include "stdafx.h"

static int lua_CreateHookableEvent(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	HANDLE res = ::CreateHookableEvent(name);
	lua_pushlightuserdata(L, res);

	CMLua::Events.insert(res);

	return 1;
}

static int lua_DestroyHookableEvent(lua_State *L)
{
	HANDLE hEvent = (HANDLE)lua_touserdata(L, 1);

	CMLua::Events.remove(hEvent);

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

	CMLua::Hooks.insert(res);
	CMLua::HookRefs.insert(new HandleRefParam(L, res, ref));

	return 1;
}

static int lua_UnhookEvent(lua_State *L)
{
	HANDLE hEvent = (HANDLE)lua_touserdata(L, 1);

	CMLua::Hooks.remove(hEvent);

	HandleRefParam *param = (HandleRefParam*)CMLua::HookRefs.find(hEvent);
	if (param != NULL)
	{
		luaL_unref(param->L, LUA_REGISTRYINDEX, param->ref);
		CMLua::HookRefs.remove(param);
		delete param;
	}

	int res = ::UnhookEvent(hEvent);
	lua_pushinteger(L, res);

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
		CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)lua_tostring(L, -1));

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

	CMLua::Services.insert(res);
	CMLua::ServiceRefs.insert(new HandleRefParam(L, res, ref));

	return 1;
}

static int lua_DestroyServiceFunction(lua_State *L)
{
	HANDLE hService = (HANDLE)lua_touserdata(L, 1);

	CMLua::Services.remove(hService);

	HandleRefParam *param = (HandleRefParam*)CMLua::ServiceRefs.find(hService);
	if (param != NULL)
	{
		luaL_unref(param->L, LUA_REGISTRYINDEX, param->ref);
		CMLua::ServiceRefs.remove(param);
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

static int lua_Utf8DecodeA(lua_State *L)
{
	return luaM_toansi(L);
}

static int lua_Utf8DecodeW(lua_State *L)
{
	return luaM_toucs2(L);
}

static int lua_Free(lua_State *L)
{
	if (lua_type(L, 1) == LUA_TLIGHTUSERDATA)
	{
		void *ptr = lua_touserdata(L, 1);
		mir_free(ptr);
	}

	return 0;
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

	{ "CreateServiceFunction", lua_CreateServiceFunction },
	{ "DestroyServiceFunction", lua_DestroyServiceFunction },

	{ "ServiceExists", lua_ServiceExists },
	{ "CallService", lua_CallService },

	{ "Utf8DecodeA", lua_Utf8DecodeA },
	{ "Utf8DecodeW", lua_Utf8DecodeW },

	{ "Free", lua_Free },

	{ "Translate", lua_Translate },
	{ "ReplaceVariables", lua_ReplaceVariables },

	{ "OnScriptLoaded", CMLua::OnScriptLoaded },
	{ "OnScriptUnload", CMLua::OnScriptUnload },

	{ "NULL", NULL },
	{ "INVALID_HANDLE_VALUE", NULL },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m(lua_State *L)
{
	luaL_newlib(L, coreApi);
	lua_pushlightuserdata(L, NULL);
	lua_setfield(L, -2, "NULL");
	lua_pushlightuserdata(L, INVALID_HANDLE_VALUE);
	lua_setfield(L, -2, "INVALID_HANDLE_VALUE");
	lua_setglobal(L, "m");

	return 1;
}