
#include "commonheaders.h"


HINSTANCE hInst;
int hLangpack;

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	"Support for new options design",
	PLUGIN_MAKE_VERSION(0,1,0,0),
	"Support for new options design.",
	"Victor Pavlychko, George Hazan",
	"ghazan@miranda-im.org",
	"é 2009 Victor Pavlychko, George Hazan",
	"http://www.miranda-im.org",
	UNICODE_AWARE,
	0,
	{ 0x621f886b, 0xa7f6, 0x457f, { 0x9d, 0x62, 0x8e, 0xe8, 0x4c, 0x27, 0x59, 0x93 }} // {621f886b-a7f6-457f-9d62-8ee84c275993}
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MirandaPluginInfoEx - returns an information about a plugin

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Performs a primary set of actions upon plugin loading

int LoadModernOptsModule();

extern "C" int __declspec(dllexport) Load(void)
{

	mir_getLP(&pluginInfoEx);

	LoadModernOptsModule();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload a plugin

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
