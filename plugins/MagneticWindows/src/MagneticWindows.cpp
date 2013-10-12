#include "MagneticWindowsCore.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Magnetic Windows
//
//  Autor: Michael Kunz
//  EMail: Michael.Kunz@s2005.tu-chemnitz.de
//  
//
//  thanks to: pescuma
//
///////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////////////////////////////////////////

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
	// {08C01613-24C8-486F-BDAE-2C3DDCAF9347}
	{0x8c01613, 0x24c8, 0x486f, { 0xbd, 0xae, 0x2c, 0x3d, 0xdc, 0xaf, 0x93, 0x47 }} 
};


HANDLE hLoadedHook, hShootDownHook,	hAddService, hRemService, hWindowEventHook;

HINSTANCE hInst;
int hLangpack;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin Functions
///////////////////////////////////////////////////////////////////////////////////////////////////


//For other Plugins to start snapping for other Windows
INT_PTR SnapPluginWindowStart(WPARAM wParam, LPARAM) {
	if (!WindowOpen((HWND)wParam)) return 1;

	return 0;
}
//For other Plugins to stop snapping for other Windows
INT_PTR SnapPluginWindowStop(WPARAM wParam, LPARAM) {
	if (!WindowClose((HWND)wParam)) return 1;

	return 0;
}

int PluginMessageWindowEvent(WPARAM, LPARAM lParam) {
	HWND hWndParent, hWnd;

	MessageWindowEventData *Data = (MessageWindowEventData*) lParam;
	
	switch (Data->uType) {
		case MSG_WINDOW_EVT_OPEN: 
			hWnd = Data->hwndWindow;
			//WindowOpen(hWnd);
			hWndParent = GetParent(hWnd);
			while ((hWndParent != 0) && (hWndParent != GetDesktopWindow()) && (IsWindowVisible(hWndParent))) {			
				hWnd = hWndParent;
				hWndParent = GetParent(hWnd);			
			}

			WindowOpen(hWnd);
			break;
		
		case MSG_WINDOW_EVT_CLOSING:
			WindowClose(Data->hwndWindow);
			break;

		default:
			break;
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Main Functions
///////////////////////////////////////////////////////////////////////////////////////////////////


int SnapPluginStart(WPARAM, LPARAM) {
	LoadOptions();

	hWindowEventHook = HookEvent(ME_MSG_WINDOWEVENT, PluginMessageWindowEvent);

	WindowOpen((HWND)CallService(MS_CLUI_GETHWND,0,0));
	return 0;
}

int SnapPluginShootDown(WPARAM, LPARAM) {
	UnhookEvent(hWindowEventHook);
	UnhookEvent(hLoadedHook);
	UnhookEvent(hShootDownHook);
	UnhookEvent(hInitOptionsHook);

	WindowCloseAll();

	DestroyServiceFunction(hAddService);
	DestroyServiceFunction(hRemService);
	DestroyServiceFunction(hSnapToListService);

	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Exportet Functions
///////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" int __declspec(dllexport) Load()
{
	mir_getLP(&pluginInfo);
	
	hLoadedHook = HookEvent(ME_SYSTEM_MODULESLOADED, SnapPluginStart);
	hShootDownHook = HookEvent(ME_SYSTEM_PRESHUTDOWN, SnapPluginShootDown);
	hInitOptionsHook = HookEvent(ME_OPT_INITIALISE, InitOptions);

	hAddService = CreateServiceFunction(MS_MW_ADDWINDOW, SnapPluginWindowStart);
	hRemService = CreateServiceFunction(MS_MW_REMWINDOW, SnapPluginWindowStop);
	hSnapToListService = CreateServiceFunction(MS_MW_SNAPTOLIST, SnapToList);

	return 0;
}

extern "C" int __declspec(dllexport) Unload()
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DLL MAIN
///////////////////////////////////////////////////////////////////////////////////////////////////


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	
    return TRUE;
}
