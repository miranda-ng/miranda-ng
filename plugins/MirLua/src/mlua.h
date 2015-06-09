#ifndef _LUA_CORE_H_
#define _LUA_CORE_H_

class CMLua
{
private:
	lua_State *L;

	void Preload(const char *name, lua_CFunction func);

public:
	CMLua();
	~CMLua();

	static luaL_Reg coreFunctions[10];

	void Load(const char *name);
};

#endif //_LUA_CORE_H_
