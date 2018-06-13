#pragma once

class CMLuaScriptLoader
{
private:
	lua_State *L;

	CMLuaScriptLoader(lua_State *L);
	
	void SetPaths();

	void LoadScript(const wchar_t *scriptDir, const wchar_t *fileName);
	void LoadScripts();

public:
	static void Load(lua_State *L);
};
