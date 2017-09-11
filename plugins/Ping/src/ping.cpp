#include "stdafx.h"

CLIST_INTERFACE *pcli;
HINSTANCE hInst;
int hLangpack = 0;

HNETLIBUSER hNetlibUser = 0;
HANDLE hFillListEvent = 0;

bool use_raw_ping = true;

// plugin stuff
PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {760EA901-C0C2-446C-8029-94C3BC47C45E}
	{ 0x760ea901, 0xc0c2, 0x446c, { 0x80, 0x29, 0x94, 0xc3, 0xbc, 0x47, 0xc4, 0x5e } }
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD, LPVOID)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

void CreatePluginServices()
{
	// general
	CreateServiceFunction(PLUG "/Ping", PluginPing);
	CreateServiceFunction(PLUG "/DblClick", DblClick);

	// list
	CreateServiceFunction(PLUG "/ClearPingList", ClearPingList);
	CreateServiceFunction(PLUG "/GetPingList", GetPingList);
	CreateServiceFunction(PLUG "/SetPingList", SetPingList);
	CreateServiceFunction(PLUG "/SetAndSavePingList", SetAndSavePingList);
	CreateServiceFunction(PLUG "/LoadPingList", LoadPingList);
	CreateServiceFunction(PLUG "/SavePingList", SavePingList);

	reload_event_handle = CreateHookableEvent(PLUG "/ListReload");

	//log
	CreateServiceFunction(PLUG "/Log", Log);
	CreateServiceFunction(PLUG "/ViewLogData", ViewLogData);
	CreateServiceFunction(PLUG "/GetLogFilename", GetLogFilename);
	CreateServiceFunction(PLUG "/SetLogFilename", SetLogFilename);

	// menu
	CreateServiceFunction(PLUG "/DisableAll", PingDisableAll);
	CreateServiceFunction(PLUG "/EnableAll", PingEnableAll);
	CreateServiceFunction(PLUG "/ToggleEnabled", ToggleEnabled);
	CreateServiceFunction(PLUG "/ShowGraph", ShowGraph);
	CreateServiceFunction(PLUG "/Edit", EditContact);

}

int OnShutdown(WPARAM, LPARAM) {
	graphs_cleanup();

	UnhookEvent(hFillListEvent);

	DeinitList();

	if (use_raw_ping)
		cleanup_raw_ping();
	else
		ICMP::cleanup();

	return 0;
}

int OnModulesLoaded(WPARAM, LPARAM)
{
	NETLIBUSER nl_user = {};
	nl_user.szSettingsModule = PLUG;
	nl_user.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nl_user.szDescriptiveName.w = TranslateT("Ping Plugin");

	hNetlibUser = Netlib_RegisterUser(&nl_user);

	InitUtils();

	InitMenus();

	hFillListEvent = HookEvent(PLUG "/ListReload", FillList);

	if (!db_get_b(0, PLUG, "PingPlugImport", 0)) {
		if (db_get_dw(0, "PingPlug", "NumEntries", 0)) {
			import_ping_addresses();
			db_set_b(0, PLUG, "PingPlugImport", 1);
		}
	}

	InitList();

	CallService(PLUG "/LoadPingList", 0, 0);

	graphs_init();

	if (options.logging) CallService(PLUG "/Log", (WPARAM)L"start", 0);

	return 0;
}

static IconItem iconList[] =
{
	{ LPGEN("Responding"), "ping_responding", IDI_ICON_RESPONDING },
	{ LPGEN("Not Responding"), "ping_not_responding", IDI_ICON_NOTRESPONDING },
	{ LPGEN("Testing"), "ping_testing", IDI_ICON_TESTING },
	{ LPGEN("Disabled"), "ping_disabled", IDI_ICON_DISABLED },
};

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	pcli = Clist_GetInterface();

	use_raw_ping = false;
	db_set_b(0, PLUG, "UsingRawSockets", (BYTE)use_raw_ping);

	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &mainThread, THREAD_SET_CONTEXT, FALSE, 0);
	hWakeEvent = CreateEvent(NULL, FALSE, FALSE, L"Local\\ThreadWaitEvent");

	// create services before loading options - so we can have the 'getlogfilename' service!
	CreatePluginServices();

	LoadOptions();

	Skin_AddSound("PingTimeout", LPGENW("Ping"), LPGENW("Timeout"));
	Skin_AddSound("PingReply", LPGENW("Ping"), LPGENW("Reply"));

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	HookEvent(ME_OPT_INITIALISE, PingOptInit);

	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnShutdown);

	Icon_Register(hInst, LPGEN("Ping"), iconList, _countof(iconList));

	HookEvent(ME_SKIN2_ICONSCHANGED, ReloadIcons);

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	SavePingList(0, 0);

	CloseHandle(mainThread);

	if (options.logging) CallService(PLUG "/Log", (WPARAM)L"stop", 0);

	return 0;
}
