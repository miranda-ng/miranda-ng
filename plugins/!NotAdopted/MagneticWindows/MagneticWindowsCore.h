#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "../include/newpluginapi.h"
#include "../include/m_clist.h"
#include "../include/m_clui.h"
#include "../include/m_message.h"
#include "../include/m_system.h"
#include "../include/m_options.h"
#include "../include/m_plugins.h"
#include "../include/m_database.h"
#include "../include/m_langpack.h"
#include "m_MagneticWindows.h"
#include "SnapToListService.h"
#include "Options.h"
#include "resource.h"


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



extern HINSTANCE hInst;
extern char ModuleName [];

bool WindowOpen(HWND);
bool WindowClose(HWND);
bool WindowCloseAll();