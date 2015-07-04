#include "stdafx.h"

CMLua::CMLua() : L(NULL)
{
	Load();
}

CMLua::~CMLua()
{
	Unload();
}

void CMLua::Load()
{
	CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)"Loading lua engine");
	L = luaL_newstate();
	CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)"Loading std modules");
	luaL_openlibs(L);

	lua_getglobal(L, "package");
	lua_pushstring(L, "");
	lua_setfield(L, -2, "path");
	lua_pushstring(L, "");
	lua_setfield(L, -2, "cpath");
	lua_pop(L, 1);

	lua_getglobal(L, "_G");
	lua_pushcclosure(L, luaM_print, 0);
	lua_setfield(L, -2, "print");
	lua_pop(L, 1);

	lua_atpanic(L, luaM_atpanic);

	MUUID muidLast = MIID_LAST;
	hScriptsLangpack = GetPluginLangId(muidLast, 0);

	CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)"Loading miranda modules");
	CLuaModuleLoader::Load(L);
	CLuaScriptLoader::Load(L);
}

void CMLua::Unload()
{
	CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)"Unloading lua engine");

	::KillModuleMenus(hScriptsLangpack);
	::KillModuleServices();
	::KillModuleEventHooks();
	//KillModuleSubclassing

	if (L)
		lua_close(L);
}

void CMLua::Reload()
{
	Unload();
	Load();
}

int CMLua::HookEventObjParam(void *obj, WPARAM wParam, LPARAM lParam, LPARAM param)
{
	lua_State *L = (lua_State*)obj;

	int ref = param;
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

	lua_pushnumber(L, wParam);
	lua_pushnumber(L, lParam);
	if (lua_pcall(L, 2, 1, 0))
		CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)lua_tostring(L, -1));

	int res = (int)lua_tointeger(L, 1);

	return res;
}