#include "../stdafx.h"

#define MT_BBBUTTON "BBButton"

EXTERN_C MIR_APP_DLL(HANDLE) Srmm_AddButton(const BBButton *, HPLUGIN);

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

static void CleanBBButton(BBButton &bbb)
{
	mir_free((void*)bbb.pszModuleName);
	mir_free((void*)bbb.pwszText);
	mir_free((void*)bbb.pwszTooltip);
}

/***********************************************/

static int lua_AddButton(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	BBButton bbb = {};
	MakeBBButton(L, bbb);

	HANDLE res = Srmm_AddButton(&bbb, CMLuaEnvironment::GetEnvironmentId(L));
	CleanBBButton(bbb);
	
	if (!res) {
		lua_pushnil(L);
		return 1;
	}

	lua_pushvalue(L, 1);
	luaL_setmetatable(L, MT_BBBUTTON);

	return 1;
}

static int lua_ModifyButton(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);
	
	lua_pushvalue(L, 1);

	BBButton bbb = {};
	MakeBBButton(L, bbb);
	INT_PTR res = Srmm_ModifyButton(&bbb);
	CleanBBButton(bbb);
	lua_pushboolean(L, !res);

	return 2;
}

static int lua_RemoveButton(lua_State *L)
{
	BBButton bbb = {};

	switch (lua_type(L, 1)) {
	case LUA_TSTRING:
		bbb.pszModuleName = mir_utf8decodeA(lua_tostring(L, 1));
		bbb.dwButtonID = luaL_checkinteger(L, 2);
		break;
	case LUA_TTABLE:
		MakeBBButton(L, bbb);
		break;
	default:
		luaL_argerror(L, 1, luaL_typename(L, 1));
	}

	INT_PTR res = Srmm_RemoveButton(&bbb);
	CleanBBButton(bbb);
	lua_pushboolean(L, !res);

	return 1;
}

static luaL_Reg srmmApi[] =
{
	{ "AddButton", lua_AddButton },
	{ "ModifyButton", lua_ModifyButton },
	{ "RemoveButton", lua_RemoveButton },

	{ nullptr, nullptr }
};

/***********************************************/

LUAMOD_API int luaopen_m_srmm(lua_State *L)
{
	luaL_newlib(L, srmmApi);

	luaL_newmetatable(L, MT_BBBUTTON);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushliteral(L, MT_BBBUTTON);
	lua_setfield(L, -2, "__metatable");
	lua_pushcfunction(L, lua_ModifyButton);
	lua_setfield(L, -2, "Modify");
	lua_pushcfunction(L, lua_RemoveButton);
	lua_setfield(L, -2, "Remove");
	lua_pop(L, 1);

	MT<CustomButtonClickData>(L, "CustomButtonClickData")
		.Field(&CustomButtonClickData::pszModule, "Module", LUA_TSTRINGA)
		.Field(&CustomButtonClickData::dwButtonId, "ButtonId", LUA_TINTEGER)
		.Field(&CustomButtonClickData::hwndFrom, "hWnd", LUA_TLIGHTUSERDATA)
		.Field(&CustomButtonClickData::hContact, "hContact", LUA_TINTEGER)
		.Field(&CustomButtonClickData::flags, "Flags", LUA_TINTEGER);

	return 1;
}
