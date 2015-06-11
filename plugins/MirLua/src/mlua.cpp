#include "stdafx.h"

CMLua::CMLua()
{
	L = luaL_newstate();
	luaL_openlibs(L);

	lua_getglobal(L, "package");
	lua_pushstring(L, "");
	lua_setfield(L, -2, "path");
	lua_pushstring(L, "");
	lua_setfield(L, -2, "cpath");
	lua_pop(L, 1);

	luaL_newlib(L, coreLib);
	lua_setglobal(L, "m");

	Preload(MLUA_DATABASE, luaopen_m_database);
	Preload(MLUA_ICOLIB, luaopen_m_icolib);
	Preload(MLUA_GENMENU, luaopen_m_genmenu);
}

CMLua::~CMLua()
{
	lua_close(L);
}

void CMLua::AddPath(const char *path)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");
	const char *oldPath = luaL_checkstring(L, -1);
	lua_pop(L, 1);
	lua_pushfstring(L, "%s;%s\\?.lua", oldPath, path);
	lua_setfield(L, -2, "path");
	lua_pop(L, 1);
}

void CMLua::Load(const char *path)
{
	if (luaL_dofile(L, path))
		printf("%s\n", lua_tostring(L, -1));
}

void CMLua::Preload(const char *name, lua_CFunction loader)
{
	luaL_getsubtable(L, LUA_REGISTRYINDEX, "_PRELOAD");
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
}