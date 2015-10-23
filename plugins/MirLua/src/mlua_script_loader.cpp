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

void CLuaScriptLoader::LoadScript(const TCHAR *scriptDir, const TCHAR *file, int iGroup)
{
	TCHAR fullPath[MAX_PATH], path[MAX_PATH];
	mir_sntprintf(fullPath, _T("%s\\%s"), scriptDir, file);
	PathToRelativeT(fullPath, path);

	CMLuaScript *script = new CMLuaScript(L, path, iGroup);
	g_mLua->Scripts.insert(script);

	TCHAR buf[4096];
	if (db_get_b(NULL, MODULE, _T2A(file), 1) == FALSE)
	{
		
		mir_sntprintf(buf, _T("%s:PASS"), path);
		CallService(MS_NETLIB_LOGW, (WPARAM)hNetlib, (LPARAM)buf);
		return;
	}

	if (script->Load())
	{
		TCHAR buf[4096];
		mir_sntprintf(buf, _T("%s:OK"), path);
		CallService(MS_NETLIB_LOGW, (WPARAM)hNetlib, (LPARAM)buf);
	}
}

void CLuaScriptLoader::LoadScripts(const TCHAR *scriptDir, int iGroup)
{
	TCHAR buf[4096];
	mir_sntprintf(buf, _T("Loading scripts from %s"), scriptDir);
	CallService(MS_NETLIB_LOGW, (WPARAM)hNetlib, (LPARAM)buf);

	RegisterScriptsFolder(ptrA(mir_utf8encodeT(scriptDir)));

	TCHAR searchMask[MAX_PATH];
	mir_sntprintf(searchMask, _T("%s\\%s"), scriptDir, _T("*.lua"));

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(searchMask, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;
			LoadScript(scriptDir, fd.cFileName, iGroup);
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
}

void CLuaScriptLoader::Load(lua_State *L)
{
	TCHAR scriptDir[MAX_PATH];
	CLuaScriptLoader loader(L);

	FoldersGetCustomPathT(g_hCommonScriptFolder, scriptDir, _countof(scriptDir), VARST(COMMON_SCRIPTS_PATHT));
	loader.LoadScripts(scriptDir, 0);
}