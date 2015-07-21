#include "stdafx.h"

void MakeHotkey(lua_State *L, HOTKEYDESC &hk)
{
	hk.cbSize = sizeof(HOTKEYDESC);

	lua_pushliteral(L, "Flags");
	lua_gettable(L, -2);
	hk.dwFlags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	if (!(hk.dwFlags & HKD_TCHAR))
		hk.dwFlags |= HKD_TCHAR;

	lua_pushliteral(L, "Name");
	lua_gettable(L, -2);
	hk.pszName = mir_utf8decodeA(luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_pushliteral(L, "Description");
	lua_gettable(L, -2);
	hk.ptszDescription = mir_utf8decodeT((char*)lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_pushliteral(L, "Section");
	lua_gettable(L, -2);
	hk.ptszSection = mir_utf8decodeT(luaL_optstring(L, -1, MODULE));
	lua_pop(L, 1);

	lua_pushliteral(L, "Hotkey");
	lua_gettable(L, -2);
	hk.DefHotKey = lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_pushliteral(L, "Service");
	lua_gettable(L, -2);
	hk.pszService = mir_utf8decodeA(luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_pushliteral(L, "lParam");
	lua_gettable(L, -2);
	hk.lParam = (LPARAM)lua_touserdata(L, -1);
	lua_pop(L, 1);
}

static int lua_Register(lua_State *L)
{
	if (lua_type(L, 1) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}

	HOTKEYDESC hk;
	MakeHotkey(L, hk);

	INT_PTR res = ::CallService("CoreHotkeys/Register", (WPARAM)hScriptsLangpack, (LPARAM)&hk);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_Unregister(lua_State *L)
{
	const char* name = luaL_checkstring(L, 1);

	INT_PTR res = ::CallService("CoreHotkeys/Unregister", 0, (LPARAM)name);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_MakeHotkey(lua_State *L)
{
	int mod = luaL_checkinteger(L, 1);
	int vk = luaL_checkinteger(L, 2);

	WORD res = HOTKEYCODE(mod, vk);
	lua_pushinteger(L, res);

	return 1;
}

static luaL_Reg hotkeysApi[] =
{
	{ "Register", lua_Register },
	{ "Unregister", lua_Unregister },

	{ "MakeHotkey", lua_MakeHotkey },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_hotkeys(lua_State *L)
{
	luaL_newlib(L, hotkeysApi);

	return 1;
}
