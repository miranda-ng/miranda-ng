#include "stdafx.h"

CLuaModuleLoader::CLuaModuleLoader(lua_State *L) : L(L)
{
}

void CLuaModuleLoader::PreloadModule(const char *name, lua_CFunction loader)
{
	luaL_getsubtable(L, LUA_REGISTRYINDEX, "_PRELOAD");
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
}

void CLuaModuleLoader::LoadModules()
{
	// load core module
	luaopen_m(L);
	// regirter delay loading of miranda modules
	PreloadModule(MLUA_CLIST, luaopen_m_clist);
	PreloadModule(MLUA_DATABASE, luaopen_m_database);
	PreloadModule(MLUA_ICOLIB, luaopen_m_icolib);
	PreloadModule(MLUA_GENMENU, luaopen_m_genmenu);
	PreloadModule(MLUA_HOTKEYS, luaopen_m_hotkeys);
	PreloadModule(MLUA_MESSAGE, luaopen_m_message);
	PreloadModule(MLUA_MSGBUTTONSBAR, luaopen_m_msg_buttonsbar);
	PreloadModule(MLUA_POPUP, luaopen_m_popup);
	PreloadModule(MLUA_PROTOCOLS, luaopen_m_protocols);
	PreloadModule(MLUA_TOPTOOLBAR, luaopen_m_toptoolbar);
	PreloadModule(MLUA_VARIABLES, luaopen_m_variables);
	PreloadModule(MLUA_WINDOWS, luaopen_m_windows);
}

void CLuaModuleLoader::Load(lua_State *L)
{
	CLuaModuleLoader loader(L);
	loader.LoadModules();
}