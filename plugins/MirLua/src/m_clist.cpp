#include "stdafx.h"

static int lua_AddMainMenuItem(lua_State *L)
{
	if (lua_type(L, 1) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}

	CMenuItem mi;
	MakeMenuItem(L, mi);

	HGENMENU res = ::Menu_AddMainMenuItem(&mi);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_BuildMainMenu(lua_State *L)
{
	HMENU res = ::Menu_GetMainMenu();
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
	
	CMenuItem mi;
	MakeMenuItem(L, mi);

	ptrA szProto(mir_utf8decode((char*)lua_tostring(L, 2), NULL));

	HGENMENU res = ::Menu_AddContactMenuItem(&mi, szProto);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_BuildContactMenu(lua_State *L)
{
	MCONTACT hContact = lua_tointeger(L, 1);

	HMENU res = ::Menu_BuildContactMenu(hContact);
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

	CMenuItem mi;
	MakeMenuItem(L, mi);

	HGENMENU res = ::Menu_AddTrayMenuItem(&mi);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_BuildTrayMenu(lua_State *L)
{
	HMENU res = Menu_BuildTrayMenu();
	lua_pushlightuserdata(L, res);

	return 1;
}

static luaL_Reg clistApi[] =
{
	{ "AddMainMenuItem", lua_AddMainMenuItem },
	{ "BuildMainMenu", lua_BuildMainMenu },

	{ "AddContactMenuItem", lua_AddContactMenuItem },
	{ "BuildContactMenu", lua_BuildContactMenu },

	{ "AddTrayMenuItem", lua_AddTrayMenuItem },
	{ "BuildTrayMenu", lua_BuildTrayMenu },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_clist(lua_State *L)
{
	luaL_newlib(L, clistApi);

	return 1;
}
