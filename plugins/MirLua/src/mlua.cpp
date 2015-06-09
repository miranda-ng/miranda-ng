#include "stdafx.h"

CMLua::CMLua()
{
	L = luaL_newstate();
	luaL_openlibs(L);

	luaopen_m(L);

	Preload(LUA_CLISTLIBNAME, luaopen_m_clist);
}

CMLua::~CMLua()
{
	lua_close(L);
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