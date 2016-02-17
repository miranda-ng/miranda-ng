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

static luaL_Reg msgbuttinsbarApi[] =
{
	{ "AddButton", lua_AddButton },
	{ "ModifyButton", lua_ModifyButton },
	{ "RemoveButton", lua_RemoveButton },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_msg_buttonsbar(lua_State *L)
{
	luaL_newlib(L, msgbuttinsbarApi);

	MT<CustomButtonClickData>(L, "CustomButtonClickData")
		.Field(&CustomButtonClickData::pszModule, "Module", LUA_TSTRINGA)
		.Field(&CustomButtonClickData::dwButtonId, "ButtonID", LUA_TINTEGER)
		.Field(&CustomButtonClickData::hContact, "hContact", LUA_TINTEGER)
		.Field(&CustomButtonClickData::flags, "Flags", LUA_TINTEGER);
		lua_pop(L, 1);

	return 1;
}
