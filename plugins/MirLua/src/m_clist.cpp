#include "stdafx.h"

static int clist_AddMainMenuRoot(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	int position = lua_tointeger(L, 2);
	HANDLE hIcon = (HANDLE)lua_touserdata(L, 3);

	HGENMENU res = Menu_CreateRoot(MO_MAIN, ptrW(Utf8DecodeW(name)), position, hIcon);
	if (res != nullptr)
		lua_pushlightuserdata(L, res);
	else
		lua_pushnil(L);

	return 1;
}

static int clist_AddMainMenuItem(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	CMenuItem mi;
	MakeMenuItem(L, mi);

	HGENMENU res = Menu_AddMainMenuItem(&mi);
	if (res != nullptr)
		lua_pushlightuserdata(L, res);
	else
		lua_pushnil(L);

	return 1;
}

static int clist_AddContactMenuRoot(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	int position = lua_tointeger(L, 2);
	HANDLE hIcon = (HANDLE)lua_touserdata(L, 3);

	HGENMENU res = Menu_CreateRoot(MO_MAIN, ptrW(Utf8DecodeW(name)), position, hIcon);
	if (res != nullptr)
		lua_pushlightuserdata(L, res);
	else
		lua_pushnil(L);

	return 1;
}

static int clist_AddContactMenuItem(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	CMenuItem mi;
	MakeMenuItem(L, mi);

	ptrA szProto(mir_utf8decodeA(lua_tostring(L, 2)));
	HGENMENU res = Menu_AddContactMenuItem(&mi, szProto);
	if (res != nullptr)
		lua_pushlightuserdata(L, res);
	else
		lua_pushnil(L);

	return 1;
}

static int clist_AddTrayMenuItem(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	CMenuItem mi;
	MakeMenuItem(L, mi);

	HGENMENU res = Menu_AddTrayMenuItem(&mi);
	if (res != nullptr)
		lua_pushlightuserdata(L, res);
	else
		lua_pushnil(L);

	return 1;
}

static luaL_Reg clistApi[] =
{
	{ "AddMainMenuRoot", clist_AddMainMenuRoot },
	{ "AddMainMenuItem", clist_AddMainMenuItem },

	{ "AddContactMenuRoot", clist_AddContactMenuRoot },
	{ "AddContactMenuItem", clist_AddContactMenuItem },

	{ "AddTrayMenuItem", clist_AddTrayMenuItem },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_clist(lua_State *L)
{
	luaL_newlib(L, clistApi);

	return 1;
}
