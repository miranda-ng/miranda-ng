#include "stdafx.h"

static int clist_AddMainMenuItem(lua_State *L)
{
	if (lua_type(L, 1) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}

	CMenuItem mi;
	MakeMenuItem(L, mi);

	HGENMENU res = Menu_AddMainMenuItem(&mi);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int clist_CreateMainMenuRoot(lua_State *L)
{
	int hMenuObject = luaL_checkinteger(L, 1);
	const char *name = luaL_checkstring(L, 2);
	int position = lua_tointeger(L, 3);
	HANDLE hIcon = (HANDLE)lua_touserdata(L, 4);

	HGENMENU res = Menu_CreateRoot(MO_MAIN, ptrT(Utf8DecodeT(name)), position, hIcon);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int clist_AddContactMenuItem(lua_State *L)
{
	if (lua_type(L, 1) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}
	
	CMenuItem mi;
	MakeMenuItem(L, mi);

	ptrA szProto(mir_utf8decode((char*)lua_tostring(L, 2), NULL));

	HGENMENU res = Menu_AddContactMenuItem(&mi, szProto);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int clist_CreateContactMenuRoot(lua_State *L)
{
	int hMenuObject = luaL_checkinteger(L, 1);
	const char *name = luaL_checkstring(L, 2);
	int position = lua_tointeger(L, 3);
	HANDLE hIcon = (HANDLE)lua_touserdata(L, 4);

	HGENMENU res = Menu_CreateRoot(MO_CONTACT, ptrT(Utf8DecodeT(name)), position, hIcon);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int clist_AddTrayMenuItem(lua_State *L)
{
	if (lua_type(L, 1) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}

	CMenuItem mi;
	MakeMenuItem(L, mi);

	HGENMENU res = Menu_AddTrayMenuItem(&mi);
	lua_pushlightuserdata(L, res);

	return 1;
}

static luaL_Reg clistApi[] =
{
	{ "AddMainMenuItem", clist_AddMainMenuItem },
	{ "CreateMainMenuRoot", clist_CreateMainMenuRoot },
	{ "AddContactMenuItem", clist_AddContactMenuItem },
	{ "CreateContactMenuRoot", clist_CreateContactMenuRoot },
	{ "AddTrayMenuItem", clist_AddTrayMenuItem },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_clist(lua_State *L)
{
	luaL_newlib(L, clistApi);

	return 1;
}
