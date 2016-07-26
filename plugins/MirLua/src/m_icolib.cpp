#include "stdafx.h"

static int icolib_AddIcon(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);
	ptrT description(mir_utf8decodeT(luaL_checkstring(L, 2)));
	ptrT section(mir_utf8decodeT(luaL_optstring(L, 3, MODULE)));
	ptrT filePath(mir_utf8decodeT(lua_tostring(L, 4)));

	if (filePath == NULL)
	{
		filePath = (wchar_t*)mir_calloc(MAX_PATH + 1);
		GetModuleFileName(g_hInstance, filePath, MAX_PATH);
	}

	SKINICONDESC si = { 0 };
	si.flags = SIDF_ALL_TCHAR;
	si.pszName = mir_utf8decodeA(name);
	si.description.w = description;
	si.section.w = section;
	si.defaultFile.w = filePath;
	si.hDefaultIcon = GetIcon(IDI_SCRIPT);

	int hScriptLangpack = CMLuaScript::GetScriptIdFromEnviroment(L);

	HANDLE res = IcoLib_AddIcon(&si, hScriptLangpack);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int icolib_GetIcon(lua_State *L)
{
	const char *name = luaL_checkstring(L, 1);

	HANDLE res = IcoLib_GetIconHandle(name);
	lua_pushlightuserdata(L, res);

	return 1;
}

static int icolib_RemoveIcon(lua_State *L)
{
	if (lua_isuserdata(L, 1))
		IcoLib_RemoveIconByHandle(lua_touserdata(L, 1));
	else if (lua_isstring(L, 1))
		IcoLib_RemoveIcon(luaL_checkstring(L, 1));

	return 0;
}

static luaL_Reg icolibApi[] =
{
	{ "AddIcon", icolib_AddIcon },
	{ "GetIcon", icolib_GetIcon },
	{ "RemoveIcon", icolib_RemoveIcon },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_icolib(lua_State *L)
{
	luaL_newlib(L, icolibApi);

	return 1;
}
