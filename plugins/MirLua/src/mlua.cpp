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
	L = luaL_newstate();
	luaL_openlibs(L);

	lua_getglobal(L, "package");
	lua_pushstring(L, "");
	lua_setfield(L, -2, "path");
	lua_pushstring(L, "");
	lua_setfield(L, -2, "cpath");
	lua_pop(L, 1);

	LoadMirandaModules();

	hScriptsLangpack = GetPluginLangId(MIID_LAST, 0);

	CLuaLoader loader(this);
	loader.LoadScripts();
}

void CMLua::Unload()
{
	if (L)
		lua_close(L);
	KillModuleMenus(hScriptsLangpack);
}

void CMLua::Reload()
{
	Unload();
	Load();
}

void CMLua::LoadModule(const char *name, lua_CFunction loader)
{
	luaL_getsubtable(L, LUA_REGISTRYINDEX, "_PRELOAD");
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
}

void CMLua::LoadCoreModule()
{
	luaL_newlib(L, coreLib);
	lua_pushlightuserdata(L, NULL);
	lua_setfield(L, -2, "NULL");
	lua_setglobal(L, "m");
}

void CMLua::LoadMirandaModules()
{
	LoadCoreModule();

	LoadModule(MLUA_DATABASE, luaopen_m_database);
	LoadModule(MLUA_ICOLIB, luaopen_m_icolib);
	LoadModule(MLUA_GENMENU, luaopen_m_genmenu);
	LoadModule(MLUA_MSGBUTTONSBAR, luaopen_m_msg_buttonsbar);
	LoadModule(MLUA_POPUP, luaopen_m_popup);
	LoadModule(MLUA_TOPTOOLBAR, luaopen_m_toptoolbar);
	LoadModule(MLUA_VARIABLES, luaopen_m_variables);
}

void CMLua::AddScriptsPath(const char *path)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");
	const char *oldPath = luaL_checkstring(L, -1);
	lua_pop(L, 1);
	lua_pushfstring(L, "%s;%s\\?.lua", oldPath, path);
	lua_setfield(L, -2, "path");
	lua_pop(L, 1);
}

void CMLua::LoadScript(const char *path)
{
	if (luaL_dofile(L, path))
		printf("%s\n", lua_tostring(L, -1));
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