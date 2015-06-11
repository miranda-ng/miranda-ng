#include "stdafx.h"

static CLISTMENUITEM* MakeMenuItem(lua_State *L)
{
	CLISTMENUITEM *pmi = (CLISTMENUITEM*)mir_calloc(sizeof(CLISTMENUITEM));
	pmi->cbSize = sizeof(CLISTMENUITEM);

	lua_getfield(L, 1, "Name");
	pmi->pszName = LPGEN((char*)luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, 1, "Flags");
	pmi->flags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, 1, "Position");
	pmi->position = lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, 1, "Icon");
	pmi->icolibItem = (HANDLE)lua_touserdata(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, 1, "Service");
	pmi->pszService = (char*)lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, 1, "Parent");
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

	lua_settop(L, 1);

	mir_ptr<CLISTMENUITEM> pmi(MakeMenuItem(L));

	lua_pop(L, 1);

	HGENMENU res = ::Menu_AddMainMenuItem(pmi);
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

	lua_settop(L, 1);

	mir_ptr<CLISTMENUITEM> pmi(MakeMenuItem(L));

	lua_pop(L, 1);

	HGENMENU res = ::Menu_AddContactMenuItem(pmi);
	lua_pushlightuserdata(L, res);

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
	{ "RemoveMenuItem", lua_RemoveMenuItem },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_genmenu(lua_State *L)
{
	luaL_newlib(L, genmenuApi);

	return 1;
}
