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
	// {00272A3E-F5FA-4090-8B67-3E62AC1EE0B4}
	{0x272a3e, 0xf5fa, 0x4090, {0x8b, 0x67, 0x3e, 0x62, 0xac, 0x1e, 0xe0, 0xb4}}
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

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = "TOX";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = (pfnInitProto)CToxProto::InitProtoInstance;
	pd.fnUninit = (pfnUninitProto)CToxProto::UninitProtoInstance;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}