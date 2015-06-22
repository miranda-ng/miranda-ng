#ifndef _LUA_CONSOLE_H_
#define _LUA_CONSOLE_H_

class CMLuaConsole
{
private:
	lua_State *L;
	HANDLE hConsole;

public:
	CMLuaConsole(lua_State *L);
	~CMLuaConsole();
};

#endif //_LUA_CONSOLE_H_