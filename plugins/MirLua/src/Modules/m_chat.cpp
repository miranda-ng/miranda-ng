#include "../stdafx.h"

static luaL_Reg chatApi[] =
{
	{ nullptr, nullptr }
};

LUAMOD_API int luaopen_m_chat(lua_State *L)
{
	luaL_newlib(L, chatApi);

	MT<GCEVENT>(L, "GCEVENT")
		.Field(&GCEVENT::iType, "Type", LUA_TINTEGER)
		.Field(&GCEVENT::time, "Timestamp", LUA_TINTEGER)
		.Field(&GCEVENT::time, "IsMe", LUA_TINTEGER)
		.Field(&GCEVENT::time, "Flags", LUA_TINTEGER)
		.Field(&GCEVENT::pszNick, "Nick", LUA_TSTRINGW)
		.Field(&GCEVENT::pszUID, "Uid", LUA_TSTRINGW)
		.Field(&GCEVENT::pszStatus, "Status", LUA_TSTRINGW)
		.Field(&GCEVENT::pszText, "Text", LUA_TSTRINGW);

	return 1;
}
