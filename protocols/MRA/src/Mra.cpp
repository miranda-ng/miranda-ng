#include "stdafx.h"

LIST<CMraProto> g_Instances(1, PtrKeySortT);

CLIST_INTERFACE *pcli;
int hLangpack;

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	PROTOCOL_DISPLAY_NAME_ORIGA,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {E7C48BAB-8ACE-4CB3-8446-D4B73481F497}
	{ 0xe7c48bab, 0x8ace, 0x4cb3, { 0x84, 0x46, 0xd4, 0xb7, 0x34, 0x81, 0xf4, 0x97 } }
};

HINSTANCE g_hInstance;
HMODULE   g_hDLLXStatusIcons;
HICON     g_hMainIcon;

bool      g_bChatExist, g_bShutdown = false;

size_t    g_dwMirWorkDirPathLen;
WCHAR     g_szMirWorkDirPath[MAX_FILEPATH];

void IconsLoad();

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		g_hInstance = hInstance;
		DisableThreadLibraryCalls(hInstance);
		break;
	}

	return TRUE;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

///////////////////////////////////////////////////////////////////////////////

static int __cdecl OnPreShutdown(WPARAM, LPARAM)
{
	g_bShutdown = true;
	return 0;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfoEx);
	pcli = Clist_GetInterface();

	IconsLoad();
	InitXStatusIcons();

	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	DestroyXStatusIcons();
	if (g_hDLLXStatusIcons) {
		FreeLibrary(g_hDLLXStatusIcons);
		g_hDLLXStatusIcons = nullptr;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static PROTO_INTERFACE* mraProtoInit(const char* pszProtoName, const wchar_t* tszUserName)
{
	CMraProto *ppro = new CMraProto(pszProtoName, tszUserName);
	g_Instances.insert(ppro);
	return ppro;
}

static int mraProtoUninit(PROTO_INTERFACE *ppro)
{
	g_Instances.remove((CMraProto*)ppro);
	delete (CMraProto*)ppro;
	return 0;
}

struct CMPlugin : public CMPluginBase
{
	CMPlugin() :
		CMPluginBase("MRA")
	{
		RegisterProtocol(PROTOTYPE_PROTOCOL, mraProtoInit, mraProtoUninit);
	}
}
	g_plugin;
