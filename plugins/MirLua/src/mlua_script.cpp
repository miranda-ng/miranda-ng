#include "stdafx.h"

CMLuaScript::CMLuaScript(lua_State *L, const TCHAR *path)
	: L(L), status(None), unloadRef(0)
{
	mir_tstrcpy(filePath, path);

	fileName = _tcsrchr(filePath, '\\') + 1;
	TCHAR *dot = _tcsrchr(fileName, '.');

	size_t length = mir_tstrlen(fileName) - mir_tstrlen(dot) + 1;

	ptrT name((TCHAR*)mir_calloc(sizeof(TCHAR) * (length + 1)));
	mir_tstrncpy(name, fileName, length);

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

const CMLuaScript::Status CMLuaScript::GetStatus() const
{
	return status;
}

bool CMLuaScript::Load()
{
	if (luaL_loadfile(L, T2Utf(filePath)))
	{
		Log(lua_tostring(L, -1));
		return false;
	}

	if (luaM_pcall(L, 0, 1))
		return false;

	if (lua_isnoneornil(L, -1))
		return true;

	luaL_getsubtable(L, LUA_REGISTRYINDEX, "_LOADED");
	lua_getfield(L, -1, moduleName);
	if (!lua_toboolean(L, -1))
	{
		lua_pop(L, 1);
		lua_pushvalue(L, -2);
		lua_setfield(L, -2, moduleName);
		lua_pop(L, 1);
	}
	else
		lua_remove(L, -2);

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

	lua_pop(L, 1);

	return true;
}

void CMLuaScript::Unload()
{
	if (status == Loaded && unloadRef)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, unloadRef);
		if (lua_isfunction(L, -1))
			luaM_pcall(L);
		luaL_unref(L, LUA_REGISTRYINDEX, unloadRef);
		status == None;
	}

	luaL_getsubtable(L, LUA_REGISTRYINDEX, "_LOADED");
	lua_pushnil(L);
	lua_setfield(L, -2, moduleName);
	lua_pop(L, 1);

	lua_pushnil(L);
	lua_setglobal(L, moduleName);
}