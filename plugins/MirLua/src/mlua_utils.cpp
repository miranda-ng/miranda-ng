#include "stdafx.h"

int luaM_print(lua_State *L)
{
	CMStringA data;
	int nargs = lua_gettop(L);
	for (int i = 1; i <= nargs; ++i)
	{
		switch (lua_type(L, i))
		{
		case LUA_TNIL:
			data.AppendFormat("%s   ", "nil");
			break;
		case LUA_TBOOLEAN:
			data.AppendFormat("%s   ", lua_toboolean(L, i) ? "true" : "false");
			break;
		case LUA_TNUMBER:
		case LUA_TSTRING:
			data.AppendFormat("%s   ", lua_tostring(L, i));
			break;
		case LUA_TTABLE:
			data.AppendFormat("%s   ", "table");
			break;
		default:
			data.AppendFormat("0x%p   ", lua_topointer(L, i));
			break;
		}
	}
	data.Delete(data.GetLength() - 3, 3);

	CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)data.GetBuffer());

	return 0;
}

int luaM_atpanic(lua_State *L)
{
	CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)lua_tostring(L, -1));

	return 0;
}

int luaM_toansi(lua_State *L)
{
	const char* value = luaL_checkstring(L, 1);
	int codepage = luaL_optinteger(L, 2, Langpack_GetDefaultCodePage());

	ptrA string(mir_strdup(value));
	lua_pushstring(L, mir_utf8decodecp(string, codepage, NULL));

	return 1;
}

int luaM_toucs2(lua_State *L)
{
	const char* value = luaL_checkstring(L, 1);

	ptrW unicode(mir_utf8decodeW(value));
	size_t length = mir_wstrlen(unicode) * sizeof(wchar_t);

	ptrA string((char*)mir_calloc(length + 1));
	memcpy(string, unicode, length);

	lua_pushlstring(L, string, length + 1);

	return 1;
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
	case LUA_TSTRING:
		wParam = (WPARAM)lua_tostring(L, idx);
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
	case LUA_TSTRING:
		lParam = (LPARAM)lua_tostring(L, idx);
		break;
	case LUA_TUSERDATA:
	case LUA_TLIGHTUSERDATA:
		lParam = (LPARAM)lua_touserdata(L, idx);
		break;
	}
	return lParam;
}