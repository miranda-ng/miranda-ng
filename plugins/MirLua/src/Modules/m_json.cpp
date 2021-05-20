#include "../stdafx.h"

#define MT_JSON "JSON"

static void json2lua(lua_State *L, const JSONNode &node)
{
	switch (node.type()) {
	case JSON_NULL:
		lua_pushnil(L);
		break;
	case JSON_STRING:
		lua_pushstring(L, node.as_string().c_str());
		break;
	case JSON_NUMBER:
	{
		lua_getglobal(L, "tonumber");
		lua_pushstring(L, node.as_string().c_str());
		luaM_pcall(L, 1, 1);
		break;
	}
	case JSON_BOOL:
		lua_pushboolean(L, node.as_bool());
		break;
	case JSON_ARRAY:
	case JSON_NODE:
		JSONNode * *udata = (JSONNode**)lua_newuserdata(L, sizeof(JSONNode*));
		*udata = json_copy(&node);
		luaL_setmetatable(L, MT_JSON);
	}
}

static void lua2json(lua_State *L, int idx, JSONNode &node)
{
	switch (lua_type(L, idx)) {
	case LUA_TNIL:
		node.nullify();
		break;
	case LUA_TSTRING:
		node = lua_tostring(L, idx);
		break;
	case LUA_TBOOLEAN:
		node = lua_toboolean(L, idx) != 0;
		break;
	case LUA_TNUMBER:
	{
		if (!lua_isinteger(L, idx)) {
			node = lua_tonumber(L, idx);
			break;
		}
		lua_Integer val = lua_tointeger(L, idx);
		node = val >= LONG_MIN && val <= LONG_MAX
			? (long)val
			: lua_tonumber(L, idx);
		break;
	}
	case LUA_TTABLE:
	{
		ptrA name(mir_strdup(node.name()));
		if (luaM_isarray(L, idx)) {
			node.cast(JSON_ARRAY);
			node.set_name((char*)name);
		}

		for (lua_pushnil(L); lua_next(L, idx); lua_pop(L, 1)) {
			JSONNode child;
			if (node.type() != JSON_ARRAY) {
				const char *key = lua_tostring(L, -2);
				child.set_name(key);
			}
			lua2json(L, lua_absindex(L, -1), child);
			node << child;
		}

		break;
	}
	case LUA_TLIGHTUSERDATA:
		if (lua_touserdata(L, idx))
			luaL_argerror(L, idx, luaL_typename(L, idx));
		node.nullify();
		break;
	case LUA_TUSERDATA:
	{
		ptrA name(mir_strdup(node.name()));
		JSONNode *other = *(JSONNode**)luaL_checkudata(L, idx, MT_JSON);
		node = other->duplicate();
		node.set_name((char*)name);
		break;
	}
	default:
		luaL_argerror(L, idx, luaL_typename(L, idx));
	}
}

/***********************************************/

static int json__index(lua_State *L)
{
	JSONNode *node = *(JSONNode**)luaL_checkudata(L, 1, MT_JSON);

	JSONNode *child;
	if (node->type() == JSON_ARRAY || lua_isinteger(L, 2)) {
		int idx = lua_tointeger(L, 2);
		child = &node->at(idx - 1);
	}
	else {
		const char *key = lua_tostring(L, 2);
		child = &node->at(key);
	}

	json2lua(L, *child);

	return 1;
}

static int json__newindex(lua_State *L)
{
	JSONNode *node = *(JSONNode**)luaL_checkudata(L, 1, MT_JSON);

	if (json_type(node) == JSON_ARRAY || lua_isinteger(L, 2)) {
		int idx = lua_tointeger(L, 2);
		JSONNode *child = json_at(node, idx - 1);
		lua2json(L, 3, *child);
		return 0;
	}

	const char *key = lua_tostring(L, 2);
	JSONNode *child = json_get(node, key);
	if (json_type(child) == JSON_NULL) {
		json_set_name(child, key);
		lua2json(L, 3, *child);
		json_push_back(node, child);
		return 0;
	}

	lua2json(L, 3, *child);

	return 0;
}

static int json__pairsIterator(lua_State *L)
{
	JSONNode *node = *(JSONNode**)luaL_checkudata(L, 1, MT_JSON);
	int i = lua_tointeger(L, lua_upvalueindex(1));

	if (i < node->size()) {
		JSONNode *child = json_at(node, i);
		if (json_type(node) == JSON_ARRAY)
			lua_pushinteger(L, i + 1);
		else
			lua_pushstring(L, child->name());

		json2lua(L, *child);

		lua_pushinteger(L, (i + 1));
		lua_replace(L, lua_upvalueindex(1));
		return 2;
	}

	lua_pushnil(L);
	return 1;
}

static int json__pairs(lua_State *L)
{
	luaL_checkudata(L, 1, MT_JSON);

	lua_pushinteger(L, 0);
	lua_pushcclosure(L, json__pairsIterator, 1);
	lua_pushvalue(L, 1);

	return 2;
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
	{ "__pairs", json__pairs },
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

static int lua_Encode(lua_State *L)
{
	JSONNode node;
	lua2json(L, 1, node);
	lua_pushstring(L, node.write().c_str());
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