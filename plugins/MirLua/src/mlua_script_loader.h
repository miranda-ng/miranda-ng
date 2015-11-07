#ifndef _LUA_SCRIPT_LOADER_H_
#define _LUA_SCRIPT_LOADER_H_

class CLuaScriptLoader
{
private:
	lua_State *L;

	CLuaScriptLoader(lua_State *L);

	void RegisterScriptsFolder(const char *path);

	void LoadScript(const TCHAR *scriptDir, const TCHAR *file);
	void LoadScripts(const TCHAR *scriptDir);

public:
	static void Load(lua_State *L);
};

#endif //_LUA_SCRIPT_LOADER_H_