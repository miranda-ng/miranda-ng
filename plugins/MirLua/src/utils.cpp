#include "stdafx.h"

void Log(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	Netlib_Log(g_plugin.hNetlib, CMStringA().FormatV(format, args));
	va_end(args);
}

void Log(const wchar_t *format, ...)
{
	va_list args;
	va_start(args, format);
	Netlib_LogW(g_plugin.hNetlib, CMStringW().FormatV(format, args));
	va_end(args);
}

void ShowNotification(const char *caption, const char *message, int flags, MCONTACT hContact)
{
	if (Miranda_IsTerminated())
		return;

	if (Popup_Enabled()) {
		POPUPDATA ppd;
		ppd.lchContact = hContact;
		mir_strncpy(ppd.lpzContactName, caption, MAX_CONTACTNAME);
		mir_strncpy(ppd.lpzText, message, MAX_SECONDLINE);

		if (!PUAddPopup(&ppd))
			return;
	}

	MessageBoxA(nullptr, message, caption, MB_OK | flags);
}

void ObsoleteMethod(lua_State *L, const char *message)
{
	lua_Debug ar;
	if (lua_getstack(L, 0, &ar) == 0 || lua_getinfo(L, "n", &ar) == 0)
		return;

	char text[512];
	mir_snprintf(text, "%s is obsolete. %s", ar.name, message);
	Log(text);
	if (g_plugin.getByte("PopupOnObsolete", 0))
		ShowNotification(MODULENAME, text, MB_OK | MB_ICONWARNING, NULL);
}

void ReportError(lua_State *L)
{
	const char *message = lua_tostring(L, -1);
	Log(message);
	if (g_plugin.getByte("PopupOnError", 0))
		ShowNotification(MODULENAME, message, MB_OK | MB_ICONERROR);
}

int luaM_atpanic(lua_State *L)
{
	ReportError(L);
	return 0;
}

int luaM_pcall(lua_State *L, int n, int r)
{
	int res = lua_pcall(L, n, r, 0);
	if (res != LUA_OK)
		ReportError(L);
	return res;
}

int luaM_getenv(lua_State *L)
{
	lua_Debug ar;
	if (lua_getstack(L, 1, &ar) == 0 ||
		lua_getinfo(L, "f", &ar) == 0 ||
		lua_iscfunction(L, -1)) {
		lua_pop(L, 1);
		return 0;
	}

	const char *env = lua_getupvalue(L, -1, 1);
	if (!env || strcmp(env, "_ENV") != 0) {
		lua_pop(L, 1);
		return 0;
	}

	return 1;
}

bool luaM_toboolean(lua_State *L, int idx)
{
	if (lua_isinteger(L, idx))
		return lua_tointeger(L, idx) > 0;
	return lua_toboolean(L, idx) > 0;
}

bool luaM_isarray(lua_State *L, int idx)
{
	luaL_checktype(L, idx, LUA_TTABLE);
	int i = 0;
	for (lua_pushnil(L); lua_next(L, idx); lua_pop(L, 2)) {
		if (lua_rawgeti(L, idx, ++i) == LUA_TNIL) {
			lua_pop(L, 3);
			return false;
		}
	}
	return true;
}
