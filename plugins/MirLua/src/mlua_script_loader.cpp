#include "stdafx.h"

CMLuaScriptLoader::CMLuaScriptLoader(lua_State *L) : L(L)
{
}

void CMLuaScriptLoader::LoadScript(const wchar_t *scriptDir, const wchar_t *file)
{
	wchar_t fullPath[MAX_PATH], path[MAX_PATH];
	mir_snwprintf(fullPath, L"%s\\%s", scriptDir, file);
	PathToRelativeW(fullPath, path);

	CMLuaScript *script = new CMLuaScript(L, path);
	g_mLua->Scripts.insert(script);

	if (db_get_b(NULL, MODULE, _T2A(file), 1) == FALSE)
	{
		Log(L"%s:PASS", path);
		return;
	}

	if (script->Load())
		Log(L"%s:OK", path);
}

void CMLuaScriptLoader::LoadScripts()
{
	wchar_t scriptDir[MAX_PATH];
	FoldersGetCustomPathT(g_hScriptsFolder, scriptDir, _countof(scriptDir), VARSW(MIRLUA_PATHT));

	Log(L"Loading scripts from %s", scriptDir);

	wchar_t searchMask[MAX_PATH];
	mir_snwprintf(searchMask, L"%s\\%s", scriptDir, L"*.lua");

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