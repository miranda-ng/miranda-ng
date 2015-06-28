#include "stdafx.h"

CMLua::CMLua() : L(NULL)
{
	console = new CMLuaConsole(L);
	hLogger = mir_createLog(MODULE, _T("MirLua log"), VARST(_T("%miranda_logpath%\\MirLua.txt")), 0);

	Load();
}

CMLua::~CMLua()
{
	Unload();
	mir_closeLog(hLogger);
	delete console;
}

void CMLua::Load()
{
	mir_writeLogT(hLogger, _T("Loading lua engine\n"));
	L = luaL_newstate();
	mir_writeLogT(hLogger, _T("Loading std modules\n"));
	luaL_openlibs(L);

	lua_getglobal(L, "package");
	lua_pushstring(L, "");
	lua_setfield(L, -2, "path");
	lua_pushstring(L, "");
	lua_setfield(L, -2, "cpath");
	lua_pop(L, 1);

	MUUID muidLast = MIID_LAST;
	hScriptsLangpack = GetPluginLangId(muidLast, 0);

	mir_writeLogT(hLogger, _T("Loading miranda modules\n"));
	CLuaModuleLoader::Load(L);
	CLuaScriptLoader::Load(L, hLogger);
}

void CMLua::Unload()
{
	mir_writeLogT(hLogger, _T("Unloading lua engine\n"));

	KillModuleMenus(hScriptsLangpack);
	KillModuleServices();
	KillModuleEventHooks();
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
		printf("%s\n", lua_tostring(L, -1));

	int res = (int)lua_tointeger(L, 1);

	//luaL_unref(L, LUA_REGISTRYINDEX, ref);

	return res;
}