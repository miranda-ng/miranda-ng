#include "stdafx.h"

static int lua_Parse(lua_State *L)
{
	if (!ServiceExists(MS_VARS_FORMATSTRING))
	{
		lua_pushvalue(L, 1);
		return 1;
	}

	ptrW format(mir_utf8decodeW(luaL_checkstring(L, 1)));
	MCONTACT hContact = lua_tointeger(L, 2);

	wchar_t *res = variables_parse_ex(format, NULL, hContact, NULL, 0);
	lua_pushstring(L, T2Utf(res));

	return 1;
}

static luaL_Reg variablesApi[] =
{
	{ "Parse", lua_Parse },

	{ NULL, NULL }
};

extern "C" LUAMOD_API int luaopen_m_variables(lua_State *L)
{
	luaL_newlib(L, variablesApi);

	return 1;
}
