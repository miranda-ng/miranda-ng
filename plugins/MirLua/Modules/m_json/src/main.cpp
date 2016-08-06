#include  "stdafx.h"

static int lua_Decode(lua_State *L)
{
	const char *string = luaL_checkstring(L, 1);

	JSONNode *node = json_parse(string);
	new (L) JSON(node);
	luaL_setmetatable(L, MT_JSON);

	return 1;
}

static int lua_Encode(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	JSONNode *node = json_new(JSON_NODE);
	lua_pushnil(L);
	lua_pushvalue(L, 1);
	lua2json(L, *node);
	lua_pop(L, 2);
	JSON *mt = new (L) JSON(node);
	luaL_setmetatable(L, MT_JSON);

	return 1;
}

static const luaL_Reg methods[] =
{
	{ "Dcode", lua_Decode },
	{ "Encode", lua_Encode },

	{ NULL, NULL }
};

extern "C" LUAMOD_API int luaopen_m_json(lua_State *L)
{
	luaL_newlib(L, methods);

	luaL_newmetatable(L, MT_JSON);
	luaL_setfuncs(L, jsonApi, 0);
	lua_pop(L, 1);

	return 1;
}