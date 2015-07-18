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

	static void KillModuleEventHooks();
	static void KillModuleServices();
	
	

public:
	static LIST<void> Hooks;
	static LIST<void> Events;
	static LIST<void> Services;
	static LIST<void> HookRefs;
	static LIST<void> ServiceRefs;

	LIST<CMLuaScript> Scripts;

	CMLua();
	~CMLua();

	void Load();
	void Unload();

	static int HookEventObjParam(void *obj, WPARAM wParam, LPARAM lParam, LPARAM param);
};

#endif //_LUA_CORE_H_
