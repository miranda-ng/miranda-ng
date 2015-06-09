#include "stdafx.h"

CMLua::CMLua()
{
	L = luaL_newstate();
	luaL_openlibs(L);

	luaL_newlib(L, CMLua::coreFunctions);
	lua_setglobal(L, "M");
}

CMLua::~CMLua()
{
	lua_close(L);
}

void CMLua::Load(const char *path)
{
	luaL_dofile(L, path);
}

void CMLua::Preload(const char *name, lua_CFunction loader)
{
	luaL_getsubtable(L, LUA_REGISTRYINDEX, "_PRELOAD");
	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
}