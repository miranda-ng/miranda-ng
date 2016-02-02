#include  "stdafx.h"

static int lua_decode(lua_State *L)
{
	const char *string = luaL_checkstring(L, 1);
	MT *udata = (MT*)lua_newuserdata(L, sizeof(MT));
	udata->node = json_parse(string);
	udata->bDelete = true;
	luaL_setmetatable(L, MT_JSON);
	return 1;
}

static const luaL_Reg methods[] = 
{
	{ "decode", lua_decode },
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