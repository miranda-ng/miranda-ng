/*/////////////////////////////////////////////
	Tooltip Notify plugin for Miranda IM
*//////////////////////////////////////////////

#include "stdafx.h"
#include "version.h"
#include "TooltipNotify.h"

// {5906A545-F31A-4726-B48F-03A09F060318}
static const MUUID MIID_TOOLTIPNOTIFY_UNICODE = 
{ 0x5906a545, 0xf31a, 0x4726, { 0xb4, 0x8f, 0x3, 0xa0, 0x9f, 0x6, 0x3, 0x18 } };

// {C4475C65-630F-4e70-980F-C0CA98767110}
static const MUUID MIID_TOOLTIPNOTIFY_ANSI = 
{ 0xc4475c65, 0x630f, 0x4e70, { 0x98, 0xf, 0xc0, 0xca, 0x98, 0x76, 0x71, 0x10 } };

// {03CD82B6-0BB5-4f26-8EB4-06CD8ECD36FF}
static const MUUID MIID_TOOLTIPNOTIFY = 
{ 0x3cd82b6, 0xbb5, 0x4f26, { 0x8e, 0xb4, 0x6, 0xcd, 0x8e, 0xcd, 0x36, 0xff } };

static int InitializeOptions(WPARAM wParam,LPARAM lParam);
static int ModulesLoaded(WPARAM wParam,LPARAM lParam);
static int ContactSettingChanged(WPARAM wParam,LPARAM lParam);
static int ProtoAck(WPARAM,LPARAM);
static int ProtoContactIsTyping(WPARAM wParam,LPARAM lParam);


// Globals
PLUGINLINK *pluginLink;	// cannot be static since it is used for mim service calls

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

#ifdef _UNICODE			
			g_bRightModule = (OsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
#else
			g_bRightModule = (OsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS);
#endif

			break;
		}

		case DLL_PROCESS_DETACH:
			break;
	}

	return TRUE;
}

extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	static const MUUID interfaces[] = {MIID_TOOLTIPNOTIFY, MIID_LAST};
	return interfaces;
}

	static char szWrongUsage9x[] = 
		"Warning! You are trying to use unicode version of the plugin on win9x system! "
		"It can not be working here. You must use ansi version of the plugin.";
	
	static char szWrongUsageNt[] = 
		"Warning! You are using ansi version of the plugin on a unicode-aware system. "
		"It is recommended to use unicode version of the plugin.";
	
	static char szFunctionalDescription[] = 
		"Shows a small tooltip above system tray area when a contact status is changed.";


	static PLUGININFOEX sPluginInfo =
	{
		sizeof(PLUGININFOEX),
		"Tooltip Notify",
		PLUGIN_MAKE_VERSION(MAJOR,MINOR,BUILD,REVISION),	// major, minor, revision, build
#ifdef _UNICODE
		g_bRightModule ? szFunctionalDescription : szWrongUsage9x,
#else
		g_bRightModule ? szFunctionalDescription : szWrongUsageNt,
#endif
		"perf",
		"perf@mail333.com",
		"© 2004-2008 Gneedah software",
		"http://addons.miranda-im.org/details.php?action=viewfile&id=1290",
		UNICODE_AWARE,
		0,		//doesn't replace anything built-in
#ifdef _UNICODE
		MIID_TOOLTIPNOTIFY_UNICODE
#else
		MIID_TOOLTIPNOTIFY_ANSI
#endif
	};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &sPluginInfo;
}

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	PLUGININFOEX* pPluginInfoEx = MirandaPluginInfoEx(mirandaVersion);

	static PLUGININFO sPluginInfo =
	{
		sizeof(PLUGININFO),
		pPluginInfoEx->shortName,
		pPluginInfoEx->version,
		pPluginInfoEx->description,
		pPluginInfoEx->author,
		pPluginInfoEx->authorEmail,
		pPluginInfoEx->copyright,
		pPluginInfoEx->homepage,
		pPluginInfoEx->flags,
		pPluginInfoEx->replacesDefaultModule
	};

	return &sPluginInfo;
}


extern "C" int __declspec(dllexport) Load(PLUGINLINK *pLink)
{
#ifdef _UNICODE
	if (!g_bRightModule) return 0;
#else
	// ansi version can work ok on winnt platform
#endif

	pluginLink = pLink;
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





