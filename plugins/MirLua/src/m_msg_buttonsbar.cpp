#include "stdafx.h"

static int CompareMBButtons(const BBButton* p1, const BBButton* p2)
{
	if (int res = mir_strcmpi(p1->pszModuleName, p2->pszModuleName))
		return res;
	return p1->dwButtonID - p2->dwButtonID;
}

static LIST<BBButton> MBButtons(1, CompareMBButtons);

void KillModuleMBButtons()
{
	while (MBButtons.getCount())
	{
		BBButton* bbb = MBButtons[0];
		::CallService(MS_BB_REMOVEBUTTON, 0, (LPARAM)bbb);
		MBButtons.remove(0);
		mir_free(bbb->pszModuleName);
		mir_free(bbb->ptszTooltip);
		mir_free(bbb);
	}
}

static BBButton* MakeBBButton(lua_State *L)
{
	BBButton *bbb = (BBButton*)mir_calloc(sizeof(BBButton));
	bbb->cbSize = sizeof(BBButton);
	bbb->dwDefPos = 100;

	lua_pushliteral(L, "Module");
	lua_gettable(L, -2);
	bbb->pszModuleName = mir_utf8decodeA(luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_pushliteral(L, "ButtonID");
	lua_gettable(L, -2);
	bbb->dwButtonID = luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	lua_pushliteral(L, "Flags");
	lua_gettable(L, -2);
	bbb->bbbFlags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	if ((bbb->bbbFlags & BBBF_ANSITOOLTIP))
		bbb->bbbFlags &= ~BBBF_ANSITOOLTIP;

	lua_pushliteral(L, "Tooltip");
	lua_gettable(L, -2);
	bbb->ptszTooltip = mir_utf8decodeT(lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_pushliteral(L, "Icon");
	lua_gettable(L, -2);
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

	INT_PTR res = ::CallService(MS_BB_ADDBUTTON, 0, (LPARAM)bbb);
	lua_pushinteger(L, res);

	if (!res)
		MBButtons.insert(bbb);

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

	INT_PTR res = ::CallService(MS_BB_MODIFYBUTTON, 0, (LPARAM)bbb);
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

	if (!res)
	{
		BBButton* bbb = MBButtons.find(&mbb);
		if (bbb)
		{
			MBButtons.remove(bbb);
			mir_free(bbb->pszModuleName);
			mir_free(bbb->ptszTooltip);
			mir_free(bbb);
		}
	}

	return 1;
}

int ButtonPressedHookEventObjParam(void *obj, WPARAM wParam, LPARAM lParam, LPARAM param)
{
	lua_State *L = (lua_State*)obj;

	int ref = param;
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

	lua_pushnumber(L, wParam);

	CustomButtonClickData *bcd = (CustomButtonClickData*)lParam;

	lua_newtable(L);
	lua_pushliteral(L, "Module");
	lua_pushstring(L, ptrA(mir_utf8encode(bcd->pszModule)));
	lua_settable(L, -3);
	lua_pushliteral(L, "ButtonID");
	lua_pushinteger(L, bcd->dwButtonId);
	lua_settable(L, -3);
	lua_pushliteral(L, "hContact");
	lua_pushinteger(L, bcd->hContact);
	lua_settable(L, -3);
	lua_pushliteral(L, "Flags");
	lua_pushinteger(L, bcd->flags);
	lua_settable(L, -3);

	if (lua_pcall(L, 2, 1, 0))
		printf("%s\n", lua_tostring(L, -1));

	int res = (int)lua_tointeger(L, 1);

	return res;
}

static int lua_OnMsgToolBarButtonPressed(lua_State *L)
{
	ObsoleteMethod(L, "Use m.HookEvent instead");

	if (!lua_isfunction(L, 1))
	{
		lua_pushlightuserdata(L, NULL);
		return 1;
	}

	lua_pushvalue(L, 1);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	HANDLE res = ::HookEventObjParam(ME_MSG_BUTTONPRESSED, ButtonPressedHookEventObjParam, L, ref);
	lua_pushlightuserdata(L, res);

	CMLua::Hooks.insert(res);
	CMLua::HookRefs.insert(new HandleRefParam(L, res, ref));

	return 1;
}

static int lua_DecodeCustomButtonClickData(lua_State *L)
{
	CustomButtonClickData *bcd = (CustomButtonClickData*)lua_tointeger(L, 1);

	lua_newtable(L);
	lua_pushliteral(L, "Module");
	lua_pushstring(L, ptrA(mir_utf8encode(bcd->pszModule)));
	lua_settable(L, -3);
	lua_pushliteral(L, "ButtonID");
	lua_pushinteger(L, bcd->dwButtonId);
	lua_settable(L, -3);
	lua_pushliteral(L, "hContact");
	lua_pushinteger(L, bcd->hContact);
	lua_settable(L, -3);
	lua_pushliteral(L, "Flags");
	lua_pushinteger(L, bcd->flags);
	lua_settable(L, -3);

	return 1;
}

static luaL_Reg msgbuttinsbarApi[] =
{
	{ "AddButton", lua_AddButton },
	{ "ModifyButton", lua_ModifyButton },
	{ "RemoveButton", lua_RemoveButton },

	{ "OnMsgToolBarButtonPressed", lua_OnMsgToolBarButtonPressed },
	{ "DecodeCustomButtonClickData", lua_DecodeCustomButtonClickData },

	{ NULL, NULL }
};

#define MT_CUSTOMBUTTONCLICKDATA "CustomButtonClickData"

static int bcd__init(lua_State *L)
{
	CustomButtonClickData *udata = (CustomButtonClickData*)lua_touserdata(L, 1);
	if (udata == NULL)
	{
		lua_pushnil(L);
		return 1;
	}

	CustomButtonClickData **bcd = (CustomButtonClickData**)lua_newuserdata(L, sizeof(CustomButtonClickData*));
	*bcd = udata;

	luaL_setmetatable(L, MT_CUSTOMBUTTONCLICKDATA);

	return 1;
}

static int bcd__index(lua_State *L)
{
	CustomButtonClickData *bcd = *(CustomButtonClickData**)luaL_checkudata(L, 1, MT_CUSTOMBUTTONCLICKDATA);
	const char *key = lua_tostring(L, 2);

	if (!mir_strcmpi(key, "Module"))
		lua_pushstring(L, ptrA(mir_utf8encode(bcd->pszModule)));
	else if (!mir_strcmpi(key, "ButtonID"))
		lua_pushinteger(L, bcd->dwButtonId);
	else if (!mir_strcmpi(key, "hContact"))
		lua_pushinteger(L, bcd->hContact);
	else if (!mir_strcmpi(key, "Flags"))
		lua_pushinteger(L, bcd->flags);
	else
		lua_pushnil(L);

	return 1;
}

static luaL_Reg bcdMeta[] =
{
	{ "__init", bcd__init },
	{ "__index", bcd__index },
	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_msg_buttonsbar(lua_State *L)
{
	luaL_newlib(L, msgbuttinsbarApi);
	
	luaL_newmetatable(L, MT_CUSTOMBUTTONCLICKDATA);
	luaL_setfuncs(L, bcdMeta, 0);
	lua_pop(L, 1);

	return 1;
}
