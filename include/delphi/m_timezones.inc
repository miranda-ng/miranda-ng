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
  TIME_API = record
    cbSize:size_t;

    createByName   :function (tszName:TChar; dwFlags:dword):THANDLE; cdecl;
    createByContact:function (hContact:HCONTACT; dwFlags:dword):THANDLE;
    storeByContact :procedure(hContact:HCONTACT; hTZ:THANDLE); cdecl;

    printDateTime:function(hTZ:THANDLE; szFormat:TChar; szDest:TChar;
          cbDest:int; dwFlags:dword):int; cdecl;
    printTimeStamp:function(hTZ:THANDLE; ts:mir_time; szFormat:TChar; szDest:TChar;
          cbDest:int; dwFlags:dword):int; cdecl;

    prepareList     :function (hContact:HCONTACT; hWnd:HWND; dwFlags:dword):int; cdecl;
    selectListItem  :function (hContact:HCONTACT; hWnd:HWND; dwFlags:dword):int; cdecl;
    storeListResults:procedure(hContact:HCONTACT; hWnd:HWND; dwFlags:dword); cdecl;

    getTimeZoneTime:function(hTZ:THANDLE; var st:SYSTEMTIME):int; cdecl;
    timeStampToTimeZoneTimeStamp:function(hTZ:THANDLE; ts:mir_time):time_t; cdecl;

    getTzi          :function(hTZ:THANDLE):PTimeZoneInformation; cdecl;
    getTzName       :function(hTZ:THANDLE):TChar; cdecl;
    getTzDescription:function(TZName:TChar):TChar; cdecl;
(*
	int printDateTimeByContact (HCONTACT hContact, LPCTSTR szFormat, LPTSTR szDest, int cbDest, DWORD dwFlags)
	{ return printDateTime(createByContact(hContact, dwFlags), szFormat, szDest, cbDest, dwFlags); }

	int printTimeStampByContact(HCONTACT hContact, mir_time ts, LPCTSTR szFormat, LPTSTR szDest, int cbDest, DWORD dwFlags)
	{ return printTimeStamp(createByContact(hContact, dwFlags), ts, szFormat, szDest, cbDest, dwFlags); }

	LPTIME_ZONE_INFORMATION getTziByContact(HCONTACT hContact)
	{ return getTzi(createByContact(hContact, 0)); }

	int getTimeZoneTimeByContact(HCONTACT hContact, SYSTEMTIME *st)
	{ return getTimeZoneTime(createByContact(hContact, 0), st); }

	mir_time timeStampToTimeZoneTimeStampByContact(HCONTACT hContact, mir_time ts)
	{ return timeStampToTimeZoneTimeStamp(createByContact(hContact, 0), ts); }
*)
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
