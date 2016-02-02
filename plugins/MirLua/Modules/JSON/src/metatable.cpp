#include "stdafx.h"

void table2json(lua_State *L, int idx, JSONNode &node)
{
	lua_pushnil(L);
	while (lua_next(L, idx) != 0) 
	{
		JSONNode &nnode = (lua_type(L, -2) == LUA_TNUMBER ? (node)[(size_t)lua_tonumber(L, -2) - 1] : (node)[lua_tostring(L, -2)]);

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
			nnode = JSONNode(nnode.name(), NULL);
			table2json(L, -1, nnode);
		}
		
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
		MT *udata = (MT*)lua_newuserdata(L, sizeof(MT));
		udata->node = &node;
		udata->bDelete = false;
		luaL_setmetatable(L, MT_JSON);
	}
	return 1;
}

int json_setvalue(lua_State *L, JSONNode &node)
{
	switch (lua_type(L, 3))
	{
	case LUA_TSTRING:
		node = lua_tostring(L, 3);
		break;
	case LUA_TBOOLEAN:
		node = lua_toboolean(L, 3) != 0;
		break;
	case LUA_TNUMBER:
		node = lua_tonumber(L, 3);
		break;
	case LUA_TNIL:
		node.nullify();
		break;
	//case LUA_TTABLE:
		//node = JSONNode(node.name(), JSONNode());
		//table2json(L, 3, node);
	}

	return 0;
}

static int json__call(lua_State *L)
{
	if (lua_istable(L, 1))
	{
/*		MT *udata = (MT*)lua_newuserdata(L, sizeof(MT));
		udata->node = json_new(JSON_NODE);
		table2json(L, 1, *udata->node);
		udata->bDelete = true;
		luaL_setmetatable(L, MT_JSON);
		return 1;
*/
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
	JSONNode *node = ((MT*)luaL_checkudata(L, 1, MT_JSON))->node;
	switch (lua_type(L, 2))
	{
	case LUA_TNUMBER:
		json_setvalue(L, (*node)[(size_t)lua_tonumber(L, 2) - 1]);
		break;
	case LUA_TSTRING:
		json_setvalue(L, (*node)[lua_tostring(L, 2)]);
		break;
	}
	return 0;
}

static int json__len(lua_State *L)
{
	JSONNode *node = ((MT*)luaL_checkudata(L, 1, MT_JSON))->node;
	lua_pushnumber(L, (*node).size());
	return 1;
}

static int json__tostring(lua_State *L)
{
	JSONNode *node = ((MT*)luaL_checkudata(L, 1, MT_JSON))->node;
	lua_pushstring(L, (*node).write().c_str());
	return 1;
}

static int json__gc(lua_State *L)
{
	MT *mt = (MT*)luaL_checkudata(L, 1, MT_JSON);
	if (mt->bDelete)
	{
		json_delete(mt->node);
		mt->node = nullptr;
	}
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
