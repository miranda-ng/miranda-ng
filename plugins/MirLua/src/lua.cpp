#include "stdafx.h"

CMLua::CMLua() : L(nullptr)
{
}

CMLua::~CMLua()
{
	Log("Unloading lua engine");

	KillObjectEventHooks(L);
	KillObjectServices(L);

	lua_close(L);
}

lua_State* CMLua::GetState()
{
	return L;
}

void CMLua::Load()
{
	Log("Loading lua engine");
	L = luaL_newstate();
	Log("Loading standard modules");
	luaL_openlibs(L);

	lua_atpanic(L, luaM_atpanic);
}
