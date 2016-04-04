#ifndef _M_LUA_H_
#define _M_LUA_H_

#include <lua.hpp>

static __inline WPARAM luaM_towparam(lua_State *L, int idx)
{
	switch (lua_type(L, idx))
	{
	case LUA_TBOOLEAN:
		return lua_toboolean(L, idx);
	case LUA_TNUMBER:
		return (WPARAM)lua_tonumber(L, idx);
	case LUA_TSTRING:
		return (WPARAM)lua_tostring(L, idx);
		break;
	case LUA_TUSERDATA:
	case LUA_TLIGHTUSERDATA:
		return (WPARAM)lua_touserdata(L, idx);
	default:
		return NULL;
	}
}

static __inline LPARAM luaM_tolparam(lua_State *L, int idx)
{
	switch (lua_type(L, idx))
	{
	case LUA_TBOOLEAN:
		return lua_toboolean(L, idx);
	case LUA_TNUMBER:
		return (LPARAM)lua_tonumber(L, idx);
	case LUA_TSTRING:
		return (LPARAM)lua_tostring(L, idx);
	case LUA_TUSERDATA:
	case LUA_TLIGHTUSERDATA:
		return (LPARAM)lua_touserdata(L, idx);
	default:
		return NULL;
	}
}

#endif //_M_LUA_H_
