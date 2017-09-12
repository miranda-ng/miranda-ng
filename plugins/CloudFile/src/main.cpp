#include "stdafx.h"

int hLangpack;
HINSTANCE hInstance;
HNETLIBUSER hNetlibConnection;

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
	// {E876FE63-0701-4CDA-BED5-7C73A379C1D1}
	{ 0xe876fe63, 0x701, 0x4cda, { 0xbe, 0xd5, 0x7c, 0x73, 0xa3, 0x79, 0xc1, 0xd1 }}
};

DWORD WINAPI DllMain(HINSTANCE hInst, DWORD, LPVOID)
{
	hInstance = hInst;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_DROPBOX, MIID_LAST };

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	HookEvent(ME_PROTO_ACK, OnProtoAck);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS;
	nlu.szSettingsModule = MODULE;
	nlu.szDescriptiveName.a = MODULE;
	hNetlibConnection = Netlib_RegisterUser(&nlu);

	InitServices();
	InitializeIcons();
	InitializeMenus();

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}