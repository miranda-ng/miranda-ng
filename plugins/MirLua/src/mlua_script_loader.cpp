#include "stdafx.h"

CLuaScriptLoader::CLuaScriptLoader(lua_State *L) : L(L)
{
}

void CLuaScriptLoader::RegisterScriptsFolder(const char *path)
{
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");
	const char *oldPath = luaL_checkstring(L, -1);
	lua_pop(L, 1);
	if (!mir_strlen(oldPath))
		lua_pushfstring(L, "%s\\?.lua", path);
	else
		lua_pushfstring(L, "%s;%s\\?.lua", oldPath, path);
	lua_setfield(L, -2, "path");
	lua_pop(L, 1);
}

void CLuaScriptLoader::LoadScript(const TCHAR *path)
{
	
	if (luaL_loadfile(L, T2Utf(path)))
	{
		CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)lua_tostring(L, -1));
		return;
	}

	if (lua_pcall(L, 0, 1, 0))
	{
		CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)lua_tostring(L, -1));
		return;
	}

	TCHAR buf[4096];
	mir_sntprintf(buf, _T("%s:OK"), path);
	CallService(MS_NETLIB_LOGW, (WPARAM)hNetlib, (LPARAM)buf);

	if (lua_istable(L, -1))
	{
		lua_pushliteral(L, "Load");
		lua_gettable(L, -2);
		if (lua_isfunction(L, -1) && lua_pcall(L, 0, 0, 0))
			CallService(MS_NETLIB_LOG, (WPARAM)hNetlib, (LPARAM)lua_tostring(L, -1));
	}
}

void CLuaScriptLoader::LoadScripts(const TCHAR *scriptDir)
{
	TCHAR buf[4096];
	mir_sntprintf(buf, _T("Loading scripts from %s"), scriptDir);
	CallService(MS_NETLIB_LOGW, (WPARAM)hNetlib, (LPARAM)buf);

	RegisterScriptsFolder(ptrA(mir_utf8encodeT(scriptDir)));

	TCHAR searchMask[MAX_PATH];
	mir_sntprintf(searchMask, _T("%s\\%s"), scriptDir, _T("*.lua"));

	TCHAR fullPath[MAX_PATH], path[MAX_PATH];

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(searchMask, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				mir_sntprintf(fullPath, _T("%s\\%s"), scriptDir, fd.cFileName);
				PathToRelativeT(fullPath, path);
				if (db_get_b(NULL, MODULE, _T2A(fd.cFileName), 1))
					LoadScript(fullPath);
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
}

void CLuaScriptLoader::UnloadScript(const TCHAR *path)
{
	const TCHAR* p = _tcsrchr(path, '\\') + 1;
	size_t length = mir_tstrlen(p) - 3;

	ptrT name((TCHAR*)mir_alloc(sizeof(TCHAR) * length));
	mir_tstrncpy(name, p, length);

	T2Utf moduleName(name);

	luaL_getsubtable(L, LUA_REGISTRYINDEX, "_LOADED");
	lua_pushnil(L);
	lua_setfield(L, -2, moduleName);
	lua_pop(L, 1);

	lua_pushnil(L);
	lua_setglobal(L, moduleName);
}

void CLuaScriptLoader::ReloadScript(const TCHAR *path)
{
	UnloadScript(path);
	LoadScript(path);
}

void CLuaScriptLoader::Load(lua_State *L)
{
	TCHAR scriptDir[MAX_PATH];
	CLuaScriptLoader loader(L);

	FoldersGetCustomPathT(g_hCommonFolderPath, scriptDir, _countof(scriptDir), VARST(COMMON_SCRIPTS_PATHT));
	loader.LoadScripts(scriptDir);

	FoldersGetCustomPathT(g_hCustomFolderPath, scriptDir, _countof(scriptDir), VARST(CUSTOM_SCRIPTS_PATHT));
	loader.LoadScripts(scriptDir);
}

void CLuaScriptLoader::Reload(lua_State *L, const TCHAR *path)
{
	CLuaScriptLoader loader(L);
	loader.ReloadScript(path);
}