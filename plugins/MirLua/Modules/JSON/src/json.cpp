#include  "stdafx.h"

static int lua_decode(lua_State *L)
{
	const char *string = luaL_checkstring(L, 1);
	new (L) MT(json_parse(string));
	luaL_setmetatable(L, MT_JSON);
	return 1;
}

int json__call(lua_State *L);
static int lua_encode(lua_State *L)
{
	return json__call(L);
}

static const luaL_Reg methods[] = 
{
	{ "decode", lua_decode },
	{ "encode", lua_encode },
	{ NULL, NULL }
};

LUA_LIBRARY_EXPORT(json)
{
	luaL_newlib(L, methods);

	luaL_newmetatable(L, MT_JSON);
	luaL_setfuncs(L, jsonApi, 0);
	lua_pop(L, 1);

	return 1;
}