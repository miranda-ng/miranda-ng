#include "stdafx.h"

CMLuaVariablesLoader::CMLuaVariablesLoader(lua_State *L) : L(L)
{
}

/***********************************************/

static int mlua__add(lua_State *L)
{
	int res = 0;

	int nargs = lua_gettop(L);
	for (int i = 1; i <= nargs; i++)
		res += luaL_optinteger(L, i, 0);

	lua_pushfstring(L, "%d", res);

	return 1;
}

static int mlua__div(lua_State *L)
{
	int x = luaL_optinteger(L, 1, 0);
	int y = luaL_optinteger(L, 2, 0);

	lua_pushfstring(L, "%d", x - y);

	return 1;
}

static int mlua__hex(lua_State *L)
{
	int x = luaL_optinteger(L, 1, 0);
	int pad = luaL_optinteger(L, 2, 0);

	CMStringA format(FORMAT, "0x%%0%dx", pad);
	CMStringA value(FORMAT, format, x);
	lua_pushstring(L, value);

	return 1;
}

static int mlua__max(lua_State *L)
{
	int res = 0;

	int nargs = lua_gettop(L);
	for (int i = 1; i <= nargs; i++) {
		int val = luaL_optinteger(L, i, INT_MIN);
		if (val > res)
			res = val;
	}

	lua_pushfstring(L, "%d", res);

	return 1;
}

static int mlua__min(lua_State *L)
{
	int res = 0;

	int nargs = lua_gettop(L);
	for (int i = 1; i <= nargs; i++) {
		int val = luaL_optinteger(L, i, INT_MAX);
		if (val < res)
			res = val;
	}

	lua_pushfstring(L, "%d", res);

	return 1;
}

static int mlua__mod(lua_State *L)
{
	int x = luaL_optinteger(L, 1, 0);
	int y = luaL_optinteger(L, 2, 0);

	lua_pushfstring(L, "%d", x % y);

	return 1;
}

static int mlua__mul(lua_State *L)
{
	int x = luaL_optinteger(L, 1, 0);
	int y = luaL_optinteger(L, 2, 0);

	lua_pushfstring(L, "%d", x * y);

	return 1;
}

static int mlua__muldiv(lua_State *L)
{
	int x = luaL_optinteger(L, 1, 0);
	int y = luaL_optinteger(L, 2, 0);
	int z = luaL_optinteger(L, 3, 0);

	lua_pushfstring(L, "%d", (x * y) / z);

	return 1;
}

static int mlua__num(lua_State *L)
{
	int x = luaL_optinteger(L, 1, 0);
	int pad = luaL_optinteger(L, 2, 0);

	CMStringA format(FORMAT, "%%0%dx", pad);
	CMStringA value(FORMAT, format, x);
	lua_pushstring(L, value);

	return 1;
}

static int mlua__rand(lua_State *L)
{
	lua_pushfstring(L, "%d", rand());

	return 1;
}

static wchar_t* translate(lua_State *L, const wchar_t *format, const wchar_t *extra, MCONTACT hContact = NULL)
{
	std::wregex regex(L"\\?([a-z_0-9]+)\\(");
	std::wstring query = std::regex_replace(format, regex, L"_v$1(");
	query.insert(0, L"return ");

	CMLuaEnvironment env(L);
	wchar_t *result = env.Eval(query.c_str());
	env.Unload();

	return result;
}

static int mlua_vars(lua_State *L)
{
	ptrW format(mir_utf8decodeW(lua_tostring(L, 1)));
	ptrW extra(mir_utf8decodeW(lua_tostring(L, 2)));
	MCONTACT hContact = lua_tointeger(L, 3);

	ptrW result(translate(L, format, extra, hContact));
	lua_pushstring(L, T2Utf(result));

	return 1;
}

/***********************************************/

INT_PTR FormatString(void *obj, WPARAM wParam, LPARAM)
{
	lua_State *L = (lua_State*)obj;
	FORMATINFO *fi = (FORMATINFO*)wParam;

	ptrW result;
	if ((fi->flags & FIF_UNICODE) == 0) {
		_A2T format(fi->szFormat.a);
		_A2T extra(fi->szExtraText.a);
		result = translate(L, format, extra, fi->hContact);
	}
	else
		result = translate(L, fi->szFormat.w, fi->szExtraText.w, fi->hContact);

	lua_pushstring(L, T2Utf(result));

	return (INT_PTR)result.detach();
}

/***********************************************/

void CMLuaVariablesLoader::LoadVariables()
{
	if (ServiceExists(MS_VARS_FORMATSTRING))
		return;

	// CreateServiceFunctionObj(MS_VARS_FORMATSTRING, FormatString, L);

	Log("Loading variables functions");

	lua_register(L, "vars", mlua_vars);
	// math
	lua_register(L, "_vadd", mlua__add);
	lua_register(L, "_vdiv", mlua__div);
	lua_register(L, "_vhex", mlua__hex);
	lua_register(L, "_vmax", mlua__max);
	lua_register(L, "_vmin", mlua__min);
	lua_register(L, "_vmod", mlua__mod);
	lua_register(L, "_vmul", mlua__mul);
	lua_register(L, "_vmuldiv", mlua__muldiv);
	lua_register(L, "_vnum", mlua__num);
	lua_register(L, "_vrand", mlua__rand);
}

void CMLuaVariablesLoader::Load(lua_State *L)
{
	CMLuaVariablesLoader loader(L);
	loader.LoadVariables();
}
