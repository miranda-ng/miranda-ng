#include "stdafx.h"

int hLangpack;
CHAT_MANAGER *pci;
CLIST_INTERFACE *pcli;
HINSTANCE g_hInstance;
HMODULE g_hToxLibrary = NULL;

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

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

extern "C" int __declspec(dllexport) Load(void)
{
	g_hToxLibrary = LoadLibrary(TOX_LIBRARY);
	if (g_hToxLibrary == NULL)
		return 1;

	if (!TOX_VERSION_IS_ABI_COMPATIBLE())
	{
		wchar_t message[100];
		mir_snwprintf(message, TranslateT("Current version of plugin is support Tox API version %i.%i.%i which is incompatible with %s"), TOX_VERSION_MAJOR, TOX_VERSION_MINOR, TOX_VERSION_PATCH, TOX_LIBRARY);
		CToxProto::ShowNotification(message, MB_ICONERROR);
		FreeLibrary(g_hToxLibrary);
		return 2;
	}

	pci = Chat_GetInterface();
	pcli = Clist_GetInterface();
	mir_getLP(&pluginInfo);

	PROTOCOLDESCRIPTOR pd = { 0 };
	pd.cbSize = sizeof(pd);
	pd.szName = "TOX";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = (pfnInitProto)CToxProto::InitAccount;
	pd.fnUninit = (pfnUninitProto)CToxProto::UninitAccount;
	Proto_RegisterModule(&pd);

	HookEvent(ME_SYSTEM_MODULESLOADED, &CToxProto::OnModulesLoaded);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	if (g_hToxLibrary)
		FreeLibrary(g_hToxLibrary);

	return 0;
}