#include "stdafx.h"

CMLuaScript::CMLuaScript(lua_State *L, const TCHAR* path, int iGroup) : L(L)
{
	mir_tstrcpy(filePath, path);

	group = iGroup;

	fileName = _tcsrchr(filePath, '\\') + 1;
	size_t length = mir_tstrlen(fileName) - 3;

	ptrT name((TCHAR*)mir_calloc(sizeof(TCHAR) * length));
	mir_tstrncpy(name, fileName, mir_tstrlen(fileName) - 3);

	moduleName = mir_utf8encodeT(name);
}

CMLuaScript::~CMLuaScript()
{
	mir_free(this->moduleName);
}

const char* CMLuaScript::GetModuleName() const
{
	return moduleName;
}

const TCHAR* CMLuaScript::GetFilePath() const
{
	return filePath;
}

const TCHAR* CMLuaScript::GetFileName() const
{
	return fileName;
}

const int CMLuaScript::GetGroup() const
{
	return group;
}

bool CMLuaScript::Load()
{
	if (luaL_loadfile(L, T2Utf(filePath)))
	{
		CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)lua_tostring(L, -1));
		return false;
	}

	if (lua_pcall(L, 0, 1, 0))
	{
		CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)lua_tostring(L, -1));
		return false;
	}

	isLoaded = true;

	if (!lua_istable(L, -1))
		return true;

	lua_pushliteral(L, "Load");
	lua_gettable(L, -2);
	if (lua_isfunction(L, -1) && lua_pcall(L, 0, 0, 0))
		CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)lua_tostring(L, -1));
	lua_pop(L, 1);

	lua_pushliteral(L, "Unload");
	lua_gettable(L, -2);
	if (lua_isfunction(L, -1))
	{
		lua_pushvalue(L, -1);
		unloadRef = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	lua_pop(L, 1);

	return true;
}

void CMLuaScript::Unload()
{
	if (isLoaded)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, unloadRef);
		if (lua_pcall(L, 0, 0, 0))
			CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)lua_tostring(L, -1));
		luaL_unref(L, LUA_REGISTRYINDEX, unloadRef);
		isLoaded = false;
	}

	luaL_getsubtable(L, LUA_REGISTRYINDEX, "_LOADED");
	lua_pushnil(L);
	lua_setfield(L, -2, moduleName);
	lua_pop(L, 1);

	lua_pushnil(L);
	lua_setglobal(L, moduleName);
}