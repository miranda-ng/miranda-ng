#include "stdafx.h"

void InitializeIcons();

int hLangpack;
CDropbox *dropbox;
HINSTANCE g_hInstance;

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
	// {B908773A-86F7-4A91-8674-6A20BA0E67D1} 
	{ 0xb908773a, 0x86f7, 0x4a91, { 0x86, 0x74, 0x6a, 0x20, 0xba, 0xe, 0x67, 0xd1 } }

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

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	InitializeIcons();

	dropbox = new CDropbox();

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	delete dropbox;

	return 0;
}