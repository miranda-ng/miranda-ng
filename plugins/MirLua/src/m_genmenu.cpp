#include "stdafx.h"

void MakeMenuItem(lua_State *L, CMenuItem &mi)
{
	mi.hLangpack = CMLuaScript::GetScriptIdFromEnviroment(L);

	lua_getfield(L, -1, "Flags");
	mi.flags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	if (!(mi.flags & CMIF_TCHAR))
		mi.flags |= CMIF_TCHAR;

	lua_getfield(L, -1, "Uid");
	const char* uuid = (char*)lua_tostring(L, -1);
	if (UuidFromStringA((RPC_CSTR)uuid, (UUID*)&mi.uid))
		UNSET_UID(mi);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Name");
	mi.name.t = mir_utf8decodeT((char*)luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Position");
	mi.position = lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Icon");
	mi.hIcolibItem = (HANDLE)lua_touserdata(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Service");
	mi.pszService = (char*)lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Parent");
	mi.root = (HGENMENU)lua_touserdata(L, -1);
	lua_pop(L, 1);
}

/*static int genmenu_CreateRoot(lua_State *L)
{
	ObsoleteMethod(L, "use m_clist module instead");

	int hMenuObject = luaL_checkinteger(L, 1);
	const char *name = luaL_checkstring(L, 2);
	int position = lua_tointeger(L, 3);
	HANDLE hIcon = (HANDLE)lua_touserdata(L, 4);

	HGENMENU res = Menu_CreateRoot(hMenuObject, ptrT(Utf8DecodeT(name)), position, hIcon);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int genmenu_AddMenuItem(lua_State *L)
{
	ObsoleteMethod(L, "use m_clist module instead");

	int hMenuObject = luaL_checkinteger(L, 1);

	if (lua_type(L, 2) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}

	CMenuItem mi;
	MakeMenuItem(L, mi);

	HGENMENU res = Menu_AddItem(hMenuObject, &mi, NULL);
	lua_pushlightuserdata(L, res);

	return 1;
}*/

static int genmenu_ModifyMenuItem(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
	HGENMENU hMenuItem = (HGENMENU)lua_touserdata(L, 1);
	ptrT name(mir_utf8decodeT(lua_tostring(L, 2)));
	HANDLE hIcolibItem = luaL_opt(L, lua_touserdata, 3, INVALID_HANDLE_VALUE);
	int flags = luaL_optinteger(L, 4, -1);

	if (!(flags & CMIF_UNICODE))
		flags |= CMIF_UNICODE;

	INT_PTR res = Menu_ModifyItem(hMenuItem, name, hIcolibItem, flags);
	lua_pushinteger(L, res);
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
	lua_pushinteger(L, res);

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
	//{ "CreateRoot", genmenu_CreateRoot },
	//{ "AddMenuItem", genmenu_AddMenuItem },
	{ "ModifyMenuItem", genmenu_ModifyMenuItem },
	{ "ConfigureMenuItem", genmenu_ConfigureMenuItem },
	{ "ShowMenuItem", genmenu_ShowMenuItem },
	{ "EnableMenuItem", genmenu_EnableMenuItem },
	{ "CheckMenuItem", genmenu_CheckMenuItem },
	{ "RemoveMenuItem", genmenu_RemoveMenuItem },

	//{ "MO_MAIN", NULL },
	//{ "MO_CONTACT", NULL },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_genmenu(lua_State *L)
{
	luaL_newlib(L, genmenuApi);
	/*lua_pushinteger(L, MO_MAIN);
	lua_setfield(L, -2, "MO_MAIN");
	lua_pushinteger(L, MO_CONTACT);
	lua_setfield(L, -2, "MO_CONTACT");*/

	return 1;
}
