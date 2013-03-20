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

PLUGININFO pluginInfo = {
	sizeof(PLUGININFO),
	"Magnetic Windows",
	PLUGIN_MAKE_VERSION(0,0,3,2),
	"Makes the main contactlist and the chat windows snapping to the desktop border and to each other.",
	"Michael Kunz",
	"Michael.Kunz@s2005.TU-Cemnitz.de",
	"(c) 2006 Michael Kunz",
	"http://addons.miranda-im.org/details.php?action=viewfile&id=2871",
	0,
	0
};

PLUGINLINK * pluginLink;

HANDLE hLoadedHook, hShootDownHook,	hAddService, hRemService, hWindowEventHook;

HINSTANCE hInst;
//char ModuleName[256];
char ModuleName[] = "MagneticWindows";

///////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin Functions
///////////////////////////////////////////////////////////////////////////////////////////////////


//For other Plugins to start snapping for other Windows
int SnapPluginWindowStart(WPARAM wParam, LPARAM lParam) {

	if (!WindowOpen((HWND)wParam)) return 1;

	return 0;
}
//For other Plugins to stop snapping for other Windows
int SnapPluginWindowStop(WPARAM wParam, LPARAM lParam) {

	if (!WindowClose((HWND)wParam)) return 1;

	return 0;
}

int PluginMessageWindowEvent(WPARAM wParam, LPARAM lParam) {
	MessageWindowEventData* Data;
	HWND hWndParent, hWnd;

	Data = (MessageWindowEventData*)(lParam);
	
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


int SnapPluginStart(WPARAM wParam, LPARAM lParam) {

	LoadOptions();

	hWindowEventHook = HookEvent(ME_MSG_WINDOWEVENT, PluginMessageWindowEvent);

	WindowOpen((HWND)CallService(MS_CLUI_GETHWND,0,0));
	return 0;
}

int SnapPluginShootDown(WPARAM wParam, LPARAM lParam) {
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

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;
	
	hLoadedHook = HookEvent(ME_SYSTEM_MODULESLOADED, SnapPluginStart);
	hShootDownHook = HookEvent(ME_SYSTEM_PRESHUTDOWN, SnapPluginShootDown);
	hInitOptionsHook = HookEvent(ME_OPT_INITIALISE, InitOptions);

	hAddService = CreateServiceFunction(MS_MW_ADDWINDOW, SnapPluginWindowStart);
	hRemService = CreateServiceFunction(MS_MW_REMWINDOW, SnapPluginWindowStop);
	hSnapToListService = CreateServiceFunction(MS_MW_SNAPTOLIST, SnapToList);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DLL MAIN
///////////////////////////////////////////////////////////////////////////////////////////////////


extern "C" bool APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	hInst = hModule;
	

/*	char * Temp;
	char * l,i;

	Temp = (char*) malloc(2048);
	GetModuleFileName(hModule, Temp, 2048);

	l = Temp;
	i = Temp;

	while (i != 0) {
		if ((*i) == '\') l = i + 1;
		i++;
	}

	memcpy(ModuleName, l, i-l);
	free(Temp);
	*/

    return true;
}
