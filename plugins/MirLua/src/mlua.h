#ifndef _LUA_CORE_H_
#define _LUA_CORE_H_

class CMLua
{
private:
	lua_State *L;
	HANDLE hLogger;
	CMLuaConsole *console;

	static void KillModuleServices();
	static void KillModuleEventHooks();

	void Load();
	void Unload();

public:
	CMLua();
	~CMLua();

	void Reload();

	static int HookEventObjParam(void *obj, WPARAM wParam, LPARAM lParam, LPARAM param);
};

#endif //_LUA_CORE_H_
