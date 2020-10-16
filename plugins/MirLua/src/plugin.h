#pragma once

struct CMPlugin : public PLUGIN<CMPlugin>
{
	friend class CMLuaOptionsMain;
	friend class CMLuaEvaluateOptions;

	HNETLIBUSER hNetlib = nullptr;

private:
	lua_State *L;

	void LoadLua();
	void UnloadLua();

	OBJLIST<CMLuaScript> m_scripts;
	void LoadLuaScripts();
	void UnloadLuaScripts();
	void ReloadLuaScripts();

	int __cdecl OnOptionsInit(WPARAM wParam, LPARAM);
	int __cdecl OnModulesLoaded(WPARAM, LPARAM);

	INT_PTR __cdecl Eval(WPARAM, LPARAM);
	INT_PTR __cdecl Call(WPARAM, LPARAM);
	INT_PTR __cdecl Exec(WPARAM, LPARAM);

public:
	CMPlugin();

	int Load() override;
	int Unload() override;
};
