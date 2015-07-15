#include "stdafx.h"

static int lua_AddIcon(lua_State *L)
{
	const char* name = luaL_checkstring(L, 1);
	const char* description = luaL_checkstring(L, 2);
	const char* section = luaL_optstring(L, 3, MODULE);

	TCHAR filePath[MAX_PATH];
	GetModuleFileName(g_hInstance, filePath, _countof(filePath));

	SKINICONDESC si = { 0 };
	si.flags = SIDF_ALL_TCHAR;
	si.pszName = ptrA(mir_utf8decodeA(name));
	si.description.t = ptrT(mir_utf8decodeT(description));
	si.section.t = ptrT(mir_utf8decodeT(section));
	si.defaultFile.t = filePath;
	si.iDefaultIndex = -IDI_ICON;

	HANDLE res = ::IcoLib_AddIcon(&si);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_GetIcon(lua_State *L)
{
	const char* name = luaL_checkstring(L, 1);

	HANDLE res = ::IcoLib_GetIconHandle(name);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_RemoveIcon(lua_State *L)
{
	INT_PTR res = 0;
	
	if (lua_isuserdata(L, 1))
		::IcoLib_RemoveIconByHandle(lua_touserdata(L, 1));
	else if (lua_isstring(L, 1))
		::IcoLib_RemoveIcon(luaL_checkstring(L, 1));
	else
		res = 1;

	lua_pushinteger(L, res);

	return 1;
}

static luaL_Reg icolibApi[] =
{
	{ "AddIcon", lua_AddIcon },
	{ "GetIcon", lua_GetIcon },
	{ "RemoveIcon", lua_RemoveIcon },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_icolib(lua_State *L)
{
	luaL_newlib(L, icolibApi);

	return 1;
}
