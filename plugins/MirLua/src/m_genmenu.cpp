#include "stdafx.h"

static CLISTMENUITEM* MakeMenuItem(lua_State *L)
{
	CLISTMENUITEM *pmi = (CLISTMENUITEM*)mir_calloc(sizeof(CLISTMENUITEM));
	pmi->cbSize = sizeof(CLISTMENUITEM);
	pmi->hLangpack = hScriptsLangpack;

	lua_pushstring(L, "Flags");
	lua_gettable(L, -2);
	pmi->flags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_pushstring(L, "Name");
	lua_gettable(L, -2);
	if (!(pmi->flags & CMIF_UNICODE))
		pmi->pszName = mir_utf8decode((char*)lua_tostring(L, -1), NULL);
	else
		pmi->ptszName = mir_utf8decodeT((char*)luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_pushstring(L, "Position");
	lua_gettable(L, -2);
	pmi->position = lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_pushstring(L, "Icon");
	lua_gettable(L, -2);
	pmi->icolibItem = (HANDLE)lua_touserdata(L, -1);
	lua_pop(L, 1);

	lua_pushstring(L, "Service");
	lua_gettable(L, -2);
	pmi->pszService = (char*)lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_pushstring(L, "Parent");
	lua_gettable(L, -2);
	pmi->hParentMenu = (HGENMENU)lua_touserdata(L, -1);
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

	mir_ptr<CLISTMENUITEM> pmi(MakeMenuItem(L));

	HGENMENU res = (HGENMENU)::CallService("CList/AddMainMenuItem", 0, (LPARAM)pmi);
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

	mir_ptr<CLISTMENUITEM> pmi(MakeMenuItem(L));

	HGENMENU res = (HGENMENU)::CallService("CList/AddContactMenuItem", 0, (LPARAM)pmi);
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

	mir_ptr<CLISTMENUITEM> pmi(MakeMenuItem(L));

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

	mir_ptr<CLISTMENUITEM> pmi(MakeMenuItem(L));

	INT_PTR res = ::CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItem, (LPARAM)pmi);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_RemoveMenuItem(lua_State *L)
{
	HGENMENU hMenuItem = (HGENMENU)lua_touserdata(L, 1);

	INT_PTR res = ::CallService(MO_REMOVEMENUITEM, (WPARAM)hMenuItem, 0);
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
