#include "commonheaders.h"

HINSTANCE hInst;

HANDLE hModulesLoaded;
int hLangpack;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"First Run Plugin",
	PLUGIN_MAKE_VERSION(0,0,0,2),
	"Displays the Accounts window at the first start.",
	"Yasnovidyashii",
	"yasnovidyashii@gmail.com",
	"© 2008 Mikhail Yuriev",
	"http://miranda-ng.org/",
	UNICODE_AWARE,
	//{49c2cf54-7898-44de-be3a-6d2e4ef90079}
	{0x49c2cf54, 0x7898, 0x44de, { 0xbe, 0x3a, 0x6d, 0x2e, 0x4e, 0xf9, 0x0, 0x79 }} 
};

int ModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	TCHAR* ptszDllName;
    TCHAR ptszDllPath[MAX_PATH];

    GetModuleFileName(hInst, ptszDllPath, MAX_PATH);
    ptszDllName = _tcslwr(_tcsrchr(ptszDllPath, '\\'));
	
	if (ptszDllName!=NULL)
		ptszDllName=ptszDllName + 1;
	
	CallService(MS_PROTO_SHOWACCMGR, (WPARAM) NULL, 0);

	DBWriteContactSettingByte(NULL, "PluginDisable", _T2A(ptszDllName), 1);
	
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load(void)
{

	mir_getLP(&pluginInfo);
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED,ModulesLoaded);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	UnhookEvent(hModulesLoaded);
	return 0;
}