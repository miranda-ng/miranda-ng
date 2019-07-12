#include "../stdafx.h"

struct EIHook
{
	EIHook(lua_State *_p1, int _p2) :
		L(_p1),
		ref(_p2)
	{}

	lua_State *L;
	int ref;	
};

static int ei_hook(WPARAM hContact, LPARAM slot, LPARAM param)
{
	EIHook *pHook = (EIHook *)param;
	if (pHook) {
		auto *L = pHook->L;
		lua_rawgeti(L, LUA_REGISTRYINDEX, pHook->ref);

		lua_pushinteger(L, hContact);
		lua_pushinteger(L, slot);
		luaM_pcall(L, 2, 0);
	}
	return 0;
}

static int ei_Register(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	const char *descr = luaL_checkstring(L, 2);
	HANDLE hIcolib = lua_touserdata(L, 3);
	luaL_checktype(L, 4, LUA_TFUNCTION);

	lua_pushvalue(L, 4);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	HANDLE res = ExtraIcon_RegisterIcolib(name, descr, hIcolib, ei_hook, (LPARAM)new EIHook(L, ref));
	lua_pushlightuserdata(L, res);
	return 1;
}

static int ei_SetIcon(lua_State *L)
{
	HANDLE hExtraIcon = lua_touserdata(L, 1);
	int hContact = luaL_checkinteger(L, 2);
	HANDLE hIcon = lua_touserdata(L, 3);
	ExtraIcon_SetIcon(hExtraIcon, hContact, hIcon);
	return 1;
}

static int ei_SetByName(lua_State *L)
{
	HANDLE hExtraIcon = lua_touserdata(L, 1);
	int hContact = luaL_checkinteger(L, 2);
	const char *icon = luaL_checkstring(L, 3);
	ExtraIcon_SetIconByName(hExtraIcon, hContact, icon);
	return 1;
}

static int ei_Clear(lua_State *L)
{
	HANDLE hExtraIcon = lua_touserdata(L, 1);
	int hContact = luaL_checkinteger(L, 2);
	ExtraIcon_Clear(hExtraIcon, hContact);
	return 1;
}

static luaL_Reg eiApi[] =
{
	{ "Register", ei_Register },

	{ "SetIcon", ei_SetIcon },
	{ "SetByName", ei_SetByName },
	{ "Clear", ei_Clear },

	{ nullptr, nullptr }
};

LUAMOD_API int luaopen_m_extraIcon(lua_State *L)
{
	luaL_newlib(L, eiApi);
	return 1;
}
