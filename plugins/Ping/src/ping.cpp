#include "stdafx.h"

CMPlugin g_plugin;

HNETLIBUSER hNetlibUser = nullptr;
HANDLE hFillListEvent = nullptr;

bool use_raw_ping = true;

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
	// {760EA901-C0C2-446C-8029-94C3BC47C45E}
	{ 0x760ea901, 0xc0c2, 0x446c, { 0x80, 0x29, 0x94, 0xc3, 0xbc, 0x47, 0xc4, 0x5e } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

static void CreatePluginServices()
{
	// general
	CreateServiceFunction(MODULENAME "/Ping", PluginPing);
	CreateServiceFunction(MODULENAME "/DblClick", DblClick);

	// list
	CreateServiceFunction(MODULENAME "/ClearPingList", ClearPingList);
	CreateServiceFunction(MODULENAME "/GetPingList", GetPingList);
	CreateServiceFunction(MODULENAME "/SetPingList", SetPingList);
	CreateServiceFunction(MODULENAME "/SetAndSavePingList", SetAndSavePingList);
	CreateServiceFunction(MODULENAME "/LoadPingList", LoadPingList);
	CreateServiceFunction(MODULENAME "/SavePingList", SavePingList);

	reload_event_handle = CreateHookableEvent(MODULENAME "/ListReload");

	//log
	CreateServiceFunction(MODULENAME "/Log", Log);
	CreateServiceFunction(MODULENAME "/ViewLogData", ViewLogData);
	CreateServiceFunction(MODULENAME "/GetLogFilename", GetLogFilename);
	CreateServiceFunction(MODULENAME "/SetLogFilename", SetLogFilename);

	// menu
	CreateServiceFunction(MODULENAME "/DisableAll", PingDisableAll);
	CreateServiceFunction(MODULENAME "/EnableAll", PingEnableAll);
	CreateServiceFunction(MODULENAME "/ToggleEnabled", ToggleEnabled);
	CreateServiceFunction(MODULENAME "/ShowGraph", ShowGraph);
	CreateServiceFunction(MODULENAME "/Edit", EditContact);

}

static int OnShutdown(WPARAM, LPARAM)
{
	graphs_cleanup();

	UnhookEvent(hFillListEvent);

	DeinitList();

	if (use_raw_ping)
		cleanup_raw_ping();
	else
		ICMP::cleanup();

	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	NETLIBUSER nl_user = {};
	nl_user.szSettingsModule = MODULENAME;
	nl_user.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nl_user.szDescriptiveName.w = TranslateT("Ping Plugin");
	hNetlibUser = Netlib_RegisterUser(&nl_user);

	InitUtils();
	InitMenus();

	hFillListEvent = HookEvent(MODULENAME "/ListReload", FillList);

	if (!g_plugin.getByte("PingPlugImport", 0)) {
		if (db_get_dw(0, "PingPlug", "NumEntries", 0)) {
			import_ping_addresses();
			g_plugin.setByte("PingPlugImport", 1);
		}
	}

	InitList();

	CallService(MODULENAME "/LoadPingList", 0, 0);

	graphs_init();

	if (options.logging)
		CallService(MODULENAME "/Log", (WPARAM)L"start", 0);

	return 0;
}

static IconItem iconList[] =
{
	{ LPGEN("Responding"), "ping_responding", IDI_ICON_RESPONDING },
	{ LPGEN("Not Responding"), "ping_not_responding", IDI_ICON_NOTRESPONDING },
	{ LPGEN("Testing"), "ping_testing", IDI_ICON_TESTING },
	{ LPGEN("Disabled"), "ping_disabled", IDI_ICON_DISABLED },
};

int CMPlugin::Load()
{
	use_raw_ping = false;
	g_plugin.setByte("UsingRawSockets", (uint8_t)use_raw_ping);

	hWakeEvent = CreateEvent(nullptr, FALSE, FALSE, L"Local\\ThreadWaitEvent");

	// create services before loading options - so we can have the 'getlogfilename' service!
	CreatePluginServices();

	LoadOptions();

	g_plugin.addSound("PingTimeout", LPGENW("Ping"), LPGENW("Timeout"));
	g_plugin.addSound("PingReply", LPGENW("Ping"), LPGENW("Reply"));

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_OPT_INITIALISE, PingOptInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnShutdown);
	HookEvent(ME_SKIN_ICONSCHANGED, ReloadIcons);

	g_plugin.registerIcon(LPGEN("Ping"), iconList);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	SavePingList(0, 0);

	if (options.logging)
		CallService(MODULENAME "/Log", (WPARAM)L"stop", 0);

	return 0;
}
