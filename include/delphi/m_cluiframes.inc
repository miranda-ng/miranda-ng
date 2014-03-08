{
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
}

{$IFNDEF M_CLUIFRAMES}
{$DEFINE M_CLUIFRAMES}

const
//  DEFAULT_TITLEBAR_HEIGHT = 18;

//this control will be the main contact list (v. 0.3.4.3+ 2004/11/02)
  CLS_SKINNEDFRAME = $0800;

//adds a frame window
//wParam=(CLISTFrame*)
//lParam=0
//returns an integer, the frame id.
type
  PCLISTFrame = ^TCLISTFrame;
  TCLISTFrame = record
    cbSize:dword;
    hWnd  :HWND;
    hIcon :HICON;
    align :int;   // al flags below
    height:int;   // or minSize - the actual meaning depends from type of frame
    Flags :int;   // F_flags below
    name  :TChar; // frame window name,will be shown in menu (DO NOT TRANSLATE)
    TBname:TChar; // titlebar caption
  // TBBackColour:COLORREF; //titlebar background colour
  end;
  PtagCLISTFrame = PCLISTFrame;
  tagCLISTFrame  = TCLISTFrame;

const
  F_VISIBLE            = 1;     // Frame visible
  F_SHOWTB             = 2;     // Show TitleBar
  F_UNCOLLAPSED        = 4;     // UnCollapse frame
  F_LOCKED             = 8;     // Lock Frame
  F_NOBORDER           = 16;    // Dont apply WS_BORDER style for window
  F_SHOWTBTIP          = 32;    // Show titlebar tooltip

  F_CANBEVERTICAL		   = 64;    // frames can be vertical
  F_CANNOTBEHORIZONTAL = 128;   // frames can NOT be horizontal	F_CANBEVERTICAL have to be set 
  F_NO_SUBCONTAINER    = 1024;  // Support skining no subcontainer needed

  F_SKINNED            = 2048;  // skinned frame (for owned subframe only)
  F_UNICODE            = 32768; // Use unicode text

// frame alignment
  alTop    = $00000001;
  alBottom = $00000002;
  alClient = $00000004; // only one alClient frame 

// since 0.7.0.20 
  alLeft   = $00000011; // frame is vertical
  alRight  = $00000012;

  alVertFrameMask = $00000010;

  FU_TBREDRAW = 1; // redraw titlebar
  FU_FMREDRAW = 2; // redraw Frame
  FU_FMPOS    = 4; // update Frame position

  FO_FLAGS       = $0001; // return set of F_VISIBLE,F_SHOWTB,F_UNCOLLAPSED,
                          // F_LOCKED,F_NOBORDER,F_SHOWTBTIP
  FO_NAME        = $0002; // Change m_cacheTName
  FO_TBNAME      = $0003; // Change TB caption
  FO_TBSTYLE     = $0004; // Change TB style
  FO_TBEXSTYLE   = $0005; // Change TB exstyle
  FO_ICON        = $0006; // Change icon
  FO_HEIGHT      = $0007; // Change height
  FO_ALIGN       = $0008; // Change align
  FO_TBTIPNAME   = $0009; // Change TB tooltip
  FO_FLOATING    = $000A; // Change floating mode

  FO_UNICODETEXT = $8000; // flag for	FO_NAME,FO_TBNAME, FO_TBTIPNAME set/get lPAram as unicode wchar_t

// want show tooltip for statusbar
// wparam=(AnsiChar *)protocolname
// lparam=0
  ME_CLIST_FRAMES_SB_SHOW_TOOLTIP:PAnsiChar = 'CListFrames/StatusBarShowToolTip';

// want hide tooltip for statusbar
// wparam=lparam=0
  ME_CLIST_FRAMES_SB_HIDE_TOOLTIP:PAnsiChar = 'CListFrames/StatusBarHideToolTip';

{
  adds a frame window
  wParam=(CLISTFrame*)
  lParam=0
  returns an integer, the frame id.
}
  MS_CLIST_FRAMES_ADDFRAME:PAnsiChar = 'CListFrames/AddFrame';

// remove frame
// wParam=Frame Id
// lParam=0
// window with hWnd field in CLISTFrame will be destroyed!!
  MS_CLIST_FRAMES_REMOVEFRAME:PAnsiChar = 'CListFrames/RemoveFrame';

//shows all frames
//wParam=lParam=0
//returns 0 on success, -1 on failure
  MS_CLIST_FRAMES_SHOWALLFRAMES:PAnsiChar = 'CListFrames/ShowALLFrames';

//shows the titlebars of all frames
//wParam=lParam=0
//returns 0 on success, -1 on failure
  MS_CLIST_FRAMES_SHOWALLFRAMESTB:PAnsiChar = 'CListFrames/ShowALLFramesTB';

//hides the titlebars of all frames
//wParam=lParam=0
//returns 0 on success, -1 on failure
  MS_CLIST_FRAMES_HIDEALLFRAMESTB:PAnsiChar = 'CListFrames/HideALLFramesTB';

{
  shows the frame if it is hidden,
  hides the frame if it is shown
  wParam=FrameId
  lParam = Frame number (can be shown in profile in CLUIFrames key)
  returns 0 on success, -1 on failure
  note that Frame number will be taken only if wParam == 0 
}
  MS_CLIST_FRAMES_SHFRAME:PAnsiChar = 'CListFrames/SHFrame';

{
  shows the frame titlebar if it is hidden,
  hides the frame titlebar if it is shown
  wParam=FrameId
  lParam = Frame number (can be shown in profile in CLUIFrames key)
  returns 0 on success, -1 on failure
  note that Frame number will be taken only if wParam == 0 
}
  MS_CLIST_FRAMES_SHFRAMETITLEBAR:PAnsiChar = 'CListFrame/SHFrameTitleBar';

{
  locks the frame if it is unlocked,
  unlock the frame if it is locked
  wParam=FrameId
  lParam = Frame number (can be shown in profile in CLUIFrames key)
  returns 0 on success, -1 on failure
  note that Frame number will be taken only if wParam == 0 
}
  MS_CLIST_FRAMES_ULFRAME:PAnsiChar = 'CListFrame/ULFrame';

{
  collapses the frame if it is uncollapsed,
  uncollapses the frame if it is collapsed
  wParam=FrameId
  lParam = Frame number (can be shown in profile in CLUIFrames key)
  returns 0 on success, -1 on failure
  note that Frame number will be taken only if wParam == 0 
}
  MS_CLIST_FRAMES_UCOLLFRAME:PAnsiChar = 'CListFrame/UCOLLFrame';

{
  trigger border flags
  wparam=frameid
  lParam = Frame number (can be shown in profile in CLUIFrames key)
  returns 0 on success, -1 on failure
  note that Frame number will be taken only if wParam == 0 
}
  MS_CLIST_FRAMES_SETUNBORDER:PAnsiChar = 'CListFrame/SetUnBorder';

//trigger skinned flags
//wparam=frameid
//lparam=0
  MS_CLIST_FRAMES_SETSKINNED:PAnsiChar = 'CListFrame/SetSkinnedFrame';

{
  redraws the frame
  wParam=FrameId, -1 for all frames
  lparam=FU_flags
  returns a pointer to option, -1 on failure
}
  MS_CLIST_FRAMES_UPDATEFRAME:PAnsiChar = 'CListFrame/UpdateFrame';

{
  gets the frame options
  (HIWORD)wParam=FrameId
  (LOWORD)wParam=FO_flag
  lParam=0
  returns a pointer to option, -1 on failure
}
  MS_CLIST_FRAMES_GETFRAMEOPTIONS:PAnsiChar = 'CListFrame/GetFrameOptions';

//sets the frame options
//(HIWORD)wParam=FrameId
//(LOWORD)wParam=FO_flag
//lParam=value
//returns 0 on success, -1 on failure
  MS_CLIST_FRAMES_SETFRAMEOPTIONS:PAnsiChar = 'CListFrame/SetFrameOptions';

//menu stuff

{
  add a new item to the context frame menu
  wParam=0
  lParam=(LPARAM)(CLISTMENUITEM*)&mi
  returns a handle to the new item
  popupposition=frameid
  contactowner=advanced parameter
}
  MS_CLIST_ADDCONTEXTFRAMEMENUITEM:PAnsiChar = 'CList/AddContextFrameMenuItem';

{
  remove a item from context frame menu
  wParam=hMenuItem returned by MS_CLIST_ADDCONTACTMENUITEM
  lParam=0
  returns 0 on success, nonzero on failure
}
  MS_CLIST_REMOVECONTEXTFRAMEMENUITEM:PAnsiChar = 'CList/RemoveContextFrameMenuItem';

{
  builds the context menu for a frame
  wparam=frameid
  lParam=0
  returns a HMENU on success, or NULL on failure
}
  MS_CLIST_MENUBUILDFRAMECONTEXT:PAnsiChar = 'CList/BuildContextFrameMenu';

{
//the frame menu is about to be built
wparam=frameid 
lparam=
-1 for build from titlebar,
    use
          MS_CLIST_ADDCONTEXTFRAMEMENUITEM 
          MS_CLIST_REMOVECONTEXTFRAMEMENUITEM

>0 for build in main menu, 
must be popupname=lparam to place your items in right popup of main menu.
    use
          MS_CLIST_ADDMAINMENUITEM
          MS_CLIST_REMOVEMAINMENUITEM

}
  ME_CLIST_PREBUILDFRAMEMENU:PAnsiChar = 'CList/PreBuildFrameMenu';

//needed by cluiframes module to add frames menu to main menu.
//it just calls NotifyEventHooks(hPreBuildFrameMenuEvent,wParam,lParam);
  MS_CLIST_FRAMEMENUNOTIFY:PAnsiChar = 'CList/ContextFrameMenuNotify';

//#define FONTID_STATUS 8
//#define FONTID_FRAMETITLE 9

//#undef FONTID_MAX
//#define FONTID_MAX 9

//  ME_CLIST_PREBUILDSTATUSMENU = 'CList/PreBuildStatusMenu';
//  MS_CLIST_ADDSTATUSMENUITEM = 'CList/AddStatusMenuItem';
{$ENDIF}
