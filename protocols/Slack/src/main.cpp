#include "stdafx.h"

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
	// {5487475F-00C2-4D88-AE41-C5969260D455}
	{ 0x5487475f, 0xc2, 0x4d88, {0xae, 0x41, 0xc5, 0x96, 0x92, 0x60, 0xd4, 0x55 }}
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

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = "SLACK";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = (pfnInitProto)CSlackProto::InitAccount;
	pd.fnUninit = (pfnUninitProto)CSlackProto::UninitAccount;
	Proto_RegisterModule(&pd);

	HookEvent(ME_SYSTEM_MODULESLOADED, &CSlackProto::OnModulesLoaded);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
