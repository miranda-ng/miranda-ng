{
Miranda IM: the free IM client for Microsoft* Windows*

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

{$IFNDEF M_ADDCONTACT}
{$DEFINE M_ADDCONTACT}

const
  HANDLE_SEARCHRESULT = 0;
  HANDLE_EVENT        = 1;
  HANDLE_CONTACT      = 2;

type
  PADDCONTACTSTRUCT = ^TADDCONTACTSTRUCT;
  TADDCONTACTSTRUCT = record
    handleType: int;
    handle    : HDBEVENT;   // HDBEVENT, SearchResult
    szProto   : PAnsiChar; // used by search result only
    psr       : pointer;   // @PROTOSEARCHRESULT
  end;

const
  {
    wParam : (HWND) Parent window of the dialog that will be presented
    lParam : Pointer to an initialised TADDCONTACTSTRUCT
    Affect : Open's the add contact dialog
    Version: 0.1.2.2+
  }
  MS_ADDCONTACT_SHOW:PAnsiChar = 'AddContact/Show';

{$ENDIF}
