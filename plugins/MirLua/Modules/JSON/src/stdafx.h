#pragma once

#define LUA_LIBRARY_EXPORT(x) EXTERN_C int __declspec(dllexport) luaopen_##x(lua_State* L)


#include <Windows.h>
#include <lua.hpp>
#include <m_system_cpp.h>
//#include "..\..\..\src\mlua_metatable.h"

#include <m_core.h>
#include <m_json.h>
#include <m_string.h>

struct MT
{
	JSONNode *node;
	bool bDelete;

	MT(JSONNode &refNode, bool bCopy = false) : node(bCopy ? json_copy(&refNode) : &refNode), bDelete(bCopy) {}
	MT(JSONNode *n, bool bD = true) : node(n), bDelete(bD) {}
	~MT()
	{
		if (bDelete) json_delete(node);
	}

	__inline void* operator new(size_t size, lua_State *L)
	{
		 return lua_newuserdata(L, size);
	}
};

extern const luaL_Reg jsonApi[];
#define MT_JSON "JSON_METATABLE"