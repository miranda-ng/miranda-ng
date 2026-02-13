#pragma once

class CMLuaScriptLoader
{
private:
	lua_State *L;

	OBJLIST<CMLuaScript> &m_scripts;

	CMLuaScriptLoader(lua_State *L, OBJLIST<CMLuaScript> &scripts);
	
	void SetPaths();

	void LoadScript(const wchar_t *scriptDir, const wchar_t *fileName);
	void LoadScripts();

public:
	static void Load(lua_State *L, OBJLIST<CMLuaScript> &scripts);
};
