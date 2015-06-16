#ifndef _LUA_CORE_H_
#define _LUA_CORE_H_

class CMLua
{
private:
	lua_State *L;
	static luaL_Reg coreLib[15];

	void Preload(const char *name, lua_CFunction func);

public:
	CMLua();
	~CMLua();

	void AddPath(const char *path);

	void Load(const char *name);

	static WPARAM GetWParam(lua_State *L, int idx);
	static LPARAM GetLParam(lua_State *L, int idx);

	static int HookEventObjParam(void *obj, WPARAM wParam, LPARAM lParam, LPARAM param);
};

#endif //_LUA_CORE_H_
