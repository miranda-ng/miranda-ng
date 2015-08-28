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
		MessageBox(NULL, TranslateT("This plugin only supports Windows 8 or higher"), _T(MODULE), MB_OK | MB_ICONERROR);
		return NULL;
	}
	else if (IsWinVer8Plus() && !IsWinVer10Plus())
	{
		if (ServiceExists("AddToStartMenu/Add"))
		{
			CallService("AddToStartMenu/Add");
		}
		else
		{
			MessageBox(NULL, TranslateT("In Windows8 desktop application must have a shortcut on the Start menu. Please, install \"AddToStartMenu\" plugin."), _T(MODULE), MB_OK | MB_ICONERROR);
			return NULL;
		}
	}
	return &pluginInfo;

}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	HookEvent(ME_SYSTEM_SHUTDOWN, &OnPreShutdown);
	InitServices();

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}

int OnPreShutdown(WPARAM, LPARAM)
{
	mir_cslock lck(csNotifications);
	for (int i = 0; i < lstNotifications.getCount(); i++)
		lstNotifications[0].Hide();

	lstNotifications.destroy();

	return 0;
}