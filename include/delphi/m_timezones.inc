{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2010 Miranda ICQ/IM project,
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

{$IFNDEF M_TIMEZONES}
{$DEFINE M_TIMEZONES}

const
  MIM_TZ_NAMELEN = 64;

  TZF_PLF_CB    = 1; // UI element is assumed to be a combo box
  TZF_PLF_LB    = 2; // UI element is assumed to be a list box
  TZF_DIFONLY   = 4;
  TZF_KNOWNONLY = 8;

const
  LOCAL_TIME_HANDLE = 0;
  UTC_TIME_HANDLE   = pointer(-1);

type
  mir_time = int_ptr;

type
  TTIME_API = record
    cbSize:size_t;

    createByName   :function (tszName:TChar; dwFlags:dword):THANDLE; cdecl;
    createByContact:function (hContact:TMCONTACT; szModule:PAnsiChar; dwFlags:dword):THANDLE;
    storeByContact :procedure(hContact:TMCONTACT; szModule:PAnsiChar; hTZ:THANDLE); cdecl;

    printDateTime:function(hTZ:THANDLE; szFormat:TChar; szDest:TChar;
          cbDest:int; dwFlags:dword):int; cdecl;
    printTimeStamp:function(hTZ:THANDLE; ts:mir_time; szFormat:TChar; szDest:TChar;
          cbDest:int; dwFlags:dword):int; cdecl;

    prepareList     :function (hContact:TMCONTACT; szModule:PAnsiChar; hWnd:HWND; dwFlags:dword):int; cdecl;
    selectListItem  :function (hContact:TMCONTACT; szModule:PAnsiChar; hWnd:HWND; dwFlags:dword):int; cdecl;
    storeListResults:procedure(hContact:TMCONTACT; szModule:PAnsiChar; hWnd:HWND; dwFlags:dword); cdecl;

    getTimeZoneTime:function(hTZ:THANDLE; var st:SYSTEMTIME):int; cdecl;
    timeStampToTimeZoneTimeStamp:function(hTZ:THANDLE; ts:mir_time):time_t; cdecl;

    getTzi          :function(hTZ:THANDLE):PTimeZoneInformation; cdecl;
    getTzName       :function(hTZ:THANDLE):TChar; cdecl;
    getTzDescription:function(TZName:TChar):TChar; cdecl;
  end;

// every protocol should declare this variable to use the Time API
//extern TIME_API tmi;

{
a service to obtain the Time API

wParam = 0;
lParam = (LPARAM)(TIME_API*).

returns TRUE if all is Ok, and FALSE otherwise
}
const
  MS_SYSTEM_GET_TMI:PAnsiChar = 'Miranda/System/GetTimeApi';

{$ENDIF}
