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
	L(nullptr),
	m_scripts(1, ScriptsCompare)
{
	MUUID muidLast = MIID_LAST;
	g_hMLuaLangpack = GetPluginLangId(muidLast, 0);

	RegisterProtocol(PROTOTYPE_FILTER);

	CreatePluginService(MS_LUA_CALL, &CMPlugin::Call);
	CreatePluginService(MS_LUA_EXEC, &CMPlugin::Exec);
	CreatePluginService(MS_LUA_EVAL, &CMPlugin::Eval);
}

void CMPlugin::LoadLuaScripts()
{
	CMLuaScriptLoader::Load(L, m_scripts);
}

void CMPlugin::UnloadLuaScripts()
{
	for (auto &script : m_scripts.rev_iter()) {
		script->Unload();
		delete m_scripts.removeItem(&script);
	}
}

void CMPlugin::LoadLua()
{
	Log("Loading lua engine");
	L = luaL_newstate();
	Log("Loading standard modules");
	luaL_openlibs(L);

	lua_atpanic(L, luaM_atpanic);

	CMLuaFunctionLoader::Load(L);
	CMLuaModuleLoader::Load(L);
	CMLuaVariablesLoader::Load(L);
}

void CMPlugin::UnloadLua()
{
	UnloadLuaScripts();

	if (L != nullptr) {
		KillObjectEventHooks(L);
		KillObjectServices(L);

		Log("Unloading lua engine");

		lua_close(L);
	}
}

void CMPlugin::ReloadLuaScripts()
{
	UnloadLuaScripts();
	LoadLuaScripts();
}

/***********************************************/

int CMPlugin::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE | ODPF_DONTTRANSLATE;
	odp.szGroup.w = LPGENW("Services");
	odp.szTitle.w = L"Lua";

	odp.szTab.w = LPGENW("Scripts");
	odp.pDialog = new CMLuaOptionsMain();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Evaluate");
	odp.pDialog = new CMLuaEvaluateOptions();
	g_plugin.addOptions(wParam, &odp);

	return 0;
}

int CMPlugin::OnModulesLoaded(WPARAM, LPARAM)
{
	g_hCLibsFolder = FoldersRegisterCustomPathW(MODULENAME, "CLibsFolder", MIRLUA_PATHT, TranslateT("C libs folder"));
	g_hScriptsFolder = FoldersRegisterCustomPathW(MODULENAME, "ScriptsFolder", MIRLUA_PATHT, TranslateT("Scripts folder"));

	LoadLuaScripts();

	HookPluginEvent(ME_OPT_INITIALISE, &CMPlugin::OnOptionsInit);

	return 0;
}

int CMPlugin::Load()
{
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS;
	nlu.szDescriptiveName.a = MODULENAME;
	nlu.szSettingsModule = MODULENAME;
	hNetlib = Netlib_RegisterUser(&nlu);

	LoadIcons();
	LoadLua();

	HookPluginEvent(ME_SYSTEM_MODULESLOADED, &CMPlugin::OnModulesLoaded);

	return 0;
}

int CMPlugin::Unload()
{
	Netlib_CloseHandle(hNetlib);

	UnloadLua();
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

	lua_pushstring(L, T2Utf(module));
	lua_pushstring(L, T2Utf(function));

	lua_pushcfunction(L, mlua_call);

	CMLuaEnvironment env(L);
	env.Call();
	wchar_t *result = mir_utf8decodeW(lua_tostring(L, -1));
	lua_pop(L, 1);
	env.Unload();

	return (INT_PTR)result;
}

INT_PTR CMPlugin::Eval(WPARAM, LPARAM lParam)
{
	const wchar_t *script = (const wchar_t*)lParam;

	CMLuaEnvironment env(L);
	env.Eval(script);
	wchar_t *result = mir_utf8decodeW(lua_tostring(L, -1));
	lua_pop(L, 1);
	env.Unload();

	return (INT_PTR)result;
}

INT_PTR CMPlugin::Exec(WPARAM, LPARAM lParam)
{
	const wchar_t *path = (const wchar_t*)lParam;

	CMLuaEnvironment env(L);
	env.Exec(path);
	wchar_t *result = mir_utf8decodeW(lua_tostring(L, -1));
	lua_pop(L, 1);
	env.Unload();

	return (INT_PTR)result;
}
