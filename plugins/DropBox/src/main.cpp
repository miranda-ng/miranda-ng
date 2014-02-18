#include "common.h"

int hLangpack;
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
	{0xb908773a, 0x86f7, 0x4a91, {0x86, 0x74, 0x6a, 0x20, 0xba, 0xe, 0x67, 0xd1}}

};

DWORD WINAPI DllMain(HINSTANCE hInstance, DWORD, LPVOID)
{
	g_hInstance = hInstance;

	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	HookEvent(ME_OPT_INITIALISE, OnOptionsInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	PROTOCOLDESCRIPTOR pd = { PROTOCOLDESCRIPTOR_V3_SIZE };
	pd.szName = MODULE;
	pd.type = PROTOTYPE_VIRTUAL;

	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	CreateProtoServiceFunction(MODULE, PS_GETCAPS, DropBoxGetCaps);

	CreateProtoServiceFunction(MODULE, PSS_FILE, DropBoxSendFile);

	CreateProtoServiceFunction(MODULE, PSS_MESSAGE, DropBoxSendMessage);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}