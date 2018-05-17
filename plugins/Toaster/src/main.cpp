#include "stdafx.h"

int hLangpack;
CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

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
	// {1FDBD8F0-3929-41BC-92D1-020779460769}
	{ 0x1fdbd8f0, 0x3929, 0x41bc, { 0x92, 0xd1, 0x2, 0x7, 0x79, 0x46, 0x7, 0x69 } }
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	if (!IsWinVer8Plus()) {
		MessageBox(nullptr, TranslateT("This plugin requires Windows 8 or higher"), _T(MODULE), MB_OK | MB_ICONERROR);
		return nullptr;
	}
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////

static int OnPreShutdown(WPARAM, LPARAM)
{
	CleanupClasses();

	SHFILEOPSTRUCT file_op = {
		nullptr,
		FO_DELETE,
		wszTempDir,
		L"",
		FOF_NOERRORUI | FOF_SILENT | FOF_NOCONFIRMATION,
		false,
		nullptr,
		L""
	};
	SHFileOperation(&file_op);

	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	HookEvent(ME_OPT_INITIALISE, OnOptionsInitialized);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, &OnPreShutdown);

	InitServices();

	if (GetEnvironmentVariableW(L"TEMP", wszTempDir, MAX_PATH) != 0) {
		wcscat_s(wszTempDir, L"\\Miranda.Toaster");

		DWORD dwAttributes = GetFileAttributes(wszTempDir);
		if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			CreateDirectoryTreeW(wszTempDir);
	}
	else MessageBox(nullptr, TranslateT("Failed to create temporary directory"), _T(MODULE), MB_OK | MB_ICONERROR);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}
