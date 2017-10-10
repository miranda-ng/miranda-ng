#ifndef _LUA_M_JSON_H_
#define _LUA_M_JSON_H_

#include <m_json.h>

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

#define MT_JSON "JSON"

#define MLUA_JSON	"m_json"
LUAMOD_API int (luaopen_m_json)(lua_State *L);

#endif //_LUA_M_PROTOCOLS_H_