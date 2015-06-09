#include "stdafx.h"

int hLangpack;
HINSTANCE g_hInst;

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
	// {31AAEF41-1411-487E-86E2-C484F4174283}
	{ 0x31aaef41, 0x1411, 0x487e, { 0x86, 0xe2, 0xc4, 0x84, 0xf4, 0x17, 0x42, 0x83 } }

};

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD, LPVOID)
{
	g_hInst = hInstance;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	UnInitModule();
	return 0;
}