#pragma once

#include "stdafx.h"

enum WindowState
{
	WINDOW_STATE_NORMAL,
	WINDOW_STATE_MINIMIZED,
	WINDOW_STATE_MAXIMIZED,
	WINDOW_STATE_HIDDEN,
	WINDOW_STATE_CLOSED, // not used ?
};

enum eWindowPosition
{
	WINDOW_POSITION_LEFT = 1,
	WINDOW_POSITION_RIGHT = 2,
};

struct sWndCoords
{
	LONG x, y, width, height;
};

struct sWindowInfo
{
	HWND hWnd;
	WindowState eState;
	RECT rLastSavedPosition;

	void saveState();
	void saveRect();
};

// critical section tools
void pluginSetProgress();
void pluginSetDone();
bool pluginIsAlreadyRunning();

// system
sWindowInfo* windowFind(HWND);
void windowAdd(HWND, bool);
HWND windowGetRoot(HWND);
void windowListUpdate();
void windowReposition(HWND);

// tools
bool calcNewWindowPosition(HWND, HWND, sWndCoords*, eWindowPosition);
//LONG calcNewWindowPosition(HWND, HWND, RECT*, eWindowPosition);

// window callbacks
LRESULT CALLBACK wndProcSync(HWND, UINT, WPARAM, LPARAM);
void allWindowsMoveAndSize(HWND);
void allWindowsActivation(HWND);
