#include "stdafx.h"

void Log(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)(CMStringA().FormatV(format, args)));
	va_end(args);
}

void Log(const wchar_t *format, ...)
{
	va_list args;
	va_start(args, format);
	CallService(MS_NETLIB_LOGW, (WPARAM)hNetlib, (LPARAM)(CMStringW().FormatV(format, args)));
	va_end(args);
}

void ShowNotification(const char *caption, const char *message, int flags, MCONTACT hContact)
{
	if (Miranda_Terminated())
		return;

	if (ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1))
	{
		POPUPDATA ppd = { 0 };
		ppd.lchContact = hContact;
		mir_strncpy(ppd.lpzContactName, caption, MAX_CONTACTNAME);
		mir_strncpy(ppd.lpzText, message, MAX_SECONDLINE);

		if (!PUAddPopup(&ppd))
			return;
	}

	MessageBoxA(NULL, message, caption, MB_OK | flags);
}

void ObsoleteMethod(lua_State *L, const char *message)
{
	lua_Debug ar;
	if (lua_getstack(L, 0, &ar) == 0 || lua_getinfo(L, "n", &ar) == 0)
		return;

	char text[512];
	mir_snprintf(text, "%s is obsolete. %s", ar.name, message);
	Log(text);
	if (db_get_b(NULL, MODULE, "PopupOnObsolete", 0))
		ShowNotification(MODULE, text, MB_OK | MB_ICONWARNING, NULL);
}

void ReportError(const char *message)
{
	Log(message);
	if (db_get_b(NULL, MODULE, "PopupOnError", 0))
		ShowNotification(MODULE, message, MB_OK | MB_ICONERROR);
}

int luaM_atpanic(lua_State *L)
{
	ReportError(lua_tostring(L, -1));

	return 0;
}

int luaM_pcall(lua_State *L, int n, int r)
{
	int res = lua_pcall(L, n, r, 0);
	if (res != LUA_OK)
		ReportError(lua_tostring(L, -1));
	return res;
}

int luaM_print(lua_State *L)
{
	CMStringA data;
	int nargs = lua_gettop(L);
	for (int i = 1; i <= nargs; i++)
	{
		switch (lua_type(L, i))
		{
		case LUA_TNIL:
			data.Append("nil   ");
			break;
		case LUA_TBOOLEAN:
			data.AppendFormat("%s   ", lua_toboolean(L, i) ? "true" : "false");
			break;
		case LUA_TNUMBER:
		case LUA_TSTRING:
			data.AppendFormat("%s   ", lua_tostring(L, i));
			break;
		default:
			data.AppendFormat("%s(0x%p)   ", luaL_typename(L, i), lua_topointer(L, i));
			break;
		}
	}
	if (data.GetLength() >= 3)
		data.Delete(data.GetLength() - 3, 3);

	Log(data.GetBuffer());

	return 0;
}

int luaM_toansi(lua_State *L)
{
	const char *value = luaL_checkstring(L, 1);
	int codepage = luaL_optinteger(L, 2, Langpack_GetDefaultCodePage());

	ptrA string(mir_strdup(value));
	lua_pushstring(L, mir_utf8decodecp(string, codepage, NULL));

	return 1;
}

int luaM_toucs2(lua_State *L)
{
	const char *value = luaL_checkstring(L, 1);

	ptrW unicode(mir_utf8decodeW(value));
	size_t length = mir_wstrlen(unicode) * sizeof(wchar_t);

	ptrA string((char*)mir_calloc(length + 1));
	memcpy(string, unicode, length);

	lua_pushlstring(L, string, length + 1);

	return 1;
}

int luaM_topointer(lua_State *L)
{
	switch (lua_type(L, 1))
	{
	case LUA_TBOOLEAN:
		lua_pushlightuserdata(L, (void*)lua_toboolean(L, 1));
		break;
	case LUA_TNUMBER:
	{
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

int luaM_tonumber(lua_State *L)
{
	if (lua_islightuserdata(L, 1))
	{
		lua_Integer value = (lua_Integer)lua_touserdata(L, 1);
		lua_pushinteger(L, value);
		return 1;
	}

	int n = lua_gettop(L);
	lua_getglobal(L, "_tonumber");
	lua_pushvalue(L, 1);
	if (n == 2)
		lua_pushvalue(L, 2);
	luaM_pcall(L, n, 1);

	return 1;
}

UINT_PTR luaM_tomparam(lua_State *L, int idx)
{
	switch (lua_type(L, idx))
	{
	case LUA_TBOOLEAN:
		return lua_toboolean(L, idx);
	case LUA_TSTRING:
		return (UINT_PTR)lua_tostring(L, idx);
	case LUA_TLIGHTUSERDATA:
		return (UINT_PTR)lua_touserdata(L, idx);
	case LUA_TNUMBER:
	{
		if (lua_isinteger(L, 1))
		{
			lua_Integer value = lua_tointeger(L, 1);
			return value <= INTPTR_MAX
				? (UINT_PTR)value
				: NULL;
		}
	}
	default:
		return NULL;
	}
}

int luaM_interpolate(lua_State *L)
{
	const char *string = luaL_checkstring(L, 1);

	char pattern[128];

	if (lua_istable(L, 2))
	{
		for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 2))
		{
			lua_pushvalue(L, -2);
			const char *key = lua_tostring(L, -1);
			const char *val = lua_tostring(L, -2);

			mir_snprintf(pattern, "{%s}", key);
			string = luaL_gsub(L, string, pattern, val);
			lua_pop(L, 1);
		}
	}
	else
	{
		int nargs = lua_gettop(L);
		for (int i = 2; i <= nargs; i++)
		{
			const char *val = lua_tostring(L, i);

			mir_snprintf(pattern, "{%d}", i - 1);
			string = luaL_gsub(L, string, pattern, val);
			lua_pop(L, 1);
		}
	}

	lua_Debug ar;
	lua_getstack(L, 1, &ar);
	const char *name = lua_getlocal(L, &ar, 1);

	size_t i = 1;
	while (const char *key = lua_getlocal(L, &ar, i++))
	{
		const char *val = lua_tostring(L, -1);

		mir_snprintf(pattern, "${%s}", name);
		string = luaL_gsub(L, string, pattern, val);
		lua_pop(L, 1);
	}

	lua_pushstring(L, string);

	return 1;
}

bool luaM_toboolean(lua_State *L, int idx)
{
	if (lua_isnumber(L, idx))
		return lua_tonumber(L, idx) > 0;
	return lua_toboolean(L, idx) > 0;
}