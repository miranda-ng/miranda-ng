#include "stdafx.h"

CMPlugin g_plugin;

HMODULE   g_hDLLXStatusIcons;
HICON     g_hMainIcon;

bool      g_bChatExist;

size_t    g_dwMirWorkDirPathLen;
WCHAR     g_szMirWorkDirPath[MAX_FILEPATH];

void IconsLoad();

/////////////////////////////////////////////////////////////////////////////////////////

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

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CMraProto>("MRA", pluginInfoEx)
{
	SetUniqueId("e-mail");
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

///////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	IconsLoad();
	InitXStatusIcons();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	DestroyXStatusIcons();
	if (g_hDLLXStatusIcons) {
		FreeLibrary(g_hDLLXStatusIcons);
		g_hDLLXStatusIcons = nullptr;
	}

	return 0;
}
