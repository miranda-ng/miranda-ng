#include "stdafx.h"

int hLangpack;

HINSTANCE g_hInstance;

CMLua *g_mLua;

HANDLE g_hCLibsFolder;
HANDLE g_hScriptsFolder;

HNETLIBUSER hNetlib = NULL;

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
	// {27d41d81-991f-4dc6-8749-b0321c87e694}
	{ 0x27d41d81, 0x991f, 0x4dc6, { 0x87, 0x49, 0xb0, 0x32, 0x1c, 0x87, 0xe6, 0x94 } }

};

DWORD WINAPI DllMain(HINSTANCE hInstance, DWORD, LPVOID)
{
	g_hInstance = hInstance;

	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

int OnModulesLoaded(WPARAM, LPARAM)
{
	g_hCLibsFolder = FoldersRegisterCustomPathT(MODULE, "CLibsFolder", MIRLUA_PATHT, TranslateT("C libs folder"));
	g_hScriptsFolder = FoldersRegisterCustomPathT(MODULE, "ScriptsFolder", MIRLUA_PATHT, TranslateT("Scripts folder"));

	HookEvent(ME_OPT_INITIALISE, CMLuaOptions::OnOptionsInit);

	InitIcons();

	g_mLua = new CMLua();
	g_mLua->Load();

	return 0;
}

INT_PTR Call(WPARAM wParam, LPARAM lParam)
{
	return g_mLua->Call(wParam, lParam);
}

INT_PTR Exec(WPARAM wParam, LPARAM lParam)
{
	return g_mLua->Exec(wParam, lParam);
}

INT_PTR Eval(WPARAM wParam, LPARAM lParam)
{
	return g_mLua->Eval(wParam, lParam);
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS;
	nlu.szDescriptiveName.a = MODULE;
	nlu.szSettingsModule = MODULE;
	hNetlib = Netlib_RegisterUser(&nlu);

	PROTOCOLDESCRIPTOR pd = { 0 };
	pd.cbSize = sizeof(pd);
	pd.szName = MODULE;
	pd.type = PROTOTYPE_FILTER;
	Proto_RegisterModule(&pd);

	hRecvMessage = CreateHookableEvent(MODULE PSR_MESSAGE);
	CreateProtoServiceFunction(MODULE, PSR_MESSAGE, FilterRecvMessage);

	CreateServiceFunction(MS_LUA_CALL, Call);
	CreateServiceFunction(MS_LUA_EXEC, Exec);
	CreateServiceFunction(MS_LUA_EVAL, Eval);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	delete g_mLua;

	if (hNetlib)
	{
		Netlib_CloseHandle(hNetlib);
		hNetlib = NULL;
	}

	return 0;
}
