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
		if (lua_isinteger(L, idx))
			return (UINT_PTR)lua_tointeger(L, idx);
	}
	return NULL;
}

#endif //_MIRLUA_H_