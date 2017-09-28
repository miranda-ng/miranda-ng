#include "stdafx.h"

static void MakeSKINICONDESC(lua_State *L, SKINICONDESC &sid)
{
	lua_getfield(L, -1, "Flags");
	sid.flags = lua_tointeger(L, -1);
	lua_pop(L, 1);

	if (!(sid.flags & SIDF_ALL_UNICODE))
		sid.flags |= SIDF_ALL_UNICODE;

	lua_getfield(L, -1, "Name");
	sid.pszName = mir_utf8decodeA(luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Description");
	sid.description.w = mir_utf8decodeW(luaL_checkstring(L, -1));
	lua_pop(L, 1);

	lua_getfield(L, -1, "Section");
	sid.section.w = mir_utf8decodeW(luaL_optstring(L, 3, MODULE));
	lua_pop(L, 1);

	lua_getfield(L, -1, "DefaultFile");
	sid.defaultFile.w = mir_utf8decodeW(lua_tostring(L, -1));
	lua_pop(L, 1);

	if (sid.defaultFile.w == NULL)
	{
		sid.defaultFile.w = (wchar_t*)mir_calloc(MAX_PATH + 1);
		GetModuleFileName(g_hInstance, sid.defaultFile.w, MAX_PATH);
	}

	lua_getfield(L, -1, "DefaultIndex");
	sid.iDefaultIndex = lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "DefaultIcon");
	sid.hDefaultIcon = (HICON)lua_touserdata(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "SizeX");
	sid.iDefaultIndex = lua_tointeger(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "SizeY");
	sid.iDefaultIndex = lua_tointeger(L, -1);
	lua_pop(L, 1);
}

static int lua_AddIcon(lua_State *L)
{
	SKINICONDESC sid;

	if (lua_type(L, 1) == LUA_TSTRING)
	{
		sid.flags = SIDF_ALL_UNICODE;
		sid.pszName = mir_utf8decodeA(luaL_checkstring(L, 1));
		sid.description.w = mir_utf8decodeW(luaL_checkstring(L, 2));
		sid.section.w = mir_utf8decodeW(luaL_optstring(L, 3, MODULE));
		sid.defaultFile.w = mir_utf8decodeW(lua_tostring(L, 4));
		sid.hDefaultIcon = GetIcon(IDI_SCRIPT);

		if (sid.defaultFile.w == NULL)
		{
			sid.defaultFile.w = (wchar_t*)mir_calloc(MAX_PATH + 1);
			GetModuleFileName(g_hInstance, sid.defaultFile.w, MAX_PATH);
		}
	}
	else if (lua_type(L, 1) == LUA_TTABLE)
		MakeSKINICONDESC(L, sid);
	else
		luaL_argerror(L, 1, luaL_typename(L, 1));

	int hScriptLangpack = CMLuaEnviroment::GetEnviromentId(L);
	HANDLE res = IcoLib_AddIcon(&sid, hScriptLangpack);
	lua_pushlightuserdata(L, res);

	mir_free((void*)sid.pszName);
	mir_free((void*)sid.description.w);
	mir_free((void*)sid.section.w);
	mir_free((void*)sid.defaultFile.w);

	return 1;
}

static int lua_GetIcon(lua_State *L)
{
	bool big = luaM_toboolean(L, 2);

	HICON hIcon = NULL;
	switch (lua_type(L, 1))
	{
	case LUA_TLIGHTUSERDATA:
		hIcon = IcoLib_GetIconByHandle(lua_touserdata(L, 1), big);
		break;
	case LUA_TSTRING:
		hIcon = IcoLib_GetIcon(lua_tostring(L, 1), big);
		break;
	default:
		luaL_argerror(L, 1, luaL_typename(L, 1));
	}

	if (hIcon)
		lua_pushlightuserdata(L, hIcon);
	else
		lua_pushnil(L);

	return 1;
}

static int lua_GetIconHandle(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	HANDLE res = IcoLib_GetIconHandle(name);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_RemoveIcon(lua_State *L)
{
	switch (lua_type(L, 1))
	{
	case LUA_TLIGHTUSERDATA:
		IcoLib_RemoveIconByHandle(lua_touserdata(L, 1));
		break;
	case LUA_TSTRING:
		IcoLib_RemoveIcon(luaL_checkstring(L, 1));
		break;
	default:
		luaL_argerror(L, 1, luaL_typename(L, 1));
	}

	return 0;
}

static luaL_Reg icolibApi[] =
{
	{ "AddIcon", lua_AddIcon },
	{ "GetIcon", lua_GetIcon },
	{ "GetHandle", lua_GetIconHandle },
	{ "GetIconHandle", lua_GetIconHandle },
	{ "RemoveIcon", lua_RemoveIcon },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_icolib(lua_State *L)
{
	luaL_newlib(L, icolibApi);

	return 1;
}
