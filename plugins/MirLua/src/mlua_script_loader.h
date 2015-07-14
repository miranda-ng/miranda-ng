#ifndef _LUA_SCRIPT_LOADER_H_
#define _LUA_SCRIPT_LOADER_H_

class CLuaScriptLoader
{
private:
	lua_State *L;
	HANDLE hLogger;

	CLuaScriptLoader(lua_State *L);

	void RegisterScriptsFolder(const char *path);

	void LoadScript(const TCHAR *path);
	void LoadScripts(const TCHAR *scriptDir);

	void UnloadScript(const TCHAR *path);

	void ReloadScript(const TCHAR *path);

public:
	static void Load(lua_State *L);
	static void Reload(lua_State *L, const TCHAR* path);
};

#endif //_LUA_SCRIPT_LOADER_H_