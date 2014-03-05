#include "Mra.h"

LIST<CMraProto> g_Instances(1, PtrKeySortT);

CLIST_INTERFACE *pcli;
int hLangpack;

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	PROTOCOL_DISPLAY_NAME_ORIGA,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {E7C48BAB-8ACE-4CB3-8446-D4B73481F497}
	{ 0xe7c48bab, 0x8ace, 0x4cb3, { 0x84, 0x46, 0xd4, 0xb7, 0x34, 0x81, 0xf4, 0x97 } }
};

HINSTANCE g_hInstance;
HMODULE   g_hDLLXStatusIcons;
HICON     g_hMainIcon;

DWORD     g_dwGlobalPluginRunning;
bool      g_bChatExist;

size_t    g_dwMirWorkDirPathLen;
WCHAR     g_szMirWorkDirPath[MAX_FILEPATH];

void IconsLoad();

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID Reserved)
{
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		g_hInstance = hInstance;
		DisableThreadLibraryCalls(hInstance);
		break;
	case DLL_PROCESS_DETACH:
		/* Nothink to do. */
		break;
	}

	return TRUE;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

///////////////////////////////////////////////////////////////////////////////

static CMraProto* mraProtoInit(const char* pszProtoName, const TCHAR* tszUserName)
{
	CMraProto *ppro = new CMraProto(pszProtoName, tszUserName);
	g_Instances.insert(ppro);
	return ppro;
}

static int mraProtoUninit(CMraProto *ppro)
{
	g_Instances.remove(ppro);
	delete ppro;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

static int OnModulesLoaded(WPARAM, LPARAM)
{
	g_dwGlobalPluginRunning = TRUE;
	return 0;
}

static int OnPreShutdown(WPARAM, LPARAM)
{
	g_dwGlobalPluginRunning = FALSE;
	return 0;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfoEx);
	mir_getCLI();

	IconsLoad();
	InitXStatusIcons();

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = "MRA";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = (pfnInitProto)mraProtoInit;
	pd.fnUninit = (pfnUninitProto)mraProtoUninit;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	DestroyXStatusIcons();
	if (g_hDLLXStatusIcons) {
		FreeLibrary(g_hDLLXStatusIcons);
		g_hDLLXStatusIcons = NULL;
	}

	return 0;
}
