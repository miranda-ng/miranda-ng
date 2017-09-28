#ifndef _LUA_CORE_H_
#define _LUA_CORE_H_

struct HandleRefParam
{
	HANDLE h;
	int ref;
	lua_State *L;
	HandleRefParam(HANDLE h) : L(NULL), h(h), ref(0) { }
	HandleRefParam(lua_State *L, HANDLE h, int ref = 0) : L(L), h(h), ref(ref) { }
};

class CMLua
{
private:
	lua_State *L;

	void SetPaths();

	static void KillLuaRefs();

public:
	static LIST<void> HookRefs;
	static LIST<void> ServiceRefs;

	LIST<CMLuaScript> Scripts;

	CMLua();
	~CMLua();

	void Load();
	void Unload();

	INT_PTR Call(WPARAM, LPARAM);
	INT_PTR Exec(WPARAM, LPARAM);
	INT_PTR Eval(WPARAM, LPARAM);
};

#endif //_LUA_CORE_H_
