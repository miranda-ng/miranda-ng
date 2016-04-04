#include "stdafx.h"

CLuaModuleLoader::CLuaModuleLoader(lua_State *L) : L(L)
{
}

void CLuaModuleLoader::Load(const char *name, lua_CFunction loader)
{
	luaL_getsubtable(L, LUA_REGISTRYINDEX, "_LOADED");
	lua_pushcfunction(L, loader);
	lua_pushstring(L, name);
	luaM_pcall(L, 1, 1);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
}

void CLuaModuleLoader::Preload(const char *name, lua_CFunction loader)
{
	luaL_getsubtable(L, LUA_REGISTRYINDEX, "_PRELOAD");
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
}

void CLuaModuleLoader::LoadModules()
{
	// load m_core module
	Load(MLUA_CORE, luaopen_m_core);
	// load all internal modules
	Preload(MLUA_CHAT, luaopen_m_chat);
	Preload(MLUA_CLIST, luaopen_m_clist);
	Preload(MLUA_DATABASE, luaopen_m_database);
	Preload(MLUA_ICOLIB, luaopen_m_icolib);
	Preload(MLUA_GENMENU, luaopen_m_genmenu);
	Preload(MLUA_HOTKEYS, luaopen_m_hotkeys);
	Preload(MLUA_MESSAGE, luaopen_m_message);
	Preload(MLUA_PROTOCOLS, luaopen_m_protocols);
	Preload(MLUA_SCHEDULE, luaopen_m_schedule);
	Preload(MLUA_SOUNDS, luaopen_m_sounds);
}

void CLuaModuleLoader::Load(lua_State *L)
{
	CLuaModuleLoader loader(L);
	loader.LoadModules();
}