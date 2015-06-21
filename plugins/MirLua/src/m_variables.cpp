#include "stdafx.h"

static int lua_FormatString(lua_State *L)
{
	if (!ServiceExists(MS_VARS_FORMATSTRING))
	{
		lua_pushvalue(L, 1);
		return 1;
	}

	ptrT format(mir_utf8decodeT(luaL_checkstring(L, 1)));
	MCONTACT hContact = lua_tointeger(L, 2);

	TCHAR *res = variables_parse(format, NULL, hContact);
	lua_pushstring(L, T2Utf(res));

	return 1;
}

static luaL_Reg variablesApi[] =
{
	{ "FormatString", lua_FormatString },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_variables(lua_State *L)
{
	luaL_newlib(L, variablesApi);

	return 1;
}
