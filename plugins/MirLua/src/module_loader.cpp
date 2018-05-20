#include "stdafx.h"

CMLuaModuleLoader::CMLuaModuleLoader(lua_State *L) : L(L)
{
}

void CMLuaModuleLoader::Load(const char *name, lua_CFunction loader)
{
	luaL_requiref(L, name, loader, 0);
}

void CMLuaModuleLoader::Preload(const char *name, lua_CFunction loader)
{
	luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
}

void CMLuaModuleLoader::LoadModules()
{
	Log("Loading miranda modules");

	// load m_core module
	Load(MLUA_CORE, luaopen_m_core);
	// load all internal modules
	Preload(MLUA_CHAT, luaopen_m_chat);
	Preload(MLUA_CLIST, luaopen_m_clist);
	Preload(MLUA_DATABASE, luaopen_m_database);
	Preload(MLUA_GENMENU, luaopen_m_genmenu);
	Preload(MLUA_HOTKEYS, luaopen_m_hotkeys);
	Preload(MLUA_HTTP, luaopen_m_http);
	Preload(MLUA_ICOLIB, luaopen_m_icolib);
	Preload(MLUA_JSON, luaopen_m_json);
	Preload(MLUA_MESSAGE, luaopen_m_message);
	Preload(MLUA_OPTIONS, luaopen_m_options);
	Preload(MLUA_PROTOCOLS, luaopen_m_protocols);
	Preload(MLUA_SOUNDS, luaopen_m_sounds);
	Preload(MLUA_SRMM, luaopen_m_srmm);
}

void CMLuaModuleLoader::Load(lua_State *L)
{
	CMLuaModuleLoader loader(L);
	loader.LoadModules();
}