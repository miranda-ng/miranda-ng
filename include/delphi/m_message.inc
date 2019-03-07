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

{$ENDIF}
