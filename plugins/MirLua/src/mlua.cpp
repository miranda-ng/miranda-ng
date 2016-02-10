#include "stdafx.h"

LIST<void> CMLua::HookRefs(1, HandleKeySortT);
LIST<void> CMLua::ServiceRefs(1, HandleKeySortT);

static int CompareScripts(const CMLuaScript* p1, const CMLuaScript* p2)
{
	return mir_strcmpi(p1->GetModuleName(), p2->GetModuleName());
}

CMLua::CMLua() : L(NULL), Scripts(10, CompareScripts)
{
	MUUID muidLast = MIID_LAST;
	hLangpack = GetPluginLangId(muidLast, 0);
}

CMLua::~CMLua()
{
	Unload();
}

const int CMLua::GetHLangpack() const
{
	return hLangpack;
}

void CMLua::SetPaths()
{
	TCHAR path[MAX_PATH];

	lua_getglobal(L, "package");

	FoldersGetCustomPathT(g_hCLibsFolder, path, _countof(path), VARST(MIRLUA_PATHT));
	lua_pushfstring(L, "%s\\?.dll", ptrA(mir_utf8encodeT(path)));
	lua_setfield(L, -2, "cpath");

	FoldersGetCustomPathT(g_hScriptsFolder, path, _countof(path), VARST(MIRLUA_PATHT));
	lua_pushfstring(L, "%s\\?.lua", ptrA(mir_utf8encodeT(path)));
	lua_setfield(L, -2, "path");

	lua_pop(L, 1);
}

void CMLua::Load()
{
	Log("Loading lua engine");
	L = luaL_newstate();
	Log("Loading std modules");
	luaL_openlibs(L);

	SetPaths();

	lua_register(L, "print", luaM_print);
	lua_register(L, "a", luaM_toansi);
	lua_register(L, "u", luaM_toucs2);
	lua_register(L, "totable", luaM_totable);

	lua_atpanic(L, luaM_atpanic);

	Log("Loading miranda modules");
	CLuaModuleLoader::Load(L);
	CLuaScriptLoader::Load(L);
}

void CMLua::Unload()
{
	Log("Unloading lua engine");

	while (int last = Scripts.getCount())
	{
		CMLuaScript* script = g_mLua->Scripts[last - 1];
		Scripts.remove(script);
		script->Unload();
		delete script;
	}

	::KillModuleScheduleTasks();
	::KillModuleMBButtons();
	::KillModuleTTBButton();

	::KillModuleIcons(hLangpack);
	::KillModuleSounds(hLangpack);
	::KillModuleMenus(hLangpack);
	::KillModuleHotkeys(hLangpack);
	::KillObjectEventHooks(L);
	::KillObjectServices(L);

	lua_close(L);
}

void CMLua::KillLuaRefs()
{
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

	lua_pushlightuserdata(L, (void*)wParam);
	lua_pushlightuserdata(L, (void*)lParam);
	luaM_pcall(L, 2, 1);

	int res = (int)lua_tointeger(L, 1);

	return res;
}