#include "stdafx.h"

static POPUPDATAT* MakePopupData(lua_State *L)
{
	POPUPDATAT *ppd = (POPUPDATAT*)mir_calloc(sizeof(POPUPDATAT));

	lua_getfield(L, -1, "ContactName");
	mir_wstrcpy(ppd->lptzContactName, ptrW(mir_utf8decodeW(lua_tostring(L, -1))));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Text");
	mir_wstrcpy(ppd->lptzText, ptrW(mir_utf8decodeW(luaL_checkstring(L, -1))));
	lua_pop(L, 1);

	lua_getfield(L, -1, "hContact");
	ppd->lchContact = lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "ColorBack");
	ppd->colorBack = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "ColorText");
	ppd->colorText = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Seconds");
	ppd->iSeconds = lua_tointeger(L, -1);
	lua_pop(L, 1);

	return ppd;
}

static int lua_AddPopup(lua_State *L)
{
	if (lua_type(L, 1) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}

	mir_ptr<POPUPDATAT> ppd(MakePopupData(L));

	INT_PTR res = ::CallService(MS_POPUP_ADDPOPUPT, (WPARAM)ppd, 0);
	lua_pushinteger(L, res);

	return 1;
}

static POPUPDATA2* MakePopupData2(lua_State *L)
{
	POPUPDATA2 *ppd = (POPUPDATA2*)mir_calloc(sizeof(POPUPDATA2));
	ppd->cbSize = sizeof(POPUPDATA2);

	lua_getfield(L, -1, "Flags");
	ppd->flags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	if (!(ppd->flags & PU2_UNICODE))
		ppd->flags |= PU2_UNICODE;

	lua_getfield(L, -1, "Title");
	ppd->lptzTitle = mir_utf8decodeW(lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Text");
	ppd->lptzText = mir_utf8decodeW(luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "hContact");
	ppd->lchContact = lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "ColorBack");
	ppd->colorBack = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "ColorText");
	ppd->colorText = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "hEvent");
	ppd->lchEvent = lua_touserdata(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Timestamp");
	ppd->dwTimestamp = lua_tonumber(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Timeout");
	ppd->iSeconds = lua_tointeger(L, -1);
	lua_pop(L, 1);

	return ppd;
}

static int lua_AddPopup2(lua_State *L)
{
	if (lua_type(L, 1) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}

	mir_ptr<POPUPDATA2> ppd(MakePopupData2(L));

	INT_PTR res = ::CallService(MS_POPUP_ADDPOPUP2, (WPARAM)ppd, 0);
	lua_pushinteger(L, res);

	mir_free(ppd->lptzTitle);
	mir_free(ppd->lptzText);

	return 1;
}

static luaL_Reg popupApi[] =
{
	{ "AddPopup", lua_AddPopup },
	{ "AddPopup2", lua_AddPopup2 },

	{ NULL, NULL }
};

extern "C" LUAMOD_API int luaopen_m_popup(lua_State *L)
{
	luaL_newlib(L, popupApi);

	return 1;
}
