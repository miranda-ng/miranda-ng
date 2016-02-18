#include "stdafx.h"

static BBButton* MakeBBButton(lua_State *L)
{
	BBButton *bbb = (BBButton*)mir_calloc(sizeof(BBButton));
	bbb->cbSize = sizeof(BBButton);
	bbb->dwDefPos = 100;

	lua_getfield(L, -1, "Module");
	bbb->pszModuleName = mir_utf8decodeA(luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "ButtonID");
	bbb->dwButtonID = luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Flags");
	bbb->bbbFlags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	if ((bbb->bbbFlags & BBBF_ANSITOOLTIP))
		bbb->bbbFlags &= ~BBBF_ANSITOOLTIP;

	lua_getfield(L, -1, "Tooltip");
	bbb->ptszTooltip = mir_utf8decodeT(lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Icon");
	bbb->hIcon = (HANDLE)lua_touserdata(L, -1);
	lua_pop(L, 1);

	return bbb;
}

static int lua_AddButton(lua_State *L)
{
	if (lua_type(L, 1) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}

	BBButton* bbb = MakeBBButton(L);

	INT_PTR res = CallService(MS_BB_ADDBUTTON, 0, (LPARAM)bbb);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_ModifyButton(lua_State *L)
{
	if (lua_type(L, 1) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}

	BBButton* bbb = MakeBBButton(L);

	INT_PTR res = CallService(MS_BB_MODIFYBUTTON, 0, (LPARAM)bbb);
	lua_pushinteger(L, res);

	mir_free(bbb->pszModuleName);
	mir_free(bbb->ptszTooltip);
	mir_free(bbb);

	return 1;
}

static int lua_RemoveButton(lua_State *L)
{
	ptrA szModuleName(mir_utf8decodeA(luaL_checkstring(L, 1)));

	BBButton mbb = { sizeof(BBButton) };
	mbb.pszModuleName = szModuleName;
	mbb.dwButtonID = luaL_checkinteger(L, 2);

	INT_PTR res = ::CallService(MS_BB_REMOVEBUTTON, 0, (LPARAM)&mbb);
	lua_pushinteger(L, res);

	return 1;
}

static luaL_Reg msgbuttinsbarApi[] =
{
	{ "AddButton", lua_AddButton },
	{ "ModifyButton", lua_ModifyButton },
	{ "RemoveButton", lua_RemoveButton },

	{ NULL, NULL }
};

/***********************************************/

#define MT_CUSTOMBUTTONCLICKDATA "CustomButtonClickData"

static int bcd__call(lua_State *L)
{
	CustomButtonClickData *bcd = (CustomButtonClickData*)lua_touserdata(L, 1);
	if (bcd == NULL)
	{
		lua_pushnil(L);
		return 1;
	}

	CustomButtonClickData **udata = (CustomButtonClickData**)lua_newuserdata(L, sizeof(CustomButtonClickData*));
	*udata = bcd;

	luaL_setmetatable(L, MT_CUSTOMBUTTONCLICKDATA);

	return 1;
}

static int bcd__index(lua_State *L)
{
	CustomButtonClickData *bcd = *(CustomButtonClickData**)luaL_checkudata(L, 1, MT_CUSTOMBUTTONCLICKDATA);
	const char *key = lua_tostring(L, 2);

	if (mir_strcmpi(key, "Module") == 0)
		lua_pushstring(L, ptrA(mir_utf8encode(bcd->pszModule)));
	else if (mir_strcmpi(key, "ButtonID") == 0)
		lua_pushinteger(L, bcd->dwButtonId);
	else if (mir_strcmpi(key, "hContact") == 0)
		lua_pushinteger(L, bcd->hContact);
	else if (mir_strcmpi(key, "Flags") == 0)
		lua_pushinteger(L, bcd->flags);
	else
		lua_pushnil(L);

	return 1;
}

static luaL_Reg bcdMeta[] =
{
	{ "__call", bcd__call },
	{ "__index", bcd__index },

	{ NULL, NULL }
};

/***********************************************/

extern "C" LUAMOD_API int luaopen_m_msg_buttonsbar(lua_State *L)
{
	luaL_newlib(L, msgbuttinsbarApi);

	luaL_newmetatable(L, MT_CUSTOMBUTTONCLICKDATA);
	luaL_setfuncs(L, bcdMeta, 0);
	lua_pop(L, 1);

	return 1;
}
