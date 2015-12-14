#include "stdafx.h"

static int lua_AddSound(lua_State *L)
{
	ptrA name(mir_utf8decodeA(luaL_checkstring(L, 1)));
	ptrT description(mir_utf8decodeT(luaL_checkstring(L, 2)));
	ptrT section(mir_utf8decodeT(luaL_optstring(L, 3, MODULE)));

	TCHAR filePath[MAX_PATH];
	GetModuleFileName(g_hInstance, filePath, _countof(filePath));

	SKINSOUNDDESCEX ssd = { sizeof(SKINSOUNDDESCEX) };
	ssd.pszName = name;
	ssd.dwFlags = SSDF_TCHAR;
	ssd.ptszDescription = description;
	ssd.ptszSection = section;
	ssd.ptszDefaultFile = filePath;

	INT_PTR res = ::CallService("Skin/Sounds/AddNew", hLangpack, (LPARAM)&ssd);
	lua_pushnumber(L, res);

	return 1;
}

static int lua_PlaySound(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	INT_PTR res = SkinPlaySound(name);
	lua_pushnumber(L, res);

	return 1;
}

static int lua_PlayFile(lua_State *L)
{
	const char *path = luaL_checkstring(L, 1);

	INT_PTR res = SkinPlaySoundFile(ptrT(mir_utf8decodeT(path)));
	lua_pushnumber(L, res);

	return 1;
}

static luaL_Reg soundApi[] =
{
	{ "AddSound", lua_AddSound },
	{ "PlaySound", lua_PlaySound },

	{ "PlayFile", lua_PlayFile },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_sounds(lua_State *L)
{
	luaL_newlib(L, soundApi);

	return 1;
}
