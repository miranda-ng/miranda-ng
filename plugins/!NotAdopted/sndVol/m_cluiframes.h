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

//#include "../../core/commonheaders.h"
//#pragma hdrstop

//adds a frame window
//wParam=(CLISTFrame*)
//lParam=0
//returns an integer, the frame id.
typedef struct tagCLISTFrame {
	DWORD cbSize;
	HWND hWnd ;
	HICON hIcon;
	int align; //al flags below
	int height;
	int Flags; //F_flags below
	char *name; //frame window name,will be shown in menu
	char *TBname; //titlebar caption
	//COLORREF TBBackColour; //titlebar background colour
} CLISTFrame;
#define F_VISIBLE			1 //Frame visible
#define F_SHOWTB			2 //Show TitleBar
#define F_UNCOLLAPSED		4 //UnCollapse frame
#define F_LOCKED			8 //Lock Frame
#define F_NOBORDER			16 //Dont apply WS_BORDER style for window
#define F_SHOWTBTIP			32 //Show titlebar tooltip


// frame alignment
#define alTop		0x00000001
#define alBottom	0x00000002
#define alClient	0x00000004 //only one alClient frame 
#define MS_CLIST_FRAMES_ADDFRAME			"CListFrames/AddFrame"

#define MS_CLIST_FRAMES_REMOVEFRAME			"CListFrames/RemoveFrame"

//shows all frames
//wParam=lParam=0
//returns 0 on success, -1 on failure
#define MS_CLIST_FRAMES_SHOWALLFRAMES		"CListFrames/ShowALLFrames"

//shows the titlebars of all frames
//wParam=lParam=0
//returns 0 on success, -1 on failure
#define MS_CLIST_FRAMES_SHOWALLFRAMESTB		"CListFrames/ShowALLFramesTB"

//hides the titlebars of all frames
//wParam=lParam=0
//returns 0 on success, -1 on failure
#define MS_CLIST_FRAMES_HIDEALLFRAMESTB		"CListFrames/HideALLFramesTB"

//shows the frame if it is hidden,
//hides the frame if it is shown
//wParam=FrameId
//lParam=0
//returns 0 on success, -1 on failure
#define MS_CLIST_FRAMES_SHFRAME				"CListFrames/SHFrame"

//shows the frame titlebar if it is hidden,
//hides the frame titlebar if it is shown
//wParam=FrameId
//lParam=0
//returns 0 on success, -1 on failure
#define MS_CLIST_FRAMES_SHFRAMETITLEBAR		"CListFrame/SHFrameTitleBar"

//locks the frame if it is unlocked,
//unlock the frame if it is locked
//wParam=FrameId
//lParam=0
//returns 0 on success, -1 on failure
#define MS_CLIST_FRAMES_ULFRAME				"CListFrame/ULFrame"

//collapses the frame if it is uncollapsed,
//uncollapses the frame if it is collapsed
//wParam=FrameId
//lParam=0
//returns 0 on success, -1 on failure
#define MS_CLIST_FRAMES_UCOLLFRAME			"CListFrame/UCOLLFrame"

//redraws the frame
//wParam=FrameId, -1 for all frames
//lparam=FU_flags
//returns a pointer to option, -1 on failure
#define FU_TBREDRAW			1 //redraw titlebar
#define FU_FMREDRAW			2 //redraw Frame
#define FU_FMPOS			4 //update Frame position
#define MS_CLIST_FRAMES_UPDATEFRAME			"CListFrame/UpdateFrame"

//gets the frame options
//(HIWORLD)wParam=FrameId
//(LOWORD)wParam=FO_flag
//lParam=0
//returns a pointer to option, -1 on failure
#define FO_FLAGS		0x0001 //return set of F_VISIBLE,F_SHOWTB,F_UNCOLLAPSED,F_LOCKED,F_NOBORDER,F_SHOWTBTIP
#define FO_NAME			0x0002 //Change name
#define FO_TBNAME		0x0003 //Change TB caption
#define FO_TBSTYLE		0x0004 //Change TB style
#define FO_TBEXSTYLE	0x0005 //Change TB exstyle
#define FO_ICON			0x0006 //Change icon
#define FO_HEIGHT		0x0007 //Change height
#define FO_ALIGN		0x0008 //Change align
#define FO_TBTIPNAME	0x0009 //Change TB tooltip

#define MS_CLIST_FRAMES_GETFRAMEOPTIONS			"CListFrame/GetFrameOptions"

//sets the frame options
//(HIWORLD)wParam=FrameId
//(LOWORD)wParam=FO_flag
//lParam=value
//returns 0 on success, -1 on failure
#define MS_CLIST_FRAMES_SETFRAMEOPTIONS			"CListFrame/SetFrameOptions"
