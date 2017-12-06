#include "stdafx.h"

static int lua__call(lua_State *L)
{
	int enumeratorRef = *(int*)lua_touserdata(L, 1);

	int top = lua_gettop(L);
	lua_rawgeti(L, LUA_REGISTRYINDEX, enumeratorRef);
	lua_call(L, 0, LUA_MULTRET);
	int nres = lua_gettop(L) - top;

	return nres;
}

static int lua__pairs(lua_State *L)
{
	int enumeratorRef = *(int*)lua_touserdata(L, 1);

	lua_rawgeti(L, LUA_REGISTRYINDEX, enumeratorRef);

	return 1;
}

static int lua__gc(lua_State *L)
{
	int enumeratorRef = *(int*)lua_touserdata(L, 1);

	luaL_unref(L, LUA_REGISTRYINDEX, enumeratorRef);

	return 0;
}

static luaL_Reg enumerableMeta[] =
{
	{ "__call", lua__call },
	{ "__pairs", lua__pairs },
	{ "__gc", lua__gc },

	{ NULL, NULL }
};

/***********************************************/

static int lua_Any(lua_State *L)
{
	int enumeratorRef = *(int*)lua_touserdata(L, lua_upvalueindex(1));

	if (lua_type(L, 1) > LUA_TNONE)
		luaL_checktype(L, 1, LUA_TFUNCTION);

	int top = lua_gettop(L);
	while (true)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, enumeratorRef);
		lua_call(L, 0, LUA_MULTRET);
		int nres = lua_gettop(L) - top;
		if (lua_isnoneornil(L, top + 1))
		{
			lua_pop(L, nres);
			break;
		}

		if (lua_isfunction(L, 1))
		{
			lua_pushvalue(L, 1);
			lua_insert(L, top + 1);
			lua_call(L, nres, 1);
			nres = lua_gettop(L) - top;
			if (lua_toboolean(L, -1))
			{
				lua_pop(L, nres);
				lua_pushboolean(L, 1);
				return 1;
			}
			lua_pop(L, nres);
			continue;
		}

		lua_pop(L, nres);
		lua_pushboolean(L, 1);

		return 1;
	}

	lua_pushboolean(L, 0);

	return 1;
}

static int lua_First(lua_State *L)
{
	int enumeratorRef = *(int*)lua_touserdata(L, lua_upvalueindex(1));
	
	if (lua_type(L, 1) > LUA_TNONE)
		luaL_checktype(L, 1, LUA_TFUNCTION);

	int top = lua_gettop(L);
	while (true)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, enumeratorRef);
		lua_call(L, 0, LUA_MULTRET);
		int nres = lua_gettop(L) - top;
		if (lua_isnoneornil(L, top + 1))
		{
			lua_pop(L, nres);
			break;
		}

		if (lua_isfunction(L, 1))
		{
			lua_pushvalue(L, 1);
			for (int i = top; i < top + nres; i++)
				lua_pushvalue(L, i + 1);
			lua_call(L, nres, 1);
			if (lua_toboolean(L, -1))
			{
				lua_pop(L, 1);
				return nres;
			}
			lua_pop(L, nres + 1);
			continue;
		}

		return nres;
	}

	lua_pushnil(L);

	return 1;
}

static int lua_Last(lua_State *L)
{
	int enumeratorRef = *(int*)lua_touserdata(L, lua_upvalueindex(1));

	if (lua_type(L, 1) > LUA_TNONE)
		luaL_checktype(L, 1, LUA_TFUNCTION);

	int top = lua_gettop(L);
	lua_pushnil(L);
	int nres = lua_gettop(L) - top;
	while (true)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, enumeratorRef);
		lua_call(L, 0, LUA_MULTRET);
		int newres = lua_gettop(L) - top - nres;
		if (lua_isnoneornil(L, top + nres + 1))
		{
			lua_pop(L, newres);
			break;
		}

		if (lua_isfunction(L, 1))
		{
			lua_pushvalue(L, 1);
			for (int i = top + nres; i < top + nres + newres; i++)
				lua_pushvalue(L, i + 1);
			lua_call(L, newres, 1);
			if (!lua_toboolean(L, -1))
			{
				lua_pop(L, newres + 1);
				continue;
			}
			lua_pop(L, 1);
		}

		for (int i = 0; i < nres; i++)
			lua_remove(L, top + 1);
		nres = newres;
	}

	return nres;
}

static int lua_Count(lua_State *L)
{
	int enumeratorRef = *(int*)lua_touserdata(L, lua_upvalueindex(1));

	int count = 0;
	while (true)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, enumeratorRef);
		lua_call(L, 0, 1);
		if (lua_isnoneornil(L, -1))
		{
			lua_pop(L, 1);
			break;
		}

		lua_pop(L, 1);
		count++;
	}

	lua_pushinteger(L, count);

	return 1;
}

static int lua_ToArray(lua_State *L)
{
	int enumeratorRef = *(int*)lua_touserdata(L, lua_upvalueindex(1));

	lua_newtable(L);
	for (int i = 1;; i++)
	{
		int top = lua_gettop(L);
		lua_rawgeti(L, LUA_REGISTRYINDEX, enumeratorRef);
		lua_call(L, 0, LUA_MULTRET);
		int nres = lua_gettop(L) - top;
		if (lua_isnoneornil(L, top + 1))
		{
			lua_pop(L, nres);
			break;
		}
		lua_rawseti(L, -nres - 1, i);
		lua_pop(L, nres - 1);
	}

	return 1;
}

static int lua_ToTable(lua_State *L)
{
	int enumeratorRef = *(int*)lua_touserdata(L, lua_upvalueindex(1));
	luaL_checktype(L, 1, LUA_TFUNCTION);

	if (lua_type(L, 2) > LUA_TNONE)
		luaL_checktype(L, 2, LUA_TFUNCTION);

	lua_newtable(L);
	int top = lua_gettop(L);
	while (true)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, enumeratorRef);
		lua_call(L, 0, LUA_MULTRET);
		int nres = lua_gettop(L) - top;
		if (lua_isnoneornil(L, top + 1))
		{
			lua_pop(L, nres);
			break;
		}

		lua_pushvalue(L, 1);
		for (int i = top; i < top + nres; i++)
			lua_pushvalue(L, i + 1);
		lua_call(L, nres, 1);

		if (lua_isfunction(L, 2))
		{
			lua_pushvalue(L, 2);
			for (int i = top; i < top + nres; i++)
				lua_pushvalue(L, i + 1);
			lua_call(L, nres, 1);
		}
		else
			lua_pushvalue(L, top + 1);

		lua_rawset(L, top);
		lua_pop(L, nres);
	}

	return 1;
}

static int lua_Where(lua_State *L);
static int lua_Select(lua_State *L);

static luaL_Reg enumerableApi[] =
{
	{ "Where", lua_Where },
	{ "Select", lua_Select },

	{ "Any", lua_Any },
	{ "First", lua_First },
	{ "Last", lua_Last },
	{ "Count", lua_Count },

	{ "ToArray", lua_ToArray },
	{ "ToTable", lua_ToTable },

	{ NULL, NULL }
};

/***********************************************/

static void CreateMetatable(lua_State *L)
{
	lua_newtable(L);
	lua_pushvalue(L, -1);
	lua_pushvalue(L, -3);
	luaL_setfuncs(L, enumerableApi, 1);
	lua_setfield(L, -2, "__index");
	luaL_setfuncs(L, enumerableMeta, 0);
}

static int lua_WhereIterator(lua_State *L)
{
	int enumeratorRef = lua_tointeger(L, lua_upvalueindex(1));
	int whereRef = lua_tointeger(L, lua_upvalueindex(2));

	int top = lua_gettop(L);
	while (true)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, enumeratorRef);
		lua_call(L, 0, LUA_MULTRET);
		int nres = lua_gettop(L) - top;
		if (lua_isnoneornil(L, top + 1))
		{
			lua_pop(L, nres);
			luaL_unref(L, LUA_REGISTRYINDEX, whereRef);
			break;
		}

		lua_rawgeti(L, LUA_REGISTRYINDEX, whereRef);
		for (int i = top; i < top + nres; i++)
			lua_pushvalue(L, i + 1);
		lua_call(L, nres, 1);
		if (lua_toboolean(L, -1))
		{
			lua_pop(L, 1);
			return nres;
		}
		lua_pop(L, nres + 1);
	}

	lua_pushnil(L);

	return 1;
}

static int lua_Where(lua_State *L)
{
	int enumeratorRef = *(int*)lua_touserdata(L, lua_upvalueindex(1));
	luaL_checktype(L, 1, LUA_TFUNCTION);

	lua_pushinteger(L, enumeratorRef);
	lua_pushvalue(L, 1);
	lua_pushinteger(L, luaL_ref(L, LUA_REGISTRYINDEX));
	lua_pushcclosure(L, lua_WhereIterator, 2);
	enumeratorRef = luaL_ref(L, LUA_REGISTRYINDEX);
	int *udata = (int*)lua_newuserdata(L, sizeof(int));
	*udata = enumeratorRef;
	CreateMetatable(L);
	lua_setmetatable(L, -2);

	return 1;
}

static int lua_SelectIterator(lua_State *L)
{
	int enumeratorRef = lua_tointeger(L, lua_upvalueindex(1));
	int selectRef = lua_tointeger(L, lua_upvalueindex(2));

	int top = lua_gettop(L);
	lua_rawgeti(L, LUA_REGISTRYINDEX, enumeratorRef);
	lua_call(L, 0, LUA_MULTRET);
	int nres = lua_gettop(L) - top;
	if (lua_isnoneornil(L, top + 1))
	{
		lua_pop(L, nres);
		luaL_unref(L, LUA_REGISTRYINDEX, selectRef);
		return 0;
	}

	lua_rawgeti(L, LUA_REGISTRYINDEX, selectRef);
	lua_insert(L, top + 1);
	lua_call(L, nres, LUA_MULTRET);
	nres = lua_gettop(L) - top;

	return nres;
}

static int lua_Select(lua_State *L)
{
	int enumeratorRef = *(int*)lua_touserdata(L, lua_upvalueindex(1));
	luaL_checktype(L, 1, LUA_TFUNCTION);

	lua_pushinteger(L, enumeratorRef);
	lua_pushvalue(L, 1);
	lua_pushinteger(L, luaL_ref(L, LUA_REGISTRYINDEX));
	lua_pushcclosure(L, lua_SelectIterator, 2);
	enumeratorRef = luaL_ref(L, LUA_REGISTRYINDEX);
	int *udata = (int*)lua_newuserdata(L, sizeof(int));
	*udata = enumeratorRef;
	CreateMetatable(L);
	lua_setmetatable(L, -2);

	return 1;
}

/***********************************************/

static int lua_tableIterator(lua_State *L)
{
	lua_pushvalue(L, lua_upvalueindex(1));
	if (lua_next(L, lua_upvalueindex(2)))
	{
		lua_pushvalue(L, -2);
		lua_replace(L, lua_upvalueindex(1));
		return 2;
	}

	lua_pushnil(L);

	return 1;
}

static int lua__new(lua_State *L)
{
	switch (lua_type(L, 1))
	{
	case LUA_TTABLE:
		lua_pushnil(L);
		lua_pushvalue(L, 1);
		lua_pushcclosure(L, lua_tableIterator, 2);
		break;
	case LUA_TFUNCTION:
		lua_pushvalue(L, -1);
		break;
	default:
		luaL_argerror(L, 1, luaL_typename(L, 1));
	}

	int enumeratorRef = luaL_ref(L, LUA_REGISTRYINDEX);
	int *udata = (int*)lua_newuserdata(L, sizeof(int));
	*udata = enumeratorRef;
	CreateMetatable(L);
	lua_setmetatable(L, -2);

	return 1;
}

/***********************************************/

extern "C" LUAMOD_API int luaopen_m_enumerable(lua_State *L)
{
	lua_createtable(L, 0, 1);
	lua_pushcfunction(L, lua__new);
	lua_setfield(L, -2, "new");

	return 1;
}
