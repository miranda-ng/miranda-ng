#ifndef _LUA_ENVIROMENT_H_
#define _LUA_ENVIROMENT_H_

class CMLuaEnviroment
{
private:
	int id;

	void CreateEnviromentTable();

public:
	lua_State *L;

	CMLuaEnviroment(lua_State *L);
	virtual ~CMLuaEnviroment();

	static CMLuaEnviroment* GetEnviroment(lua_State *L);
	static int GetEnviromentId(lua_State *L);

	int GetId() const;

	bool Load();
};

#endif //_LUA_ENVIROMENT_H_
