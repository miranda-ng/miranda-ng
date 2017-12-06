#ifndef _MIRLUA_H_
#define _MIRLUA_H_

#include <lua.hpp>

__forceinline UINT_PTR luaM_tomparam(lua_State *L, int idx)
{
	switch (lua_type(L, idx))
	{
	case LUA_TBOOLEAN:
		return lua_toboolean(L, idx);
	case LUA_TSTRING:
		return (UINT_PTR)lua_tostring(L, idx);
	case LUA_TLIGHTUSERDATA:
		return (UINT_PTR)lua_touserdata(L, idx);
	case LUA_TNUMBER:
	{
		lua_Integer res = 0;
		lua_Number num = lua_tonumber(L, idx);
		if (lua_numbertointeger(num, &res))
			return res;
	}
	}
	return NULL;
}

#endif //_MIRLUA_H_