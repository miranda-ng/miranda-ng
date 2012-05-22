/* 
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
*/


#ifndef __M_LISTENINGTO_H__
# define __M_LISTENINGTO_H__


#define MIID_LISTENINGTO { 0x1fc1efa, 0xaa9f, 0x461b, { 0x92, 0x69, 0xaf, 0x66, 0x6b, 0x89, 0x31, 0xee } }


// To be used by other plugins to send listening info to miranda
#define MIRANDA_WINDOWCLASS _T("Miranda.ListeningTo")
#define MIRANDA_DW_PROTECTION 0x8754

#define LISTENINGTO_ANSI 1
#define LISTENINGTO_UNICODE 2

#ifdef UNICODE
# define LISTENINGTO_TCHAR LISTENINGTO_UNICODE
#else
# define LISTENINGTO_TCHAR LISTENINGTO_ANSI
#endif


/*
Return TRUE if sending listening to is enabled for this protocol

wParam: char * - protocol name or NULL for all protocols
lParam: ignored
*/
#define MS_LISTENINGTO_ENABLED		"ListeningTo/Enabled"


/*
Enable/disable sending listening to this protocol

wParam: char * - protocol name or NULL for all protocols
lParam: BOOL - TRUE to enable, FALSE to disable
*/
#define MS_LISTENINGTO_ENABLE		"ListeningTo/Enable"


/*
Notification fired when enable state changed

wParam: char * - protocol name or NULL for all protocols
lParam: BOOL - enabled
*/
#define ME_LISTENINGTO_ENABLE_STATE_CHANGED		"ListeningTo/EnableStateChanged"


/*
Notification fired when listening info changed

wParam: LISTENINGTOINFO * - pointer to listening info structure or NULL
lParam: NULL
*/
#define ME_LISTENINGTO_LISTENING_INFO_CHANGED	"ListeningTo/ListeningInfoChanged"


/*
Provide new info about a song change to listening to

wParam: WCHAR * or char * - song data, in format "<Status 0-stoped 1-playing>\\0<Player>\\0<Type>\\0<Title>\\0<Artist>\\0<Album>\\0<Track>\\0<Year>\\0<Genre>\\0<Length (secs)>\\0<Radio Station>\\0"
lParam: format of wParam: one of LISTENINGTO_ANSI or LISTENINGTO_UNICODE . Anything else will be handled as unicode
*/
#define MS_LISTENINGTO_SET_NEW_SONG		"ListeningTo/SetNewSong"



#endif // __M_LISTENINGTO_H__
