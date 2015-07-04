#include "stdafx.h"

static int lua_ShellExecute(lua_State *L)
{
	ptrT command(mir_utf8decodeT(lua_tostring(L, 1)));
	ptrT file(mir_utf8decodeT(lua_tostring(L, 2)));
	ptrT args(mir_utf8decodeT(lua_tostring(L, 3)));
	int flags = lua_tointeger(L, 4);

	::ShellExecute(NULL, command, file, args, NULL, flags);

	return 0;
}

static luaL_Reg winApi[] =
{
	{ "ShellExecute", lua_ShellExecute },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_windows(lua_State *L)
{
	luaL_newlib(L, winApi);

	return 1;
}
