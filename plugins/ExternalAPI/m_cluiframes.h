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

//Extra columns type.
//column arranged in this way
//
//	[statusicon] ContactName	[WEB][ADV1][ADV2][SMS][EMAIL][PROTO][CLIENT]
//
#define  EXTRA_ICON_RES0	0	// only used by nicer
#define  EXTRA_ICON_EMAIL	1
#define  EXTRA_ICON_PROTO	2	// used by mwclist and modern
#define  EXTRA_ICON_RES1	2	// only used by nicer
#define  EXTRA_ICON_SMS		3
#define  EXTRA_ICON_ADV1	4
#define  EXTRA_ICON_ADV2	5
#define  EXTRA_ICON_WEB		6
#define  EXTRA_ICON_CLIENT	7
#define  EXTRA_ICON_VISMODE	8	// only used by modern
#define  EXTRA_ICON_RES2	8	// only used by nicer
#define  EXTRA_ICON_ADV3	9
#define  EXTRA_ICON_ADV4	10

#ifndef EXTRA_ICON_COUNT
#define  EXTRA_ICON_COUNT	10	// define this inside clist-plugin depending on used icon slots
#endif

typedef struct
{
	int cbSize;			//must be sizeof(IconExtraColumn)
	int ColumnType;
	HANDLE hImage;		//return value from MS_CLIST_EXTRA_ADD_ICON
}IconExtraColumn,*pIconExtraColumn;

//Set icon for contact at needed column
//wparam=hContact
//lparam=pIconExtraColumn
//return 0 on success,-1 on failure
//
//See above for supported columns
#define MS_CLIST_EXTRA_SET_ICON			"CListFrames/SetIconForExraColumn"

//Adding icon to extra image list. 
//Call this in ME_CLIST_EXTRA_LIST_REBUILD event
//
//wparam=hIcon
//lparam=0
//return hImage on success,-1 on failure
#define MS_CLIST_EXTRA_ADD_ICON			"CListFrames/AddIconToExtraImageList"

#define ME_CLIST_EXTRA_LIST_REBUILD			"CListFrames/OnExtraListRebuild"

//called with wparam=hContact
#define ME_CLIST_EXTRA_IMAGE_APPLY			"CListFrames/OnExtraImageApply"

//called with wparam=hContact lparam=extra
#define ME_CLIST_EXTRA_CLICK				"CListFrames/OnExtraClick"

//End of extra images header. TODO move it to separate m_extraimages.h file
//Cause it has not any relationship to cluiframes engine


/************************************************************************/
/*               CLUI Frames Support				                    */
/************************************************************************/


// Constants used below
typedef struct tagCLISTFrame {
	DWORD cbSize;
	HWND hWnd ;
	HICON hIcon;
	int align;	//al flags below
	union {
		int height;
		int minSize;   //the actual meaning depends from type of frame
	};
	int Flags;	//F_flags below
	union {
		char *name; //frame window name indentifier (DO NOT TRANSLATE)
		wchar_t *wname;
		LPTSTR tname;
	};
	union {
		char *TBname; //titlebar & menu caption
		wchar_t *TBwname;
		LPTSTR TBtname;
	};
} CLISTFrame;

#define F_VISIBLE			1 //Frame visible
#define F_SHOWTB			2 //Show TitleBar
#define F_UNCOLLAPSED		4 //UnCollapse frame
#define F_LOCKED			8 //Lock Frame
#define F_NOBORDER			16 //Dont apply WS_BORDER style for window
#define F_SHOWTBTIP			32 //Show titlebar tooltip
#define F_CANBEVERTICAL		64 //frames can be vertical
#define F_CANNOTBEHORIZONTAL 128 //frames can NOT be horizontal	F_CANBEVERTICAL have to be set 
#define F_NO_SUBCONTAINER   1024   //Support skining no subcontainer needed
#define F_SKINNED           2048    // skinned frame (for owned subframe only)
#define F_UNICODE			32768 //Use unicode text
#ifdef _UNICODE
# define F_TCHAR			F_UNICODE
#else
# define F_TCHAR			0
#endif

// frame alignment
#define alTop		0x00000001
#define alBottom	0x00000002
#define alClient	0x00000004				//only one alClient frame

// since 0.7.0.20 
#define alLeft		0x00000011			   // frame is vertical
#define alRight		0x00000012

#define alVertFrameMask 0x00000010			

#define FU_TBREDRAW			1 //redraw titlebar
#define FU_FMREDRAW			2 //redraw Frame
#define FU_FMPOS			4 //update Frame position

#define FO_FLAGS		0x0001 //return set of F_VISIBLE,F_SHOWTB,F_UNCOLLAPSED,F_LOCKED,F_NOBORDER,F_SHOWTBTIP
#define FO_NAME			0x0002 //Change m_cacheTName
#define FO_TBNAME		0x0003 //Change TB caption
#define FO_TBSTYLE		0x0004 //Change TB style
#define FO_TBEXSTYLE	0x0005 //Change TB exstyle
#define FO_ICON			0x0006 //Change icon
#define FO_HEIGHT		0x0007 //Change height
#define FO_ALIGN		0x0008 //Change align
#define FO_TBTIPNAME	0x0009 //Change TB tooltip
#define FO_FLOATING		0x000a //Change floating mode

#define FO_UNICODETEXT	0x8000 // flag for	FO_NAME,FO_TBNAME, FO_TBTIPNAME set/get lPAram as unicode wchar_t
#ifdef _UNICODE
	#define FO_TCHAR FO_UNICODETEXT
#else
	#define FO_TCHAR 0x0000
#endif


//////////////////////////////////////////////////////////////////////////
//want show tooltip for statusbar
//wparam=(char *)protocolname
//lparam=0
#define ME_CLIST_FRAMES_SB_SHOW_TOOLTIP							"CListFrames/StatusBarShowToolTip"

//////////////////////////////////////////////////////////////////////////
//want hide tooltip for statusbar
//wparam=lparam=0
#define ME_CLIST_FRAMES_SB_HIDE_TOOLTIP							"CListFrames/StatusBarHideToolTip"

//////////////////////////////////////////////////////////////////////////
//adds a frame window
//wParam=(CLISTFrame*)
//lParam=0
//returns an integer, the frame id.
#define MS_CLIST_FRAMES_ADDFRAME			"CListFrames/AddFrame"

//////////////////////////////////////////////////////////////////////////
// remove frame. It destroy your window
//
#define MS_CLIST_FRAMES_REMOVEFRAME			"CListFrames/RemoveFrame"

//////////////////////////////////////////////////////////////////////////
//shows all frames
//wParam=lParam=0
//returns 0 on success, -1 on failure
#define MS_CLIST_FRAMES_SHOWALLFRAMES		"CListFrames/ShowALLFrames"

//////////////////////////////////////////////////////////////////////////
//shows the titlebars of all frames
//wParam=lParam=0
//returns 0 on success, -1 on failure
#define MS_CLIST_FRAMES_SHOWALLFRAMESTB		"CListFrames/ShowALLFramesTB"

//////////////////////////////////////////////////////////////////////////
//hides the titlebars of all frames
//wParam=lParam=0
//returns 0 on success, -1 on failure
#define MS_CLIST_FRAMES_HIDEALLFRAMESTB		"CListFrames/HideALLFramesTB"
 
//////////////////////////////////////////////////////////////////////////
//shows the frame if it is hidden,
//hides the frame if it is shown
//wParam = FrameId
//lParam = Frame number (can be shown in profile in CLUIFrames key)
//returns 0 on success, -1 on failure
//note that Frame number will be taken only if wParam == 0 
#define MS_CLIST_FRAMES_SHFRAME				"CListFrames/SHFrame"

//////////////////////////////////////////////////////////////////////////
//shows the frame titlebar if it is hidden,
//hides the frame titlebar if it is shown
//wParam=FrameId
//lParam = Frame number (can be shown in profile in CLUIFrames key)
//returns 0 on success, -1 on failure
//note that Frame number will be taken only if wParam == 0 
#define MS_CLIST_FRAMES_SHFRAMETITLEBAR		"CListFrame/SHFrameTitleBar"

//////////////////////////////////////////////////////////////////////////
//locks the frame if it is unlocked,
//unlock the frame if it is locked
//wParam=FrameId
//lParam = Frame number (can be shown in profile in CLUIFrames key)
//returns 0 on success, -1 on failure
//note that Frame number will be taken only if wParam == 0 
#define MS_CLIST_FRAMES_ULFRAME				"CListFrame/ULFrame"

//////////////////////////////////////////////////////////////////////////
//collapses the frame if it is uncollapsed,
//uncollapses the frame if it is collapsed
//wParam=FrameId
//lParam = Frame number (can be shown in profile in CLUIFrames key)
//returns 0 on success, -1 on failure
//note that Frame number will be taken only if wParam == 0 
#define MS_CLIST_FRAMES_UCOLLFRAME			"CListFrame/UCOLLFrame"

//////////////////////////////////////////////////////////////////////////
//trigger border flags
//wparam=frameid
//lParam = Frame number (can be shown in profile in CLUIFrames key)
//returns 0 on success, -1 on failure
//note that Frame number will be taken only if wParam == 0 
#define MS_CLIST_FRAMES_SETUNBORDER			"CListFrame/SetUnBorder"

//////////////////////////////////////////////////////////////////////////
//trigger skinned flags
//wparam=frameid
//lparam=0
#define MS_CLIST_FRAMES_SETSKINNED			"CListFrame/SetSkinnedFrame"

//////////////////////////////////////////////////////////////////////////
//redraws the frame
//wParam=FrameId, -1 for all frames
//lparam=FU_flags
//returns a pointer to option, -1 on failure
#define MS_CLIST_FRAMES_UPDATEFRAME			"CListFrame/UpdateFrame"

//////////////////////////////////////////////////////////////////////////
//gets the frame options
//(HIWORD)wParam=FrameId
//(LOWORD)wParam=FO_flag
//lParam=0
//returns a pointer to option, -1 on failure
#define MS_CLIST_FRAMES_GETFRAMEOPTIONS			"CListFrame/GetFrameOptions"

//sets the frame options
//(HIWORLD)wParam=FrameId
//(LOWORD)wParam=FO_flag
//lParam=value
//returns 0 on success, -1 on failure
#define MS_CLIST_FRAMES_SETFRAMEOPTIONS			"CListFrame/SetFrameOptions"

//////////////////////////////////////////////////////////////////////////
//Frames related menu stuff
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//add a new item to the context frame menu
//wParam=0
//lParam=(LPARAM)(CLISTMENUITEM*)&mi
//returns a handle to the new item
//popupposition=frameid
//contactowner=advanced parameter
#define MS_CLIST_ADDCONTEXTFRAMEMENUITEM			"CList/AddContextFrameMenuItem"

//////////////////////////////////////////////////////////////////////////
//remove a item from context frame menu
//wParam=hMenuItem returned by MS_CLIST_ADDCONTACTMENUITEM
//lParam=0
//returns 0 on success, nonzero on failure
#define MS_CLIST_REMOVECONTEXTFRAMEMENUITEM			"CList/RemoveContextFrameMenuItem"

//////////////////////////////////////////////////////////////////////////
//builds the context menu for a frame
//wparam=frameid
//lParam=0
//returns a HMENU on success, or NULL on failure
#define MS_CLIST_MENUBUILDFRAMECONTEXT				"CList/BuildContextFrameMenu"

//////////////////////////////////////////////////////////////////////////
//	the frame menu is about to be built
//		wparam=frameid 
//		lparam=
//	-1 for build from titlebar,
//		use
//			MS_CLIST_ADDCONTEXTFRAMEMENUITEM 
//			MS_CLIST_REMOVECONTEXTFRAMEMENUITEM
//
//	>0 for build in main menu, 
//		must be popupname=lparam to place your items in right popup of main menu.
//		use
//			MS_CLIST_ADDMAINMENUITEM
//			MS_CLIST_REMOVEMAINMENUITEM
//
#define ME_CLIST_PREBUILDFRAMEMENU					"CList/PreBuildFrameMenu"

//////////////////////////////////////////////////////////////////////////
//needed by cluiframes module to add frames menu to main menu.
//it just calls NotifyEventHooks(hPreBuildFrameMenuEvent,wParam,lParam);
#define MS_CLIST_FRAMEMENUNOTIFY					"CList/ContextFrameMenuNotify"

