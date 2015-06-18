#include "stdafx.h"

CLuaLoader::CLuaLoader(CMLua *mLua) : mLua(mLua)
{
}

void CLuaLoader::LoadScripts(const TCHAR *scriptDir)
{
	mLua->AddScriptsPath(ptrA(mir_utf8encodeT(scriptDir)));

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
					mLua->LoadScript(T2Utf(path));
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
}

void CLuaLoader::LoadScripts()
{
	TCHAR scriptDir[MAX_PATH];

	FoldersGetCustomPathT(g_hCommonFolderPath, scriptDir, SIZEOF(scriptDir), VARST(COMMON_SCRIPTS_PATHT));
	LoadScripts(scriptDir);

	FoldersGetCustomPathT(g_hCustomFolderPath, scriptDir, SIZEOF(scriptDir), VARST(CUSTOM_SCRIPTS_PATHT));
	LoadScripts(scriptDir);
}