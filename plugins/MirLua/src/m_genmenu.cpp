#include "stdafx.h"

static TMO_MenuItem* MakeMenuItem(lua_State *L)
{
	TMO_MenuItem *pmi = (TMO_MenuItem*)mir_calloc(sizeof(TMO_MenuItem));
	pmi->hLangpack = hScriptsLangpack;

	lua_pushstring(L, "Flags");
	lua_gettable(L, -2);
	pmi->flags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	if (!(pmi->flags & CMIF_UNICODE))
		pmi->flags |= CMIF_UNICODE;

	lua_pushstring(L, "Name");
	lua_gettable(L, -2);
	pmi->name.t = mir_utf8decodeT((char*)luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_pushstring(L, "Position");
	lua_gettable(L, -2);
	pmi->position = lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_pushstring(L, "Icon");
	lua_gettable(L, -2);
	pmi->hIcolibItem = (HANDLE)lua_touserdata(L, -1);
	lua_pop(L, 1);

	lua_pushstring(L, "Service");
	lua_gettable(L, -2);
	pmi->pszService = (char*)lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_pushstring(L, "Parent");
	lua_gettable(L, -2);
	pmi->root = (HGENMENU)lua_touserdata(L, -1);
	lua_pop(L, 1);

	return pmi;
}

static int lua_AddMainMenuItem(lua_State *L)
{
	if (lua_type(L, 1) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}

	mir_ptr<TMO_MenuItem> pmi(MakeMenuItem(L));

	HGENMENU res = Menu_AddMainMenuItem(pmi);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_AddContactMenuItem(lua_State *L)
{
	if (lua_type(L, 1) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}

	mir_ptr<TMO_MenuItem> pmi(MakeMenuItem(L));

	HGENMENU res = Menu_AddContactMenuItem(pmi);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_AddTrayMenuItem(lua_State *L)
{
	if (lua_type(L, 1) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}

	mir_ptr<TMO_MenuItem> pmi(MakeMenuItem(L));

	HGENMENU res = (HGENMENU)::CallService("CList/AddTrayMenuItem", 0, (LPARAM)pmi);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_ModifyMenuItem(lua_State *L)
{
	HGENMENU hMenuItem = (HGENMENU)lua_touserdata(L, 1);

	if (lua_type(L, 2) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}

	mir_ptr<TMO_MenuItem> pmi(MakeMenuItem(L));

	INT_PTR res = ::Menu_ModifyItem(hMenuItem, pmi->name.t, pmi->hIcolibItem, pmi->flags);
	lua_pushinteger(L, res);
	return 1;
}

static int lua_RemoveMenuItem(lua_State *L)
{
	HGENMENU hMenuItem = (HGENMENU)lua_touserdata(L, 1);

	INT_PTR res = ::Menu_RemoveItem(hMenuItem);
	lua_pushinteger(L, res);

	return 1;
}

static luaL_Reg genmenuApi[] =
{
	{ "AddMainMenuItem", lua_AddMainMenuItem },
	{ "AddContactMenuItem", lua_AddContactMenuItem },
	{ "AddTrayMenuItem", lua_AddTrayMenuItem },
	{ "ModifyMenuItem", lua_ModifyMenuItem },
	{ "RemoveMenuItem", lua_RemoveMenuItem },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_genmenu(lua_State *L)
{
	luaL_newlib(L, genmenuApi);

	return 1;
}
