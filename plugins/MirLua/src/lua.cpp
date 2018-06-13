#include "stdafx.h"

CMLua::CMLua() : L(nullptr)
{
	//MUUID muidLast = MIID_LAST;
	//g_hMLuaLangpack = GetPluginLangId(muidLast, 0);
}

CMLua::~CMLua()
{
	Log("Unloading lua engine");

	KillModuleIcons(g_hMLuaLangpack);
	KillModuleSounds(g_hMLuaLangpack);
	KillModuleMenus(g_hMLuaLangpack);
	KillModuleHotkeys(g_hMLuaLangpack);

	KillObjectEventHooks(L);
	KillObjectServices(L);

	lua_close(L);
}

lua_State* CMLua::GetState()
{
	return L;
}

void CMLua::Load()
{
	Log("Loading lua engine");
	L = luaL_newstate();
	Log("Loading standard modules");
	luaL_openlibs(L);

	lua_atpanic(L, luaM_atpanic);
}
