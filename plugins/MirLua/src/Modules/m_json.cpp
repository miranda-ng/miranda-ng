#include "../stdafx.h"

#define MT_JSON "JSON"

static void lua2json(lua_State *L, JSONNode &node)
{
	switch (lua_type(L, -1)) {
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
		if (lua_isinteger(L, -1) && val >= LONG_MIN && val <= LONG_MAX)
			node = (long)val;
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
		while (lua_next(L, -2) != 0) {
			JSONNode child;
			if (!lua_isnumber(L, -2)) {
				if (node.type() == JSON_ARRAY) {
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
	case LUA_TLIGHTUSERDATA:
		if (lua_touserdata(L, -1))
			luaL_argerror(L, -1, luaL_typename(L, -1));
		node.nullify();
		break;
	case LUA_TUSERDATA:
	{
		ptrA name(mir_strdup(node.name()));
		JSONNode *other = *(JSONNode**)luaL_checkudata(L, -1, MT_JSON);
		node = other->duplicate();
		node.set_name((char*)name);
		break;
	}
	default:
		luaL_argerror(L, -1, luaL_typename(L, -1));
	}
}

/***********************************************/

static int json__index(lua_State *L)
{
	JSONNode *node = *(JSONNode**)luaL_checkudata(L, 1, MT_JSON);

	JSONNode *child;
	if (node->type() == JSON_ARRAY) {
		int idx = lua_tointeger(L, 2);
		child = &node->at(idx - 1);
	}
	else {
		const char *key = lua_tostring(L, 2);
		child = &node->at(key);
	}

	switch (child->type())
	{
	case JSON_NULL:
		lua_pushnil(L);
		break;
	case JSON_STRING:
		lua_pushstring(L, child->as_string().c_str());
		break;
	case JSON_NUMBER:
		lua_pushnumber(L, child->as_int());
		break;
	case JSON_BOOL:
		lua_pushboolean(L, child->as_bool());
		break;
	case JSON_ARRAY:
	case JSON_NODE:
		JSONNode **udata = (JSONNode**)lua_newuserdata(L, sizeof(JSONNode*));
		*udata = child;
		luaL_setmetatable(L, MT_JSON);
	}

	return 1;
}

static int json__newindex(lua_State *L)
{
	JSONNode *node = *(JSONNode**)luaL_checkudata(L, 1, MT_JSON);
	const char *key = lua_tostring(L, 2);

	if (json_type(node) == JSON_ARRAY) {
		int idx = lua_tointeger(L, 2);
		JSONNode *child = json_at(node, idx - 1);
		lua2json(L, *child);
		return 0;
	}

	JSONNode *child = json_get(node, key);
	if (json_type(child) == JSON_NULL) {
		json_set_name(child, key);
		lua2json(L, *child);
		json_push_back(node, child);
		return 0;
	}

	lua2json(L, *child);

	return 0;
}

static int json__len(lua_State *L)
{
	JSONNode *node = *(JSONNode**)luaL_checkudata(L, 1, MT_JSON);
	lua_pushnumber(L, json_size(node));

	return 1;
}

static int json__tostring(lua_State *L)
{
	JSONNode *node = *(JSONNode**)luaL_checkudata(L, 1, MT_JSON);

	lua_pushstring(L, node->write().c_str());

	return 1;
}

static int json__gc(lua_State *L)
{
	JSONNode *node = *(JSONNode**)luaL_checkudata(L, 1, MT_JSON);

	json_delete(node);

	return 0;
}

const struct luaL_Reg jsonApi[] =
{
	{ "__index", json__index },
	{ "__newindex", json__newindex },
	{ "__len", json__len },
	{ "__tostring", json__tostring },
	{ "__gc", json__gc },

	{ nullptr, nullptr }
};


/***********************************************/

static int lua_Decode(lua_State *L)
{
	const char *string = luaL_checkstring(L, 1);

	JSONNode **udata = (JSONNode**)lua_newuserdata(L, sizeof(JSONNode*));
	*udata = json_parse(string);
	luaL_setmetatable(L, MT_JSON);

	return 1;
}

/***********************************************/

static int lua_Encode(lua_State *L)
{
	//JSONNode node;
	//lua_pushnil(L);
	//lua_pushvalue(L, 1);
	//lua2json(L, node);
	//lua_pop(L, 2);
	//lua_pushstring(L, node.write().c_str());
	
	switch (lua_type(L, 1)) {
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
			JSONNode *node = *(JSONNode**)luaL_checkudata(L, 1, MT_JSON);
			lua_pushstring(L, node->write().c_str());
			break;
		}
		case LUA_TLIGHTUSERDATA:
			if (lua_touserdata(L, 1) == nullptr)
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

	{ "null", nullptr },

	{ nullptr, nullptr }
};

LUAMOD_API int luaopen_m_json(lua_State *L)
{
	luaL_newlib(L, methods);
	lua_pushlightuserdata(L, nullptr);
	lua_setfield(L, -2, "null");

	luaL_newmetatable(L, MT_JSON);
	luaL_setfuncs(L, jsonApi, 0);
	lua_pop(L, 1);

	return 1;
}