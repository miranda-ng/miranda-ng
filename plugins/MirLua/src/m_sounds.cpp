#include "stdafx.h"

static int lua_AddSound(lua_State *L)
{
	ptrA name(mir_utf8decodeA(luaL_checkstring(L, 1)));
	ptrT description(mir_utf8decodeT(luaL_checkstring(L, 2)));
	ptrT section(mir_utf8decodeT(luaL_optstring(L, 3, MODULE)));
	ptrT filePath(mir_utf8decodeT(lua_tostring(L, 4)));

	SKINSOUNDDESCEX ssd = { sizeof(SKINSOUNDDESCEX) };
	ssd.pszName = name;
	ssd.dwFlags = SSDF_TCHAR;
	ssd.ptszDescription = description;
	ssd.ptszSection = section;
	ssd.ptszDefaultFile = filePath;

	INT_PTR res = ::CallService("Skin/Sounds/AddNew", hLangpack, (LPARAM)&ssd);
	lua_pushboolean(L, res == 0);

	return 1;
}

static int lua_PlaySound(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	INT_PTR res = ::SkinPlaySound(name);
	lua_pushboolean(L, res == 0);

	return 1;
}

static int lua_PlayFile(lua_State *L)
{
	ptrT filePath(mir_utf8decodeT(luaL_checkstring(L, 1)));

	INT_PTR res = ::SkinPlaySoundFile(filePath);
	lua_pushboolean(L, res == 0);

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
