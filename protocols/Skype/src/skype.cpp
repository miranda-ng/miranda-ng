#include "skype.h"
#include "skype_proto.h"

int hLangpack;
HINSTANCE g_hInstance;
XML_API  xi = {0};
TIME_API tmi = {0};

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
	// {9C448C61-FC3F-42F9-B9F0-4A30E1CF8671}
	{0x9c448c61, 0xfc3f, 0x42f9, {0xb9, 0xf0, 0x4a, 0x30, 0xe1, 0xcf, 0x86, 0x71}}
};

DWORD WINAPI DllMain(HINSTANCE hInstance, DWORD, LPVOID)
{
	g_hInstance = hInstance;

	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};


extern "C" int __declspec(dllexport) Load(void)
{
	mir_getXI(&xi);
	mir_getTMI(&tmi);
	mir_getLP(&pluginInfo);

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName =		"SKYPE";
	pd.type =		PROTOTYPE_PROTOCOL;
	pd.fnInit =		(pfnInitProto)	CSkypeProto::InitSkypeProto;
	pd.fnUninit =	(pfnUninitProto)CSkypeProto::UninitSkypeProto;
	CallService(MS_PROTO_REGISTERMODULE, 0, reinterpret_cast<LPARAM>(&pd));

	CSkypeProto::InitIcons();
	CSkypeProto::InitServiceList();
	CSkypeProto::InitMenus();

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	CSkypeProto::UninitMenus();
	CSkypeProto::UninitIcons();

	return 0;
}