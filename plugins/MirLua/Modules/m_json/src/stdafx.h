#pragma once

#include <Windows.h>
#include <lua.hpp>
#include <m_system_cpp.h>
//#include "..\..\..\src\mlua_metatable.h"

#include <m_core.h>
#include <m_json.h>
#include <m_string.h>

struct JSON
{
	JSONNode *node;
	bool bDelete;

	JSON(JSONNode &refNode, bool bCopy = false)
		: node(bCopy ? json_copy(&refNode) : &refNode), bDelete(bCopy) { }
	JSON(JSONNode *n, bool bD = true)
		: node(n), bDelete(bD) { }
	~JSON()
	{
		if (bDelete)
			json_delete(node);
	}

	__inline void* operator new(size_t size, lua_State *L)
	{
		 return lua_newuserdata(L, size);
	}
};

void lua2json(lua_State *L, JSONNode &node);

extern const luaL_Reg jsonApi[];

#define MT_JSON "JSON"