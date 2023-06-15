#include "stdafx.h"

CMPlugin g_plugin;

HANDLE hExtraXStatus;

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
	// {68F5A030-BA32-48EC-9507-5C2FBDEA5217}
	{ 0x68f5a030, 0xba32, 0x48ec, { 0x95, 0x7, 0x5c, 0x2f, 0xbd, 0xea, 0x52, 0x17 }}
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CSteamProto>("STEAM", pluginInfoEx)
{
	SetUniqueId(DBKEY_STEAM_ID);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	char iconName[100];
	mir_snprintf(iconName, "%s_%s", MODULE, "gaming");

	// extra statuses
	HookEvent(ME_SKIN_ICONSCHANGED, OnReloadIcons);
	hExtraXStatus = ExtraIcon_RegisterIcolib("steam_game", LPGEN("Steam game"), iconName);

	CSteamProto::InitMenus();
	return 0;
}
