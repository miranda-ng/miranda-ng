#include "stdafx.h"

#define MT_ENVIRONMENT "ENVIRONMENT"

CMLuaEnvironment::CMLuaEnvironment(lua_State *L)
	: CMPluginBase(nullptr, *(PLUGININFOEX*)nullptr), L(L)
{
	MUUID muidLast = MIID_LAST;
	m_hLang = GetPluginLangId(muidLast, 0);
}

CMLuaEnvironment::~CMLuaEnvironment()
{
	KillModuleIcons(m_hLang);
	KillModuleSounds(m_hLang);
	KillModuleMenus(m_hLang);
	KillModuleHotkeys(m_hLang);

	KillObjectEventHooks(this);
	KillObjectServices(this);

	for (auto &it : m_hookRefs)
		luaL_unref(L, LUA_REGISTRYINDEX, it.second);

	for (auto &it : m_serviceRefs)
		luaL_unref(L, LUA_REGISTRYINDEX, it.second);
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

int CMLuaEnvironment::GetEnvironmentId(lua_State *L)
{
	CMLuaEnvironment *env = GetEnvironment(L);
	return env != nullptr
		? env->m_hLang
		: hMLuaLangpack;
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
		lua_pushlightuserdata(env->L, (void*)lParam);
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

	lua_pushlightuserdata(env->L, (void*)wParam);
	lua_pushlightuserdata(env->L, (void*)lParam);
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
}

bool CMLuaEnvironment::Load()
{
	luaL_checktype(L, -1, LUA_TFUNCTION);

	CreateEnvironmentTable();
	lua_setupvalue(L, -2, 1);

	return lua_pcall(L, 0, 1, 0) == LUA_OK;
}
