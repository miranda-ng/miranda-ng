#ifndef _LUA_CORE_H_
#define _LUA_CORE_H_

class CMLua
{
private:
	lua_State *lua;

public:
	CMLua();
	~CMLua();

	static luaL_Reg CoreFunctions[10];

	void Load(const char *name);
};

#endif //_LUA_CORE_H_
