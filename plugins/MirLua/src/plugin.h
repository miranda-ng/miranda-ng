#pragma once

struct CMPlugin : public PLUGIN<CMPlugin>
{
	friend class CMLuaOptions;

private:
	CMLua *lua;

	void LoadLua();
	void UnloadLua();

	INT_PTR __cdecl Eval(WPARAM, LPARAM);
	INT_PTR __cdecl Call(WPARAM, LPARAM);
	INT_PTR __cdecl Exec(WPARAM, LPARAM);

public:
	OBJLIST<CMLuaScript> Scripts;

	CMPlugin();

	void Reload();

	int Load() override;
	int Unload() override;
};
