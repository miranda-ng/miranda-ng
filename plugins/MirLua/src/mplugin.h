#ifndef _LUA_CORE_H_
#define _LUA_CORE_H_

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

	int Load() override;
	int Unload() override;

	void Reload();
};

#endif //_LUA_CORE_H_
