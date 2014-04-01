#include "commonheaders.h"

int hLangpack;
LPCSTR szModuleName = MODULENAME;
LPCSTR szVersionStr = MODULENAME" DLL ("__VERSION_STRING_DOTS")";
HINSTANCE g_hInst;

HANDLE hPGPPRIV = NULL;
HANDLE hRSA4096 = NULL;

CRITICAL_SECTION localQueueMutex;
CRITICAL_SECTION localContextMutex;

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	//3613F2D9-C040-4361-A44F-DF7B5AAACF6E
	{0x3613F2D9, 0xC040, 0x4361, {0xA4, 0x4F, 0xDF, 0x7B, 0x5A, 0xAA, 0xCF, 0x6E}}
};

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH) {
		g_hInst = hInst;
		InitializeCriticalSection(&localQueueMutex);
		InitializeCriticalSection(&localContextMutex);
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
		DeleteCriticalSection(&localQueueMutex);
		DeleteCriticalSection(&localContextMutex);
	}

	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

int onModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	// updater plugin support
#if defined(_DEBUG) || defined(NETLIB_LOG)
	InitNetlib();
#endif
	return 0;
}

extern "C" __declspec(dllexport) int Load()
{
	DisableThreadLibraryCalls(g_hInst);

	// get memoryManagerInterface address
	mir_getLP(&pluginInfoEx);

	// register plugin module
	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = (char*)szModuleName;
	pd.type = PROTOTYPE_ENCRYPTION;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	// hook events
	HookEvent(ME_SYSTEM_MODULESLOADED, onModulesLoaded);
	return 0;
}

extern "C" __declspec(dllexport) int Unload()
{
	return 0;
}

BOOL ExtractFileFromResource(HANDLE FH, int ResType, int ResId, DWORD* Size)
{
	HRSRC RH = FindResource(g_hInst, MAKEINTRESOURCE(ResId), MAKEINTRESOURCE(ResType));
	if (RH == NULL)
		return FALSE;

	PBYTE	RP = (PBYTE)LoadResource(g_hInst, RH);
	if (RP == NULL)
		return FALSE;

	DWORD	x, s = SizeofResource(g_hInst, RH);
	if (!WriteFile(FH, RP, s, &x, NULL)) return FALSE;
	if (x != s) return FALSE;
	if (Size) *Size = s;
	return TRUE;
}

void ExtractFile(char *FileName, int ResType, int ResId)
{
	HANDLE FH = CreateFile(FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (FH == INVALID_HANDLE_VALUE)
		return;

	if (!ExtractFileFromResource(FH, ResType, ResId, NULL))
		MessageBoxA(0, "Can't extract", "!!!", MB_OK);
	CloseHandle(FH);
}

size_t rtrim(LPCSTR str)
{
	size_t len = strlen(str);
	LPSTR ptr = (LPSTR)str + len - 1;

	while (len) {
		char c = *ptr;
		if (c != '\x20' && c != '\x09' && c != '\x0A' && c != '\x0D') {
			*(ptr + 1) = '\0';
			break;
		}
		len--; ptr--;
	}
	return len;
}


#if defined(_DEBUG) || defined(NETLIB_LOG)
HANDLE hNetlibUser;

void InitNetlib()
{
	NETLIBUSER nl_user;
	memset(&nl_user, 0, sizeof(nl_user));
	nl_user.cbSize = sizeof(nl_user);
	nl_user.szSettingsModule = (LPSTR)szModuleName;
	nl_user.szDescriptiveName = (LPSTR)szModuleName;
	nl_user.flags = NUF_NOOPTIONS;
	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nl_user);
}

void DeinitNetlib()
{
	if (hNetlibUser)
		CallService(MS_NETLIB_CLOSEHANDLE, (WPARAM)hNetlibUser, 0);
}

int Sent_NetLog(const char *fmt, ...)
{
	va_list va;
	char szText[1024];

	va_start(va, fmt);
	mir_vsnprintf(szText, sizeof(szText), fmt, va);
	va_end(va);
	if (hNetlibUser)
		return CallService(MS_NETLIB_LOG, (WPARAM)hNetlibUser, (LPARAM)szText);
	return 0;
}
#endif
