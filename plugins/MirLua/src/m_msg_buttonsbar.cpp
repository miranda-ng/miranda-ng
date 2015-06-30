#include "stdafx.h"

static BBButton* MakeBBButton(lua_State *L)
{
	BBButton *tbb = (BBButton*)mir_calloc(sizeof(BBButton));
	tbb->cbSize = sizeof(BBButton);
	tbb->dwDefPos = 100;

	lua_pushstring(L, "Module");
	lua_gettable(L, -2);
	tbb->pszModuleName = mir_utf8decode((char*)luaL_checkstring(L, -1), NULL);
	lua_pop(L, 1);

	lua_pushstring(L, "ButtonID");
	lua_gettable(L, -2);
	tbb->dwButtonID = luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	lua_pushstring(L, "Flags");
	lua_gettable(L, -2);
	tbb->bbbFlags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	if ((tbb->bbbFlags & BBBF_ANSITOOLTIP))
		tbb->bbbFlags &= ~BBBF_ANSITOOLTIP;

	lua_pushstring(L, "Tooltip");
	lua_gettable(L, -2);
	tbb->ptszTooltip = mir_utf8decodeT((char*)lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_pushstring(L, "Icon");
	lua_gettable(L, -2);
	tbb->hIcon = (HANDLE)lua_touserdata(L, -1);
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

	mir_ptr<BBButton> bbb(MakeBBButton(L));

	INT_PTR res = ::CallService(MS_BB_ADDBUTTON, 0, (LPARAM)bbb);
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

	mir_ptr<BBButton> bbb(MakeBBButton(L));

	INT_PTR res = ::CallService(MS_BB_MODIFYBUTTON, 0, (LPARAM)bbb);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_RemoveButton(lua_State *L)
{
	BBButton bbb = { sizeof(BBButton) };
	bbb.pszModuleName = mir_utf8decode((char*)luaL_checkstring(L, 1), NULL);
	bbb.dwButtonID = luaL_checkinteger(L, 2);

	INT_PTR res = ::CallService(MS_BB_REMOVEBUTTON, 0, (LPARAM)&bbb);
	lua_pushinteger(L, res);

	mir_free(bbb.pszModuleName);

	return 1;
}

static int lua_OnMsgToolBarLoaded(lua_State *L)
{
	if (!lua_isfunction(L, 1))
	{
		lua_pushlightuserdata(L, NULL);
		return 1;
	}

	lua_pushvalue(L, 1);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	HANDLE res = ::HookEventObjParam(ME_MSG_TOOLBARLOADED, CMLua::HookEventObjParam, L, ref);
	lua_pushlightuserdata(L, res);

	Hooks.insert(res);
	HookRefs.insert(new HandleRefParam(L, res, ref));

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
	lua_pushstring(L, "Module");
	lua_pushstring(L, ptrA(mir_utf8encode(bcd->pszModule)));
	lua_settable(L, -3);
	lua_pushstring(L, "ButtonID");
	lua_pushinteger(L, bcd->dwButtonId);
	lua_settable(L, -3);
	lua_pushstring(L, "hContact");
	lua_pushinteger(L, bcd->hContact);
	lua_settable(L, -3);
	lua_pushstring(L, "Flags");
	lua_pushinteger(L, bcd->flags);
	lua_settable(L, -3);

	if (lua_pcall(L, 2, 1, 0))
		printf("%s\n", lua_tostring(L, -1));

	int res = (int)lua_tointeger(L, 1);

	return res;
}

static int lua_OnMsgToolBarButtonPressed(lua_State *L)
{
	if (!lua_isfunction(L, 1))
	{
		lua_pushlightuserdata(L, NULL);
		return 1;
	}

	lua_pushvalue(L, 1);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	HANDLE res = ::HookEventObjParam(ME_MSG_BUTTONPRESSED, ButtonPressedHookEventObjParam, L, ref);
	lua_pushlightuserdata(L, res);

	Hooks.insert(res);
	HookRefs.insert(new HandleRefParam(L, res, ref));

	return 1;
}

static luaL_Reg msgbuttinsbarApi[] =
{
	{ "AddButton", lua_AddButton },
	{ "ModifyButton", lua_ModifyButton },
	{ "RemoveButton", lua_RemoveButton },
	
	{ "OnMsgToolBarLoaded", lua_OnMsgToolBarLoaded },
	{ "OnMsgToolBarButtonPressed", lua_OnMsgToolBarButtonPressed },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_msg_buttonsbar(lua_State *L)
{
	luaL_newlib(L, msgbuttinsbarApi);

	return 1;
}
