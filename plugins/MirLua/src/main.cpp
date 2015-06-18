#include "stdafx.h"

int hLangpack;
HINSTANCE g_hInstance;

HANDLE g_hCommonFolderPath;
HANDLE g_hCustomFolderPath;

CMLua *g_mLua;
HANDLE hConsole = NULL;

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

int OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;
	odp.ptszGroup = LPGENT("Customize");
	odp.ptszTitle = LPGENT("Scripts");
	odp.ptszTab = _T("Lua");
	odp.pDialog = CLuaOptions::CreateOptionsPage();
	Options_AddPage(wParam, &odp);

	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM)
{
	HookEvent(ME_OPT_INITIALISE, OnOptionsInit);

	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	if (db_get_b(NULL, MODULE, "ShowConsole", 0))
	{
		if (AllocConsole())
		{
			freopen("CONOUT$", "wt", stdout);
			hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
			SetConsoleTitle(_T("MirLua Console"));
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
		}
	}

	g_hCommonFolderPath = FoldersRegisterCustomPathT("MirLua", Translate("Common scripts folder"), COMMON_SCRIPTS_PATHT);
	g_hCustomFolderPath = FoldersRegisterCustomPathT("MirLua", Translate("Custom scripts folder"), CUSTOM_SCRIPTS_PATHT);

	g_mLua = new CMLua();

	CLuaLoader loader(g_mLua);
	loader.LoadScripts();

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	if (hConsole)
		CloseHandle(hConsole);
	FreeConsole();

	delete g_mLua;

	return 0;
}
