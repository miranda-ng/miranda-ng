#include "stdafx.h"

#define MT_SCRIPT "SCRIPT"

CMLuaScript::CMLuaScript(lua_State *L, const wchar_t *path)
	: L(L), status(None), unloadRef(LUA_NOREF)
{
	mir_wstrcpy(filePath, path);

	fileName = wcsrchr(filePath, '\\') + 1;
	wchar_t *dot = wcsrchr(fileName, '.');

	size_t length = mir_wstrlen(fileName) - mir_wstrlen(dot) + 1;

	ptrW name((wchar_t*)mir_calloc(sizeof(wchar_t) * (length + 1)));
	mir_wstrncpy(name, fileName, length);

	moduleName = mir_utf8encodeW(name);

	MUUID muidLast = MIID_LAST;
	id = GetPluginLangId(muidLast, 0);
}

CMLuaScript::~CMLuaScript()
{
	mir_free(moduleName);
}

CMLuaScript* CMLuaScript::GetScriptFromEnviroment(lua_State *L)
{
	if (!luaM_getenv(L))
		return NULL;

	lua_rawgeti(L, -1, NULL);
	CMLuaScript *script = (CMLuaScript*)lua_touserdata(L, -1);
	lua_pop(L, 3);

	return script;
}

int CMLuaScript::GetScriptIdFromEnviroment(lua_State *L)
{
	CMLuaScript *script = GetScriptFromEnviroment(L);
	if (script != NULL)
		return script->GetId();

	return hMLuaLangpack;
}

int CMLuaScript::GetId() const
{
	return id;
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

const CMLuaScript::Status CMLuaScript::GetStatus() const
{
	return status;
}

bool CMLuaScript::Load()
{
	status = Failed;

	if (luaL_loadfile(L, _T2A(filePath)))
	{
		Log(lua_tostring(L, -1));
		return false;
	}

	lua_createtable(L, 1, 1);
	lua_pushlightuserdata(L, this);
	lua_rawseti(L, -2, NULL);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "_G");
	lua_createtable(L, 0, 2);
	lua_pushliteral(L, MT_SCRIPT);
	lua_setfield(L, -2, "__metatable");
	lua_getglobal(L, "_G");
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);
	lua_setupvalue(L, -2, 1);

	if (luaM_pcall(L, 0, 1))
		return false;

	status = Loaded;

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

	KillModuleIcons(id);
	KillModuleSounds(id);
	KillModuleMenus(id);
	KillModuleHotkeys(id);

	KillObjectEventHooks(this);
	KillObjectServices(this);
}