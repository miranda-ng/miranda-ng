#include "stdafx.h"

static TTBButton* MakeTBButton(lua_State *L)
{
	TTBButton *tbb = (TTBButton*)mir_calloc(sizeof(TTBButton));

	lua_pushstring(L, "Name");
	lua_gettable(L, -2);
	tbb->name = mir_utf8decode((char*)luaL_checkstring(L, -1), NULL);
	lua_pop(L, 1);

	lua_pushstring(L, "Service");
	lua_gettable(L, -2);
	tbb->pszService = (char*)lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_pushstring(L, "Flags");
	lua_gettable(L, -2);
	tbb->dwFlags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	// up state
	lua_pushstring(L, "IconUp");
	lua_gettable(L, -2);
	tbb->hIconHandleUp = (HANDLE)lua_touserdata(L, -1);
	lua_pop(L, 1);

	lua_pushstring(L, "TooltipUp");
	lua_gettable(L, -2);
	tbb->pszTooltipUp = mir_utf8decode((char*)lua_tostring(L, -1), NULL);
	lua_pop(L, 1);

	lua_pushstring(L, "wParamUp");
	lua_gettable(L, -2);
	tbb->wParamUp = luaM_towparam(L, -1);
	lua_pop(L, 1);

	lua_pushstring(L, "lParamUp");
	lua_gettable(L, -2);
	tbb->lParamUp = luaM_tolparam(L, -1);
	lua_pop(L, 1);

	// dn state
	lua_pushstring(L, "IconDown");
	lua_gettable(L, -2);
	tbb->hIconHandleDn = (HANDLE)lua_touserdata(L, -1);
	lua_pop(L, 1);

	lua_pushstring(L, "TooltipDown");
	lua_gettable(L, -2);
	tbb->pszTooltipDn = mir_utf8decode((char*)lua_tostring(L, -1), NULL);
	lua_pop(L, 1);

	lua_pushstring(L, "wParamDown");
	lua_gettable(L, -2);
	tbb->wParamDown = luaM_towparam(L, -1);
	lua_pop(L, 1);

	lua_pushstring(L, "lParamDown");
	lua_gettable(L, -2);
	tbb->lParamDown = luaM_tolparam(L, -1);
	lua_pop(L, 1);

	return tbb;
}

static int lua_AddButton(lua_State *L)
{
	if (lua_type(L, 1) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}

	mir_ptr<TTBButton> tbb(MakeTBButton(L));

	HANDLE res = ::TopToolbar_AddButton(tbb);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_RemoveButton(lua_State *L)
{
	HANDLE hTTButton = (HANDLE)lua_touserdata(L, 1);

	INT_PTR res = ::CallService(MS_TTB_REMOVEBUTTON, (WPARAM)hTTButton, 0);
	lua_pushinteger(L, res);

	return 1;
}

static luaL_Reg toptoolbarApi[] =
{
	{ "AddButton", lua_AddButton },
	{ "RemoveButton", lua_RemoveButton },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_toptoolbar(lua_State *L)
{
	luaL_newlib(L, toptoolbarApi);

	return 1;
}
