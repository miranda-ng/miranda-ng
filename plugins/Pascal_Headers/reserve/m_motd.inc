{
Copyright (C) 2005 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
}

{$IFNDEF M_MOTD}
{$DEFINE M_MOTD}

//=== SERVICES ===

{
MOTD/GetMessage service
Get a MOTD

wparam = (MOTDMessage*)lpMOTDMessage
lparam = 0
returns: 0 on success, -1 on failure. On success, lpMOTDMessage is filled with the MOTD
}
type
  PMOTDMessage = ^TMOTDMessage;
  TMOTDMessage = record
	  cbSize       :int;   // sizeof(MOTDMessage)
	  pszMessage   :PAnsiChar; // Buffer to store message
	  cbMessageSize:dword; // Size of message buffer (including '\0')
	  pszAuthor    :PAnsiChar; // Buffer to store author
	  cbAuthorSize :dword; // Size of author buffer (including '\0')
  end;
const
  MS_MOTD_GETMESSAGE = 'MOTD/GetMessage';

{
MOTD/AddMessage service
Add a message to the MOTD list

wparam = (MOTDMessage*)lpMOTDMessage
lparam = 0
returns: 0 on success, -1 on failure
}
  MS_MOTD_ADDMESSAGE = 'MOTD/AddMessage';

{
MOTD/ShowMessage service
Show a popup with a MOTD

wparam = 0
lparam = 0
returns: 0
}
  MS_MOTD_SHOWMESSAGE ='MOTD/ShowMessage';

{
MOTD/ShowAddMessage service
Show a dialog to add a MOTD

wparam = 0
lparam = 0
returns: 0
}
  MS_MOTD_SHOWADDMESSAGE = 'MOTD/ShowAddMessage';

{$ENDIF}
