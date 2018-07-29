#include "stdafx.h"

CMLuaVariablesLoader::CMLuaVariablesLoader(lua_State *L) : L(L)
{
}

/***********************************************/

static int mlua__and(lua_State *L)
{
	int nargs = lua_gettop(L);
	if (nargs < 1)
		luaL_error(L, "bad count of arguments");

	int res = 0;
	for (int i = 1; i <= nargs; i++) {
		bool res = luaM_toboolean(L, i);
		if (res == false) {
			lua_pushboolean(L, 0);
			return 1;
		}
	}

	lua_pushboolean(L, 1);

	return 1;
}

static int mlua__equal(lua_State *L)
{
	int nargs = lua_gettop(L);
	if (nargs != 2)
		luaL_error(L, "bad count of arguments");

	int x = atoi(luaL_checkstring(L, 1));
	int y = atoi(luaL_checkstring(L, 2));

	lua_pushboolean(L, x == y);

	return 1;
}

static int mlua__if(lua_State *L)
{
	int nargs = lua_gettop(L);
	if (nargs != 3)
		luaL_error(L, "bad count of arguments");

	int x = luaM_toboolean(L, 1);
	const char *res = x
		? luaL_checkstring(L, 2)
		: luaL_checkstring(L, 3);
	lua_pushstring(L, res);

	return 1;
}

/***********************************************/

static int mlua__add(lua_State *L)
{
	int nargs = lua_gettop(L);
	if (nargs < 1)
		luaL_error(L, "bad count of arguments");

	int res = 0;
	for (int i = 1; i <= nargs; i++)
		res += luaL_checkinteger(L, i);

	lua_pushfstring(L, "%d", res);

	return 1;
}

static int mlua__div(lua_State *L)
{
	int nargs = lua_gettop(L);
	if (nargs != 2)
		luaL_error(L, "bad count of arguments");

	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);

	lua_pushfstring(L, "%d", x - y);

	return 1;
}

static int mlua__hex(lua_State *L)
{
	int nargs = lua_gettop(L);
	if (nargs != 2)
		luaL_error(L, "bad count of arguments");

	int x = luaL_checkinteger(L, 1);
	int pad = luaL_checkinteger(L, 2);

	CMStringA format(FORMAT, "0x%%0%dx", pad);
	CMStringA value(FORMAT, format, x);
	lua_pushstring(L, value);

	return 1;
}

static int mlua__max(lua_State *L)
{
	int nargs = lua_gettop(L);
	if (nargs < 1)
		luaL_error(L, "bad count of arguments");

	int res = 0;
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
	int nargs = lua_gettop(L);
	if (nargs < 1)
		luaL_error(L, "bad count of arguments");

	int res = 0;

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
	int nargs = lua_gettop(L);
	if (nargs != 2)
		luaL_error(L, "bad count of arguments");

	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);

	lua_pushfstring(L, "%d", x % y);

	return 1;
}

static int mlua__mul(lua_State *L)
{
	int nargs = lua_gettop(L);
	if (nargs != 2)
		luaL_error(L, "bad count of arguments");

	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);

	lua_pushfstring(L, "%d", x * y);

	return 1;
}

static int mlua__muldiv(lua_State *L)
{
	int nargs = lua_gettop(L);
	if (nargs != 3)
		luaL_error(L, "bad count of arguments");

	int x = luaL_checkinteger(L, 1);
	int y = luaL_checkinteger(L, 2);
	int z = luaL_checkinteger(L, 3);

	lua_pushfstring(L, "%d", (x * y) / z);

	return 1;
}

static int mlua__num(lua_State *L)
{
	int nargs = lua_gettop(L);
	if (nargs != 2)
		luaL_error(L, "bad count of arguments");

	int x = luaL_checkinteger(L, 1);
	int pad = luaL_checkinteger(L, 2);

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

static luaL_Reg varsApi[] =
{
	// logic
	{ "_vand", mlua__and },
	{ "_vequal", mlua__equal },
	{ "_vif", mlua__if },
	// math
	{ "_vadd", mlua__add },
	{ "_vdiv", mlua__div },
	{ "_vhex", mlua__hex },
	{ "_vmax", mlua__max },
	{ "_vmin", mlua__min },
	{ "_vmod", mlua__mod },
	{ "_vmul", mlua__mul },
	{ "_vmuldiv", mlua__muldiv },
	{ "_vnum", mlua__num },
	{ "_vrand", mlua__rand }
};

static wchar_t* translate(lua_State *L, const wchar_t *format, const wchar_t* /*extra*/, MCONTACT /*hContact*/ = NULL)
{
	// this should quote string params but is not work yet
	std::wregex regex(L"(?<=\\(|,)`?([^0-9][a-z_0-9 ]+)`?(?=,|\\))");
	std::wstring query = std::regex_replace(format, regex, L"'$1'");

	regex = L"\\?([a-z_0-9]+)\\(";
	query = std::regex_replace(query, regex, L"_v$1(");
	query.insert(0, L"return ");

	wchar_t *result = nullptr;

	CMLuaEnvironment env(L);
	if (env.Eval(query.c_str()) == LUA_OK)
		result = mir_utf8decodeW(lua_tostring(L, -1));
	else
		result = mir_wstrdup(format);
	lua_pop(L, 1);
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

static INT_PTR FormatString(void *obj, WPARAM wParam, LPARAM)
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

static int CompareTokens(const TOKENREGISTER *p1, const TOKENREGISTER *p2)
{
	return mir_wstrcmpi(p1->szTokenString.w, p2->szTokenString.w);
}

LIST<TOKENREGISTER> tokens(10, CompareTokens);

static INT_PTR RegisterToken(void *obj, WPARAM, LPARAM lParam)
{
	lua_State *L = (lua_State*)obj;
	TOKENREGISTER *tr = (TOKENREGISTER*)lParam;

	if (tr == nullptr || tr->szTokenString.w == nullptr || tr->cbSize <= 0)
		return -1;
}

/***********************************************/

void CMLuaVariablesLoader::LoadVariables()
{
	if (ServiceExists(MS_VARS_FORMATSTRING))
		return;

	//CreateServiceFunctionObj(MS_VARS_FORMATSTRING, FormatString, L);
	//CreateServiceFunctionObj(MS_VARS_REGISTERTOKEN, RegisterToken, L);

	Log("Loading variables functions");

	lua_register(L, "vars", mlua_vars);
	
	lua_pushglobaltable(L);
	luaL_setfuncs(L, varsApi, 0);
}

void CMLuaVariablesLoader::Load(lua_State *L)
{
	CMLuaVariablesLoader loader(L);
	loader.LoadVariables();
}
