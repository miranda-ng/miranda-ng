#include "stdafx.h"

CLuaScriptLoader::CLuaScriptLoader(lua_State *L, HANDLE hLogger) : L(L), hLogger(hLogger)
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

void CLuaScriptLoader::LoadScript(const TCHAR *path, const TCHAR *name)
{
	if (luaL_dofile(L, T2Utf(path)))
	{
		ptrT error(mir_utf8decodeT(lua_tostring(L, -1)));
		mir_writeLogT(hLogger, _T("  %s:FAIL\n    %s\n"), name, error);
		printf("%s\n", lua_tostring(L, -1));
	}
	else mir_writeLogT(hLogger, _T("  %s:OK\n"), name);
}

void CLuaScriptLoader::LoadScripts(const TCHAR *scriptDir)
{
	mir_writeLogT(hLogger, _T("Loading scripts from path %s\n"), scriptDir);
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
					LoadScript(fullPath, fd.cFileName);
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
	mir_writeLogT(hLogger, _T("\n"), scriptDir);
}

void CLuaScriptLoader::Load(lua_State *L, HANDLE hLogger)
{
	TCHAR scriptDir[MAX_PATH];
	CLuaScriptLoader loader(L, hLogger);

	FoldersGetCustomPathT(g_hCommonFolderPath, scriptDir, _countof(scriptDir), VARST(COMMON_SCRIPTS_PATHT));
	loader.LoadScripts(scriptDir);

	FoldersGetCustomPathT(g_hCustomFolderPath, scriptDir, _countof(scriptDir), VARST(CUSTOM_SCRIPTS_PATHT));
	loader.LoadScripts(scriptDir);
	
	mir_writeLogT(hLogger, _T("\n"), scriptDir);
}