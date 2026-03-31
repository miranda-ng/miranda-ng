#include "stdafx.h"

CMPlugin g_plugin;

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
	{ 0x73f7fc9d, 0x6eb5, 0x4f50, { 0x95, 0x1e, 0x57, 0x1f, 0x21, 0xd0, 0x52, 0xa0 } }
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CMaxProto>("Max", pluginInfoEx)
{
	SetUniqueId(MAX_SETTINGS_ID);
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };
