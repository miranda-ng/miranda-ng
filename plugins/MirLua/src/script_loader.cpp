#include "stdafx.h"

HANDLE g_hCLibsFolder = nullptr;
HANDLE g_hScriptsFolder = nullptr;

CMLuaScriptLoader::CMLuaScriptLoader(lua_State *L, OBJLIST<CMLuaScript> &scripts)
	: L(L), m_scripts(scripts)
{
}

void CMLuaScriptLoader::SetPaths()
{
	wchar_t path[MAX_PATH];
	ptrA pathA(nullptr);

	lua_getglobal(L, LUA_LOADLIBNAME);

	FoldersGetCustomPathW(g_hCLibsFolder, path, _countof(path), VARSW(MIRLUA_PATHT));
	pathA = mir_utf8encodeW(path);
	lua_pushfstring(L, "%s\\?.%s", pathA.get(), _T2A(LUACLIBSCRIPTEXT).get());
	lua_setfield(L, -2, "cpath");

	FoldersGetCustomPathW(g_hScriptsFolder, path, _countof(path), VARSW(MIRLUA_PATHT));
	pathA = mir_utf8encodeW(path);
	lua_pushfstring(L, "%s\\?.%s;%s\\?.%s", pathA.get(), _T2A(LUATEXTSCRIPTEXT).get(), pathA.get(), _T2A(LUAPRECSCRIPTEXT).get());
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

	const CMLuaScript *found = m_scripts.find(script);
	if (found != nullptr) {
		Log(L"%s:PASS", script->GetFilePath());
		delete script;
		return;
	}

	m_scripts.insert(script);

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
	FoldersGetCustomPathW(g_hScriptsFolder, scriptDir, _countof(scriptDir), VARSW(MIRLUA_PATHT));

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

void CMLuaScriptLoader::Load(lua_State *L, OBJLIST<CMLuaScript> &scripts)
{
	CMLuaScriptLoader(L, scripts).LoadScripts();
}