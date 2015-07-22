#include "stdafx.h"

typedef int(__stdcall *MSGBOXAAPI)(IN HWND hWnd, IN LPCSTR lpText, IN LPCSTR lpCaption, IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);
typedef int(__stdcall *MSGBOXWAPI)(IN HWND hWnd, IN LPCWSTR lpText, IN LPCWSTR lpCaption, IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);

int MessageBoxTimeoutA(IN HWND hWnd, IN LPCSTR lpText, IN LPCSTR lpCaption, IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);
int MessageBoxTimeoutW(IN HWND hWnd, IN LPCWSTR lpText, IN LPCWSTR lpCaption, IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);

#ifdef UNICODE
	#define MessageBoxTimeout MessageBoxTimeoutW
#else
	#define MessageBoxTimeout MessageBoxTimeoutA
#endif

#define MB_TIMEDOUT 32000

int MessageBoxTimeoutA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType, WORD wLanguageId, DWORD dwMilliseconds)
{
	static MSGBOXAAPI MsgBoxTOA = NULL;

	if (!MsgBoxTOA)
	{
		if (HMODULE hUser32 = GetModuleHandle(_T("user32.dll")))
		{
			MsgBoxTOA = (MSGBOXAAPI)GetProcAddress(hUser32, "MessageBoxTimeoutA");
			FreeLibrary(hUser32);
		}
	}

	if (MsgBoxTOA)
	{
		return MsgBoxTOA(hWnd, lpText, lpCaption, uType, wLanguageId, dwMilliseconds);
	}

	return MessageBoxA(hWnd, lpText, lpCaption, uType);
}

int MessageBoxTimeoutW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType, WORD wLanguageId, DWORD dwMilliseconds)
{
	static MSGBOXWAPI MsgBoxTOW = NULL;

	if (!MsgBoxTOW)
	{
		if (HMODULE hUser32 = GetModuleHandle(_T("user32.dll")))
		{
			MsgBoxTOW = (MSGBOXWAPI)GetProcAddress(hUser32, "MessageBoxTimeoutW");
			FreeLibrary(hUser32);
		}
		
	}

	if (MsgBoxTOW)
	{
		return MsgBoxTOW(hWnd, lpText, lpCaption, uType, wLanguageId, dwMilliseconds);
	}

	return MessageBoxW(hWnd, lpText, lpCaption, uType);
}

static int lua_MessageBox(lua_State *L)
{
	HWND hwnd = (HWND)lua_touserdata(L, 1);
	ptrT text(mir_utf8decodeT(lua_tostring(L, 2)));
	ptrT caption(mir_utf8decodeT(lua_tostring(L, 3)));
	UINT flags = lua_tointeger(L, 4);
	LANGID langId = GetUserDefaultUILanguage();
	DWORD timeout = luaL_optinteger(L, 5, 0xFFFFFFFF);

	int res = ::MessageBoxTimeout(hwnd, text, caption, flags, langId, timeout);
	lua_pushinteger(L, res);

	return 1;
}

static int lua_ShellExecute(lua_State *L)
{
	ptrT command(mir_utf8decodeT(lua_tostring(L, 1)));
	ptrT file(mir_utf8decodeT(lua_tostring(L, 2)));
	ptrT args(mir_utf8decodeT(lua_tostring(L, 3)));
	int flags = lua_tointeger(L, 4);

	::ShellExecute(NULL, command, file, args, NULL, flags);

	return 0;
}

static int lua_GetIniValue(lua_State *L)
{
	ptrT path(mir_utf8decodeT(luaL_checkstring(L, 1)));
	ptrT section(mir_utf8decodeT(luaL_checkstring(L, 2)));
	ptrT key(mir_utf8decodeT(luaL_checkstring(L, 3)));

	if (lua_isinteger(L, 4))
	{
		int default = lua_tointeger(L, 4);

		UINT res = ::GetPrivateProfileInt(section, key, default, path);
		lua_pushinteger(L, res);

		return 1;
	}

	ptrT default(mir_utf8decodeT(lua_tostring(L, 4)));

	TCHAR value[MAX_PATH] = { 0 };
	if (!::GetPrivateProfileString(section, key, default, value, _countof(value), path))
	{
		lua_pushvalue(L, 4);
	}

	ptrA res(mir_utf8encodeT(value));
	lua_pushstring(L, res);

	return 1;
}

static int lua_SetIniValue(lua_State *L)
{
	ptrT path(mir_utf8decodeT(luaL_checkstring(L, 1)));
	ptrT section(mir_utf8decodeT(luaL_checkstring(L, 2)));
	ptrT key(mir_utf8decodeT(luaL_checkstring(L, 3)));
	ptrT value(mir_utf8decodeT(lua_tostring(L, 4)));

	bool res = ::WritePrivateProfileString(section, key, value, path) != 0;
	lua_pushboolean(L, res);

	return 1;
}

static luaL_Reg winApi[] =
{
	{ "MessageBox", lua_MessageBox },

	{ "GetIniValue", lua_GetIniValue },
	{ "SetIniValue", lua_SetIniValue },

	{ "ShellExecute", lua_ShellExecute },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_windows(lua_State *L)
{
	luaL_newlib(L, winApi);

	return 1;
}
