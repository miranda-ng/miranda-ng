#include "stdafx.h"

CMLuaScript::CMLuaScript(lua_State *L, const TCHAR* path)
	: L(L), unloadRef(0)
{
	mir_tstrcpy(filePath, path);

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

bool CMLuaScript::Load()
{
	if (luaL_loadfile(L, T2Utf(filePath)))
	{
		CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)lua_tostring(L, -1));
		return false;
	}

	if (luaM_pcall(L, 0, 1))
		return false;

	isLoaded = true;

	if (!lua_istable(L, -1))
		return true;

	lua_getfield(L, -1, "Load");
	if (lua_isfunction(L, -1))
		luaM_pcall(L);
	else
		lua_pop(L, 1);

	lua_getfield(L, -1, "Unload");
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
	if (isLoaded && unloadRef)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, unloadRef);
		if (lua_isfunction(L, -1))
			luaM_pcall(L);
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