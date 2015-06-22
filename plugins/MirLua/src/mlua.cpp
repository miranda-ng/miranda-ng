#include "stdafx.h"

CMLua::CMLua() : L(NULL)
{
	console = new CMLuaConsole(L);

	Load();
}

CMLua::~CMLua()
{
	Unload();

	delete console;
}

void CMLua::Load()
{
	L = luaL_newstate();
	luaL_openlibs(L);

	lua_getglobal(L, "package");
	lua_pushstring(L, "");
	lua_setfield(L, -2, "path");
	lua_pushstring(L, "");
	lua_setfield(L, -2, "cpath");
	lua_pop(L, 1);

	MUUID muidLast = MIID_LAST;
	hScriptsLangpack = GetPluginLangId(muidLast, 0);

	CLuaModuleLoader::Load(L);
	CLuaScriptLoader::Load(L);
}

void CMLua::Unload()
{
	if (L)
		lua_close(L);
	KillModuleMenus(hScriptsLangpack);
	//KillModuleSubclassing
	//KillModuleServices
	//KillModuleEventHooks
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
		printf("%s\n", lua_tostring(L, -1));

	int res = (int)lua_tointeger(L, 1);

	//luaL_unref(L, LUA_REGISTRYINDEX, ref);

	return res;
}