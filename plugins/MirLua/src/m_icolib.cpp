#include "stdafx.h"

static int lua_AddIcon(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	ptrT description(mir_utf8decodeT(luaL_checkstring(L, 2)));
	ptrT section(mir_utf8decodeT(luaL_optstring(L, 3, MODULE)));

	TCHAR filePath[MAX_PATH];
	GetModuleFileName(g_hInstance, filePath, _countof(filePath));

	SKINICONDESC si = { 0 };
	si.flags = SIDF_ALL_TCHAR;
	si.pszName = mir_utf8decodeA(name);
	si.description.t = description;
	si.section.t = section;
	si.defaultFile.t = filePath;
	si.hDefaultIcon = GetIcon(IDI_SCRIPT);

	HANDLE res = ::IcoLib_AddIcon(&si, hScriptsLangpack);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_GetIcon(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	HANDLE res = ::IcoLib_GetIconHandle(name);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_RemoveIcon(lua_State *L)
{
	if (lua_isuserdata(L, 1))
		::IcoLib_RemoveIconByHandle(lua_touserdata(L, 1));
	else if (lua_isstring(L, 1))
		::IcoLib_RemoveIcon(luaL_checkstring(L, 1));

	return 0;
}

static luaL_Reg icolibApi[] =
{
	{ "AddIcon", lua_AddIcon },
	{ "Add", lua_AddIcon },
	{ "GetIcon", lua_GetIcon },
	{ "Get", lua_GetIcon },
	{ "RemoveIcon", lua_RemoveIcon },
	{ "Remove", lua_RemoveIcon },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_icolib(lua_State *L)
{
	luaL_newlib(L, icolibApi);

	return 1;
}
