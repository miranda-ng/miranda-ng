#include "stdafx.h"

#define MT_ENVIRONMENT "ENVIRONMENT"

extern PLUGININFOEX pluginInfoEx;

EXTERN_C MIR_APP_DLL(void) RegisterPlugin(CMPluginBase *pPlugin);
EXTERN_C MIR_APP_DLL(void) UnregisterPlugin(CMPluginBase *pPlugin);

CMLuaEnvironment::CMLuaEnvironment(lua_State *L) :
	CMPluginBase(nullptr, pluginInfoEx), L(L)
{
	m_hInst = (HINSTANCE)this;
	::RegisterPlugin(this);
}

int CMLuaEnvironment::Unload()
{
	::UnregisterPlugin(this);

	KillObjectEventHooks(this);
	KillObjectServices(this);

	for (auto &it : m_hookRefs)
		luaL_unref(L, LUA_REGISTRYINDEX, it.second);

	for (auto &it : m_serviceRefs)
		luaL_unref(L, LUA_REGISTRYINDEX, it.second);

	return 0;
}

CMLuaEnvironment* CMLuaEnvironment::GetEnvironment(lua_State *L)
{
	if (!luaM_getenv(L))
		return nullptr;

	lua_rawgeti(L, -1, NULL);
	CMLuaEnvironment *env = (CMLuaEnvironment*)lua_touserdata(L, -1);
	lua_pop(L, 3);

	return env;
}

HPLUGIN CMLuaEnvironment::GetEnvironmentId(lua_State *L)
{
	CMLuaEnvironment *env = GetEnvironment(L);
	return env != nullptr ? HPLUGIN(env) : &g_plugin;
}

static int HookEventEnvParam(void *obj, WPARAM wParam, LPARAM lParam, LPARAM param)
{
	CMLuaEnvironment *env = (CMLuaEnvironment*)obj;

	int ref = param;
	lua_rawgeti(env->L, LUA_REGISTRYINDEX, ref);

	if (wParam)
		lua_pushlightuserdata(env->L, (void*)wParam);
	else
		lua_pushnil(env->L);

	if (lParam)
		lua_pushlightuserdata(env->L, (void *)lParam);
	else
		lua_pushnil(env->L);

	luaM_pcall(env->L, 2, 1);

	return lua_tointeger(env->L, -1);
}

HANDLE CMLuaEnvironment::HookEvent(const char *name, int ref)
{
	HANDLE hHook = HookEventObjParam(name, HookEventEnvParam, this, ref);
	if (hHook)
		m_hookRefs[hHook] = ref;
	return hHook;
}

int CMLuaEnvironment::UnhookEvent(HANDLE hEvent)
{
	int res = ::UnhookEvent(hEvent);
	if (res) {
		auto it = m_hookRefs.find(hEvent);
		if (it != m_hookRefs.end())
			luaL_unref(L, LUA_REGISTRYINDEX, it->second);
	}
	return res;
}

static INT_PTR CreateServiceFunctionEnvParam(void *obj, WPARAM wParam, LPARAM lParam, LPARAM param)
{
	CMLuaEnvironment *env = (CMLuaEnvironment*)obj;

	int ref = param;
	lua_rawgeti(env->L, LUA_REGISTRYINDEX, ref);

	lua_pushlightuserdata(env->L, (void *)wParam);
	lua_pushlightuserdata(env->L, (void *)lParam);
	luaM_pcall(env->L, 2, 1);

	INT_PTR res = lua_tointeger(env->L, 1);
	lua_pushinteger(env->L, res);

	return res;
}

HANDLE CMLuaEnvironment::CreateServiceFunction(const char *name, int ref)
{
	HANDLE hService = CreateServiceFunctionObjParam(name, CreateServiceFunctionEnvParam, this, ref);
	if (hService)
		m_serviceRefs[hService] = ref;
	return hService;
}

void CMLuaEnvironment::DestroyServiceFunction(HANDLE hService)
{
	auto it = m_serviceRefs.find(hService);
	if (it != m_serviceRefs.end())
		luaL_unref(L, LUA_REGISTRYINDEX, it->second);
	::DestroyServiceFunction(hService);
}

void CMLuaEnvironment::CreateEnvironmentTable()
{
	if (!lua_isfunction(L, -1))
		return;

	lua_createtable(L, 1, 1);
	lua_pushlightuserdata(L, this);
	lua_rawseti(L, -2, NULL);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "_G");
	lua_createtable(L, 0, 2);
	lua_pushliteral(L, MT_ENVIRONMENT);
	lua_setfield(L, -2, "__metatable");
	lua_getglobal(L, "_G");
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);

	lua_setupvalue(L, -2, 1);
}

/***********************************************/

wchar_t* CMLuaEnvironment::Error()
{
	const char *message = lua_tostring(L, -1);
	wchar_t *error = mir_utf8decodeW(message);
	lua_pop(L, 1);
	return error;
}

int CMLuaEnvironment::Call()
{
	CreateEnvironmentTable();
	return lua_pcall(L, 0, 1, 0);
}

int CMLuaEnvironment::Eval(const wchar_t *script)
{
	if (int res = luaL_loadstring(L, T2Utf(script)))
		return res;
	return Call();
}

int CMLuaEnvironment::Exec(const wchar_t *path)
{
	if (int res = luaL_loadfile(L, T2Utf(path)))
		return res;
	return Call();
}
