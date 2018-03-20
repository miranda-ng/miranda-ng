#ifndef _LUA_CORE_H_
#define _LUA_CORE_H_

class CMLua
{
private:
	lua_State *L;

	void SetPaths();

public:
	OBJLIST<CMLuaScript> Scripts;

	CMLua();
	~CMLua();

	void Load();
	void Unload();

	INT_PTR Call(WPARAM, LPARAM);
	INT_PTR Exec(WPARAM, LPARAM);
	INT_PTR Eval(WPARAM, LPARAM);
};

#endif //_LUA_CORE_H_
