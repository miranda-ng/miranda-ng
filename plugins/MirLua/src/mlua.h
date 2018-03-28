#ifndef _LUA_CORE_H_
#define _LUA_CORE_H_

class CMLua : public PLUGIN<CMLua>
{
	friend class CMLuaOptions;

private:
	lua_State *L;

	void Unload();

	INT_PTR __cdecl Eval(WPARAM, LPARAM);
	INT_PTR __cdecl Call(WPARAM, LPARAM);
	INT_PTR __cdecl Exec(WPARAM, LPARAM);

public:
	OBJLIST<CMLuaScript> Scripts;

	CMLua();
	~CMLua();

	void Load();
	void Reload();
};

#endif //_LUA_CORE_H_
