#include "stdafx.h"

#define MT_SCRIPT "SCRIPT"

CMLuaScript::CMLuaScript(lua_State *L, const wchar_t *path)
	: CMLuaEnvironment(L),
	isBinary(false),
	status(ScriptStatus::None),
	unloadRef(LUA_NOREF)
{
	mir_wstrcpy(filePath, path);

	const wchar_t *fileName = wcsrchr(filePath, L'\\') + 1;
	const wchar_t *dot = wcsrchr(fileName, '.');

	size_t length = mir_wstrlen(fileName) - mir_wstrlen(dot) + 1;

	scriptName = (wchar_t*)mir_calloc(sizeof(wchar_t) * (length + 1));
	mir_wstrncpy(scriptName, fileName, length);

	m_szModuleName = mir_utf8encodeW(scriptName);

	isBinary = mir_wstrcmpi(dot + 1, LUAPRECSCRIPTEXT) == 0;
}

CMLuaScript::CMLuaScript(const CMLuaScript &script)
	: CMLuaEnvironment(script.L), isBinary(script.isBinary),
	status(ScriptStatus::None), unloadRef(LUA_NOREF)
{
	mir_wstrcpy(filePath, script.filePath);
	scriptName = mir_wstrdup(script.scriptName);
	m_szModuleName = mir_strdup(script.m_szModuleName);
}

CMLuaScript::~CMLuaScript()
{
	//Unload();
	mir_free((void*)m_szModuleName);
	mir_free((void*)scriptName);
}

const wchar_t* CMLuaScript::GetFilePath() const
{
	return filePath;
}

const wchar_t* CMLuaScript::GetName() const
{
	return scriptName;
}

bool CMLuaScript::IsBinary() const
{
	return isBinary;
}

bool CMLuaScript::IsEnabled() const
{
	return db_get_b(NULL, MODULENAME, _T2A(scriptName), 1);
}

void CMLuaScript::Enable()
{
	db_unset(NULL, MODULENAME, _T2A(scriptName));
}

void CMLuaScript::Disable()
{
	db_set_b(NULL, MODULENAME, _T2A(scriptName), 0);
}

ScriptStatus CMLuaScript::GetStatus() const
{
	return status;
}

int CMLuaScript::Load()
{
	status = ScriptStatus::Failed;

	if (luaL_loadfile(L, _T2A(filePath))) {
		ReportError(L);
		return false;
	}

	CMLuaEnvironment::CreateEnvironmentTable();

	if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
		ReportError(L);
		return false;
	}

	status = ScriptStatus::Loaded;

	if (lua_isnoneornil(L, -1))
		return true;

	luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
	lua_getfield(L, -1, m_szModuleName);
	if (!lua_toboolean(L, -1)) {
		lua_pop(L, 1);
		lua_pushvalue(L, -2);
		lua_setfield(L, -2, m_szModuleName);
		lua_pop(L, 1);
	}
	else
		lua_remove(L, -2);

	if (!lua_istable(L, -1))
		return true;

	lua_getfield(L, -1, "Load");
	if (lua_isfunction(L, -1))
		luaM_pcall(L);
	else
		lua_pop(L, 1);

	lua_getfield(L, -1, "Unload");
	if (lua_isfunction(L, -1)) {
		lua_pushvalue(L, -1);
		unloadRef = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	lua_pop(L, 1);

	lua_pop(L, 1);

	return true;
}

int CMLuaScript::Unload()
{
	if (status == ScriptStatus::Loaded) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, unloadRef);
		if (lua_isfunction(L, -1))
			luaM_pcall(L);
		lua_pushnil(L);
		lua_rawsetp(L, LUA_REGISTRYINDEX, this);
		status = ScriptStatus::None;
	}

	luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
	lua_pushnil(L);
	lua_setfield(L, -2, m_szModuleName);
	lua_pop(L, 1);
	
	return CMLuaEnvironment::Unload();
}

bool CMLuaScript::Reload()
{
	Log(L"Reloading script %s", filePath);
	Unload();
	return Load();
}

static int luc_Writer(lua_State* /*L*/, const void *p, size_t sz, void *u)
{
	return (fwrite(p, sz, 1, (FILE*)u) != 1) && (sz != 0);
}

bool CMLuaScript::Compile()
{
	Unload();

	Log(L"Compiling script %s", filePath);

	wchar_t scriptDir[MAX_PATH];
	FoldersGetCustomPathT(g_hScriptsFolder, scriptDir, _countof(scriptDir), VARSW(MIRLUA_PATHT));
	wchar_t fullPath[MAX_PATH];
	mir_snwprintf(fullPath, L"%s\\%s.%s", scriptDir, scriptName, LUAPRECSCRIPTEXT);
	wchar_t path[MAX_PATH];
	PathToRelativeW(fullPath, path);

	FILE *file = _wfopen(path, L"wb");
	if (file == nullptr) {
		Log(L"Failed to save compiled script to %s", file);
		return false;
	}

	if (luaL_loadfile(L, _T2A(filePath))) {
		ReportError(L);
		fclose(file);
		return false;
	}

	int res = lua_dump(L, luc_Writer, file, 1);
	if (res != 0) {
		fclose(file);
		return false;
	}

	fclose(file);

	ptrW newPath(mir_wstrdup(filePath));
	newPath[mir_wstrlen(newPath) - 1] = L'_';
	MoveFile(filePath, newPath);
	mir_wstrcpy(filePath, path);

	return Load();
}
