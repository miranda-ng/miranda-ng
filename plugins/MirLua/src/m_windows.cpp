#include "stdafx.h"

static int lua_MessageBox(lua_State *L)
{
	HWND hwnd = (HWND)lua_touserdata(L, 1);
	ptrT text(mir_utf8decodeT(lua_tostring(L, 2)));
	ptrT caption(mir_utf8decodeT(lua_tostring(L, 3)));
	UINT flags = lua_tointeger(L, 4);

	int res = ::MessageBox(hwnd, text, caption, flags);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_ShellExecute(lua_State *L)
{
	ptrT command(mir_utf8decodeT(lua_tostring(L, 1)));
	ptrT file(mir_utf8decodeT(lua_tostring(L, 2)));
	ptrT args(mir_utf8decodeT(lua_tostring(L, 3)));
	int flags = lua_tointeger(L, 4);

	::ShellExecute(NULL, command, file, args, NULL, flags);

	return 0;
}

static int lua_GetIniValue(lua_State *L)
{
	ptrT path(mir_utf8decodeT(luaL_checkstring(L, 1)));
	ptrT section(mir_utf8decodeT(luaL_checkstring(L, 2)));
	ptrT key(mir_utf8decodeT(luaL_checkstring(L, 3)));

	if (lua_isinteger(L, 4))
	{
		int default = lua_tointeger(L, 4);

		UINT res = ::GetPrivateProfileInt(section, key, default, path);
		lua_pushinteger(L, res);

		return 1;
	}

	ptrT default(mir_utf8decodeT(lua_tostring(L, 4)));

	TCHAR value[MAX_PATH] = { 0 };
	if (!::GetPrivateProfileString(section, key, default, value, _countof(value), path))
	{
		lua_pushvalue(L, 4);
	}

	ptrA res(mir_utf8encodeT(value));
	lua_pushstring(L, res);

	return 1;
}

static int lua_SetIniValue(lua_State *L)
{
	ptrT path(mir_utf8decodeT(luaL_checkstring(L, 1)));
	ptrT section(mir_utf8decodeT(luaL_checkstring(L, 2)));
	ptrT key(mir_utf8decodeT(luaL_checkstring(L, 3)));
	ptrT value(mir_utf8decodeT(lua_tostring(L, 4)));

	bool res = ::WritePrivateProfileString(section, key, value, path) != 0;
	lua_pushboolean(L, res);

	return 1;
}

static luaL_Reg winApi[] =
{
	{ "MessageBox", lua_MessageBox },

	{ "GetIniValue", lua_GetIniValue },
	{ "SetIniValue", lua_SetIniValue },

	{ "ShellExecute", lua_ShellExecute },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_windows(lua_State *L)
{
	luaL_newlib(L, winApi);

	return 1;
}
