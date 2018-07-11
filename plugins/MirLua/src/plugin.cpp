#include "stdafx.h"

int g_hMLuaLangpack;
CMPlugin g_plugin;

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {27d41d81-991f-4dc6-8749-b0321c87e694}
	{ 0x27d41d81, 0x991f, 0x4dc6,{ 0x87, 0x49, 0xb0, 0x32, 0x1c, 0x87, 0xe6, 0x94 } }
};

static int ScriptsCompare(const CMLuaScript* p1, const CMLuaScript* p2)
{
	return mir_wstrcmpi(p1->GetName(), p2->GetName());
}

CMPlugin::CMPlugin()
	: PLUGIN(MODULENAME, pluginInfoEx),
	lua(nullptr),
	Scripts(1, ScriptsCompare)
{
	MUUID muidLast = MIID_LAST;
	g_hMLuaLangpack = GetPluginLangId(muidLast, 0);

	RegisterProtocol(PROTOTYPE_FILTER);

	CreatePluginService(MS_LUA_CALL, &CMPlugin::Call);
	CreatePluginService(MS_LUA_EXEC, &CMPlugin::Exec);
	CreatePluginService(MS_LUA_EVAL, &CMPlugin::Eval);
}

void CMPlugin::LoadLua()
{
	lua = new CMLua();
	lua->Load();
	CMLuaFunctionLoader::Load(lua->L);
	CMLuaModuleLoader::Load(lua->L);
	CMLuaScriptLoader::Load(lua->L);
}

void CMPlugin::UnloadLua()
{
	Scripts.destroy();

	KillModuleIcons(this);
	KillModuleSounds(this);
	KillModuleMenus(this);
	KillModuleHotkeys(this);

	KillObjectEventHooks(lua->L);
	KillObjectServices(lua->L);

	if (lua != nullptr) {
		delete lua;
		lua = nullptr;
	}
}

void CMPlugin::Reload()
{
	Unload();
	Load();
}

/***********************************************/

static int OnModulesLoaded(WPARAM, LPARAM)
{
	g_hCLibsFolder = FoldersRegisterCustomPathT(MODULENAME, "CLibsFolder", MIRLUA_PATHT, TranslateT("C libs folder"));
	g_hScriptsFolder = FoldersRegisterCustomPathT(MODULENAME, "ScriptsFolder", MIRLUA_PATHT, TranslateT("Scripts folder"));

	HookEvent(ME_OPT_INITIALISE, OnOptionsInit);
	return 0;
}

int CMPlugin::Load()
{
	LoadIcons();
	LoadNetlib();
	LoadLua();

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	return 0;
}

int CMPlugin::Unload()
{
	UnloadLua();
	UnloadNetlib();
	return 0;
}

/***********************************************/

static int mlua_call(lua_State *L)
{
	const char *module = luaL_checkstring(L, -3);
	const char *function = luaL_checkstring(L, -2);

	if (module && module[0]) {
		lua_getglobal(L, "require");
		lua_pushstring(L, module);
		lua_pcall(L, 1, 1, 0);

		lua_getfield(L, -1, function);
		lua_replace(L, -2);
	}
	else
		lua_getglobal(L, function);

	lua_pcall(L, 0, 1, 0);

	return 1;
}

INT_PTR CMPlugin::Call(WPARAM wParam, LPARAM lParam)
{
	const wchar_t *module = (const wchar_t*)wParam;
	const wchar_t *function = (const wchar_t*)lParam;

	lua_pushstring(lua->L, ptrA(mir_utf8encodeW(module)));
	lua_pushstring(lua->L, ptrA(mir_utf8encodeW(function)));

	lua_newtable(lua->L);
	lua_pushcclosure(lua->L, mlua_call, 1);

	CMLuaEnvironment env(lua->L);
	env.Load();

	wchar_t *result = mir_utf8decodeW(lua_tostring(lua->L, -1));
	lua_pop(lua->L, 1);

	return (INT_PTR)result;
}

INT_PTR CMPlugin::Eval(WPARAM, LPARAM lParam)
{
	const wchar_t *script = (const wchar_t*)lParam;

	if (luaL_loadstring(lua->L, ptrA(mir_utf8encodeW(script)))) {
		ReportError(lua->L);
		return NULL;
	}

	CMLuaEnvironment env(lua->L);
	env.Load();

	wchar_t *result = mir_utf8decodeW(lua_tostring(lua->L, -1));
	lua_pop(lua->L, 1);

	return (INT_PTR)result;
}

INT_PTR CMPlugin::Exec(WPARAM, LPARAM lParam)
{
	const wchar_t *path = (const wchar_t*)lParam;

	if (luaL_loadfile(lua->L, ptrA(mir_utf8encodeW(path)))) {
		ReportError(lua->L);
		return NULL;
	}

	CMLuaEnvironment env(lua->L);
	env.Load();

	wchar_t *result = mir_utf8decodeW(lua_tostring(lua->L, -1));
	lua_pop(lua->L, 1);

	return (INT_PTR)result;
}
