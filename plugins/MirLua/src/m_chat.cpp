#include "stdafx.h"

static luaL_Reg chatApi[] =
{
	{ NULL, NULL }
};

template <>
int MT<GCEVENT>::Index(lua_State *L, GCEVENT *gce)
{
	const char *key = lua_tostring(L, 2);

	if (mir_strcmp(key, "Destination") == 0)
		MT<GCDEST>::Set(L, gce->pDest);
	else
		lua_pushnil(L);

	return 1;
}

LUAMOD_API int luaopen_m_chat(lua_State *L)
{
	luaL_newlib(L, chatApi);

	MT<GCDEST>(L, "GCDEST")
		.Field(&GCDEST::pszModule, "Module", LUA_TSTRINGA)
		.Field(&GCDEST::ptszID, "Id", LUA_TSTRINGW)
		.Field(&GCDEST::iType, "Type", LUA_TINTEGER);

	MT<GCEVENT>(L, "GCEVENT")
		.Field(&GCEVENT::time, "Timestamp", LUA_TINTEGER)
		.Field(&GCEVENT::time, "IsMe", LUA_TINTEGER)
		.Field(&GCEVENT::time, "Flags", LUA_TINTEGER)
		.Field(&GCEVENT::ptszNick, "Nick", LUA_TSTRINGW)
		.Field(&GCEVENT::ptszUID, "Uid", LUA_TSTRINGW)
		.Field(&GCEVENT::ptszStatus, "Status", LUA_TSTRINGW)
		.Field(&GCEVENT::ptszText, "Text", LUA_TSTRINGW);

	return 1;
}
