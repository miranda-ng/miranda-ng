/*/////////////////////////////////////////////
	Tooltip Notify plugin for Miranda IM
*//////////////////////////////////////////////

#include "stdafx.h"

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

HINSTANCE g_hInstDLL = 0;

// Main global object
static CTooltipNotify *g_pTooltipNotify = 0;
int hLangpack;


//================================================================================
// plugin init/deinit routines
//================================================================================

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInstDLL = hInstDLL;
	return TRUE;
}

static PLUGININFOEX sPluginInfo =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {5906A545-F31A-4726-B48F-03A09F060318}
	{0x5906a545, 0xf31a, 0x4726, {0xb4, 0x8f, 0x3, 0xa0, 0x9f, 0x6, 0x3, 0x18}}
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &sPluginInfo;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&sPluginInfo);

	g_pTooltipNotify = new CTooltipNotify();
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





