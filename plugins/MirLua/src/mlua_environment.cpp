#include "stdafx.h"

#define MT_ENVIRONMENT "ENVIRONMENT"

CMLuaEnvironment::CMLuaEnvironment(lua_State *L)
	: L(L)
{
	MUUID muidLast = MIID_LAST;
	id = GetPluginLangId(muidLast, 0);
}

CMLuaEnvironment::~CMLuaEnvironment()
{
	KillModuleIcons(id);
	KillModuleSounds(id);
	KillModuleMenus(id);
	KillModuleHotkeys(id);

	KillObjectEventHooks(this);
	KillObjectServices(this);
}

CMLuaEnvironment* CMLuaEnvironment::GetEnvironment(lua_State *L)
{
	if (!luaM_getenv(L))
		return nullptr;

	lua_rawgeti(L, -1, NULL);
	CMLuaEnvironment *env = (CMLuaEnvironment*)lua_touserdata(L, -1);
	lua_pop(L, 3);

	return env;
}

int CMLuaEnvironment::GetEnvironmentId(lua_State *L)
{
	CMLuaEnvironment *script = GetEnvironment(L);
	return script != nullptr
		? script->GetId()
		: hMLuaLangpack;
}

int CMLuaEnvironment::GetId() const
{
	return id;
}

void CMLuaEnvironment::CreateEnvironmentTable()
{
	lua_createtable(L, 1, 1);
	lua_pushlightuserdata(L, this);
	lua_rawseti(L, -2, NULL);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "_G");
	lua_createtable(L, 0, 2);
	lua_pushliteral(L, MT_ENVIRONMENT);
	lua_setfield(L, -2, "__metatable");
	lua_getglobal(L, "_G");
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);
}

bool CMLuaEnvironment::Load()
{
	luaL_checktype(L, -1, LUA_TFUNCTION);

	CreateEnvironmentTable();
	lua_setupvalue(L, -2, 1);

	return lua_pcall(L, 0, 1, 0) == LUA_OK;
}
