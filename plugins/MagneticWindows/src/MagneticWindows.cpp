#include "stdafx.h"

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

CMPlugin g_plugin;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {08C01613-24C8-486F-BDAE-2C3DDCAF9347}
	{0x8c01613, 0x24c8, 0x486f, { 0xbd, 0xae, 0x2c, 0x3d, 0xdc, 0xaf, 0x93, 0x47 }}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("MagneticWindows", pluginInfoEx)
{}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin Functions
///////////////////////////////////////////////////////////////////////////////////////////////////

//For other Plugins to start snapping for other Windows
INT_PTR SnapPluginWindowStart(WPARAM wParam, LPARAM)
{
	if (!WindowOpen((HWND)wParam)) return 1;
	return 0;
}

//For other Plugins to stop snapping for other Windows
INT_PTR SnapPluginWindowStop(WPARAM wParam, LPARAM)
{
	if (!WindowClose((HWND)wParam)) return 1;
	return 0;
}

int PluginMessageWindowEvent(WPARAM uType, LPARAM lParam)
{
	auto *pDlg = (CSrmmBaseDialog *)lParam;

	switch (uType) {
	case MSG_WINDOW_EVT_OPEN:
	{
		HWND hWnd = pDlg->GetHwnd();
		HWND hWndParent = GetParent(hWnd);
		while ((hWndParent != 0) && (hWndParent != GetDesktopWindow()) && (IsWindowVisible(hWndParent))) {
			hWnd = hWndParent;
			hWndParent = GetParent(hWnd);
		}

		WindowOpen(hWnd);
	}
	break;

	case MSG_WINDOW_EVT_CLOSING:
		WindowClose(pDlg->GetHwnd());
		break;
	}

	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Main Functions
///////////////////////////////////////////////////////////////////////////////////////////////////

int SnapPluginStart(WPARAM, LPARAM)
{
	LoadOptions();

	HookEvent(ME_MSG_WINDOWEVENT, PluginMessageWindowEvent);

	WindowOpen(g_clistApi.hwndContactList);
	return 0;
}

int SnapPluginShutDown(WPARAM, LPARAM)
{
	WindowCloseAll();
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Exportet Functions
///////////////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, SnapPluginStart);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, SnapPluginShutDown);
	HookEvent(ME_OPT_INITIALISE, InitOptions);

	CreateServiceFunction(MS_MW_ADDWINDOW, SnapPluginWindowStart);
	CreateServiceFunction(MS_MW_REMWINDOW, SnapPluginWindowStop);
	CreateServiceFunction(MS_MW_SNAPTOLIST, SnapToList);

	WindowStart();
	return 0;
}
