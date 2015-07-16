#include "stdafx.h"

int hLangpack;
int hScriptsLangpack;

HINSTANCE g_hInstance;

HANDLE g_hCommonFolderPath;
HANDLE g_hCustomFolderPath;

HANDLE hNetlib = NULL;

CMLua *g_mLua;

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

int OnModulesLoaded(WPARAM, LPARAM)
{
	g_hCommonFolderPath = FoldersRegisterCustomPathT(MODULE, Translate("Common scripts folder"), COMMON_SCRIPTS_PATHT);
	g_hCustomFolderPath = FoldersRegisterCustomPathT(MODULE, Translate("Custom scripts folder"), CUSTOM_SCRIPTS_PATHT);

	g_mLua = new CMLua();

	HookEvent(ME_OPT_INITIALISE, CLuaOptions::OnOptionsInit);

	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	NETLIBUSER nlu = { 0 };
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.ptszDescriptiveName = _T(MODULE);
	nlu.szSettingsModule = MODULE;
	hNetlib = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	delete g_mLua;

	if (hNetlib)
	{
		Netlib_CloseHandle(hNetlib);
		hNetlib = NULL;
	}

	return 0;
}
