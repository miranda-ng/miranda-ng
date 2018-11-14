/*
Wannabe OSD
This plugin tries to become miranda's standard OSD ;-)

(C) 2005 Andrej Krutak

Distributed under GNU's GPL 2 or later
*/

#include "stdafx.h"

CMPlugin g_plugin;

HWND g_hWnd = nullptr;
HANDLE hHookContactStatusChanged;

void logmsg2(char *str);
int MainInit(WPARAM,LPARAM);

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {FC718BC7-ABC8-43CD-AAD9-761614617738}
	{0xfc718bc7, 0xabc8, 0x43cd, {0xaa, 0xd9, 0x76, 0x16, 0x14, 0x61, 0x77, 0x38}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("mirandaosd", pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	logmsg("Load");
	HookEvent(ME_SYSTEM_MODULESLOADED, MainInit);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	logmsg("Unload");

	DestroyHookableEvent(hHookContactStatusChanged);
	return 0;
}
