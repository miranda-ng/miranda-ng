#include "stdafx.h"

int hLangpack;
CMPlugin g_plugin;

#pragma comment(lib, "tdactor.lib")
#pragma comment(lib, "tdcore.lib")
#pragma comment(lib, "tddb.lib")
#pragma comment(lib, "tdlib.lib")
#pragma comment(lib, "tdnet.lib")
#pragma comment(lib, "tdutils.lib")

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
	// {AE708252-0DF8-42BA-9EF9-9ACC038EEDA7}
	{0xae708252, 0xdf8, 0x42ba, {0x9e, 0xf9, 0x9a, 0xcc, 0x3, 0x8e, 0xed, 0xa7}}
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CTelegramProto>("Telegram", pluginInfo)
{
	SetUniqueId(DBKEY_ID);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] =
{
	{ LPGEN("Telegram Premium user"), "premuim", IDI_PREMIUM },
	{ LPGEN("Forward"), "forward", IDI_FORWARD },
	{ LPGEN("Reaction"), "reaction", IDI_REACTION },
};

int CMPlugin::Load()
{
	registerIcon("Protocols/Telegram", iconList, "tg");
	
	m_hIcon = ExtraIcon_RegisterIcolib("tg_premium", LPGEN("Telegram Premium user"), getIconHandle(IDI_PREMIUM));
	return 0;
}
