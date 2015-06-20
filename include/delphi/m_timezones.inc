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

function TimeZone_ToLocal(timestamp:DWORD) : DWORD; stdcall;
              external CoreDll name 'TimeZone_ToLocal';

function TimeZone_ToString(timeVal:mir_time; const szFormat:PAnsiChar; szDest:PAnsiChar; cchDest:size_t) : mir_time; stdcall;
              external CoreDll name 'TimeZone_ToString';

function TimeZone_ToStringW(timeVal:mir_time; const szFormat:PWideChar; szDest:PWideChar; cchDest:size_t) : mir_time; stdcall;
              external CoreDll name 'TimeZone_ToStringW';

{$ENDIF}
