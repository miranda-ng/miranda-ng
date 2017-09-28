#include "stdafx.h"

#define MT_ENVIROMENT "ENVIROMENT"

CMLuaEnviroment::CMLuaEnviroment(lua_State *L)
	: L(L)
{
	MUUID muidLast = MIID_LAST;
	id = GetPluginLangId(muidLast, 0);
}

CMLuaEnviroment::~CMLuaEnviroment()
{
	KillModuleIcons(id);
	KillModuleSounds(id);
	KillModuleMenus(id);
	KillModuleHotkeys(id);

	KillObjectEventHooks(this);
	KillObjectServices(this);
}

CMLuaEnviroment* CMLuaEnviroment::GetEnviroment(lua_State *L)
{
	if (!luaM_getenv(L))
		return NULL;

	lua_rawgeti(L, -1, NULL);
	CMLuaEnviroment *env = (CMLuaEnviroment*)lua_touserdata(L, -1);
	lua_pop(L, 3);

	return env;
}

int CMLuaEnviroment::GetEnviromentId(lua_State *L)
{
	CMLuaEnviroment *script = GetEnviroment(L);
	return script != nullptr
		? script->GetId()
		: hMLuaLangpack;
}

int CMLuaEnviroment::GetId() const
{
	return id;
}

void CMLuaEnviroment::CreateEnviromentTable()
{
	lua_createtable(L, 1, 1);
	lua_pushlightuserdata(L, this);
	lua_rawseti(L, -2, NULL);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "_G");
	lua_createtable(L, 0, 2);
	lua_pushliteral(L, MT_ENVIROMENT);
	lua_setfield(L, -2, "__metatable");
	lua_getglobal(L, "_G");
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);
}

bool CMLuaEnviroment::Load()
{
	luaL_checktype(L, -1, LUA_TFUNCTION);

	CreateEnviromentTable();
	lua_setupvalue(L, -2, 1);

	return lua_pcall(L, 0, 1, 0) == LUA_OK;
}
