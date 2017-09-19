#pragma once

#ifndef _ASSINGLEWINDOW_H
#define _ASSINGLEWINDOW_H

#include "stdafx.h"
#include "WindowsManager.h"

#define ASW_CLWINDOWPOS_RIGHT 0x01
#define ASW_CLWINDOWPOS_LEFT 0x02
#define ASW_CLWINDOWPOS_DISABLED 0x03

#define ASW_WINDOWS_MERGEALL 0x01
#define ASW_WINDOWS_MERGEONE 0x02
#define ASW_WINDOWS_MERGEDISABLE 0x03

//typedef std::map<HWND, sWindowInfo> windowsList;
typedef std::list<sWindowInfo> windowsList;

struct sPluginVars {
    HINSTANCE hInst;
    CRITICAL_SECTION m_CS;

    HWND contactListHWND;
    windowsList allWindows;

    HANDLE heModulesLoaded;
    HANDLE heOptionsLoaded;
    HANDLE heMsgWndEvent;

    bool IsUpdateInProgress;

    struct {
        UINT8 DrivenWindowPos;
        UINT8 WindowsMerging;
    } Options;
};

extern sPluginVars pluginVars;
extern PLUGININFOEX pluginInfo;

int OnModulesLoaded(WPARAM, LPARAM);
int MsgWindowEvent(WPARAM, LPARAM);

#endif

// end of file