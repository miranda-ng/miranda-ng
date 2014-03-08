{
Copyright (C) 2006 Ricardo Pescuma Domenecci

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

{$IFNDEF M_LISTENINGTO}
{$DEFINE M_LISTENINGTO}

// To be used by other plugins to send listening info to miranda
const
  MIRANDA_WINDOWCLASS = 'Miranda.ListeningTo';
  MIRANDA_DW_PROTECTION = $8754;
const
  LISTENINGTO_ANSI    = 1;
  LISTENINGTO_UNICODE = 2;

{
Return TRUE if sending listening to is enabled for this protocol

wParam: AnsiChar * - protocol name or NULL for all protocols
lParam: ignored
}
  MS_LISTENINGTO_ENABLED:PAnsiChar = 'ListeningTo/Enabled';

{
Enable/disable sending listening to this protocol

wParam: AnsiChar * - protocol name or NULL for all protocols
lParam: bool - TRUE to enable, FALSE to disable
}
  MS_LISTENINGTO_ENABLE:PAnsiChar = 'ListeningTo/Enable';

{
Notification fired when enable state changed

wParam: AnsiChar * - protocol name or NULL for all protocols
lParam: bool - enabled
}
  ME_LISTENINGTO_ENABLE_STATE_CHANGED:PAnsiChar = 'ListeningTo/EnableStateChanged';

{
Provide new info about a song change to listening to

wParam: WCHAR * or char * - song data, in format "<Status 0-stoped 1-playing>\\0<Player>\\0<Type>\\0<Title>\\0<Artist>\\0<Album>\\0<Track>\\0<Year>\\0<Genre>\\0<Length (secs)>\\0<Radio Station>\\0"
lParam: format of wParam: one of LISTENINGTO_ANSI or LISTENINGTO_UNICODE . Anything else will be handled as unicode
}
  MS_LISTENINGTO_SET_NEW_SONG:PAnsiChar = 'ListeningTo/SetNewSong';

{$ENDIF}
