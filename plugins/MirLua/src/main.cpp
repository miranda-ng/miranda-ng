#include "stdafx.h"

int hLangpack;
HINSTANCE g_hInstance;

CMLua *mLua;
HANDLE hCommonFolderPath;
HANDLE hCustomFolderPath;

#ifdef _DEBUG
HANDLE hConsole = NULL;
#endif

PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {27d41d81-991f-4dc6-8749-b0321c87e694}
	{ 0x27d41d81, 0x991f, 0x4dc6, { 0x87, 0x49, 0xb0, 0x32, 0x1c, 0x87, 0xe6, 0x94 } }

};

DWORD WINAPI DllMain(HINSTANCE hInstance, DWORD, LPVOID)
{
	g_hInstance = hInstance;

	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

void LoadScripts(const TCHAR *scriptDir)
{
	mLua->AddPath(ptrA(mir_utf8encodeT(scriptDir)));

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
				mLua->Load(T2Utf(path));
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
}

#include <io.h>
#include <fcntl.h>

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

#ifdef _DEBUG
	if (AllocConsole())
	{
		freopen("CONOUT$", "wt", stdout);
		hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
		SetConsoleTitle(_T("MirLua Console"));
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
	}
#endif

	mLua = new CMLua();

	hCommonFolderPath = FoldersRegisterCustomPathT("MirLua", Translate("Common scripts folder"), COMMON_SCRIPTS_PATHT);
	hCustomFolderPath = FoldersRegisterCustomPathT("MirLua", Translate("Custom scripts folder"), CUSTOM_SCRIPTS_PATHT);

	TCHAR commonScriptDir[MAX_PATH];
	FoldersGetCustomPathT(hCommonFolderPath, commonScriptDir, SIZEOF(commonScriptDir), VARST(COMMON_SCRIPTS_PATHT));
	LoadScripts(commonScriptDir);

	TCHAR customScriptDir[MAX_PATH];
	FoldersGetCustomPathT(hCustomFolderPath, customScriptDir, SIZEOF(customScriptDir), VARST(CUSTOM_SCRIPTS_PATHT));
	LoadScripts(customScriptDir);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
#ifdef _DEBUG
	if (hConsole)
		CloseHandle(hConsole);
	FreeConsole();
#endif

	delete mLua;

	return 0;
}
