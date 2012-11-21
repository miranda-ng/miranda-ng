{
Miranda NG: the free IM client for Microsoft* Windows*

Copyright 2012 Miranda NG project,
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


{$IFNDEF M_XSTATUS}
{$DEFINE M_XSTATUS}

// Custom Status info

const
  CSSF_MASK_STATUS    = $0001;  // status member valid for set/get
  CSSF_MASK_NAME      = $0002;  // pszName member valid for set/get
  CSSF_MASK_MESSAGE   = $0004;  // pszMessage member valid for set/get
  CSSF_DISABLE_MENU   = $0020;  // disable default custom status menu, wParam = bEnable
  CSSF_DISABLE_UI     = $0040;  // disable default custom status UI, wParam = bEnable
  CSSF_DEFAULT_NAME   = $0080;  // only with CSSF_MASK_NAME and get API to get default custom status name (wParam = status)
  CSSF_STATUSES_COUNT = $0100;  // returns number of custom statuses in wParam, only get API
  CSSF_STR_SIZES      = $0200;  // returns sizes of custom status name & message (wParam & lParam members) in chars
  CSSF_UNICODE        = $1000;  // strings are in UCS-2

type
  TCUSTOM_STATUS = record
    cbSize   :int;        // size of the structure
    flags    :int;        // combination of CSSF_*
    status   :^int;       // custom status id
    szName   :TChar;
    szMessage:TChar;
    wParam   :^WPARAM;    // extra params, see flags
    lParam   :^LPARAM;
  end;

const
{
  Retrieves custom status details for specified hContact
  wParam = (HANDLE)hContact
  lParam = (CUSTOM_STATUS*)pData  // receives details (members must be prepared)
  return = 0 (for success)
}
  PS_GETCUSTOMSTATUSEX = '/GetXStatusEx';

{
  Sets owner current custom status
  wParam = 0                          // reserved
  lParam = (CUSTOM_STATUS*)pData  // contains what to set and new values
  return = 0 (for success)
}
  PS_SETCUSTOMSTATUSEX = '/SetXStatusEx';

  LR_BIGICON = $40;

{
  Retrieves specified custom status icon
  wParam = (int)N  // custom status id (1-32), 0 = my current custom status
  lParam = flags   // use LR_SHARED for shared HICON, LR_BIGICON for 32x32 icon
  return = HICON   // custom status icon (use DestroyIcon to release resources if not LR_SHARED)
}
  PS_GETCUSTOMSTATUSICON = '/GetXStatusIcon';

{
  Called from contact list in order to get index of custom status icon in list
  wParam = hContact
  lParam = 0
  return = (int)index of extra contact icon shifted <<16 (the low word will be normal status icon, the high will be xStatus Icon
}
  PS_GETADVANCEDSTATUSICON = '/GetAdvancedStatusIcon';

{$ENDIF}
