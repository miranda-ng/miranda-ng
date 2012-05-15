/*
Miranda ICQ: the free icq client for MS Windows 
Copyright (C) 2000-2  Richard Hughes, Roland Rabien & Tristan Van de Vreede

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#pragma once

#ifndef _CLUIFRAMES_H_
#define _CLUIFRAMES_H_

#include "../m_api/m_skin_eng.h"

int LoadCLUIFramesModule(void);
int UnLoadCLUIFramesModule(void);
int CLUIFramesGetMinHeight();
int CLUIFramesOnClistResize(WPARAM wParam,LPARAM lParam);
int CLUIFrames_GetTotalHeight();

typedef struct tagProtocolData {
	char *RealName;
	int protopos;
	boolean show;
} ProtocolData;

typedef struct 
{
	int order;
	int realpos;
}SortData;




//============
#define CLUIFRAMESSETALIGN			"CLUIFramesSetAlign"

#define CLUIFRAMESSETALIGNALTOP		"CLUIFramesSetAlignalTop"
#define CLUIFRAMESSETALIGNALCLIENT	"CLUIFramesSetAlignalClient"
#define CLUIFRAMESSETALIGNALBOTTOM	"CLUIFramesSetAlignalBottom"
#define CLUIFRAMESSETFLOATING		"Set_Floating"

#define CLUIFRAMESMOVEUPDOWN		"CLUIFramesMoveUpDown"
#define CLUIFRAMESMOVEUP			"CLUIFramesMoveUp"
#define CLUIFRAMESMOVEDOWN			"CLUIFramesMoveDown"

typedef struct tagMenuHandles
{
	HANDLE MainMenuItem;
	HANDLE MIVisible,MITitle,MITBVisible,MILock,MIColl,MIFloating,MIAlignRoot;
	HANDLE MIAlignTop,MIAlignClient,MIAlignBottom;
	HANDLE MIBorder;
	HANDLE MIPosRoot;
    HANDLE MIPosUp,MIPosDown;
} FrameMenuHandles;

typedef struct tagFrameTitleBar{
	HWND hwnd;
	HWND TitleBarbutt;
	HWND hwndTip;

	TCHAR* tbname;
	TCHAR* tooltip;

	char * sztbname;
	char * sztooltip;

	HMENU hmenu;
	HICON hicon;

	BOOLEAN ShowTitleBar;
	BOOLEAN ShowTitleBarTip;
	COLORREF BackColour;
	COLORREF TextColour;
	int oldstyles;
	POINT oldpos;
	RECT wndSize;
} FrameTitleBar;

typedef struct _DockOpt
{
	HWND	hwndLeft;
	HWND	hwndRight;
}
DockOpt;
typedef struct _tagFrameWnd{
	int id;
	HWND hWnd ;
	RECT wndSize;
	TCHAR * Name;
	char  * szName;
	int align;
	int height;
	int dwFlags;
	BOOLEAN Locked;
	BOOLEAN visible;
	BOOLEAN needhide;
	BOOLEAN collapsed;
	int prevvisframe;
	int HeightWhenCollapsed;
	FrameTitleBar TitleBar;
	FrameMenuHandles MenuHandles;
	int oldstyles;
	BOOLEAN floating;
	HWND ContainerWnd;
	POINT FloatingPos;
	POINT FloatingSize;
	BOOLEAN minmaxenabled;
	BOOLEAN UseBorder;
	int order;
	DockOpt dockOpt;
    HWND OwnerWindow;
    tPaintCallbackProc PaintCallbackProc;
    sPaintRequest * PaintData;
    BOOLEAN bQueued;
    HRGN UpdateRgn;

} FRAMEWND;

#define OFFSET_PROTOPOS 200
#define OFFSET_VISIBLE 400

#define CLUIFrameSubContainerClassName			_T("CLUIFrameSubContainer")
#define CLUIFrameTitleBarClassName				_T("CLUIFrameTitleBar")
#define CLUIFrameModule							"CLUIFrames"

//integrated menu module
#define MS_INT_MENUMEASUREITEM "CLUIFrames/IntMenuMeasureItem"
#define MS_INT_MENUDRAWITEM "CLUIFrames/IntMenuDrawItem"
#define MS_INT_MENUPROCESSCOMMAND "CLUIFrames/IntMenuProcessCommand"
#define MS_INT_MODIFYMENUITEM "CLUIFrames/IntModifyMenuItem"

#endif




