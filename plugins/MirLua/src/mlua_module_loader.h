#ifndef _LUA_MODULE_LOADER_H_
#define _LUA_MODULE_LOADER_H_

class CLuaModuleLoader
{
private:
	lua_State *L;

	CLuaModuleLoader(lua_State *L);

	void PreloadModule(const char *name, lua_CFunction loader);
	void LoadModules();

public:
	static void Load(lua_State *L);
};

#endif //_LUA_MODULE_LOADER_H_