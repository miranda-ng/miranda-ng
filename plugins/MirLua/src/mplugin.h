#pragma once

struct CMPlugin : public PLUGIN<CMPlugin>
{
	friend class CMLuaOptions;

private:
	lua_State *L;


	INT_PTR __cdecl Eval(WPARAM, LPARAM);
	INT_PTR __cdecl Call(WPARAM, LPARAM);
	INT_PTR __cdecl Exec(WPARAM, LPARAM);

public:
	OBJLIST<CMLuaScript> Scripts;

	CMPlugin();
	~CMPlugin();

	void Load();
	void Unload();
	void Reload();
};
