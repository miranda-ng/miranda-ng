#include "commonheaders.h"

CMPlugin g_plugin;
LPCSTR szModuleName = MODULENAME;
LPCSTR szVersionStr = MODULENAME" DLL (" __VERSION_STRING_DOTS ")";

HANDLE hPGPPRIV = nullptr;
HANDLE hRSA4096 = nullptr;

mir_cs localQueueMutex;
mir_cs localContextMutex;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	//3613F2D9-C040-4361-A44F-DF7B5AAACF6E
	{0x3613F2D9, 0xC040, 0x4361, {0xA4, 0x4F, 0xDF, 0x7B, 0x5A, 0xAA, 0xCF, 0x6E}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

int onModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	// updater plugin support
#if defined(_DEBUG) || defined(NETLIB_LOG)
	InitNetlib();
#endif
	return 0;
}

int CMPlugin::Load()
{
	DisableThreadLibraryCalls(g_plugin.getInst());

	// register plugin module
	Proto_RegisterModule(PROTOTYPE_ENCRYPTION, szModuleName);

	// hook events
	HookEvent(ME_SYSTEM_MODULESLOADED, onModulesLoaded);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL ExtractFileFromResource(HANDLE FH, int ResType, int ResId, uint32_t* Size)
{
	HRSRC RH = FindResource(g_plugin.getInst(), MAKEINTRESOURCE(ResId), MAKEINTRESOURCE(ResType));
	if (RH == nullptr)
		return FALSE;

	uint8_t *RP = (uint8_t*)LoadResource(g_plugin.getInst(), RH);
	if (RP == nullptr)
		return FALSE;

	DWORD x, s = SizeofResource(g_plugin.getInst(), RH);
	if (!WriteFile(FH, RP, s, &x, nullptr)) return FALSE;
	if (x != s) return FALSE;
	if (Size) *Size = s;
	return TRUE;
}

void ExtractFile(char *FileName, int ResType, int ResId)
{
	HANDLE FH = CreateFile(FileName, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, 0, nullptr);
	if (FH == INVALID_HANDLE_VALUE)
		return;

	if (!ExtractFileFromResource(FH, ResType, ResId, nullptr))
		MessageBoxA(nullptr, "Can't extract", "!!!", MB_OK);
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
HNETLIBUSER hNetlibUser;

void InitNetlib()
{
	NETLIBUSER nl_user = {};
	nl_user.szSettingsModule = (LPSTR)szModuleName;
	nl_user.szDescriptiveName.a = (LPSTR)szModuleName;
	nl_user.flags = NUF_NOOPTIONS;
	hNetlibUser = Netlib_RegisterUser(&nl_user);
}

void DeinitNetlib()
{
	Netlib_CloseHandle(hNetlibUser);
}

int Sent_NetLog(const char *fmt, ...)
{
	va_list va;
	char szText[1024];

	va_start(va, fmt);
	mir_vsnprintf(szText, sizeof(szText), fmt, va);
	va_end(va);
	if (hNetlibUser)
		return Netlib_Log(hNetlibUser, szText);
	return 0;
}
#endif
