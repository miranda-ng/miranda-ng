#include "stdafx.h"

void MakeHotkey(lua_State *L, HOTKEYDESC &hk)
{
	hk.cbSize = sizeof(HOTKEYDESC);

	lua_getfield(L, -1, "Flags");
	hk.dwFlags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	if (!(hk.dwFlags & HKD_TCHAR))
		hk.dwFlags |= HKD_TCHAR;

	lua_getfield(L, -1, "Name");
	hk.pszName = mir_utf8decodeA(luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Description");
	hk.ptszDescription = mir_utf8decodeT((char*)lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Section");
	hk.ptszSection = mir_utf8decodeT(luaL_optstring(L, -1, MODULE));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Hotkey");
	hk.DefHotKey = lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Service");
	hk.pszService = mir_utf8decodeA(luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "lParam");
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
	lua_pushboolean(L, res);

	return 1;
}

static int lua_Unregister(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	::CallService("CoreHotkeys/Unregister", 0, (LPARAM)name);

	return 0;
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
	{ "MakeHotkey", lua_MakeHotkey },
	{ "Register", lua_Register },
	{ "Unregister", lua_Unregister },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_hotkeys(lua_State *L)
{
	luaL_newlib(L, hotkeysApi);

	return 1;
}
