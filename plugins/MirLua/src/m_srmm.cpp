#include "stdafx.h"

#define MT_BBBUTTON "BBButton"

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

/***********************************************/

static int lua_AddButton(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TTABLE);

	BBButton *bbb = (BBButton*)mir_calloc(sizeof(BBButton));
	MakeBBButton(L, *bbb);

	int hScriptLangpack = CMLuaScript::GetScriptIdFromEnviroment(L);
	if (Srmm_AddButton(bbb, hScriptLangpack))
	{
		lua_pushnil(L);
		return 1;
	}
	
	MT<BBButton>::Apply(L, bbb);

	return 1;
}

static int lua_ModifyButton(lua_State *L)
{
	switch (lua_type(L, 1))
	{
	case LUA_TTABLE:
	{
		BBButton bbb;
		MakeBBButton(L, bbb);
		INT_PTR res = Srmm_ModifyButton(&bbb);
		mir_free((void*)bbb.pszModuleName);
		mir_free((void*)bbb.pwszText);
		mir_free((void*)bbb.pwszTooltip);
		lua_pushboolean(L, !res);
		break;
	}
	case LUA_TUSERDATA:
	{
		BBButton *bbb = *(BBButton**)luaL_checkudata(L, 1, MT_BBBUTTON);
		luaL_checktype(L, 2, LUA_TTABLE);
		{
			if (lua_getfield(L, -1, "Text"))
			{
				mir_free((void*)bbb->pwszText);
				bbb->pwszText = mir_utf8decodeW(lua_tostring(L, -1));
			}
			lua_pop(L, 1);

			if (lua_getfield(L, -1, "Tooltip"))
			{
				mir_free((void*)bbb->pwszTooltip);
				bbb->pwszTooltip = mir_utf8decodeW(lua_tostring(L, -1));
			}
			lua_pop(L, 1);

			if (lua_getfield(L, -1, "Flags"))
				bbb->bbbFlags = luaL_optinteger(L, -1, BBBF_ISIMBUTTON);
			lua_pop(L, 1);

			if (lua_getfield(L, -1, "Icon"))
				bbb->hIcon = (HANDLE)lua_touserdata(L, -1);
			lua_pop(L, 1);
		}
		INT_PTR res = Srmm_ModifyButton(bbb);
		lua_pushvalue(L, 1);
		break;
	}
	default:
		luaL_argerror(L, 1, luaL_typename(L, 1));
	}

	return 1;
}

static int lua_RemoveButton(lua_State *L)
{
	switch (lua_type(L, 1))
	{
	case LUA_TSTRING:
	{
		BBButton bbb;
		bbb.pszModuleName = mir_utf8decodeA(lua_tostring(L, 1));
		bbb.dwButtonID = luaL_checkinteger(L, 2);
		INT_PTR res = Srmm_RemoveButton(&bbb);
		mir_free((void*)bbb.pszModuleName);
		lua_pushboolean(L, !res);
		break;
	}
	case LUA_TTABLE:
	{
		BBButton bbb;
		bbb.pszModuleName = mir_utf8decodeA(luaL_checkstring(L, 1));
		bbb.dwButtonID = luaL_checkinteger(L, 2);
		INT_PTR res = Srmm_RemoveButton(&bbb);
		mir_free((void*)bbb.pszModuleName);
		lua_pushboolean(L, !res);
		break;
	}
	case LUA_TUSERDATA:
	{
		BBButton *bbb = *(BBButton**)luaL_checkudata(L, 1, MT_BBBUTTON);
		INT_PTR res = Srmm_RemoveButton(bbb);
		lua_pushboolean(L, !res);
		break;
	}
	default:
		luaL_argerror(L, 1, luaL_typename(L, 1));
	}

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

template <>
void MT<BBButton>::Free(lua_State*, BBButton **bbb)
{
	mir_free((void*)(*bbb)->pszModuleName);
	mir_free((void*)(*bbb)->pwszText);
	mir_free((void*)(*bbb)->pwszTooltip);
	mir_free(*bbb);
}

/***********************************************/

LUAMOD_API int luaopen_m_srmm(lua_State *L)
{
	luaL_newlib(L, srmmApi);

	MT<BBButton>(L, MT_BBBUTTON)
		.Field(&BBButton::pszModuleName, "Module", LUA_TSTRINGA)
		.Field(&BBButton::dwButtonID, "ButtonId", LUA_TINTEGER)
		.Field(&BBButton::pwszText, "Text", LUA_TSTRINGW)
		.Field(&BBButton::pwszTooltip, "Tooltip", LUA_TSTRINGW)
		.Field(&BBButton::bbbFlags, "Flags", LUA_TINTEGER)
		.Field(&BBButton::hIcon, "Icon", LUA_TLIGHTUSERDATA)
		.Field(lua_ModifyButton, "Modify")
		.Field(lua_RemoveButton, "Remove");

	MT<CustomButtonClickData>(L, "CustomButtonClickData")
		.Field(&CustomButtonClickData::pszModule, "Module", LUA_TSTRINGA)
		.Field(&CustomButtonClickData::dwButtonId, "ButtonId", LUA_TINTEGER)
		.Field(&CustomButtonClickData::hContact, "hContact", LUA_TINTEGER)
		.Field(&CustomButtonClickData::flags, "Flags", LUA_TINTEGER);

	return 1;
}
