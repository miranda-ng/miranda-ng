/*
Copyright (c) 2026 Miranda NG team
GPLv2
*/

#include "stdafx.h"

CMPlugin g_plugin;

IconItem g_iconList[] = {
	{ LPGEN("Protocol icon"), "main", IDI_MAIN },
	{ LPGEN("Forward"), "forward", IDI_FORWARD },
};

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {A7E4F9C2-1D8B-4E5A-9C3F-8B2E1D0C9A8F}
	{ 0xa7e4f9c2, 0x1d8b, 0x4e5a, { 0x9c, 0x3f, 0x8b, 0x2e, 0x1d, 0x0c, 0x9a, 0x8f } }
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CMaxProto>("Max", pluginInfoEx)
{
	SetUniqueId(DB_KEY_MAX_UID);
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

int CMPlugin::Load()
{
	g_plugin.registerIcon(LPGEN("Protocol icons"), g_iconList);
	return 0;
}
