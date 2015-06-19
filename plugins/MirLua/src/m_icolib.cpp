#include "stdafx.h"

static int lua_AddIcon(lua_State *L)
{
	TCHAR filePath[MAX_PATH];
	GetModuleFileName(g_hInstance, filePath, _countof(filePath));

	char iconName[MAX_PATH];
	mir_snprintf(iconName, _countof(iconName), "%s_%s", MODULE, luaL_checkstring(L, 1));

	SKINICONDESC si = { 0 };
	si.flags = SIDF_PATH_TCHAR;
	si.pszName = iconName;
	si.description.a = mir_utf8decode((char*)luaL_checkstring(L, 2), NULL);
	si.section.a = lua_isnone(L, 3) ? MODULE : mir_utf8decode((char*)luaL_checkstring(L, 3), NULL);
	si.defaultFile.t = filePath;
	si.iDefaultIndex = -IDI_ICON;

	HANDLE res = ::IcoLib_AddIcon(&si);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_GetIcon(lua_State *L)
{
	char iconName[MAX_PATH];
	mir_snprintf(iconName, _countof(iconName), "%s_%s", MODULE, luaL_checkstring(L, 1));

	HANDLE res = ::IcoLib_GetIconHandle(iconName);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_RemoveIcon(lua_State *L)
{
	INT_PTR res = 0;
	
	if (lua_isuserdata(L, 1))
		::IcoLib_RemoveIconByHandle(lua_touserdata(L, 1));
	else if (lua_isstring(L, 1))
	{
		char iconName[MAX_PATH];
		mir_snprintf(iconName, _countof(iconName), "%s_%s", MODULE, lua_tostring(L, 1));
		::IcoLib_RemoveIcon(iconName);
	}
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
