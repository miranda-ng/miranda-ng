#include "stdafx.h"

static int sounds_AddSound(lua_State *L)
{
	ptrA name(mir_utf8decodeA(luaL_checkstring(L, 1)));
	ptrW description(mir_utf8decodeW(luaL_checkstring(L, 2)));
	ptrW section(mir_utf8decodeW(luaL_optstring(L, 3, MODULE)));
	ptrW filePath(mir_utf8decodeW(lua_tostring(L, 4)));

	SKINSOUNDDESCEX ssd = { sizeof(SKINSOUNDDESCEX) };
	ssd.pszName = name;
	ssd.dwFlags = SSDF_UNICODE;
	ssd.pwszDescription = description;
	ssd.pwszSection = section;
	ssd.pwszDefaultFile = filePath;

	int hScriptLangpack = CMLuaScript::GetScriptIdFromEnviroment(L);

	INT_PTR res = CallService("Skin/Sounds/AddNew", hScriptLangpack, (LPARAM)&ssd);
	lua_pushboolean(L, res == 0);

	return 1;
}

static int sounds_PlaySound(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	INT_PTR res = SkinPlaySound(name);
	lua_pushboolean(L, res == 0);

	return 1;
}

static int sounds_PlayFile(lua_State *L)
{
	ptrW filePath(mir_utf8decodeW(luaL_checkstring(L, 1)));

	INT_PTR res = SkinPlaySoundFile(filePath);
	lua_pushboolean(L, res == 0);

	return 1;
}

static luaL_Reg soundApi[] =
{
	{ "AddSound", sounds_AddSound },
	{ "PlaySound", sounds_PlaySound },

	{ "PlayFile", sounds_PlayFile },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_sounds(lua_State *L)
{
	luaL_newlib(L, soundApi);

	return 1;
}
