#ifndef _LUA_ENVIRONMENT_H_
#define _LUA_ENVIRONMENT_H_

class CMLuaEnvironment
{
private:
	int id;

	void CreateEnvironmentTable();

public:
	lua_State *L;

	CMLuaEnvironment(lua_State *L);
	virtual ~CMLuaEnvironment();

	static CMLuaEnvironment* GetEnvironment(lua_State *L);
	static int GetEnvironmentId(lua_State *L);

	int GetId() const;

	bool Load();
};

#endif //_LUA_ENVIRONMENT_H_
