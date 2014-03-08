{
Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2006 Miranda ICQ/IM project,
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

//this module was created in v0.6.0.0

{$IFNDEF M_PROTO_LISTENINGTO}
{$DEFINE M_PROTO_LISTENINGTO}

// Protocol Services /////////////////////////////////////////////////////////////////

// This is the services a protocol have to support to support listening info

type
  PLISTENINGTOINFO = ^TLISTENINGTOINFO;
  TLISTENINGTOINFO = record
    cbSize  :int;
    szType  :TCHAR; // Media type: Music, Video, etc...
    szArtist:TCHAR; // Artist name
    szAlbum :TCHAR; // Album name
    szTitle :TCHAR; // Song name
    szTrack :TCHAR; // Track number
    szYear  :TCHAR; // Song year
    szGenre :TCHAR; // Song genre
    szLength:TCHAR; // Song length
    szPlayer:TCHAR; // Player name
    dwFlags :dword;
  end;

const
  LTI_UNICODE = 1;

const
{
  Set the listening info for the protocol.
  Pass NULL to remove it.
  wParam = NULL
  lParam = LISTENINGTOINFO *
}
  PS_SET_LISTENINGTO = '/SetListeningTo';

{
  Get the listening info for the protocol
  wParam = NULL
  lParam = LISTENINGTOINFO *
  The strings inside the struct need to be free using miranda free.
}
  PS_GET_LISTENINGT = '/GetListeningTo';

// Also the protocol have to save a string with the text the other user is (probabily) 
// seeing under the main db key: <protocol>/ListeningTo

// For a contact, the protocol should store the listening info as an string inside
// the contact db key: <protocol>/ListeningTo

// ListeningTo configuration plugin //////////////////////////////////////////////////

// One plugin can be used to set some options relative to the listening to information.
// But protocols should not assume this plugin exists. If it does not exist, protocols
// have to decide what default to use.
// This plugin have to support the following services:
{
   Get the text format the user wants him / his contacts to see. Some strings represents
   the text information:
   %artist%, %album%, %title%, %track%, %year%, %genre%, %length%, %player%, %type%
   This service is optional
   wParam = TCHAR* - default text for this protocol
   lParam = 0
   Returns a TCHAR* containg the user setting. This need to be free using miranda free.
}
  MS_LISTENINGTO_GETTEXTFORMAT:PAnsiChar = 'ListeningTo/GetTextFormat';

{
  Get the text the user wants him / his contacts to see, parsed with the info sent to 
  this service.  Uses the same variables as the above service to the default text.
  wParam = TCHAR* - default text for this protocol
  lParam = LISTENINGTOINFO *
  Returns a TCHAR* containg the parsed text. This need to be free using miranda free.
}
  MS_LISTENINGTO_GETPARSEDTEXT:PAnsiChar = 'ListeningTo/GetParsedText';

{
  Get if the contact options about how to show the music info should be overriten or 
  not.
  wParam = NULL
  lParam = hContact
  Returns a bool
}
  MS_LISTENINGTO_OVERRIDECONTACTOPTION:PAnsiChar = 'ListeningTo/OverrideContactOption';

{
  Get the text to show if some info of the contact is empty.
  wParam = NULL
  lParam = NULL
  Returns a TCHAR *. Don't free
}
  MS_LISTENINGTO_GETUNKNOWNTEXT:PAnsiChar = 'ListeningTo/GetUnknownText';

{$ENDIF}
