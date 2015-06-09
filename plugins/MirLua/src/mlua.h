#ifndef _LUA_CORE_H_
#define _LUA_CORE_H_

class CMLua
{
private:
	lua_State *L;
	HANDLE hConsole;

	int luaopen_m(lua_State *L);

	void Preload(const char *name, lua_CFunction func);

public:
	CMLua();
	~CMLua();

	void Load(const char *name);
};

#endif //_LUA_CORE_H_
