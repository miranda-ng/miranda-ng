/*/////////////////////////////////////////////
	Tooltip Notify plugin for Miranda IM
*//////////////////////////////////////////////

#include "stdafx.h"
#include "version.h"
#include "TooltipNotify.h"

// {5906A545-F31A-4726-B48F-03A09F060318}
static const MUUID MIID_TOOLTIPNOTIFY_UNICODE = 
{ 0x5906a545, 0xf31a, 0x4726, { 0xb4, 0x8f, 0x3, 0xa0, 0x9f, 0x6, 0x3, 0x18 } };

// {03CD82B6-0BB5-4f26-8EB4-06CD8ECD36FF}
static const MUUID MIID_TOOLTIPNOTIFY = 
{ 0x3cd82b6, 0xbb5, 0x4f26, { 0x8e, 0xb4, 0x6, 0xcd, 0x8e, 0xcd, 0x36, 0xff } };

static int InitializeOptions(WPARAM wParam,LPARAM lParam);
static int ModulesLoaded(WPARAM wParam,LPARAM lParam);
static int ContactSettingChanged(WPARAM wParam,LPARAM lParam);
static int ProtoAck(WPARAM,LPARAM);
static int ProtoContactIsTyping(WPARAM wParam,LPARAM lParam);

static HANDLE g_hContactSettingChanged = 0;
static HANDLE g_hOptionsInitialize = 0;
static HANDLE g_hModulesLoaded = 0;
static HANDLE g_hProtoAck = 0;
static HANDLE g_hProtoContactIsTyping = 0;
static HINSTANCE g_hInstDLL = 0;
static bool g_bRightModule = false;	// i.e. ansi for win9x, and unicode for winnt

// Main global object
static CTooltipNotify *g_pTooltipNotify = 0;
int hLangpack;


//================================================================================
// plugin init/deinit routines
//================================================================================

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hInstDLL);
			g_hInstDLL = hInstDLL;

			OSVERSIONINFO OsVersionInfo;
			OsVersionInfo.dwOSVersionInfoSize = sizeof(OsVersionInfo);
			GetVersionEx(&OsVersionInfo);

		
			g_bRightModule = (OsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);


			break;
		}

		case DLL_PROCESS_DETACH:
			break;
	}

	return TRUE;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_TOOLTIPNOTIFY, MIID_LAST};

static PLUGININFOEX sPluginInfo =
{
	sizeof(PLUGININFOEX),
	"Tooltip Notify",
	PLUGIN_MAKE_VERSION(MAJOR,MINOR,BUILD,REVISION),	// major, minor, revision, build
	"Shows a small tooltip above system tray area when a contact status is changed.",
	"perf",
	"perf@mail333.com",
	"© 2004-2008 Gneedah software",
	"http://miranda-ng.org/",
	UNICODE_AWARE,		//doesn't replace anything built-in
	MIID_TOOLTIPNOTIFY_UNICODE
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &sPluginInfo;
}

extern "C" int __declspec(dllexport) Load(void)
{
	if (!g_bRightModule) return 0;

	mir_getLP(&sPluginInfo);

	g_pTooltipNotify = new CTooltipNotify(g_hInstDLL);
	assert(g_pTooltipNotify!=0);
	
	g_hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	if (g_hContactSettingChanged) UnhookEvent(g_hContactSettingChanged);
	if (g_hProtoContactIsTyping) UnhookEvent(g_hProtoContactIsTyping);
	if (g_hProtoAck) UnhookEvent(g_hProtoAck);
	if (g_hOptionsInitialize) UnhookEvent(g_hOptionsInitialize);
	if (g_hModulesLoaded) UnhookEvent(g_hModulesLoaded);
	delete g_pTooltipNotify;

	return 0;
}



//================================================================================
//================================================================================
//================================================================================


int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	g_hContactSettingChanged = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);
	g_hProtoAck = HookEvent(ME_PROTO_ACK, ProtoAck);
	g_hProtoContactIsTyping = HookEvent(ME_PROTO_CONTACTISTYPING, ProtoContactIsTyping);
	g_hOptionsInitialize = HookEvent(ME_OPT_INITIALISE, InitializeOptions);	

	return CTooltipNotify::GetObjInstance()->ModulesLoaded(wParam, lParam);
}


int ProtoContactIsTyping(WPARAM wParam, LPARAM lParam)
{
	return CTooltipNotify::GetObjInstance()->ProtoContactIsTyping(wParam, lParam);
}


int ProtoAck(WPARAM wParam, LPARAM lParam)
{
	return CTooltipNotify::GetObjInstance()->ProtoAck(wParam, lParam);
}


int ContactSettingChanged(WPARAM wParam, LPARAM lParam) 
{ 
	return CTooltipNotify::GetObjInstance()->ContactSettingChanged(wParam, lParam);
}


int InitializeOptions(WPARAM wParam, LPARAM lParam)
{
	return CTooltipNotify::GetObjInstance()->InitializeOptions(wParam, lParam);
}





