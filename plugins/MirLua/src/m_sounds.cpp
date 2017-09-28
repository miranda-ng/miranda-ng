#include "stdafx.h"

static int lua_AddSound(lua_State *L)
{
	ptrA name(mir_utf8decodeA(luaL_checkstring(L, 1)));
	ptrW description(mir_utf8decodeW(luaL_checkstring(L, 2)));
	ptrW section(mir_utf8decodeW(luaL_optstring(L, 3, MODULE)));
	ptrW filePath(mir_utf8decodeW(lua_tostring(L, 4)));

	int res = Skin_AddSound(name, section, description, filePath, CMLuaEnviroment::GetEnviromentId(L));
	lua_pushboolean(L, res == 0);

	return 1;
}

static int lua_PlaySound(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	INT_PTR res = Skin_PlaySound(name);
	lua_pushboolean(L, res == 0);

	return 1;
}

static int lua_PlayFile(lua_State *L)
{
	ptrW filePath(mir_utf8decodeW(luaL_checkstring(L, 1)));

	INT_PTR res = Skin_PlaySoundFile(filePath);
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
