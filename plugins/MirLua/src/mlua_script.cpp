#include "stdafx.h"

#define MT_SCRIPT "SCRIPT"

CMLuaScript::CMLuaScript(lua_State *L, const wchar_t *path)
	: CMLuaEnviroment(L), status(None), unloadRef(LUA_NOREF)
{
	mir_wstrcpy(filePath, path);

	fileName = wcsrchr(filePath, '\\') + 1;
	wchar_t *dot = wcsrchr(fileName, '.');

	size_t length = mir_wstrlen(fileName) - mir_wstrlen(dot) + 1;

	ptrW name((wchar_t*)mir_calloc(sizeof(wchar_t) * (length + 1)));
	mir_wstrncpy(name, fileName, length);

	moduleName = mir_utf8encodeW(name);
}

CMLuaScript::CMLuaScript(const CMLuaScript &script)
	: CMLuaEnviroment(L), status(None), unloadRef(LUA_NOREF)
{
	mir_wstrcpy(filePath, script.filePath);
	fileName = mir_wstrdup(script.fileName);
	moduleName = mir_strdup(script.moduleName);
}

CMLuaScript::~CMLuaScript()
{
	if (status == Loaded)
	{
		lua_rawgeti(L, LUA_REGISTRYINDEX, unloadRef);
		if (lua_isfunction(L, -1))
			luaM_pcall(L);
		lua_pushnil(L);
		lua_rawsetp(L, LUA_REGISTRYINDEX, this);
		status = None;
	}

	luaL_getsubtable(L, LUA_REGISTRYINDEX, "_LOADED");
	lua_pushnil(L);
	lua_setfield(L, -2, moduleName);
	lua_pop(L, 1);

	mir_free(moduleName);
}

const char* CMLuaScript::GetModuleName() const
{
	return moduleName;
}

const wchar_t* CMLuaScript::GetFilePath() const
{
	return filePath;
}

const wchar_t* CMLuaScript::GetFileName() const
{
	return fileName;
}

CMLuaScript::Status CMLuaScript::GetStatus() const
{
	return status;
}

bool CMLuaScript::Load()
{
	status = Failed;

	if (luaL_loadfile(L, _T2A(filePath))) {
		ReportError(L);
		return false;
	}

	if (!CMLuaEnviroment::Load()) {
		ReportError(L);
		return false;
	}

	status = Loaded;

	if (lua_isnoneornil(L, -1))
		return true;

	luaL_getsubtable(L, LUA_REGISTRYINDEX, "_LOADED");
	lua_getfield(L, -1, moduleName);
	if (!lua_toboolean(L, -1)) {
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
	if (lua_isfunction(L, -1)) {
		lua_pushvalue(L, -1);
		unloadRef = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	lua_pop(L, 1);

	lua_pop(L, 1);

	return true;
}