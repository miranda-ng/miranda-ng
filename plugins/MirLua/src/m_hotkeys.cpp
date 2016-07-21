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
	hk.ptszDescription = mir_utf8decodeT(lua_tostring(L, -1));
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

static int hotkeys_Register(lua_State *L)
{
	if (lua_type(L, 1) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}

	HOTKEYDESC hk;
	MakeHotkey(L, hk);

	int hScriptLangpack = CMLuaScript::GetScriptIdFromEnviroment(L);

	INT_PTR res = ::CallService("CoreHotkeys/Register", (WPARAM)hScriptLangpack, (LPARAM)&hk);
	lua_pushboolean(L, res);

	return 1;
}

static int hotkeys_Unregister(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	CallService(MS_HOTKEY_UNREGISTER, 0, (LPARAM)name);

	return 0;
}

static const char *mods[] = { "shift", "ctrl", "alt", "ext", NULL };

static int hotkeys_MakeHotkey(lua_State *L)
{
	int mod = 0;
	switch (lua_type(L, 1))
	{
	case LUA_TNUMBER:
		mod = luaL_checkinteger(L, 1);
		break;
	case LUA_TSTRING:
		mod = (1 << (luaL_checkoption(L, 1, NULL, mods) - 1));
		break;
	case LUA_TTABLE:
		lua_pushnil(L);
		while (lua_next(L, 1) != 0)
		{
			mod |= (1 << (luaL_checkoption(L, -1, NULL, mods) - 1));
			lua_pop(L, 1);
		}
		break;
	default:
		luaL_argerror(L, 1, luaL_typename(L, 1));
	}
	int vk = luaL_checknumber(L, 2);

	WORD res = HOTKEYCODE(mod, vk);
	lua_pushinteger(L, res);

	return 1;
}

static luaL_Reg hotkeysApi[] =
{
	{ "MakeHotkey", hotkeys_MakeHotkey },
	{ "Register", hotkeys_Register },
	{ "Unregister", hotkeys_Unregister },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_hotkeys(lua_State *L)
{
	luaL_newlib(L, hotkeysApi);

	return 1;
}
