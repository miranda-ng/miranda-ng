#include "stdafx.h"
#include "AsSingleWindow.h"
#include "Options.h"
#include "WindowsManager.h"

CLIST_INTERFACE *pcli;
int hLangpack;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX), // PLUGININFOEX
    "AsSingleWindow",
    PLUGIN_MAKE_VERSION(0, 1, 2, 1),
    "Makes easier windows manipulation: allows you to move, minimize and activate Miranda's windows as if it were a single window.",
    "Aleksey Smyrnov aka Soar",
    "i@soar.name",
    "© Soar, 2010-2011",
    "http://soar.name/tag/assinglewindow/",
    UNICODE_AWARE,
    {0xF6C73B4, 0x2B2B, 0x711D, {0xFB, 0xB6, 0xBB, 0x26, 0x7D, 0xFD, 0x72, 0x08}}, // 0xF6C73B42B2B711DFBB6BB267DFD7208
};

sPluginVars pluginVars;

bool WINAPI DllMain(HINSTANCE hInstDLL, DWORD, LPVOID)
{
    pluginVars.hInst = hInstDLL;
    return true;
}

static const MUUID interfaces[] = {MIID_CLIST, MIID_SRMM, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
    return interfaces;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
    return &pluginInfo;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	pcli = Clist_GetInterface();
	
	::InitializeCriticalSection(&pluginVars.m_CS);
    pluginVars.IsUpdateInProgress = false;
    pluginVars.heModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
    pluginVars.heOptionsLoaded = HookEvent(ME_OPT_INITIALISE, InitOptions);
    
    return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
    UnhookEvent(pluginVars.heOptionsLoaded);
    UnhookEvent(pluginVars.heModulesLoaded);

    ::DeleteCriticalSection(&pluginVars.m_CS);

    return 0;
}

int OnModulesLoaded(WPARAM, LPARAM)
{
	HWND hWndCListWindow = pcli->hwndContactList;
    windowAdd(hWndCListWindow, true);

    pluginVars.heMsgWndEvent = HookEvent(ME_MSG_WINDOWEVENT, MsgWindowEvent);

    optionsLoad();

    return 0;
}


int MsgWindowEvent(WPARAM, LPARAM lParam)
{
    MessageWindowEventData* data = (MessageWindowEventData*) lParam;

    if (data == NULL)
        return 0;

    switch (data->uType)
    {
        // Здесь можно отлавливать только открытие окна,
        // т.к. закрытие может быть закрытием вкладки
        case MSG_WINDOW_EVT_OPEN:
            windowAdd(data->hwndWindow, false);            
            break;
    }

    return 0;
}

// end of file