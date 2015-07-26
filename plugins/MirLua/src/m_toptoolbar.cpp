#include "stdafx.h"

struct HandleTBBParam
{
	HANDLE h;
	TTBButton* tbb;
	HandleTBBParam(HANDLE h, TTBButton* tbb) : h(h), tbb(tbb) { }
};

static LIST<void> TBButtons(1, HandleKeySortT);

void KillModuleTTBButton()
{
	while (TBButtons.getCount())
	{
		HandleTBBParam* param = (HandleTBBParam*)TBButtons[0];
		::CallService(MS_TTB_REMOVEBUTTON, (WPARAM)param->h, 0);
		TBButtons.remove(0);
		mir_free(param->tbb->name);
		mir_free(param->tbb->pszTooltipUp);
		mir_free(param->tbb->pszTooltipDn);
		mir_free(param->tbb);
		delete param;
	}
}

static TTBButton* MakeTBButton(lua_State *L)
{
	TTBButton *tbb = (TTBButton*)mir_calloc(sizeof(TTBButton));
	tbb->dwFlags = TTBBF_ISLBUTTON;

	lua_pushliteral(L, "Name");
	lua_gettable(L, -2);
	tbb->name = mir_utf8decodeA(luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_pushliteral(L, "Service");
	lua_gettable(L, -2);
	tbb->pszService = (char*)lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_pushliteral(L, "Flags");
	lua_gettable(L, -2);
	tbb->dwFlags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	if (!(tbb->dwFlags & TTBBF_ISLBUTTON))
		tbb->dwFlags |= TTBBF_ISLBUTTON;

	// up state
	lua_pushliteral(L, "IconUp");
	lua_gettable(L, -2);
	tbb->hIconHandleUp = (HANDLE)lua_touserdata(L, -1);
	lua_pop(L, 1);

	lua_pushliteral(L, "TooltipUp");
	lua_gettable(L, -2);
	tbb->pszTooltipUp = mir_utf8decodeA(lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_pushliteral(L, "wParamUp");
	lua_gettable(L, -2);
	tbb->wParamUp = luaM_towparam(L, -1);
	lua_pop(L, 1);

	lua_pushliteral(L, "lParamUp");
	lua_gettable(L, -2);
	tbb->lParamUp = luaM_tolparam(L, -1);
	lua_pop(L, 1);

	// dn state
	lua_pushliteral(L, "IconDown");
	lua_gettable(L, -2);
	tbb->hIconHandleDn = (HANDLE)lua_touserdata(L, -1);
	lua_pop(L, 1);

	lua_pushliteral(L, "TooltipDown");
	lua_gettable(L, -2);
	tbb->pszTooltipDn = mir_utf8decodeA(lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_pushliteral(L, "wParamDown");
	lua_gettable(L, -2);
	tbb->wParamDown = luaM_towparam(L, -1);
	lua_pop(L, 1);

	lua_pushliteral(L, "lParamDown");
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

	TTBButton* tbb = MakeTBButton(L);

	HANDLE res = ::TopToolbar_AddButton(tbb);
	lua_pushlightuserdata(L, res);

	if (res != INVALID_HANDLE_VALUE)
		TBButtons.insert(new HandleTBBParam(res, tbb));
	else
	{
		mir_free(tbb->name);
		mir_free(tbb->pszTooltipUp);
		mir_free(tbb->pszTooltipDn);
		mir_free(tbb);
	}

	return 1;
}

static int lua_RemoveButton(lua_State *L)
{
	HANDLE hTTButton = (HANDLE)lua_touserdata(L, 1);

	INT_PTR res = ::CallService(MS_TTB_REMOVEBUTTON, (WPARAM)hTTButton, 0);
	lua_pushinteger(L, res);

	if (!res)
	{
		HandleTBBParam* param = (HandleTBBParam*)TBButtons.find(&hTTButton);
		if (param)
		{
			TBButtons.remove(param);
			mir_free(param->tbb->name);
			mir_free(param->tbb->pszTooltipUp);
			mir_free(param->tbb->pszTooltipDn);
			mir_free(param->tbb);
			delete param;
		}
	}

	return 1;
}

static int lua_OnTopToolBarLoaded(lua_State *L)
{
	if (!lua_isfunction(L, 1))
	{
		lua_pushlightuserdata(L, NULL);
		return 1;
	}

	lua_pushvalue(L, 1);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	HANDLE res = ::HookEventObjParam(ME_TTB_MODULELOADED, CMLua::HookEventObjParam, L, ref);
	lua_pushlightuserdata(L, res);

	CMLua::Hooks.insert(res);
	CMLua::HookRefs.insert(new HandleRefParam(L, res, ref));

	return 1;
}

static luaL_Reg toptoolbarApi[] =
{
	{ "AddButton", lua_AddButton },
	{ "RemoveButton", lua_RemoveButton },

	{ "OnTopToolBarLoaded", lua_OnTopToolBarLoaded },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_toptoolbar(lua_State *L)
{
	luaL_newlib(L, toptoolbarApi);

	return 1;
}
