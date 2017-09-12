#include "stdafx.h"

static void MakeBBButton(lua_State *L, BBButton &bbb)
{
	bbb.dwDefPos = 100;

	lua_getfield(L, -1, "Module");
	bbb.pszModuleName = mir_utf8decodeA(luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "ButtonId");
	bbb.dwButtonID = luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Flags");
	bbb.bbbFlags = luaL_optinteger(L, -1, BBBF_ISIMBUTTON);
	lua_pop(L, 1);

	lua_getfield(L, -1, "Text");
	bbb.pwszText = mir_utf8decodeW(lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Tooltip");
	bbb.pwszTooltip = mir_utf8decodeW(lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Icon");
	bbb.hIcon = (HANDLE)lua_touserdata(L, -1);
	lua_pop(L, 1);
}

static int lua_AddButton(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	BBButton bbb;
	MakeBBButton(L, bbb);

	int hScriptLangpack = CMLuaScript::GetScriptIdFromEnviroment(L);

	INT_PTR res = Srmm_AddButton(&bbb, hScriptLangpack);
	lua_pushboolean(L, res == 0);

	mir_free((void*)bbb.pszModuleName);
	mir_free((void*)bbb.pwszText);
	mir_free((void*)bbb.pwszTooltip);

	return 1;
}

static int lua_ModifyButton(lua_State *L)
{
	if (lua_type(L, 1) != LUA_TTABLE)
	{
		lua_pushlightuserdata(L, 0);
		return 1;
	}

	BBButton bbb;
	MakeBBButton(L, bbb);

	INT_PTR res = Srmm_ModifyButton(&bbb);
	lua_pushinteger(L, res);

	mir_free((void*)bbb.pszModuleName);
	mir_free((void*)bbb.pwszText);
	mir_free((void*)bbb.pwszTooltip);

	return 1;
}

static int lua_RemoveButton(lua_State *L)
{
	ptrA szModuleName(mir_utf8decodeA(luaL_checkstring(L, 1)));

	BBButton mbb = {};
	mbb.pszModuleName = szModuleName;
	mbb.dwButtonID = luaL_checkinteger(L, 2);

	INT_PTR res = Srmm_RemoveButton(&mbb);
	lua_pushinteger(L, res);

	return 1;
}

static luaL_Reg srmmApi[] =
{
	{ "AddButton", lua_AddButton },
	{ "ModifyButton", lua_ModifyButton },
	{ "RemoveButton", lua_RemoveButton },

	{ NULL, NULL }
};

/***********************************************/

LUAMOD_API int luaopen_m_srmm(lua_State *L)
{
	luaL_newlib(L, srmmApi);

	MT<CustomButtonClickData>(L, "CustomButtonClickData")
		.Field(&CustomButtonClickData::pszModule, "Module", LUA_TSTRINGA)
		.Field(&CustomButtonClickData::dwButtonId, "ButtonId", LUA_TINTEGER)
		.Field(&CustomButtonClickData::hContact, "hContact", LUA_TINTEGER)
		.Field(&CustomButtonClickData::flags, "Flags", LUA_TINTEGER);

	return 1;
}
