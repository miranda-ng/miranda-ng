#pragma once

#define LUA_LIBRARY_EXPORT(x) EXTERN_C int __declspec(dllexport) luaopen_##x(lua_State* L)


#include <Windows.h>
#include <lua.hpp>
#include <m_system_cpp.h>
//#include "..\..\..\src\mlua_metatable.h"

#include <m_core.h>
#include <m_json.h>

struct MT
{
	JSONNode *node;
	bool bDelete;
};

extern const luaL_Reg jsonApi[];
#define MT_JSON "JSON_METATABLE"