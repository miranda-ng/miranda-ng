#include "stdafx.h"

void MakeMenuItem(lua_State *L, CMenuItem &mi)
{
	mi.hLangpack = hScriptsLangpack;

	lua_pushstring(L, "Flags");
	lua_gettable(L, -2);
	mi.flags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	if (!(mi.flags & CMIF_TCHAR))
		mi.flags |= CMIF_TCHAR;

	lua_pushstring(L, "Name");
	lua_gettable(L, -2);
	mi.name.t = mir_utf8decodeT((char*)luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_pushstring(L, "Position");
	lua_gettable(L, -2);
	mi.position = lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_pushstring(L, "Icon");
	lua_gettable(L, -2);
	mi.hIcolibItem = (HANDLE)lua_touserdata(L, -1);
	lua_pop(L, 1);

	lua_pushstring(L, "Service");
	lua_gettable(L, -2);
	mi.pszService = (char*)lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_pushstring(L, "Parent");
	lua_gettable(L, -2);
	mi.root = (HGENMENU)lua_touserdata(L, -1);
	lua_pop(L, 1);
}

static int lua_AddMenuItem(lua_State *L)
{
	int hMenuObject = lua_tointeger(L, 1);

	if (lua_type(L, 2) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}

	CMenuItem mi;
	MakeMenuItem(L, mi);

	HGENMENU res = ::Menu_AddItem(hMenuObject, &mi, NULL);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_ModifyMenuItem(lua_State *L)
{
	HGENMENU hMenuItem = (HGENMENU)lua_touserdata(L, 1);
	ptrT name(mir_utf8decodeT(lua_tostring(L, 2)));
	HANDLE hIcolibItem = lua_touserdata(L, 3);
	int flags = lua_tointeger(L, 4);

	if (!(flags & CMIF_UNICODE))
		flags |= CMIF_UNICODE;

	INT_PTR res = ::Menu_ModifyItem(hMenuItem, name, hIcolibItem, flags);
	lua_pushinteger(L, res);
	return 1;
}

static int lua_ShowMenuItem(lua_State *L)
{
	HGENMENU hMenuItem = (HGENMENU)lua_touserdata(L, 1);
	bool isShow = luaM_toboolean(L, 2);

	::Menu_ShowItem(hMenuItem, isShow);

	return 0;
}

static int lua_EnableMenuItem(lua_State *L)
{
	HGENMENU hMenuItem = (HGENMENU)lua_touserdata(L, 1);
	bool isEnable = luaM_toboolean(L, 2);

	::Menu_EnableItem(hMenuItem, isEnable);

	return 0;
}

static int lua_CheckMenuItem(lua_State *L)
{
	HGENMENU hMenuItem = (HGENMENU)lua_touserdata(L, 1);
	bool isChecked = luaM_toboolean(L, 2);

	::Menu_SetChecked(hMenuItem, isChecked);

	return 0;
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
	{ "AddMenuItem", lua_AddMenuItem },
	{ "ModifyMenuItem", lua_ModifyMenuItem },
	{ "ShowMenuItem", lua_ShowMenuItem },
	{ "EnableMenuItem", lua_EnableMenuItem },
	{ "CheckMenuItem", lua_CheckMenuItem },
	{ "RemoveMenuItem", lua_RemoveMenuItem },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_genmenu(lua_State *L)
{
	luaL_newlib(L, genmenuApi);

	return 1;
}
