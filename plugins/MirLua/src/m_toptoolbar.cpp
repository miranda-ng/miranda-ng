#include "stdafx.h"

static LIST<void> TBButtons(1, PtrKeySortT);

void KillModuleTTBButton()
{
	while (TBButtons.getCount())
	{
		HANDLE hTTButton = TBButtons[0];
		::CallService(MS_TTB_REMOVEBUTTON, (WPARAM)hTTButton, 0);
		TBButtons.remove(0);
	}
}

static TTBButton* MakeTBButton(lua_State *L)
{
	TTBButton *tbb = (TTBButton*)mir_calloc(sizeof(TTBButton));
	tbb->dwFlags = TTBBF_ISLBUTTON;

	lua_getfield(L, -1, "Name");
	tbb->name = mir_utf8decodeA(luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Service");
	tbb->pszService = (char*)lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Flags");
	tbb->dwFlags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	// up state
	lua_getfield(L, -1, "IconUp");
	tbb->hIconHandleUp = (HANDLE)lua_touserdata(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "TooltipUp");
	tbb->pszTooltipUp = mir_utf8decodeA(lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "wParamUp");
	tbb->wParamUp = luaM_towparam(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "lParamUp");
	tbb->lParamUp = luaM_tolparam(L, -1);
	lua_pop(L, 1);

	// dn state
	lua_getfield(L, -1, "IconDown");
	tbb->hIconHandleDn = (HANDLE)lua_touserdata(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "TooltipDown");
	tbb->pszTooltipDn = mir_utf8decodeA(lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "wParamDown");
	tbb->wParamDown = luaM_towparam(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "lParamDown");
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

	TTBButton* tbb = MakeTBButton(L);

	HANDLE res = ::TopToolbar_AddButton(tbb);
	lua_pushlightuserdata(L, res);

	if (res != INVALID_HANDLE_VALUE)
		TBButtons.insert(res);

	mir_free(tbb->name);
	mir_free(tbb->pszTooltipUp);
	mir_free(tbb->pszTooltipDn);
	mir_free(tbb);

	return 1;
}

static int lua_RemoveButton(lua_State *L)
{
	HANDLE hTTButton = (HANDLE)lua_touserdata(L, 1);

	INT_PTR res = ::CallService(MS_TTB_REMOVEBUTTON, (WPARAM)hTTButton, 0);
	lua_pushinteger(L, res);

	if (!res)
		TBButtons.remove(hTTButton);

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
