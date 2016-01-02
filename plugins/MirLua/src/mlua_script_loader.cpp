#include "stdafx.h"

CLuaScriptLoader::CLuaScriptLoader(lua_State *L) : L(L)
{
}

void CLuaScriptLoader::LoadScript(const TCHAR *scriptDir, const TCHAR *file)
{
	TCHAR fullPath[MAX_PATH], path[MAX_PATH];
	mir_sntprintf(fullPath, _T("%s\\%s"), scriptDir, file);
	PathToRelativeT(fullPath, path);

	CMLuaScript *script = new CMLuaScript(L, path);
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
		mir_sntprintf(buf, _T("%s:OK"), path);
		CallService(MS_NETLIB_LOGW, (WPARAM)hNetlib, (LPARAM)buf);
	}
}

void CLuaScriptLoader::LoadScripts()
{
	TCHAR scriptDir[MAX_PATH];
	FoldersGetCustomPathT(g_hScriptsFolder, scriptDir, _countof(scriptDir), VARST(MIRLUA_PATHT));

	TCHAR buf[4096];
	mir_sntprintf(buf, _T("Loading scripts from %s"), scriptDir);
	CallService(MS_NETLIB_LOGW, (WPARAM)hNetlib, (LPARAM)buf);

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

void CLuaScriptLoader::Load(lua_State *L)
{
	CLuaScriptLoader(L).LoadScripts();
}