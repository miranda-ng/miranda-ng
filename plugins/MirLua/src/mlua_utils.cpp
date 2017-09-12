#include "stdafx.h"

void Log(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	Netlib_Log(hNetlib, CMStringA().FormatV(format, args));
	va_end(args);
}

void Log(const wchar_t *format, ...)
{
	va_list args;
	va_start(args, format);
	Netlib_LogW(hNetlib, CMStringW().FormatV(format, args));
	va_end(args);
}

void ShowNotification(const char *caption, const char *message, int flags, MCONTACT hContact)
{
	if (Miranda_IsTerminated())
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

void ReportError(lua_State *L)
{
	const char *message = lua_tostring(L, -1);
	Log(message);
	if (db_get_b(NULL, MODULE, "PopupOnError", 0))
		ShowNotification(MODULE, message, MB_OK | MB_ICONERROR);
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
		lua_iscfunction(L, -1))
	{
		lua_pop(L, 1);
		return 0;
	}

	const char *env = lua_getupvalue(L, -1, 1);
	if (!env || strcmp(env, "_ENV") != 0)
	{
		lua_pop(L, 1);
		return 0;
	}

	return 1;
}

bool luaM_toboolean(lua_State *L, int idx)
{
	if (lua_isnumber(L, idx))
		return lua_tonumber(L, idx) > 0;
	return lua_toboolean(L, idx) > 0;
}