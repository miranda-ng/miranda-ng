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
	switch (lua_type(L, 1))
	{
	case LUA_TNIL:
		lua_pushliteral(L, "null");
		break;
	case LUA_TBOOLEAN:
		lua_pushstring(L, lua_toboolean(L, 1) ? "true" : "false");
		break;
	case LUA_TNUMBER:
	{
		if (lua_isinteger(L, 1)) {
			lua_pushfstring(L, "%I", lua_tointeger(L, 1));
			break;
		}
		char decpoint = lua_getlocaledecpoint();
		if (decpoint != '.') {
			char p[2] = { decpoint };
			luaL_gsub(L, lua_tostring(L, 1), p, ".");
		}
		else
			lua_pushfstring(L, "%f", lua_tonumber(L, 1));
		break;
	}
	case LUA_TSTRING:
		lua_pushfstring(L, "\"%s\"", lua_tostring(L, 1));
		break;
	case LUA_TTABLE:
	{
		JSONNode node;
		lua_pushnil(L);
		lua_pushvalue(L, 1);
		lua2json(L, node);
		lua_pop(L, 2);
		lua_pushstring(L, node.write().c_str());
		break;
	}
	case LUA_TUSERDATA:
	{
		JSONNode &node = *((JSON*)luaL_checkudata(L, 1, MT_JSON))->node;
		lua_pushstring(L, node.write().c_str());
		break;
	}
	case LUA_TLIGHTUSERDATA:
		if (lua_touserdata(L, 1) == NULL)
		{
			lua_pushliteral(L, "null");
			break;
		}
	default:
		luaL_argerror(L, 1, luaL_typename(L, 1));
	}

	return 1;
}

static const luaL_Reg methods[] =
{
	{ "Decode", lua_Decode },
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