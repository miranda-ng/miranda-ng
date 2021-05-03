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
	// {E876FE63-0701-4CDA-BED5-7C73A379C1D1}
	{ 0xe876fe63, 0x701, 0x4cda, { 0xbe, 0xd5, 0x7c, 0x73, 0xa3, 0x79, 0xc1, 0xd1 }}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////
// Interface information

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu);
	HookEvent(ME_MSG_WINDOWEVENT, OnSrmmWindowOpened);
	HookEvent(ME_MSG_BUTTONPRESSED, OnSrmmButtonPressed);
	HookEvent(ME_OPT_INITIALISE, OnOptionsInitialized);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	InitializeIcons();
	InitializeMenus();
	InitializeServices();
	return 0;
}
