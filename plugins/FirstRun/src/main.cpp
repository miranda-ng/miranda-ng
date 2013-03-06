#include "commonheaders.h"

HINSTANCE hInst;

int hLangpack;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {49C2CF54-7898-44DE-BE3A-6D2E4EF90079}
	{0x49c2cf54, 0x7898, 0x44de, {0xbe, 0x3a, 0x6d, 0x2e, 0x4e, 0xf9, 0x0, 0x79}}
};

int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	TCHAR* ptszDllName;
    TCHAR ptszDllPath[MAX_PATH];

    GetModuleFileName(hInst, ptszDllPath, MAX_PATH);
    ptszDllName = _tcslwr(_tcsrchr(ptszDllPath, '\\'));
	
	if (ptszDllName != NULL)
		ptszDllName = ptszDllName + 1;
	
	CallService(MS_PROTO_SHOWACCMGR, 0, 0);

	db_set_b(NULL, "PluginDisable", _T2A(ptszDllName), 1);
	
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}