#include "stdafx.h"

int luaM_print(lua_State *L)
{
	CMStringA data;
	int nargs = lua_gettop(L);
	for (int i = 1; i <= nargs; ++i)
		data.AppendFormat("%s   ", lua_tostring(L, i));
	data.Delete(data.GetLength() - 3, 3);

	CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)data.GetBuffer());

	return 0;
}

int luaM_atpanic(lua_State *L)
{
	CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)lua_tostring(L, -1));

	return 0;
}

bool luaM_toboolean(lua_State *L, int idx)
{
	if (lua_type(L, idx) == LUA_TNUMBER)
		return lua_tonumber(L, idx) != 0;
	return lua_toboolean(L, idx) > 0;
}

WPARAM luaM_towparam(lua_State *L, int idx)
{
	WPARAM wParam = NULL;
	switch (lua_type(L, idx))
	{
	case LUA_TBOOLEAN:
		wParam = lua_toboolean(L, idx);
		break;
	case LUA_TNUMBER:
		wParam = lua_tonumber(L, idx);
		break;
	case LUA_TUSERDATA:
	case LUA_TLIGHTUSERDATA:
		wParam = (WPARAM)lua_touserdata(L, idx);
		break;
	}
	return wParam;
}

LPARAM luaM_tolparam(lua_State *L, int idx)
{
	LPARAM lParam = NULL;
	switch (lua_type(L, idx))
	{
	case LUA_TBOOLEAN:
		lParam = lua_toboolean(L, idx);
		break;
	case LUA_TNUMBER:
		lParam = lua_tonumber(L, idx);
		break;
	case LUA_TUSERDATA:
	case LUA_TLIGHTUSERDATA:
		lParam = (LPARAM)lua_touserdata(L, idx);
		break;
	}
	return lParam;
}