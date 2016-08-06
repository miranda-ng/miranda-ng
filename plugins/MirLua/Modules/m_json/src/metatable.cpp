#include "stdafx.h"

void lua2json(lua_State *L, JSONNode &node)
{
	switch (lua_type(L, -1))
	{
	case LUA_TNIL:
		node.nullify();
		break;
	case LUA_TSTRING:
		node = lua_tostring(L, -1);
		break;
	case LUA_TBOOLEAN:
		node = lua_toboolean(L, -1) != 0;
		break;
	case LUA_TNUMBER:
	{
		lua_Integer val = lua_tointeger(L, -1);
		if (lua_isinteger(L, -1) && val >= INT_MIN && val <= INT_MAX)
			node = (int)val;
		else
			node = lua_tonumber(L, -1);
		break;
	}
	case LUA_TTABLE:
	{
		ptrA name(mir_strdup(node.name()));
		node.cast(JSON_ARRAY);
		node.set_name((char*)name);

		lua_pushnil(L);
		while (lua_next(L, -2) != 0)
		{
			JSONNode child;
			if (!lua_isnumber(L, -2))
			{
				if (node.type() == JSON_ARRAY)
				{
					node.cast(JSON_NODE);
					node.set_name((char*)name);
				}

				const char *key = lua_tostring(L, -2);
				child.set_name(key);
			}
			lua2json(L, child);
			node << child;

			lua_pop(L, 1);
		}

		break;
	}
	}
}

static int json__index(lua_State *L)
{
	JSONNode &node = *((JSON*)luaL_checkudata(L, 1, MT_JSON))->node;

	switch (node.type())
	{
	case JSON_NULL:
		lua_pushnil(L);
		break;
	case JSON_STRING:
		lua_pushstring(L, node.as_string().c_str());
		break;
	case JSON_NUMBER:
		lua_pushnumber(L, node.as_int());
		break;
	case JSON_BOOL:
		lua_pushboolean(L, node.as_bool());
		break;
	case JSON_NODE:
	case JSON_ARRAY:
		new (L) JSONNode(node);
		luaL_setmetatable(L, MT_JSON);
	}

	return 1;
}

static int json__newindex(lua_State *L)
{
	JSONNode &node = *((JSON*)luaL_checkudata(L, 1, MT_JSON))->node;
	const char *key = lua_tostring(L, 2);

	JSONNode child = node[key];
	if (child.isnull())
	{
		child.set_name(key);
		lua2json(L, child);
		node << child;
	}
	else
	{
		lua2json(L, child);
		node[key] = child;
	}

	return 0;
}

static int json__len(lua_State *L)
{
	JSONNode &node = *((JSON*)luaL_checkudata(L, 1, MT_JSON))->node;

	lua_pushnumber(L, node.size());

	return 1;
}

static int json__tostring(lua_State *L)
{
	JSONNode &node = *((JSON*)luaL_checkudata(L, 1, MT_JSON))->node;

	lua_pushstring(L, node.write().c_str());

	return 1;
}

static int json__gc(lua_State *L)
{
	JSON *json = (JSON*)luaL_checkudata(L, 1, MT_JSON);

	delete json;

	return 0;
}

static const struct luaL_Reg jsonApi[] =
{
	{ "__index", json__index },
	{ "__newindex", json__newindex },
	{ "__len", json__len },
	{ "__tostring", json__tostring },
	{ "__gc", json__gc },

	{ NULL, NULL }
};
