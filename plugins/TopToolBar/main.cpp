
#include "common.h"
#include "version.h"

HINSTANCE hInst;

HANDLE hHookTTBModuleLoaded;
int hLangpack;

#define MIID_TTB {0xf593c752, 0x51d8, 0x4d46, {0xba, 0x27, 0x37, 0x57, 0x79, 0x53, 0xf5, 0x5c}}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfo  =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__PLUGIN_DESC,
	__PLUGIN_AUTHOR,
	__PLUGIN_EMAIL,
	__PLUGIN_RIGHTS,
	__PLUGIN_AUTHORWEB,
	UNICODE_AWARE, 		//doesn't replace anything built-in
	MIID_TTB
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" int __declspec(dllexport) Load(void)
{

	mir_getLP(&pluginInfo);

	LoadToolbarModule();

	hHookTTBModuleLoaded = CreateHookableEvent(ME_TTB_MODULELOADED);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" int __declspec(dllexport) Unload(void)
{
	UnLoadInternalButtons();
	UnloadToolbarModule();

	DestroyHookableEvent(hHookTTBModuleLoaded);
	return 0;
}
