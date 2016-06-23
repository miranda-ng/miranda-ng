#include "stdafx.h"

CMLuaScriptLoader::CMLuaScriptLoader(lua_State *L) : L(L)
{
}

void CMLuaScriptLoader::LoadScript(const TCHAR *scriptDir, const TCHAR *file)
{
	TCHAR fullPath[MAX_PATH], path[MAX_PATH];
	mir_sntprintf(fullPath, _T("%s\\%s"), scriptDir, file);
	PathToRelativeT(fullPath, path);

	CMLuaScript *script = new CMLuaScript(path);
	g_mLua->Scripts.insert(script);

	if (db_get_b(NULL, MODULE, _T2A(file), 1) == FALSE)
	{
		Log(_T("%s:PASS"), path);
		return;
	}

	if (script->Load(L))
		Log(_T("%s:OK"), path);
}

void CMLuaScriptLoader::LoadScripts()
{
	TCHAR scriptDir[MAX_PATH];
	FoldersGetCustomPathT(g_hScriptsFolder, scriptDir, _countof(scriptDir), VARST(MIRLUA_PATHT));

	Log(_T("Loading scripts from %s"), scriptDir);

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
			LoadScript(scriptDir, fd.cFileName);
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
}

void CMLuaScriptLoader::Load(lua_State *L)
{
	CMLuaScriptLoader(L).LoadScripts();
}