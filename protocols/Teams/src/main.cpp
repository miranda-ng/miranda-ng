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
	// {DCD56CEC-C61B-4275-A010-8C65C5B48815}
	{ 0xDCD56CEC, 0xC61B, 0x4275, { 0xa0, 0x10, 0x8c, 0x65, 0xc5, 0x84, 0x88, 0x15 }}
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CTeamsProto>("Teams", pluginInfoEx)
{
	SetUniqueId("id");
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

static IconItem iconList[] = {
	{ LPGEN("Protocol icon"), "main", IDI_TEAMS },
};

int CMPlugin::Load()
{
	g_plugin.registerIcon("Protocols/" MODULENAME, iconList, MODULENAME);
	return 0;
}
