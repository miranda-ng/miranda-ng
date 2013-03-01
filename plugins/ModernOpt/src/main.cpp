#include "commonheaders.h"


HINSTANCE hInst;
int hLangpack;

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {621F886B-A7F6-457F-9D62-8EE84C275993}
	{0x621f886b, 0xa7f6, 0x457f, {0x9d, 0x62, 0x8e, 0xe8, 0x4c, 0x27, 0x59, 0x93}}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MirandaPluginInfoEx - returns an information about a plugin

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Performs a primary set of actions upon plugin loading

void LoadModernOptsModule();

extern "C" int __declspec(dllexport) Load(void)
{

	mir_getLP(&pluginInfoEx);

	LoadModernOptsModule();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload a plugin

void UnloadModernOptsModule();

extern "C" int __declspec(dllexport) Unload(void)
{
	UnloadModernOptsModule();
	return 0;
}
