{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2007 Miranda ICQ/IM project, 
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

{$IFNDEF M_HOTKEYS}
{$DEFINE M_HOTKEYS}

const
  HKD_UNICODE = $0001;
  HKF_MIRANDA_LOCAL = $8000;

type
  PHOTKEYDESC = ^THOTKEYDESC;
  THOTKEYDESC = record
    pszName       :PAnsiChar;  // name to refer to hotkey when playing and in db
    pszDescription:TChar;      // description for options dialog
    pszSection    :TChar;      // section name used to group sounds (NULL is acceptable)
    pszService    :PAnsiChar;  // Service to call when HotKey Pressed
    DefHotKey     :word;       // default hot key for action
    lParam        :LPARAM;     // lParam to pass to service
    dwFlags       :dword;      // Miranda ver >=9.0
  end;


function _Hotkey_Register(hk:PHOTKEYDESC; _hLang:int): int; stdcall; external AppDll name 'Hotkey_Register';
function Hotkey_Unregister(hk:PAnsiChar): int; stdcall; external AppDll;

{
  This event is fired when hotkeys were changed (Mirver 9.0+ ?)
  wParam = lParam = 0
}

const
  ME_HOTKEYS_CHANGED:PAnsiChar = 'CoreHotkeys/Changed';

{$ENDIF}
