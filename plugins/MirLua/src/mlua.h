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
	HANDLE hLoadedEvent;
	HANDLE hUnloadEvent;

	static void KillModuleEventHooks();
	static void KillModuleServices();

	void Load();
	void Unload();

public:
	static LIST<void> Hooks;
	static LIST<void> Events;
	static LIST<void> Services;
	static LIST<void> HookRefs;
	static LIST<void> ServiceRefs;

	CMLua();
	~CMLua();

	void Reload();
	void Reload(const TCHAR* path);

	static int OnScriptLoaded(lua_State *L);
	static int OnScriptUnload(lua_State *L);

	static int HookEventObjParam(void *obj, WPARAM wParam, LPARAM lParam, LPARAM param);
};

#endif //_LUA_CORE_H_
