{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
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

{$IFNDEF M_SSL}
{$DEFINE M_SSL}

type
  HSSL = THANDLE;

type
  SSL_API = record
	  cbSize:int;
    connect  : function(s:dword{TSOCKET}; const host:PAnsiChar; verify:int):HSSL; cdecl;
    pending  : function(ssl:HSSL):BOOL; cdecl;
    read     : function(ssl:HSSL; buf:pAnsiChar; num:int; peek:int):int; cdecl;
    write    : function(ssl:HSSL; const buf:pAnsiChar; num:int):int; cdecl;
    shutdown : procedure(ssl:HSSL); cdecl;
    sfree    : procedure(ssl:HSSL); cdecl;
	end;

// every protocol should declare this variable to use the SSL API
// extern SSL_API si;

{
a service to obtain the SSL API 

wParam = 0;
lParam = (LPARAM)(SSL_API*).

returns TRUE if all is Ok, and FALSE otherwise
}
const
  MS_SYSTEM_GET_SI:PAnsiChar = 'Miranda/System/GetSslApi';

{$ENDIF}
