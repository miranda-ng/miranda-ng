#include <windows.h>
#include <commctrl.h>
#include <stdio.h>

#include <newpluginapi.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_message.h>
#include <m_system.h>
#include <m_options.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_MagneticWindows.h>

#include "SnapToListService.h"
#include "Options.h"

#include "resource.h"
#include "Version.h"

typedef 
	struct TDockingWindow {
		HWND hWnd;
		WNDPROC OldWindowProc;
		TDockingWindow* Next;
	} TDockingWindow, *PDockingWindow;
typedef 
	struct TRectList {
		RECT Rect;
		TRectList* Next;
	} TRectList, *PRectList;
typedef
	struct {
		PDockingWindow WindowList;
		PRectList Rects;		
		int MouseX, MouseY;
		bool SnappedX, SnappedY;
	} TWorkingVariables;



#define MODULE_NAME "MagneticWindows"
extern HINSTANCE hInst;

bool WindowOpen(HWND);
bool WindowClose(HWND);
bool WindowCloseAll();