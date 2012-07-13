(*
    History++ plugin for Miranda IM: the free IM client for Microsoft* Windows*

    Copyright (C) 2006-2009 theMIROn, 2003-2006 Art Fedorov.
    History+ parts (C) 2001 Christian Kastner

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*)

{-----------------------------------------------------------------------------
 hpp_messages (historypp project)

 Version:   1.0
 Created:   31.03.2003
 Author:    Oxygen

 [ Description ]

 Some helper utilities to process messages

 [ History ]
 1.0 (31.03.2003) - Initial version

 [ Modifications ]

 [ Knows Inssues ]
 None

 Contributors: theMIROn, Art Fedorov, Christian Kastner
-----------------------------------------------------------------------------}


unit hpp_messages;

interface

uses
  Windows, m_api;

function SendMessageTo(hContact: THandle; const Text: String = ''): Boolean;
//function ForwardMessage(Text: AnsiString): Boolean;

implementation

function SendMessageTo(hContact: THandle; const Text: String): Boolean;
var
  buff: AnsiString;
begin
  if boolean(ServiceExists(MS_MSG_SENDMESSAGEW)) then
    Result := (CallService(MS_MSG_SENDMESSAGEW,WPARAM(hContact),LPARAM(PChar(Text))) = 0)
  else
  begin
    buff := AnsiString(Text);
    Result := (CallService(MS_MSG_SENDMESSAGE,WPARAM(hContact),LPARAM(PAnsiChar(buff))) = 0);
    if not Result then
      Result := (CallService('SRMsg/LaunchMessageWindow',WPARAM(hContact),LPARAM(PAnsiChar(buff))) = 0);
  end;
end;

{function ForwardMessage(Text: AnsiString): Boolean;
begin
  Result := (CallService(MS_MSG_FORWARDMESSAGE,0,LPARAM(PAnsiChar(Text)))=0);
end;}

end.
