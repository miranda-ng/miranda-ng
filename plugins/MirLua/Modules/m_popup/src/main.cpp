#include "stdafx.h"

static POPUPDATAW* MakePopupData(lua_State *L)
{
	POPUPDATAW *ppd = (POPUPDATAW*)mir_calloc(sizeof(POPUPDATAW));

	lua_getfield(L, -1, "ContactName");
	mir_wstrcpy(ppd->lpwzContactName, ptrW(mir_utf8decodeW(lua_tostring(L, -1))));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Text");
	mir_wstrcpy(ppd->lpwzText, ptrW(mir_utf8decodeW(luaL_checkstring(L, -1))));
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
		lua_pushlightuserdata(L, nullptr);
		return 1;
	}

	mir_ptr<POPUPDATAW> ppd(MakePopupData(L));

	INT_PTR res = (INT_PTR)::PUAddPopupW(ppd);
	lua_pushinteger(L, res);

	return 1;
}

static POPUPDATA2* MakePopupData2(lua_State *L)
{
	POPUPDATA2 *ppd = (POPUPDATA2*)mir_calloc(sizeof(POPUPDATA2));

	lua_getfield(L, -1, "Flags");
	ppd->flags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	if (!(ppd->flags & PU2_UNICODE))
		ppd->flags |= PU2_UNICODE;

	lua_getfield(L, -1, "Title");
	ppd->szTitle.w = mir_utf8decodeW(lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Text");
	ppd->szText.w = mir_utf8decodeW(luaL_checkstring(L, -1));
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
		lua_pushlightuserdata(L, nullptr);
		return 1;
	}

	mir_ptr<POPUPDATA2> ppd(MakePopupData2(L));

	INT_PTR res = (INT_PTR)Popup_Add(ppd.get());
	lua_pushinteger(L, res);

	mir_free((void *)ppd->szTitle.w);
	mir_free((void *)ppd->szText.w);

	return 1;
}

static luaL_Reg popupApi[] =
{
	{ "AddPopup", lua_AddPopup },
	{ "AddPopup2", lua_AddPopup2 },

	{ nullptr, nullptr }
};

extern "C" LUAMOD_API int luaopen_m_popup(lua_State *L)
{
	luaL_newlib(L, popupApi);

	return 1;
}
