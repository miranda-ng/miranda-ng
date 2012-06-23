#include "commonheaders.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;
HANDLE hModulesLoaded;
int hLangpack;
struct MM_INTERFACE mmi;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"First Run Plugin",
	PLUGIN_MAKE_VERSION(0,0,0,2),
	"Displays the Accounts window at the first start.",
	"Yasnovidyashii",
	"yasnovidyashii@gmail.com",
	"© 2008 Mikhail Yuriev",
	"http://miranda-im.org/",
	UNICODE_AWARE,		//not transient
	0,		//doesn't replace anything built-in
    // Generate your own unique id for your plugin.
    // Do not use this UUID!
    // Use uuidgen.exe to generate the uuuid
    MIID_FIRSTRUN
};

int ModulesLoaded(WPARAM wParam,LPARAM lParam)
{
	TCHAR* ptszDllName;
    TCHAR ptszDllPath[MAX_PATH];

    GetModuleFileName(hInst, ptszDllPath, MAX_PATH);
    ptszDllName = _tcslwr(_tcsrchr(ptszDllPath, '\\'));
	
	if (ptszDllName!=NULL)
		ptszDllName=ptszDllName + 1;
	
	CallService(MS_PROTO_SHOWACCMGR, (WPARAM) NULL, (LPARAM)NULL);

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

extern "C" __declspec(dllexport) int Load(PLUGINLINK *link)
{
	pluginLink=link;
	mir_getLP(&pluginInfo);
	mir_getMMI(&mmi);
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED,ModulesLoaded);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	UnhookEvent(hModulesLoaded);
	return 0;
}