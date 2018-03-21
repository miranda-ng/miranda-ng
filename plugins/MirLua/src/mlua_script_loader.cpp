#include "stdafx.h"

CMLuaScriptLoader::CMLuaScriptLoader(lua_State *L)
	: L(L)
{
}

void CMLuaScriptLoader::SetPaths()
{
	wchar_t path[MAX_PATH];

	lua_getglobal(L, LUA_LOADLIBNAME);

	FoldersGetCustomPathT(g_hCLibsFolder, path, _countof(path), VARSW(MIRLUA_PATHT));
	lua_pushfstring(L, "%s\\?.dll", T2Utf(path));
	lua_setfield(L, -2, "cpath");

	FoldersGetCustomPathT(g_hScriptsFolder, path, _countof(path), VARSW(MIRLUA_PATHT));
	lua_pushfstring(L, "%s\\?.lua", T2Utf(path));
	lua_setfield(L, -2, "path");

	lua_pop(L, 1);
}

void CMLuaScriptLoader::LoadScript(const wchar_t *scriptDir, const wchar_t *file)
{
	wchar_t fullPath[MAX_PATH], path[MAX_PATH];
	mir_snwprintf(fullPath, L"%s\\%s", scriptDir, file);
	PathToRelativeW(fullPath, path);

	CMLuaScript *script = new CMLuaScript(L, path);
	g_mLua->Scripts.insert(script);

	if (!script->IsEnabled()) {
		Log(L"%s:PASS", path);
		return;
	}

	if (script->Load())
		Log(L"%s:OK", path);
}

void CMLuaScriptLoader::LoadScripts()
{
	SetPaths();

	wchar_t scriptDir[MAX_PATH];
	FoldersGetCustomPathT(g_hScriptsFolder, scriptDir, _countof(scriptDir), VARSW(MIRLUA_PATHT));

	Log(L"Loading scripts from %s", scriptDir);

	wchar_t searchMask[MAX_PATH];
	mir_snwprintf(searchMask, L"%s\\%s", scriptDir, L"*.lua");

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(searchMask, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
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