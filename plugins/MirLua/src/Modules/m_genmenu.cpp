#include "../stdafx.h"

static int genmenu_ModifyMenuItem(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	HGENMENU hMenuItem = (HGENMENU)lua_touserdata(L, 1);
	ptrW name(mir_utf8decodeW(lua_tostring(L, 2)));
	HANDLE hIcolibItem = luaL_opt(L, lua_touserdata, 3, INVALID_HANDLE_VALUE);
	int flags = luaL_optinteger(L, 4, -1);

	if (!(flags & CMIF_UNICODE))
		flags |= CMIF_UNICODE;

	INT_PTR res = Menu_ModifyItem(hMenuItem, name, hIcolibItem, flags);
	lua_pushboolean(L, res == 0);

	return 1;
}

static int genmenu_ConfigureMenuItem(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	HGENMENU hMenuItem = (HGENMENU)lua_touserdata(L, 1);
	int option = luaL_checkinteger(L, 2);
	luaL_checktype(L, 3, LUA_TLIGHTUSERDATA);
	INT_PTR value = (INT_PTR)lua_touserdata(L, 3);
	
	int res = Menu_ConfigureItem(hMenuItem, option, value);
	lua_pushboolean(L, res >= 0);

	return 1;
}

static int genmenu_ShowMenuItem(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	HGENMENU hMenuItem = (HGENMENU)lua_touserdata(L, 1);
	bool isShow = luaM_toboolean(L, 2);

	Menu_ShowItem(hMenuItem, isShow);

	return 0;
}

static int genmenu_EnableMenuItem(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	HGENMENU hMenuItem = (HGENMENU)lua_touserdata(L, 1);
	bool isEnable = luaM_toboolean(L, 2);

	Menu_EnableItem(hMenuItem, isEnable);

	return 0;
}

static int genmenu_CheckMenuItem(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	HGENMENU hMenuItem = (HGENMENU)lua_touserdata(L, 1);
	bool isChecked = luaM_toboolean(L, 2);

	Menu_SetChecked(hMenuItem, isChecked);

	return 0;
}

static int genmenu_RemoveMenuItem(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	HGENMENU hMenuItem = (HGENMENU)lua_touserdata(L, 1);

	INT_PTR res = Menu_RemoveItem(hMenuItem);
	lua_pushboolean(L, res == 0);

	return 1;
}

static luaL_Reg genmenuApi[] =
{
	{ "ModifyMenuItem", genmenu_ModifyMenuItem },
	{ "ConfigureMenuItem", genmenu_ConfigureMenuItem },
	{ "ShowMenuItem", genmenu_ShowMenuItem },
	{ "EnableMenuItem", genmenu_EnableMenuItem },
	{ "CheckMenuItem", genmenu_CheckMenuItem },
	{ "RemoveMenuItem", genmenu_RemoveMenuItem },

	{ nullptr, nullptr }
};

LUAMOD_API int luaopen_m_genmenu(lua_State *L)
{
	luaL_newlib(L, genmenuApi);

	return 1;
}
