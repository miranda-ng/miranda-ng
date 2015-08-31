#include "stdafx.h"

int hLangpack;

HINSTANCE g_hInstance;

PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {1FDBD8F0-3929-41BC-92D1-020779460769}
	{ 0x1fdbd8f0, 0x3929, 0x41bc, { 0x92, 0xd1, 0x2, 0x7, 0x79, 0x46, 0x7, 0x69 } }
};

DWORD WINAPI DllMain(HINSTANCE hInstance, DWORD, LPVOID)
{
	g_hInstance = hInstance;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	if (!IsWinVer8Plus())
	{
		MessageBox(NULL, TranslateT("This plugin requires Windows 8 or higher"), _T(MODULE), MB_OK | MB_ICONERROR);
		return NULL;
	}
	return &pluginInfo;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	HookEvent(ME_SYSTEM_PRESHUTDOWN, &OnPreShutdown);
	InitServices();

	GetEnvironmentVariableW(L"TEMP", wszTempDir, MAX_PATH);
	wcscat_s(wszTempDir, L"\\Miranda.Toaster");

	DWORD dwAttributes = GetFileAttributes(wszTempDir);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeT(wszTempDir);

	if (IsWinVer8Plus() && !IsWinVer10Plus())
	{
		if (FAILED(TryCreateShortcut()))
		{
			MessageBox(NULL, TranslateT("Failed to create shortcut"), _T(MODULE), MB_OK | MB_ICONERROR);
		}
	}

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	SHFILEOPSTRUCT file_op = {
		NULL,
		FO_DELETE,
		wszTempDir,
		_T(""),
		FOF_NOERRORUI | FOF_SILENT | FOF_NOCONFIRMATION,
		false,
		0,
		_T("")
	};
	SHFileOperation(&file_op);
	return 0;
}

int OnPreShutdown(WPARAM, LPARAM)
{
	CallFunctionAsync(&HideAllToasts, NULL);
	return 0;
}