#include "stdafx.h"

static int lua_AddContactMenuItem(lua_State *L)
{
	CLISTMENUITEM mi = { sizeof(CLISTMENUITEM) };
	mi.pszName = LPGEN((char*)luaL_checkstring(L, 1));
	mi.flags = lua_tointeger(L, 2);
	mi.position = lua_tointeger(L, 3);
	mi.icolibItem = (HANDLE)lua_touserdata(L, 4);
	mi.pszService = (char*)lua_tostring(L, 5);

	HGENMENU res = Menu_AddContactMenuItem(&mi);
	lua_pushlightuserdata(L, res);

	return 1;
}

static luaL_Reg clistLib[] =
{
	{ "AddContactMenuItem", lua_AddContactMenuItem },

	{ NULL, NULL }
};

int luaopen_m_clist(lua_State *L)
{
	//luaL_newlib(L, CMLua::clistLib);

	lua_getglobal(L, "M");
	luaL_checktype(L, -1, LUA_TTABLE);

	lua_newtable(L);
	luaL_setfuncs(L, clistLib, 0);
	lua_setfield(L, -2, "CList");

	return 1;
}
