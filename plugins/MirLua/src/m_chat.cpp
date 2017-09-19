#include "stdafx.h"

static luaL_Reg chatApi[] =
{
	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_chat(lua_State *L)
{
	luaL_newlib(L, chatApi);

	MT<GCEVENT>(L, "GCEVENT")
		.Field(&GCEVENT::pszModule, "Module", LUA_TSTRINGA)
		.Field(&GCEVENT::ptszID, "Id", LUA_TSTRINGW)
		.Field(&GCEVENT::iType, "Type", LUA_TINTEGER)
		.Field(&GCEVENT::time, "Timestamp", LUA_TINTEGER)
		.Field(&GCEVENT::time, "IsMe", LUA_TINTEGER)
		.Field(&GCEVENT::time, "Flags", LUA_TINTEGER)
		.Field(&GCEVENT::ptszNick, "Nick", LUA_TSTRINGW)
		.Field(&GCEVENT::ptszUID, "Uid", LUA_TSTRINGW)
		.Field(&GCEVENT::ptszStatus, "Status", LUA_TSTRINGW)
		.Field(&GCEVENT::ptszText, "Text", LUA_TSTRINGW);

	return 1;
}
