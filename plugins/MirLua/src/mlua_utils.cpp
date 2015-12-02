#include "stdafx.h"

int luaM_print(lua_State *L)
{
	CMStringA data;
	int nargs = lua_gettop(L);
	for (int i = 1; i <= nargs; i++)
	{
		switch (lua_type(L, i))
		{
		case LUA_TNIL:
			data.AppendFormat("%s   ", "nil");
			break;
		case LUA_TBOOLEAN:
			data.AppendFormat("%s   ", lua_toboolean(L, i) ? "true" : "false");
			break;
		case LUA_TNUMBER:
		case LUA_TSTRING:
			data.AppendFormat("%s   ", lua_tostring(L, i));
			break;
		case LUA_TTABLE:
			data.AppendFormat("%s   ", "table");
			break;
		default:
			data.AppendFormat("0x%p   ", lua_topointer(L, i));
			break;
		}
	}
	if (data.GetLength() >= 3)
		data.Delete(data.GetLength() - 3, 3);

	CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)data.GetBuffer());

	return 0;
}

int luaM_atpanic(lua_State *L)
{
	CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)lua_tostring(L, -1));

	return 0;
}

int luaM_toansi(lua_State *L)
{
	const char* value = luaL_checkstring(L, 1);
	int codepage = luaL_optinteger(L, 2, Langpack_GetDefaultCodePage());

	ptrA string(mir_strdup(value));
	lua_pushstring(L, mir_utf8decodecp(string, codepage, NULL));

	return 1;
}

int luaM_toucs2(lua_State *L)
{
	const char* value = luaL_checkstring(L, 1);

	ptrW unicode(mir_utf8decodeW(value));
	size_t length = mir_wstrlen(unicode) * sizeof(wchar_t);

	ptrA string((char*)mir_calloc(length + 1));
	memcpy(string, unicode, length);

	lua_pushlstring(L, string, length + 1);

	return 1;
}

bool luaM_toboolean(lua_State *L, int idx)
{
	if (lua_type(L, idx) == LUA_TNUMBER)
		return lua_tonumber(L, idx) != 0;
	return lua_toboolean(L, idx) > 0;
}

WPARAM luaM_towparam(lua_State *L, int idx)
{
	switch (lua_type(L, idx))
	{
	case LUA_TBOOLEAN:
		return lua_toboolean(L, idx);
	case LUA_TNUMBER:
		return lua_tonumber(L, idx);
	case LUA_TSTRING:
		return (WPARAM)lua_tostring(L, idx);
		break;
	//case LUA_TUSERDATA:
	case LUA_TLIGHTUSERDATA:
		return (WPARAM)lua_touserdata(L, idx);
	default:
		return NULL;
	}
}

LPARAM luaM_tolparam(lua_State *L, int idx)
{
	switch (lua_type(L, idx))
	{
	case LUA_TBOOLEAN:
		return lua_toboolean(L, idx);
	case LUA_TNUMBER:
		return lua_tonumber(L, idx);
	case LUA_TSTRING:
		return (LPARAM)lua_tostring(L, idx);
	//case LUA_TUSERDATA:
	case LUA_TLIGHTUSERDATA:
		return (LPARAM)lua_touserdata(L, idx);
	default:
		return NULL;
	}
}

int luaM_totable(lua_State *L)
{
	const char *tname = luaL_checkstring(L, 2);

	//luaL_getmetatable(L, tname);
	//lua_getfield(L, -1, "__init");
	lua_getglobal(L, tname);
	lua_pushvalue(L, 1);
	if (lua_pcall(L, 1, 1, 0))
		CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)lua_tostring(L, -1));

	return 1;
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

	::MessageBoxA(NULL, message, caption, MB_OK | flags);
}

void ObsoleteMethod(lua_State *L, const char *message)
{
	lua_Debug info;
	lua_getstack(L, 0, &info);
	lua_getinfo(L, "n", &info);

	char text[512];
	mir_snprintf(text, "%s is obsolete. %s", info.name, message);
	CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)text);
	ShowNotification(MODULE, text, MB_OK | MB_ICONWARNING, NULL);
}