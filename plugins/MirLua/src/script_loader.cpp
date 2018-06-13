#include "stdafx.h"

HANDLE g_hCLibsFolder = nullptr;
HANDLE g_hScriptsFolder = nullptr;

CMLuaScriptLoader::CMLuaScriptLoader(lua_State *L)
	: L(L)
{
}

void CMLuaScriptLoader::SetPaths()
{
	wchar_t path[MAX_PATH];

	lua_getglobal(L, LUA_LOADLIBNAME);

	FoldersGetCustomPathT(g_hCLibsFolder, path, _countof(path), VARSW(MIRLUA_PATHT));
	lua_pushfstring(L, "%s\\?.%s", T2Utf(path), _T2A(LUACLIBSCRIPTEXT));
	lua_setfield(L, -2, "cpath");

	FoldersGetCustomPathT(g_hScriptsFolder, path, _countof(path), VARSW(MIRLUA_PATHT));
	lua_pushfstring(L, "%s\\?.%s", T2Utf(path), _T2A(LUATEXTSCRIPTEXT));
	lua_setfield(L, -2, "path");

	lua_pushfstring(L, "%s\\?.%s", T2Utf(path), _T2A(LUAPRECSCRIPTEXT));
	lua_setfield(L, -2, "path");

	lua_pop(L, 1);
}

void CMLuaScriptLoader::LoadScript(const wchar_t *scriptDir, const wchar_t *fileName)
{
	wchar_t fullPath[MAX_PATH];
	wchar_t path[MAX_PATH];
	mir_snwprintf(fullPath, L"%s\\%s", scriptDir, fileName);
	PathToRelativeW(fullPath, path);

	CMLuaScript *script = new CMLuaScript(L, path);

	const CMLuaScript *found = g_plugin.Scripts.find(script);
	if (found != nullptr) {
		Log(L"%s:PASS", script->GetFilePath());
		delete script;
		return;
	}

	if (!script->IsEnabled()) {
		Log(L"%s:PASS", path);
		return;
	}

	g_plugin.Scripts.insert(script);

	if (script->Load())
		Log(L"%s:OK", path);
}

void CMLuaScriptLoader::LoadScripts()
{
	SetPaths();

	wchar_t scriptDir[MAX_PATH];
	FoldersGetCustomPathT(g_hScriptsFolder, scriptDir, _countof(scriptDir), VARSW(MIRLUA_PATHT));

	Log(L"Loading scripts from %s", scriptDir);

	WIN32_FIND_DATA fd;
	wchar_t searchMask[MAX_PATH];

	// load compiled scripts
	mir_snwprintf(searchMask, L"%s\\*.%s", scriptDir, LUAPRECSCRIPTEXT);
	HANDLE hFind = FindFirstFile(searchMask, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;
			LoadScript(scriptDir, fd.cFileName);
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}

	// load text scripts
	mir_snwprintf(searchMask, L"%s\\*.%s", scriptDir, LUATEXTSCRIPTEXT);
	hFind = FindFirstFile(searchMask, &fd);
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