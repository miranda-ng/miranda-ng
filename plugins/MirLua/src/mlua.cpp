#include "stdafx.h"

int hMLuaLangpack;

LIST<void> CMLua::HookRefs(1, HandleKeySortT);
LIST<void> CMLua::ServiceRefs(1, HandleKeySortT);

static int CompareScripts(const CMLuaScript* p1, const CMLuaScript* p2)
{
	return mir_strcmpi(p1->GetModuleName(), p2->GetModuleName());
}

CMLua::CMLua() : L(NULL), Scripts(10, CompareScripts)
{
	MUUID muidLast = MIID_LAST;
	hMLuaLangpack = GetPluginLangId(muidLast, 0);
}

CMLua::~CMLua()
{
	Unload();
}

/***********************************************/

static int mlua_print(lua_State *L)
{
	CMStringA data;
	int nargs = lua_gettop(L);
	for (int i = 1; i <= nargs; i++)
	{
		switch (lua_type(L, i))
		{
		case LUA_TNIL:
			data.Append("nil");
			break;
		case LUA_TBOOLEAN:
			data.AppendFormat("%s", lua_toboolean(L, i) ? "true" : "false");
			break;
		case LUA_TNUMBER:
			data.AppendFormat("%s", lua_tostring(L, i));
			break;
		case LUA_TSTRING:
			data.AppendFormat("'%s'", lua_tostring(L, i));
			break;
		default:
			if (luaL_callmeta(L, i, "__tostring")) {
				data.AppendFormat("'%s'", lua_tostring(L, -1));
				break;
			}
			data.AppendFormat("%s(0x%p)", luaL_typename(L, i), lua_topointer(L, i));
			break;
		}
		data.Append(", ");
	}
	if (data.GetLength() >= 1)
		data.Delete(data.GetLength() - 2, 2);

	Log(data.GetBuffer());

	return 0;
}

static int mlua_toansi(lua_State *L)
{
	const char *value = luaL_checkstring(L, 1);
	int codepage = luaL_optinteger(L, 2, Langpack_GetDefaultCodePage());

	ptrA string(mir_strdup(value));
	lua_pushstring(L, mir_utf8decodecp(string, codepage, NULL));

	return 1;
}

static int mlua_toucs2(lua_State *L)
{
	const char *value = luaL_checkstring(L, 1);

	ptrW unicode(mir_utf8decodeW(value));
	size_t length = mir_wstrlen(unicode) * sizeof(wchar_t);

	ptrA string((char*)mir_calloc(length + 1));
	memcpy(string, unicode, length);

	lua_pushlstring(L, string, length + 1);

	return 1;
}

static int mlua_topointer(lua_State *L)
{
	switch (lua_type(L, 1))
	{
	case LUA_TBOOLEAN:
		lua_pushlightuserdata(L, (void*)lua_toboolean(L, 1));
		break;
	case LUA_TNUMBER:
		if (lua_isinteger(L, 1))
		{
			lua_Integer value = lua_tointeger(L, 1);
			if (value > INTPTR_MAX)
			{
				const char *msg = lua_pushfstring(L, "%f is larger than %d", value, INTPTR_MAX);
				return luaL_argerror(L, 1, msg);
			}
			lua_pushlightuserdata(L, (void*)value);
		}
		break;
	case LUA_TSTRING:
		lua_pushlightuserdata(L, (void*)lua_tostring(L, 1));
		break;
	case LUA_TLIGHTUSERDATA:
		lua_pushvalue(L, 1);
	default:
		return luaL_argerror(L, 1, luaL_typename(L, 1));
	}

	return 1;
}

static int mlua_tonumber(lua_State *L)
{
	if (lua_islightuserdata(L, 1))
	{
		lua_Integer value = (lua_Integer)lua_touserdata(L, 1);
		lua_pushinteger(L, value);
		return 1;
	}

	int n = lua_gettop(L);
	lua_pushvalue(L, lua_upvalueindex(1));
	lua_pushvalue(L, 1);
	if (n == 2)
		lua_pushvalue(L, 2);
	luaM_pcall(L, n, 1);

	return 1;
}

static int mlua_interpolate(lua_State *L)
{
	const char *string = luaL_checkstring(L, 1);

	char pattern[128];

	if (lua_istable(L, 2)) {
		for (lua_pushnil(L); lua_next(L, 2); lua_pop(L, 2)) {
			lua_pushvalue(L, -2);
			const char *key = lua_tostring(L, -1);
			const char *val = lua_tostring(L, -2);

			mir_snprintf(pattern, "{%s}", key);
			string = luaL_gsub(L, string, pattern, val);
		}
	}
	else {
		int nargs = lua_gettop(L);
		for (int i = 2; i <= nargs; i++) {
			const char *val = lua_tostring(L, i);

			mir_snprintf(pattern, "{%d}", i - 1);
			string = luaL_gsub(L, string, pattern, val);
			lua_pop(L, 1);
		}
	}

	lua_Debug ar;

	size_t level = 1;

	while (lua_getstack(L, level++, &ar))
	{
		size_t i = 1;
		while (const char *name = lua_getlocal(L, &ar, i++))
		{
			const char *val = lua_tostring(L, -1);
			if (val)
			{
				mir_snprintf(pattern, "${%s}", name);
				string = luaL_gsub(L, string, pattern, val);
				lua_pop(L, 1);
			}
		}
	}
	lua_pushstring(L, string);

	return 1;
}

/***********************************************/

void CMLua::SetPaths()
{
	wchar_t path[MAX_PATH];

	lua_getglobal(L, "package");

	FoldersGetCustomPathT(g_hCLibsFolder, path, _countof(path), VARSW(MIRLUA_PATHT));
	lua_pushfstring(L, "%s\\?.dll", ptrA(mir_utf8encodeW(path)));
	lua_setfield(L, -2, "cpath");

	FoldersGetCustomPathT(g_hScriptsFolder, path, _countof(path), VARSW(MIRLUA_PATHT));
	lua_pushfstring(L, "%s\\?.lua", ptrA(mir_utf8encodeW(path)));
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

	lua_register(L, "print", mlua_print);
	lua_register(L, "a", mlua_toansi);
	lua_register(L, "u", mlua_toucs2);
	lua_register(L, "topointer", mlua_topointer);

	lua_getglobal(L, "tonumber");
	lua_pushcclosure(L, mlua_tonumber, 1);
	lua_setglobal(L, "tonumber");

	lua_pushstring(L, "");
	lua_getmetatable(L, -1);
	lua_pushstring(L, "__mod");
	lua_pushcfunction(L, mlua_interpolate);
	lua_rawset(L, -3);
	lua_pushstring(L, "__index");
	lua_rawget(L, -2);
	lua_pushcfunction(L, mlua_interpolate);
	lua_setfield(L, -2, "interpolate");
	lua_pop(L, 3);

	lua_atpanic(L, luaM_atpanic);

	Log("Loading miranda modules");
	CMLuaModuleLoader::Load(L);
	Log("Loading scripts");
	CMLuaScriptLoader::Load(L);
}

void CMLua::Unload()
{
	Log("Unloading lua engine");

	while (int last = Scripts.getCount())
	{
		CMLuaScript *script = g_mLua->Scripts[last - 1];
		Scripts.remove(script);
		delete script;
	}

	KillModuleIcons(hMLuaLangpack);
	KillModuleSounds(hMLuaLangpack);
	KillModuleMenus(hMLuaLangpack);
	KillModuleHotkeys(hMLuaLangpack);

	KillObjectEventHooks(L);
	KillObjectServices(L);

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

/***********************************************/

static int mlua_call(lua_State *L)
{
	const char *module = luaL_checkstring(L, -3);
	const char *function = luaL_checkstring(L, -2);

	if (module && module[0])
	{
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

INT_PTR CMLua::Call(WPARAM wParam, LPARAM lParam)
{
	const wchar_t *module = (const wchar_t*)wParam;
	const wchar_t *function = (const wchar_t*)lParam;

	lua_pushstring(L, ptrA(mir_utf8encodeW(module)));
	lua_pushstring(L, ptrA(mir_utf8encodeW(function)));

	lua_newtable(L);
	lua_pushcclosure(L, mlua_call, 1);
	
	CMLuaEnviroment env(L);
	env.Load();

	wchar_t *result = mir_utf8decodeW(lua_tostring(L, -1));
	lua_pop(L, 1);

	return (INT_PTR)result;
}

INT_PTR CMLua::Exec(WPARAM, LPARAM lParam)
{
	const wchar_t *path = (const wchar_t*)lParam;

	if (luaL_loadfile(L, ptrA(mir_utf8encodeW(path)))) {
		ReportError(L);
		return NULL;
	}

	CMLuaEnviroment env(L);
	env.Load();

	wchar_t *result = mir_utf8decodeW(lua_tostring(L, -1));
	lua_pop(L, 1);

	return (INT_PTR)result;
}

INT_PTR CMLua::Eval(WPARAM, LPARAM lParam)
{
	const wchar_t *script = (const wchar_t*)lParam;

	if (luaL_loadstring(L, ptrA(mir_utf8encodeW(script)))) {
		ReportError(L);
		return NULL;
	}

	CMLuaEnviroment env(L);
	env.Load();

	wchar_t *result = mir_utf8decodeW(lua_tostring(L, -1));
	lua_pop(L, 1);

	return (INT_PTR)result;
}