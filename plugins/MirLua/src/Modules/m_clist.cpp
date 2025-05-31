#include "../stdafx.h"

void MakeMenuItem(lua_State *L, CMenuItem &mi)
{
	mi.pPlugin = CMLuaEnvironment::GetEnvironmentId(L);

	lua_getfield(L, -1, "Flags");
	mi.flags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	if (!(mi.flags & CMIF_UNICODE))
		mi.flags |= CMIF_UNICODE;

	lua_getfield(L, -1, "Uid");
	const char* uuid = lua_tostring(L, -1);
	if (UuidFromStringA((RPC_CSTR)uuid, (UUID*)&mi.uid))
		UNSET_UID(mi);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Name");
	mi.name.w = mir_utf8decodeW(luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Position");
	mi.position = lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Icon");
	mi.hIcolibItem = (HANDLE)lua_touserdata(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Service");
	mi.pszService = lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Parent");
	mi.root = (HGENMENU)lua_touserdata(L, -1);
	lua_pop(L, 1);
}

static int clist_AddMainMenuRoot(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	int position = lua_tointeger(L, 2);
	HANDLE hIcon = (HANDLE)lua_touserdata(L, 3);

	HGENMENU res = g_plugin.addRootMenu(MO_MAIN, ptrW(mir_utf8decodeW(name)), position, hIcon);
	if (res != nullptr)
		lua_pushlightuserdata(L, res);
	else
		lua_pushnil(L);

	return 1;
}

static int clist_AddMainMenuItem(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	CMenuItem mi(&g_plugin);
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

	HGENMENU res = g_plugin.addRootMenu(MO_MAIN, ptrW(mir_utf8decodeW(name)), position, hIcon);
	if (res != nullptr)
		lua_pushlightuserdata(L, res);
	else
		lua_pushnil(L);

	return 1;
}

static int clist_AddContactMenuItem(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	CMenuItem mi(&g_plugin);
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

	CMenuItem mi(&g_plugin);
	MakeMenuItem(L, mi);

	HGENMENU res = Menu_AddTrayMenuItem(&mi);
	if (res != nullptr)
		lua_pushlightuserdata(L, res);
	else
		lua_pushnil(L);

	return 1;
}

static int clist_AddGroupMenuItem(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	CMenuItem mi(&g_plugin);
	MakeMenuItem(L, mi);

	HGENMENU res = Menu_AddGroupMenuItem(&mi);
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
	{ "AddGroupMenuItem", clist_AddGroupMenuItem },

	{ nullptr, nullptr }
};

LUAMOD_API int luaopen_m_clist(lua_State *L)
{
	luaL_newlib(L, clistApi);

	return 1;
}
