#pragma once

class CMLuaVariablesLoader
{
private:
	lua_State *L;

	CMLuaVariablesLoader(lua_State *L);

	void LoadVariables();

public:
	static void Load(lua_State *L);
};
