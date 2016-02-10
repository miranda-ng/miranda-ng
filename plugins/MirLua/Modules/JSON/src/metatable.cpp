#include "stdafx.h"

void table2json(lua_State *L, int idx, JSONNode &node)
{
	idx = lua_absindex(L, idx);
	lua_pushnil(L);
	while (lua_next(L, idx) != 0) 
	{
		JSONNode nnode;
		if (lua_type(L, -2) == LUA_TSTRING) nnode.set_name(lua_tostring(L, -2));

		switch (lua_type(L, -1))
		{
		case LUA_TSTRING:
			nnode = lua_tostring(L, -1);
			break;
		case LUA_TBOOLEAN:
			nnode = lua_toboolean(L, -1) != 0;
			break;
		case LUA_TNUMBER:
			nnode = lua_tonumber(L, -1);
			break;
		case LUA_TNIL:
			nnode.nullify();
			break;
		case LUA_TTABLE:
			table2json(L, -1, nnode);
		}
		node << nnode;
		lua_pop(L, 1);
	}
}

int json_pushvalue(lua_State *L, JSONNode &node)
{
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
	case JSON_ARRAY:
	case JSON_NODE:
		new (L) MT(node);
		luaL_setmetatable(L, MT_JSON);
	}
	return 1;
}

int json__call(lua_State *L)
{
	if (lua_istable(L, 1))
	{
		MT *mt = new (L) MT(json_new(JSON_NODE));
		table2json(L, 1, *mt->node);
		luaL_setmetatable(L, MT_JSON);
		return 1;
	}
	return 0;
}

static int json__index(lua_State *L)
{
	JSONNode *node = ((MT*)luaL_checkudata(L, 1, MT_JSON))->node;
	switch (lua_type(L, 2))
	{
	case LUA_TNUMBER:
		return json_pushvalue(L, (*node)[(size_t)lua_tonumber(L, 2) - 1]);
	case LUA_TSTRING:
		return json_pushvalue(L, (*node)[lua_tostring(L, 2)]);
	}
	return 0;
}

static int json__newindex(lua_State *L)
{
	JSONNode &node = *((MT*)luaL_checkudata(L, 1, MT_JSON))->node;

	JSONNode &jNode = lua_type(L, 2) == LUA_TNUMBER ? node[(size_t)lua_tonumber(L, 2) - 1] : node[lua_tostring(L, 2)];

	switch (lua_type(L, 3))
	{
	case LUA_TSTRING:
		jNode = lua_tostring(L, 3);
		break;
	case LUA_TBOOLEAN:
		jNode = lua_toboolean(L, 3) != 0;
		break;
	case LUA_TNUMBER:
		jNode = lua_tonumber(L, 3);
		break;
	case LUA_TNIL:
		jNode.nullify();
		break;
	case LUA_TTABLE:
		JSONNode tmpNode(JSON_NODE);
		tmpNode.set_name(lua_tostring(L, 2));
		table2json(L, 3, tmpNode);
		node[lua_tostring(L, 2)] = tmpNode;
	}

	return 0;
}

static int json__len(lua_State *L)
{
	lua_pushnumber(L, ((MT*)luaL_checkudata(L, 1, MT_JSON))->node->size());
	return 1;
}

static int json__tostring(lua_State *L)
{
	lua_pushstring(L, ((MT*)luaL_checkudata(L, 1, MT_JSON))->node->write().c_str());
	return 1;
}

static int json__gc(lua_State *L)
{
	((MT*)luaL_checkudata(L, 1, MT_JSON))->~MT();
	return 0;
}

static const struct luaL_Reg jsonApi[] =
{
	{ "__call", json__call },
	{ "__index", json__index },
	{ "__newindex", json__newindex },
	{ "__len", json__len },
	{ "__tostring", json__tostring },
	{ "__gc", json__gc },
	{ NULL, NULL }
};
