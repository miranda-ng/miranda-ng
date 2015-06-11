#include "stdafx.h"

static int lua_AddMainMenuItem(lua_State *L)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszName = LPGEN((char*)luaL_checkstring(L, 1));
	mi.flags = lua_tointeger(L, 2);
	mi.position = lua_tointeger(L, 3);
	mi.icolibItem = (HANDLE)lua_touserdata(L, 4);
	mi.pszService = (char*)lua_tostring(L, 5);
	mi.hParentMenu = (HGENMENU)lua_touserdata(L, 6);

	HGENMENU res = ::Menu_AddMainMenuItem(&mi);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_AddContactMenuItem(lua_State *L)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszName = LPGEN((char*)luaL_checkstring(L, 1));
	mi.flags = lua_tointeger(L, 2);
	mi.position = lua_tointeger(L, 3);
	mi.icolibItem = (HANDLE)lua_touserdata(L, 4);
	mi.pszService = (char*)lua_tostring(L, 5);
	mi.hParentMenu = (HGENMENU)lua_touserdata(L, 6);

	HGENMENU res = ::Menu_AddContactMenuItem(&mi);
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
