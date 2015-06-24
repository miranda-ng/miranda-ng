#ifndef _LUA_SCRIPT_LOADER_H_
#define _LUA_SCRIPT_LOADER_H_

class CLuaScriptLoader
{
private:
	lua_State *L;
	HANDLE hLogger;

	CLuaScriptLoader(lua_State *L, HANDLE hLogger);

	void RegisterScriptsFolder(const char *path);

	void LoadScript(const TCHAR *path, const TCHAR *name);
	void LoadScripts(const TCHAR *scriptDir);

public:
	static void Load(lua_State *L, HANDLE hLogger = NULL);
};

#endif //_LUA_SCRIPT_LOADER_H_