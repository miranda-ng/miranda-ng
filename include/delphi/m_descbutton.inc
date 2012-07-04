{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2007 Artem Shpynov
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

{$IFNDEF M_DESCBUTTON}
{$DEFINE M_DESCBUTTON}

const
  MIRANDADESCBUTTONCLASS = 'MirandaDescButtonClass';

  MDBCF_SHAREDICON = $01;
  MDBCF_UNICODE    = $02;

  DBCM_FIRST = (WM_USER+1024);
  DBCM_LAST  = (DBCM_FIRST+64);

  DBCM_SETTITLE       = (DBCM_FIRST+1); //WPARAM is set of flags, LPARAM is string pointer
  DBCM_SETDESCRIPTION = (DBCM_FIRST+2); //WPARAM is set of flags, LPARAM is string pointer
  DBCM_SETICON        = (DBCM_FIRST+3); //WPARAM is set of flags, LPARAM icon handle

{
#define MDescButton_SetTitle(hwnd, title)	\
	(SendMessage((hwnd), DBCM_SETTITLE, MDBCF_TCHAR, (LPARAM)(title)))

#define MDescButton_SetDescription(hwnd, title)	\
	(SendMessage((hwnd), DBCM_SETDESCRIPTION, MDBCF_TCHAR, (LPARAM)(title)))

#define MDescButton_SetIcon(hwnd, hicon)	\
	(SendMessage((hwnd), DBCM_SETICON, 0, (LPARAM)(hicon)))

#define MDescButton_SetIconShared(hwnd, hicon)	\
	(SendMessage((hwnd), DBCM_SETICON, MDBCF_SHAREDICON, (LPARAM)(hicon)))
}
{$ENDIF}
