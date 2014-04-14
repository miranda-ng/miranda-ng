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
	// {68F5A030-BA32-48EC-9507-5C2FBDEA5217}
	{ 0x68f5a030, 0xba32, 0x48ec, { 0x95, 0x7, 0x5c, 0x2f, 0xbd, 0xea, 0x52, 0x17 }}
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
	pd.szName = "STEAM";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = (pfnInitProto)CSteamProto::InitProtoInstance;
	pd.fnUninit = (pfnUninitProto)CSteamProto::UninitProtoInstance;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	CSteamProto::InitMenus();

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	CSteamProto::UninitProtoInstances();

	CSteamProto::UninitMenus();

	return 0;
}