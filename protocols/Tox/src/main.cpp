#include "stdafx.h"

int hLangpack;
CMPlugin g_plugin;
CHAT_MANAGER *pci;
CLIST_INTERFACE *pcli;
HINSTANCE g_hInstance;
HANDLE hProfileFolderPath;

PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
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

int OnModulesLoaded(WPARAM, LPARAM)
{
	CToxProto::InitIcons();

	hProfileFolderPath = FoldersRegisterCustomPathT("Tox", "ProfilesFolder", MIRANDA_USERDATAT, TranslateT("Profiles folder"));

	if (ServiceExists(MS_ASSOCMGR_ADDNEWURLTYPE)) {
		CreateServiceFunction(MODULE "/ParseUri", CToxProto::ParseToxUri);
		AssocMgr_AddNewUrlTypeT("tox:", TranslateT("Tox URI scheme"), g_hInstance, IDI_TOX, MODULE "/ParseUri", 0);
	}

	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	pci = Chat_GetInterface();
	pcli = Clist_GetInterface();

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
