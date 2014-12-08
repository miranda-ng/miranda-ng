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

{$IFNDEF M_MYDETAILS}
{$DEFINE M_MYDETAILS}

const

{
  MyDetails/SetMyNickname service
  Set the nickname for all possible protocols

  wparam = (const AnsiChar *) protocol name or NULL for all protocols
  lparam = (const AnsiChar *) new nickname
  returns: -2 if proto can't set this, -1 on protocol not found, else 0
}
  MS_MYDETAILS_SETMYNICKNAME:PAnsiChar = 'MyDetails/SetMyNickname';

{
  MyDetails/SetMyNicknameUI service
  Shows a dialog to set the nickname for all possible protocols

  wparam = 0
  lparam = (const AnsiChar *) protocol name or NULL for all protocols
  returns: -2 if proto can't set this, -1 on protocol not found, else 0 
}
  MS_MYDETAILS_SETMYNICKNAMEUI:PAnsiChar = 'MyDetails/SetMyNicknameUI';

{
  MyDetails/SetMyAvatar service
  Set the avatar for all possible protocols

  wparam = (const AnsiChar *) protocol name or NULL for all protocols
  lparam = (const AnsiChar *) new avatar file name
  returns: -2 if proto can't set this, -1 on protocol not found, else 0 
}
  MS_MYDETAILS_SETMYAVATAR:PAnsiChar = 'MyDetails/SetMyAvatar';

{
  MyDetails/SetMyAvatarUI service
  Shows a dialog to set the avatar for all possible protocols

  wparam = 0
  lparam = (const AnsiChar *) protocol name or NULL for all protocols
  returns: -2 if proto can't set this, -1 on protocol not found, else 0 
}
  MS_MYDETAILS_SETMYAVATARUI:PAnsiChar = 'MyDetails/SetMyAvatarUI';

{
  MyDetails/GetMyNickname service
  Get the nickname

  wparam = (const AnsiChar *) protocol name or NULL for default nick
  lparam = (AnsiChar *) the buffer to save the nickname. Has to have at least 1024 chars
  returns: -1 on protocol not found, else 0 
}
  MS_MYDETAILS_GETMYNICKNAME:PAnsiChar = 'MyDetails/GetMyNickname';
  MS_MYDETAILS_GETMYNICKNAME_BUFFER_SIZE = 1024;

{
  MyDetails/GetMyAvatar service
  Get the avatar file name

  wparam = (const AnsiChar *) protocol name or NULL for default avatar
  lparam = (AnsiChar *) the buffer to save the file name. Has to have at least 1024 chars
  returns: -2 if proto can't get this, -1 on protocol not found, else 0 
}
  MS_MYDETAILS_GETMYAVATAR:PAnsiChar = 'MyDetails/GetMyAvatar';
  MS_MYDETAILS_GETMYAVATAR_BUFFER_SIZE = 1024;

{
  MyDetails/SetMyStatusMessageUI service
  Shows a dialog to set the status message for all possible protocols
  Today only works if NAS is installed.

  wparam = 0
  lparam = (const AnsiChar *) protocol name or NULL for all protocols
  returns: -2 if proto can't set this, -1 on protocol not found, else 0 
}
  MS_MYDETAILS_SETMYSTATUSMESSAGEUI:PAnsiChar = 'MyDetails/SetMyStatusMessageUI';
  MS_MYDETAILS_GETMYSTATUSMESSAGE_BUFFER_SIZE = 1024;

{
  MyDetails/ShowNextProtocol service
  Shows the next protocol in the frame

  wparam = 0
  lparam = 0
  returns: -1 on error, 0 on success
}
  MS_MYDETAILS_SHOWNEXTPROTOCOL:PAnsiChar = 'MyDetails/ShowNextProtocol';

{
  MyDetails/ShowPreviousProtocol service
  Shows the previous protocol in the frame

  wparam = 0
  lparam = 0
  returns: -1 on error, 0 on success
}
  MS_MYDETAILS_SHOWPREVIOUSPROTOCOL:PAnsiChar = 'MyDetails/ShowPreviousProtocol';

{
  MyDetails/ShowProtocol service
  Shows a protocol given its name in the frame

  wparam = 0
  lparam = protocol name
  returns: -1 on error, 0 on success
}
  MS_MYDETAILS_SHOWPROTOCOL:PAnsiChar = 'MyDetails/ShowProtocol';

{
  MyDetails/CicleThroughtProtocols service
  Start/stops the cicling throught protocols

  wparam = FALSE to stop, TRUE to start
  lparam = 0
  returns: -1 on error, 0 on success
}
  MS_MYDETAILS_CICLE_THROUGHT_PROTOCOLS:PAnsiChar = 'MyDetails/CicleThroughtProtocols';


// Since ver.1.9

{
MyDetails/ShowFrame service
Shows the MyDetails frame/window if it is hidden

wparam = 0
lparam = 0
returns: 0
}
  MS_MYDETAILS_SHOWFRAME:PAnsiChar = 'MyDetails/ShowFrame';


{
MyDetails/HideFrame service
Hides the MyDetails frame/window if it is shown

wparam = 0
lparam = 0
returns: 0
}
  MS_MYDETAILS_HIDEFRAME:PAnsiChar = 'MyDetails/HideFrame';


{
MyDetails/ShowHideMyDetails service
Shows the MyDetails frame/window if it is hidden or hides the MyDetails frame/window if it is shown

wparam = 0
lparam = 0
returns: 0
}
  MS_MYDETAILS_SHOWHIDEFRAME:PAnsiChar = 'MyDetails/ShowHideMyDetails';

{$ENDIF}
