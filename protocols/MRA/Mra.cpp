#include "Mra.h"

MRA_SETTINGS masMraSettings;
int hLangpack;

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX), 
	PROTOCOL_DISPLAY_NAME_ORIGA, 
	__VERSION_DWORD, 
	"Provides support for Mail.ru agent Instant Messenger protocol.", 
	"Rozhuk Ivan", 
	"Rozhuk_I@mail.ru", 
	"© 2006-2011 Rozhuk Ivan", 
	"http://miranda-ng.org/", 
	UNICODE_AWARE, 
	// {E7C48BAB-8ACE-4CB3-8446-D4B73481F497}
	{ 0xe7c48bab, 0x8ace, 0x4cb3, { 0x84, 0x46, 0xd4, 0xb7, 0x34, 0x81, 0xf4, 0x97 } }
};

void IconsLoad();

int  OnModulesLoaded(WPARAM wParam, LPARAM lParam);
int  OnPreShutdown(WPARAM wParam, LPARAM lParam);

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID Reserved)
{
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		bzero(&masMraSettings, sizeof(masMraSettings));
		masMraSettings.hInstance = hInstance;
		masMraSettings.hHeap = HeapCreate(0, 0, 0);//GetProcessHeap();
		DisableThreadLibraryCalls(hInstance);
		break;

	case DLL_PROCESS_DETACH:
		HeapDestroy(masMraSettings.hHeap);
		masMraSettings.hHeap = NULL;
	}

	return TRUE;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

extern "C" MRA_API PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

///////////////////////////////////////////////////////////////////////////////
// Protocol instances

static int sttCompareProtocols(const CMraProto *p1, const CMraProto *p2)
{
	return lstrcmp(p1->m_tszUserName, p2->m_tszUserName);
}

LIST<CMraProto> g_Instances(1, sttCompareProtocols);

///////////////////////////////////////////////////////////////////////////////

static CMraProto* mraProtoInit(const char* pszProtoName, const TCHAR* tszUserName)
{
	CMraProto *ppro = new CMraProto( pszProtoName, tszUserName );
	g_Instances.insert(ppro);
	return ppro;
}

static int mraProtoUninit(CMraProto *ppro)
{
	g_Instances.remove(ppro);
	delete ppro;
	return 0;
}

extern "C" MRA_API int Load(void)
{
	mir_getLP(&pluginInfoEx);

	IconsLoad();

	size_t dwBuffLen;
	WCHAR szBuff[MAX_FILEPATH];
	LPSTR lpszFullFileName = (LPSTR)szBuff;
	LPWSTR lpwszFileName;

	// load libs
	if (GetModuleFileName(NULL, szBuff, MAX_FILEPATH)) {
		masMraSettings.dwMirWorkDirPathLen = GetFullPathName(szBuff, MAX_FILEPATH, masMraSettings.szMirWorkDirPath, &lpwszFileName);
		if (masMraSettings.dwMirWorkDirPathLen) {
			masMraSettings.dwMirWorkDirPathLen -= lstrlenW(lpwszFileName);
			masMraSettings.szMirWorkDirPath[masMraSettings.dwMirWorkDirPathLen] = 0;

			// load xstatus icons lib
			DWORD dwErrorCode = FindFile(masMraSettings.szMirWorkDirPath, masMraSettings.dwMirWorkDirPathLen, L"xstatus_MRA.dll", -1, szBuff, SIZEOF(szBuff), (DWORD*)&dwBuffLen);
			if (dwErrorCode == NO_ERROR) {
				masMraSettings.hDLLXStatusIcons = LoadLibraryEx(szBuff, NULL, 0);
				if (masMraSettings.hDLLXStatusIcons) {
					if ((dwBuffLen = LoadStringW(masMraSettings.hDLLXStatusIcons, IDS_IDENTIFY, szBuff, MAX_FILEPATH)) == 0 || CompareStringW( MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), NORM_IGNORECASE, L"# Custom Status Icons #", 23, szBuff, dwBuffLen) != CSTR_EQUAL) {
						FreeLibrary(masMraSettings.hDLLXStatusIcons);
						masMraSettings.hDLLXStatusIcons = NULL;
					}
				}
			}

			// load zlib
			dwErrorCode = FindFile(masMraSettings.szMirWorkDirPath, masMraSettings.dwMirWorkDirPathLen, L"zlib.dll", -1, szBuff, SIZEOF(szBuff), (DWORD*)&dwBuffLen);
			if (dwErrorCode == NO_ERROR) {
				masMraSettings.hDLLZLib = LoadLibraryEx(szBuff, NULL, 0);
				if (masMraSettings.hDLLZLib) {
					masMraSettings.lpfnCompress2 = (HANDLE)GetProcAddress(masMraSettings.hDLLZLib, "compress2");
					masMraSettings.lpfnUncompress = (HANDLE)GetProcAddress(masMraSettings.hDLLZLib, "uncompress");
				}
			}
		}
	}

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);

	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize = sizeof(pd);
	pd.szName = "MRA";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = (pfnInitProto)mraProtoInit;
	pd.fnUninit = (pfnUninitProto)mraProtoUninit;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	DebugPrintCRLFW(L"Load - DONE");
	return 0;
}

extern "C" MRA_API int Unload(void)
{
	if (masMraSettings.hDLLXStatusIcons) {
		FreeLibrary(masMraSettings.hDLLXStatusIcons);
		masMraSettings.hDLLXStatusIcons = NULL;
	}

	if (masMraSettings.hDLLZLib) {
		FreeLibrary(masMraSettings.hDLLZLib);
		masMraSettings.hDLLZLib = NULL;
		masMraSettings.lpfnCompress2 = NULL;
		masMraSettings.lpfnUncompress = NULL;
	}

	DebugPrintCRLFW(L"Unload - DONE");
	return 0;
}


static int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	MraAntiSpamLoadBadWordsW();

	LoadModules();

	InterlockedExchange((volatile LONG*)&masMraSettings.dwGlobalPluginRunning, TRUE);

	DebugPrintCRLFW(L"OnModulesLoaded - DONE");
	return 0;
}

int OnPreShutdown(WPARAM wParam, LPARAM lParam)
{
	InterlockedExchange((volatile LONG*)&masMraSettings.dwGlobalPluginRunning, FALSE);

	UnloadModules();

	MraAntiSpamFreeBadWords();

	DebugPrintCRLFW(L"OnPreShutdown - DONE");
	return 0;
}
