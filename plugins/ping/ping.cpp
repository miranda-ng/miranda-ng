#include "common.h"
#include "ping.h"
#include <list>
HINSTANCE hInst;
int hLangpack = 0;

HANDLE hNetlibUser = 0;
HANDLE hFillListEvent = 0;

bool use_raw_ping = true;

// plugin stuff
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"Ping Plugin",
	PLUGIN_MAKE_VERSION(0, 9, 1, 1),
	"Ping labelled IP addresses or domain names.",
	"Scott Ellis",
	"mail@scottellis.com.au",
	"© 2005 Scott Ellis",
	"http://www.scottellis.com.au/",
	UNICODE_AWARE,		//not transient
	{ 0x760ea901, 0xc0c2, 0x446c, { 0x80, 0x29, 0x94, 0xc3, 0xbc, 0x47, 0xc4, 0x5e } } // {760EA901-C0C2-446c-8029-94C3BC47C45E}
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	DisableThreadLibraryCalls(hInst);
	return TRUE;
}

extern "C" PING_API PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_PING, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}



void CreatePluginServices() {
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

int OnShutdown(WPARAM wParam, LPARAM lParam) {
	graphs_cleanup();

	UnhookEvent(hFillListEvent);

	if(use_raw_ping)
		cleanup_raw_ping();
	else
		ICMP::cleanup();

	DeinitList();

	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam) {
	NETLIBUSER nl_user = {0};
	nl_user.cbSize = sizeof(nl_user);
	nl_user.szSettingsModule = PLUG;
	//nl_user.flags = NUF_OUTGOING | NUF_HTTPGATEWAY | NUF_NOOPTIONS;
	//nl_user.flags = NUF_OUTGOING | NUF_NOOPTIONS;
	nl_user.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nl_user.ptszDescriptiveName = LPGENT("Ping Plugin");
	nl_user.szHttpGatewayHello = 0;
	nl_user.szHttpGatewayUserAgent = 0;
	nl_user.pfnHttpGatewayInit = 0;
	nl_user.pfnHttpGatewayWrapSend = 0;
	nl_user.pfnHttpGatewayUnwrapRecv = 0;

	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nl_user);

	InitUtils();

	InitMenus();
	
	hFillListEvent = HookEvent(PLUG "/ListReload", FillList);

	if(!DBGetContactSettingByte(0, PLUG, "PingPlugImport", 0)) {
		if(DBGetContactSettingDword(0, "PingPlug", "NumEntries", 0)) {
			import_ping_addresses();
			DBWriteContactSettingByte(0, PLUG, "PingPlugImport", 1);
		}
	}

	InitList();

	CallService(PLUG "/LoadPingList", 0, 0);	
	
	graphs_init();

	if(options.logging) CallService(PLUG "/Log", (WPARAM)"start", 0);

	return 0;
}

extern "C" PING_API int Load(void)
{
	//if(init_raw_ping()) {
		//MessageBox(0, Translate("Failed to initialize. Plugin disabled."), Translate("Ping Plugin"), MB_OK | MB_ICONERROR);
		//return 1;
		use_raw_ping = false;
	//}
	DBWriteContactSettingByte(0, PLUG, "UsingRawSockets", (BYTE)use_raw_ping);

	DuplicateHandle( GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &mainThread, THREAD_SET_CONTEXT, FALSE, 0 );
	hWakeEvent = CreateEvent(NULL, FALSE, FALSE, _T("Local\\ThreadWaitEvent"));

	InitializeCriticalSection(&list_cs);
	InitializeCriticalSection(&thread_finished_cs);
	InitializeCriticalSection(&list_changed_cs);
	InitializeCriticalSection(&data_list_cs);
	
	// create services before loading options - so we can have the 'getlogfilename' service!
	CreatePluginServices();

	LoadOptions();

	SkinAddNewSound("PingTimeout", "Ping Timout", 0);
	SkinAddNewSound("PingReply", "Ping Reply", 0);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);	

	HookEvent(ME_OPT_INITIALISE, PingOptInit );

	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnShutdown);

	return 0;
}

extern "C" PING_API int Unload(void)
{
	SavePingList(0, 0);

	DeleteCriticalSection(&list_cs);
	DeleteCriticalSection(&thread_finished_cs);
	DeleteCriticalSection(&list_changed_cs);
	DeleteCriticalSection(&data_list_cs);

	CloseHandle( mainThread );

	if(options.logging) CallService(PLUG "/Log", (WPARAM)"stop", 0);

	return 0;
}
