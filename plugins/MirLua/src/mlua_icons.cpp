#include "stdafx.h"

static int lua_AddIcon(lua_State *L)
{
	TCHAR filePath[MAX_PATH];
	GetModuleFileName(g_hInstance, filePath, SIZEOF(filePath));

	char iconName[MAX_PATH];
	mir_snprintf(iconName, SIZEOF(iconName), "%s_%s", MODULE, luaL_checkstring(L, 1));

	SKINICONDESC si = { 0 };
	si.flags = SIDF_PATH_TCHAR;
	si.pszName = iconName;
	si.description.a = (char*)lua_tostring(L, 2);
	si.section.a = lua_isnone(L, 3) ? "MirLua" : (char*)lua_tostring(L, 3);
	si.defaultFile.t = filePath;
	si.iDefaultIndex = -IDI_ICON;

	HANDLE res = ::Skin_AddIcon(&si);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_GetIcon(lua_State *L)
{
	char iconName[MAX_PATH];
	mir_snprintf(iconName, SIZEOF(iconName), "%s_%s", MODULE, luaL_checkstring(L, 1));

	HANDLE res = ::Skin_GetIconHandle(iconName);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int lua_RemoveIcon(lua_State *L)
{
	INT_PTR res = 0;
	
	if (lua_isuserdata(L, 1))
		res = ::CallService(MS_SKIN2_REMOVEICON, (WPARAM)lua_touserdata(L, 1), 0);
	else if (lua_isstring(L, 1))
	{
		char iconName[MAX_PATH];
		mir_snprintf(iconName, SIZEOF(iconName), "%s_%s", MODULE, lua_tostring(L, 1));
		res = ::CallService(MS_SKIN2_REMOVEICON, 0, (LPARAM)iconName);
	}
	else
		res = 1;

	lua_pushinteger(L, res);

	return 1;
}

static luaL_Reg iconsLib[] =
{
	{ "AddIcon", lua_AddIcon },
	{ "GetIcon", lua_GetIcon },
	{ "RemoveIcon", lua_RemoveIcon },

	{ NULL, NULL }
};

int luaopen_m_icons(lua_State *L)
{
	lua_getglobal(L, "M");
	luaL_checktype(L, -1, LUA_TTABLE);

	lua_newtable(L);
	luaL_setfuncs(L, iconsLib, 0);
	lua_setfield(L, -2, "Icons");

	lua_pop(L, 1);

	return 1;
}
