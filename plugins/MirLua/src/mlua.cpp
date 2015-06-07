#include "stdafx.h"

CMLua::CMLua()
{
	lua = luaL_newstate();

	luaL_openlibs(lua);
	luaL_newlib(lua, CMLua::CoreFunctions);
	lua_setglobal(lua, "M");
}

CMLua::~CMLua()
{
	lua_close(lua);
}

void CMLua::Load(const char *name)
{
	luaL_dofile(lua, name);
}