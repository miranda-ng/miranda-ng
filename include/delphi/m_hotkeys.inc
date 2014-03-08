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
type
  PHOTKEYDESC = ^THOTKEYDESC;
  THOTKEYDESC = record
    cbSize        :int;
    pszName       :PAnsiChar;  // name to refer to hotkey when playing and in db
    pszDescription:TChar;      // description for options dialog
    pszSection    :TChar;      // section name used to group sounds (NULL is acceptable)
    pszService    :PAnsiChar;  // Service to call when HotKey Pressed
    DefHotKey     :word;       // default hot key for action
    lParam        :LPARAM;     // lParam to pass to service
    dwFlags       :dword;      // Miranda ver >=9.0
  end;

const
  HKF_MIRANDA_LOCAL = $8000;

{ use this macro to defile hotkey codes like this:
  hkd.DefHotkey = HOTKEYCODE(HOTKEYF_SHIFT|HOTKEYF_EXT, 'A');
}
//  HOTKEYCODE(mod,vk)      (MAKEWORD((vk),(mod)))

{ CoreHotkeys/Register service
Registers new hotkey
  wParam=(WPARAM)0
  lParam=(LPARAM)(HOTKEYDESC *)hotkey
Returns 0 on failure or hotkey atom id on success
}
  MS_HOTKEY_REGISTER:PAnsiChar = 'CoreHotkeys/Register';

{ CoreHotkeys/Unregister service
Unregister existing hotkey
  wParam=(WPARAM)0
  lParam=(LPARAM)(AnsiChar *)pszName
Returns 0 on success or nonzero otherwise
}
  MS_HOTKEY_UNREGISTER:PAnsiChar = 'CoreHotkeys/Unregister';

{ CoreHotkeys/Check service
Checks if "manual" hotkey was activated and returns its id.
  wParam=(WPARAM)(MSG *)message
  lParam=(LPARAM)(AnsiChar *)pszSection
Returns lParam associated with activated hotkey
}
  MS_HOTKEY_CHECK:PAnsiChar = 'CoreHotkeys/Check';

{ Subclass/unsubclass edit box to act as hotkey control
  wParam=(WPARAM)(HWND)hwndEdit
  lParam=(LPARAM)0
Returns zero on success

You will get notification with LOWORD(wParam) == 0 when users sets hotkey.

Subclassed control processes HKM_SETHOTKEY and HKM_GETHOTKEY similarly to
windows' hotkey, in all other it acts just like normal editbox (beware of
standart notifications that occur on text updates!)

Subclass procedure uses GWL_USERDATA to store internal information. Do not
use it for own purposes.
}
  MS_HOTKEY_SUBCLASS  :PAnsiChar = 'CoreHotkeys/Subclass';
  MS_HOTKEY_UNSUBCLASS:PAnsiChar = 'CoreHotkeys/Unsubclass';

{
 This event is fired when hotkeys were changed (Mirver 9.0+ ?)
   wParam = lParam = 0
}

  ME_HOTKEYS_CHANGED:PAnsiChar = 'CoreHotkeys/Changed';

{$ENDIF}
