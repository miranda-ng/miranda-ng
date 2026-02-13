#pragma once

class CMLuaFunctionLoader
{
private:
	lua_State *L;

	CMLuaFunctionLoader(lua_State *L);

	void LoadFunctions();

public:
	static void Load(lua_State *L);
};
