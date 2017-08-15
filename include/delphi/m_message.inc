{
  Miranda IM: the free IM client for Microsoft  Windows

  Copyright 2000-2003 Miranda ICQ/IM project,
  all portions of this codebase are copyrighted to the people
  listed in contributors.txt.

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
{$IFNDEF M_MESSAGE}
{$DEFINE M_MESSAGE}
const
  {brings up the send message dialog for a contact
   wParam=(WPARAM)(HANDLE)hContact
   lParam=(LPARAM)(AnsiChar*)szText
   returns 0 on success or nonzero on failure
   returns immediately, just after the dialog is shown
   szText is the text to put in the edit box of the window (but not send)
   szText=NULL will not use any text
   szText!=NULL is only supported on v0.1.2.0+
   NB: Current versions of the convers plugin use the name
   "SRMsg/LaunchMessageWindow" instead. For compatibility you should call
   both names and the correct one will work. }
   MS_MSG_SENDMESSAGE :PAnsiChar = 'SRMsg/SendCommand';
   MS_MSG_SENDMESSAGEW:PAnsiChar = 'SRMsg/SendCommandW';

   ME_MSG_WINDOWEVENT:PAnsiChar = 'MessageAPI/WindowEvent';

   MS_MSG_CONVERS:PAnsiChar = 'SRMsg/LaunchMessageWindow';

  {wparam=0 }
  {lparam=(WPARAM)(MessageWindowEventData*)hWindowEvent; }
  {Event types }
  MSG_WINDOW_EVT_OPENING = 1; // window is about to be opened
  MSG_WINDOW_EVT_OPEN    = 2; // window has been opened
  MSG_WINDOW_EVT_CLOSING = 3; // window is about to be closed
  MSG_WINDOW_EVT_CLOSE   = 4; // window has been closed

const
  { wparam=0
    lparam=0
    Returns a dword with the current message api version
    Current version is 0,0,0,4
  }
  MS_MSG_GETWINDOWAPI:PAnsiChar = 'MessageAPI/WindowAPI';

  { wparam=(AnsiChar*)szBuf
    lparam=(int)cbSize size of buffer
    Sets the window class name in wParam (ex. "SRMM" for srmm.dll)
  }
  MS_MSG_GETWINDOWCLASS:PAnsiChar = 'MessageAPI/WindowClass';

const
  MSG_WINDOW_STATE_EXISTS  = $00000001; // Window exists should always be true if hwndWindow exists
  MSG_WINDOW_STATE_VISIBLE = $00000002;
  MSG_WINDOW_STATE_FOCUS   = $00000004;
  MSG_WINDOW_STATE_ICONIC  = $00000008;

type
  PMessageWindowData = ^TMessageWindowData;
  TMessageWindowData = record
    hwndWindow: HWND;    // top level window for the contact or NULL if no window exists
    uState    : int;     // see window states
    local     : pointer; // used to store pointer to custom data
  end;

function Srmm_GetWindowData(hContact:TMCONTACT; pResult:PMessageWindowData) : int; stdcall; external AppDll;

{ wparam=0(unused)
  lparam=(pMessageWindowEvent) event written
  fired before SRMM writes an entered message into the database
}

type
  pMessageWindowEvent = ^tMessageWindowEvent;
  tMessageWindowEvent = record
    seq      :int;          // number returned by PSS_MESSAGE
    hContact :TMCONTACT;
    dbei     :PDBEVENTINFO; // database event written on the basis of message sent
  end;

const
  ME_MSG_PRECREATEEVENT:PAnsiChar = 'MessageAPI/PreCreateEvent';

{ wParam = 0
 lParam = (MessageWindowPopupData *)&MessageWindowPopupData;
 Fired to allow plugins to add itens to the msg window popup menu
 Always fired twice: once with MSG_WINDOWPOPUP_SHOWING and once with MSG_WINDOWPOPUP_SELECTED.
 This is done to allow cleaning of resources.
}
  ME_MSG_WINDOWPOPUP:PAnsiChar = 'MessageAPI/WindowPopupRequested';

  MSG_WINDOWPOPUP_SHOWING  = 1;
  MSG_WINDOWPOPUP_SELECTED = 2;

  MSG_WINDOWPOPUP_INPUT = 1;
  MSG_WINDOWPOPUP_LOG   = 2;

type
  tMessageWindowPopupData = record
    uType    :uint;     // see popup types above
    uFlags   :uint;     // used to indicate in which window the popup was requested
    hContact :TMCONTACT;
    hwnd     :HWND;     // window where the popup was requested
    hMenu    :HMENU;    // The handle to the menu
    pt       :TPOINT;   // The point, in screen coords
    selection:int;      // The menu control id or 0 if no one was selected
  end;

// status icons
const
  MBF_DISABLED = 1;
  MBF_HIDDEN   = 2;
  MBF_UNICODE  = 4;

type
  pStatusIconData = ^tStatusIconData;
  tStatusIconData = record
    szModule     :PAnsiChar; // used in combo with the dwId below to create a unique identifier
    dwId         :dword;     // uniquely defines a button inside a module
    hIcon,
    hIconDisabled:HICON;     // hIconDisabled is optional - if null, will use hIcon in the disabled state
    flags        :int;       // one of MBF_* above
    szTooltip    :TChar;     // controlled by MBF_UNICODE
  end;

const
  MBCF_RIGHTBUTTON = 1; // if this flag is specified, the click was a right button -
                        // otherwize it was a left click

type
  pStatusIconClickData = ^tStatusIconClickData;
  tStatusIconClickData = record
    clickLocation:TPOINT;    // click location, in screen coordinates
    szModule     :PAnsiChar;
    dwId         :dword;
    flags        :int;       // one of MBCF_* above
  end;

const
{
 wParam = (HANDLE)hContact;
 lParam = (StatusIconClickData *)&StatusIconClickData;
 catch to show a popup menu, etc.
}
  ME_MSG_ICONPRESSED:PAnsiChar = 'MessageAPI/IconPressed';

{
  wParam = (HANDLE)hContact;
  lParam = (StatusIconkData*)pIcon
  catch to be notified about the icon list's change.
}
  ME_MSG_ICONSCHANGED:PAnsiChar = 'MessageAPI/IconsChanged';

{$ENDIF}
