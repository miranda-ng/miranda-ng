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

static int lua_FindIterator(lua_State *L)
{
	HANDLE hFind = lua_touserdata(L, lua_upvalueindex(1));
	TCHAR* path = (TCHAR*)lua_touserdata(L, lua_upvalueindex(2));

	WIN32_FIND_DATA ffd = { 0 };
	if (hFind == NULL)
	{
		hFind = FindFirstFile(path, &ffd);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			mir_free(path);
			lua_pushnil(L);
			return 1;
		}
	}
	else
	{
		if (FindNextFile(hFind, &ffd) == 0)
		{
			FindClose(hFind);
			mir_free(path);
			lua_pushnil(L);
			return 1;
		}
	}

	if (!mir_tstrcmpi(ffd.cFileName, _T(".")) ||
		!mir_tstrcmpi(ffd.cFileName, _T("..")) ||
		(ffd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
	{
		lua_pushlightuserdata(L, hFind);
		lua_replace(L, lua_upvalueindex(1));

		return lua_FindFileIterator(L);
	}

	lua_newtable(L);
	lua_pushliteral(L, "Name");
	lua_pushstring(L, T2Utf(ffd.cFileName));
	lua_settable(L, -3);
	lua_pushliteral(L, "Size");
	lua_pushinteger(L, (ffd.nFileSizeHigh * (MAXDWORD + 1)) + ffd.nFileSizeLow);
	lua_settable(L, -3);
	lua_pushliteral(L, "IsFile");
	lua_pushboolean(L, !(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
	lua_settable(L, -3);
	lua_pushliteral(L, "IsDirectory");
	lua_pushboolean(L, ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	lua_settable(L, -3);

	lua_pushlightuserdata(L, hFind);
	lua_replace(L, lua_upvalueindex(1));

	return 1;
}

static int lua_Find(lua_State *L)
{
	TCHAR* path = mir_utf8decodeT(luaL_checkstring(L, 1));

	lua_pushlightuserdata(L, NULL);
	lua_pushlightuserdata(L, path);
	lua_pushcclosure(L, lua_FindIterator, 2);

	return 1;
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

static int lua_DeleteIniValue(lua_State *L)
{
	ptrT path(mir_utf8decodeT(luaL_checkstring(L, 1)));
	ptrT section(mir_utf8decodeT(luaL_checkstring(L, 2)));
	ptrT key(mir_utf8decodeT(luaL_checkstring(L, 3)));

	bool res = ::WritePrivateProfileString(section, key, NULL, path) != 0;
	lua_pushboolean(L, res);

	return 1;
}

static int lua_GetRegValue(lua_State *L)
{
	HKEY hRootKey = (HKEY)lua_touserdata(L, 1);
	ptrT path(mir_utf8decodeT(luaL_checkstring(L, 2)));
	ptrT valueName(mir_utf8decodeT(luaL_checkstring(L, 3)));

	HKEY hKey = 0;
	LSTATUS res = ::RegOpenKeyEx(hRootKey, path, NULL, KEY_WRITE, &hKey);
	if (res != ERROR_SUCCESS)
	{
		lua_pushvalue(L, 4);
		return 1;
	}

	DWORD type = 0;
	DWORD length = 1024;
	BYTE* value = (BYTE*)mir_alloc(length);
	res = ::RegQueryValueEx(hKey, valueName, NULL, &type, (LPBYTE)value, &length);
	while (res == ERROR_MORE_DATA)
	{
		length += length;
		value = (BYTE*)mir_realloc(value, length);
		res = ::RegQueryValueEx(hKey, valueName, NULL, &type, (LPBYTE)value, &length);
	}

	if (res == ERROR_SUCCESS)
	{
		switch (type)
		{
		case REG_DWORD:
		case REG_DWORD_BIG_ENDIAN:
			lua_pushinteger(L, (int)value);
			break;

		case REG_QWORD:
			lua_pushnumber(L, (int)value);
			break;

		case REG_SZ:
		case REG_LINK:
		case REG_EXPAND_SZ:
			lua_pushlstring(L, ptrA(Utf8EncodeT((TCHAR*)value)), length);
			break;

		default:
			lua_pushvalue(L, 4);
			break;
		}
	}
	else
		lua_pushvalue(L, 4);

	::RegCloseKey(hKey);
	mir_free(value);

	return 1;
}

static int lua_SetRegValue(lua_State *L)
{
	HKEY hRootKey = (HKEY)lua_touserdata(L, 1);
	ptrT path(mir_utf8decodeT(luaL_checkstring(L, 2)));
	ptrT valueName(mir_utf8decodeT(luaL_checkstring(L, 3)));

	HKEY hKey = 0;
	LSTATUS res = ::RegOpenKeyEx(hRootKey, path, NULL, KEY_WRITE, &hKey);
	if (res != ERROR_SUCCESS)
	{
		lua_pushboolean(L, FALSE);
		return 1;
	}

	DWORD type = 0;
	DWORD length = 0;
	BYTE* value = NULL;
	switch (lua_type(L, 4))
	{
	case LUA_TNUMBER:
		if (lua_isinteger(L, 4) && lua_tointeger(L, 4) < UINT32_MAX)
		{
			type = REG_DWORD;
			length = sizeof(DWORD);
			value = (BYTE*)lua_tointeger(L, 4);
		}
		else
		{
			type = REG_QWORD;
			length = sizeof(DWORD) * 2;
			lua_Number num = lua_tonumber(L, 4);
			value = (BYTE*)&num;
		}
		break;

	case LUA_TSTRING:
		type = REG_SZ;
		length = mir_strlen(lua_tostring(L, 4));
		value = (BYTE*)mir_utf8decodeT(lua_tostring(L, 4));
		break;

	default:
		lua_pushboolean(L, FALSE);
		break;
	}

	res = ::RegSetValueEx(hKey, valueName, NULL, type, value, length);
	lua_pushboolean(L, res == ERROR_SUCCESS);
	
	::RegCloseKey(hKey);
	if (lua_isstring(L, 4))
		mir_free(value);

	return 1;
}

static int lua_DeleteRegValue(lua_State *L)
{
	HKEY hRootKey = (HKEY)lua_touserdata(L, 1);
	ptrT path(mir_utf8decodeT(luaL_checkstring(L, 2)));
	ptrT valueName(mir_utf8decodeT(luaL_checkstring(L, 3)));

	HKEY hKey = 0;
	LSTATUS res = ::RegOpenKeyEx(hRootKey, path, NULL, KEY_WRITE, &hKey);
	if (res != ERROR_SUCCESS)
	{
		lua_pushboolean(L, FALSE);
		return 1;
	}

	res = ::RegDeleteValue(hKey, valueName);
	lua_pushboolean(L, res == ERROR_SUCCESS);

	::RegCloseKey(hKey);

	return 1;
}

static luaL_Reg winApi[] =
{
	{ "MessageBox", lua_MessageBox },

	{ "ShellExecute", lua_ShellExecute },

	{ "Find", lua_Find },

	{ "GetIniValue", lua_GetIniValue },
	{ "SetIniValue", lua_SetIniValue },
	{ "DeleteIniValue", lua_DeleteIniValue },

	{ "GetRegValue", lua_GetRegValue },
	{ "SetRegValue", lua_SetRegValue },
	{ "DeleteRegValue", lua_DeleteRegValue },

	{ "HKEY_CLASSES_ROOT", NULL },
	{ "HKEY_CURRENT_USER", NULL },
	{ "HKEY_LOCAL_MACHINE", NULL },
	{ "HKEY_USERS", NULL },
	{ "HKEY_CURRENT_CONFIG", NULL },

	{ NULL, NULL }
};

LUAMOD_API int luaopen_m_windows(lua_State *L)
{
	luaL_newlib(L, winApi);
	lua_pushlightuserdata(L, HKEY_CLASSES_ROOT);
	lua_setfield(L, -2, "HKEY_CLASSES_ROOT");
	lua_pushlightuserdata(L, HKEY_CURRENT_USER);
	lua_setfield(L, -2, "HKEY_CURRENT_USER");
	lua_pushlightuserdata(L, HKEY_LOCAL_MACHINE);
	lua_setfield(L, -2, "HKEY_LOCAL_MACHINE");
	lua_pushlightuserdata(L, HKEY_USERS);
	lua_setfield(L, -2, "HKEY_USERS");
	lua_pushlightuserdata(L, HKEY_CURRENT_CONFIG);
	lua_setfield(L, -2, "HKEY_CURRENT_CONFIG");

	return 1;
}
