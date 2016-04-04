#ifndef _LUA_SCRIPT_LOADER_H_
#define _LUA_SCRIPT_LOADER_H_

class CMLuaScriptLoader
{
private:
	lua_State *L;

	CMLuaScriptLoader(lua_State *L);

	void LoadScript(const TCHAR *scriptDir, const TCHAR *file);
	void LoadScripts();

public:
	static void Load(lua_State *L);
};

#endif //_LUA_SCRIPT_LOADER_H_