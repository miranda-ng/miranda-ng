#include "stdafx.h"
#include "AsSingleWindow.h"
#include "Options.h"
#include "WindowsManager.h"
#include "version.h"

CLIST_INTERFACE *pcli;
int hLangpack;

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
	 {0xF6C73B4, 0x2B2B, 0x711D, {0xFB, 0xB6, 0xBB, 0x26, 0x7D, 0xFD, 0x72, 0x08}}, // 0xF6C73B42B2B711DFBB6BB267DFD7208
};

sPluginVars pluginVars;

bool WINAPI DllMain(HINSTANCE hInstDLL, DWORD, LPVOID)
{
	pluginVars.hInst = hInstDLL;
	return true;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

static int OnShutdown(WPARAM, LPARAM)
{
	for (auto itr = pluginVars.allWindows.begin(); itr != pluginVars.allWindows.end(); ++itr)
		mir_unsubclassWindow(itr->hWnd, wndProcSync);
	return 0;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	pcli = Clist_GetInterface();

	::InitializeCriticalSection(&pluginVars.m_CS);
	pluginVars.IsUpdateInProgress = false;
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);
	HookEvent(ME_OPT_INITIALISE, InitOptions);
	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	::DeleteCriticalSection(&pluginVars.m_CS);
	return 0;
}

int OnModulesLoaded(WPARAM, LPARAM)
{
	windowAdd(pcli->hwndContactList, true);

	HookEvent(ME_MSG_WINDOWEVENT, MsgWindowEvent);

	optionsLoad();
	return 0;
}

int MsgWindowEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData* data = (MessageWindowEventData*)lParam;

	if (data == NULL)
		return 0;

	switch (data->uType) {
	case MSG_WINDOW_EVT_OPEN:
		// Здесь можно отлавливать только открытие окна,
		// т.к. закрытие может быть закрытием вкладки
		windowAdd(data->hwndWindow, false);
		break;
	}

	return 0;
}
