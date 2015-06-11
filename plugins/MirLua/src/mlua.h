#ifndef _LUA_CORE_H_
#define _LUA_CORE_H_

class CMLua
{
private:
	lua_State *L;
	static luaL_Reg coreLib[14];

	void Preload(const char *name, lua_CFunction func);

public:
	CMLua();
	~CMLua();

	void AddPath(const char *path);

	void Load(const char *name);
};

#endif //_LUA_CORE_H_
