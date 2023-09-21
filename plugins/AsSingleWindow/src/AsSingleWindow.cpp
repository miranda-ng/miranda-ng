#include "stdafx.h"
#include "AsSingleWindow.h"
#include "Options.h"
#include "WindowsManager.h"
#include "version.h"

sPluginVars pluginVars;

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	 sizeof(PLUGININFOEX),
	 __PLUGIN_NAME,
	 PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	 __DESCRIPTION,
	 __AUTHOR,
	 __COPYRIGHT,
	 __AUTHORWEB,
	 UNICODE_AWARE,
	 {0xF6C73B4, 0x2B2B, 0x711D, {0xFB, 0xB6, 0xBB, 0x26, 0x7D, 0xFD, 0x72, 0x08}}, // 0xF6C73B42B2B711DFBB6BB267DFD7208
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("AsSingleWindow", pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

static int MsgWindowEvent(WPARAM uType, LPARAM lParam)
{
	auto *pDlg = (CSrmmBaseDialog *)lParam;

	switch (uType) {
	case MSG_WINDOW_EVT_OPEN:
		// Здесь можно отлавливать только открытие окна,
		// т.к. закрытие может быть закрытием вкладки
		windowAdd(pDlg->GetHwnd(), false);
		break;
	}

	return 0;
}

static int OnModulesLoaded(WPARAM, LPARAM)
{
	windowAdd(g_clistApi.hwndContactList, true);

	HookEvent(ME_MSG_WINDOWEVENT, MsgWindowEvent);

	optionsLoad();
	return 0;
}

static int OnShutdown(WPARAM, LPARAM)
{
	for (auto itr = pluginVars.allWindows.begin(); itr != pluginVars.allWindows.end(); ++itr)
		mir_unsubclassWindow(itr->hWnd, wndProcSync);
	return 0;
}

int CMPlugin::Load()
{
	::InitializeCriticalSection(&pluginVars.m_CS);
	pluginVars.IsUpdateInProgress = false;
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);
	HookEvent(ME_OPT_INITIALISE, InitOptions);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	::DeleteCriticalSection(&pluginVars.m_CS);
	return 0;
}
