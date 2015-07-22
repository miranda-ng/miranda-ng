#include "stdafx.h"

LIST<void> CMLua::Hooks(1, PtrKeySortT);
LIST<void> CMLua::Events(1, PtrKeySortT);
LIST<void> CMLua::Services(1, PtrKeySortT);
LIST<void> CMLua::HookRefs(1, HandleKeySortT);
LIST<void> CMLua::ServiceRefs(1, HandleKeySortT);

static int CompareScripts(const CMLuaScript* p1, const CMLuaScript* p2)
{
	return mir_strcmpi(p1->GetModuleName(), p2->GetModuleName());
}

CMLua::CMLua() : L(NULL), Scripts(10, CompareScripts)
{
}

CMLua::~CMLua()
{
	Unload();
}

void CMLua::Load()
{
	CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)"Loading lua engine");
	L = luaL_newstate();
	CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)"Loading std modules");
	luaL_openlibs(L);

	lua_getglobal(L, "package");
	lua_pushliteral(L, "");
	lua_setfield(L, -2, "path");
	lua_pushliteral(L, "");
	lua_setfield(L, -2, "cpath");
	lua_pop(L, 1);

	lua_getglobal(L, "_G");
	lua_pushcclosure(L, luaM_print, 0);
	lua_setfield(L, -2, "print");
	lua_pushcclosure(L, luaM_toansi, 0);
	lua_setfield(L, -2, "a");
	lua_pushcclosure(L, luaM_toucs2, 0);
	lua_setfield(L, -2, "u");
	lua_pop(L, 1);

	lua_atpanic(L, luaM_atpanic);

	MUUID muidLast = MIID_LAST;
	hScriptsLangpack = GetPluginLangId(muidLast, 0);

	CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)"Loading miranda modules");
	CLuaModuleLoader::Load(L);
	CLuaScriptLoader::Load(L);
}

void CMLua::Unload()
{
	CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)"Unloading lua engine");

	while (int last = Scripts.getCount())
	{
		CMLuaScript* script = g_mLua->Scripts[last - 1];
		Scripts.remove(script);
		script->Unload();
		delete script;
	}
	
	::KillModuleIcons(hScriptsLangpack);
	::KillModuleMenus(hScriptsLangpack);
	::KillModuleHotkeys(hScriptsLangpack);
	CMLua::KillModuleServices();
	CMLua::KillModuleEventHooks();
	//KillModuleSubclassing

	if (L)
		lua_close(L);
}

void CMLua::KillModuleEventHooks()
{
	while (Hooks.getCount())
	{
		HANDLE hHook = Hooks[0];
		Hooks.remove(0);
		UnhookEvent(hHook);
	}

	while (Events.getCount())
	{
		HANDLE hEvent = Events[0];
		Events.remove(hEvent);
		DestroyHookableEvent(hEvent);
	}

	while (HookRefs.getCount())
	{
		HandleRefParam *param = (HandleRefParam*)HookRefs[0];
		if (param != NULL)
		{
			luaL_unref(param->L, LUA_REGISTRYINDEX, param->ref);
			HookRefs.remove(0);
			delete param;
		}
	}
}

void CMLua::KillModuleServices()
{
	while (Services.getCount())
	{
		HANDLE hService = Services[0];
		Services.remove(0);
		DestroyServiceFunction(hService);
	}

	while (ServiceRefs.getCount())
	{
		HandleRefParam *param = (HandleRefParam*)ServiceRefs[0];
		if (param != NULL)
		{
			luaL_unref(param->L, LUA_REGISTRYINDEX, param->ref);
			ServiceRefs.remove(0);
			delete param;
		}
	}
}

int CMLua::HookEventObjParam(void *obj, WPARAM wParam, LPARAM lParam, LPARAM param)
{
	lua_State *L = (lua_State*)obj;

	int ref = param;
	lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

	lua_pushnumber(L, wParam);
	lua_pushnumber(L, lParam);
	if (lua_pcall(L, 2, 1, 0))
		CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)lua_tostring(L, -1));

	int res = (int)lua_tointeger(L, 1);

	return res;
}