
#include "common.h"
#include "version.h"
#pragma hdrstop

HINSTANCE hInst;
PLUGINLINK *pluginLink;
MM_INTERFACE mmi;
LIST_INTERFACE li;
HANDLE hHookTTBModuleLoaded;
int hLangpack;

LIST<void> arHooks(10), arServices(10);

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
	UNICODE_AWARE,
	0, 		//doesn't replace anything built-in
	MIID_TTB
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;
	mir_getMMI( &mmi );
	mir_getLI( &li );
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

	for (int i=0; i < arHooks.getCount(); i++ )
		UnhookEvent( arHooks[i] );

	for (int j=0; j < arServices.getCount(); j++ )
		DestroyServiceFunction( arServices[j] );

	DestroyHookableEvent(hHookTTBModuleLoaded);
	return 0;
}
