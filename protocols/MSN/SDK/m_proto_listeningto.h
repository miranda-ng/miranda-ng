/*

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
*/

//this module was created in v0.6.0.0

#ifndef M_PROTO_LISTENINGTO_H__
#define M_PROTO_LISTENINGTO_H__ 1


// Protocol Services /////////////////////////////////////////////////////////////////

// This is the services a protocol have to support to support listening info

typedef struct {
	int cbSize;

	union {
		char* pszType;			// Media type: Music, Video, etc...
		TCHAR* ptszType;
	};
	union {
		char* pszArtist;		// Artist name
		TCHAR* ptszArtist;
	};
	union {
		char* pszAlbum;			// Algum name
		TCHAR* ptszAlbum;
	};
	union {
		char* pszTitle;			// Song name
		TCHAR* ptszTitle;
	};
	union {
		char* pszTrack;			// Track number
		TCHAR* ptszTrack;
	};
	union {
		char* pszYear;			// Song year
		TCHAR* ptszYear;
	};
	union {
		char* pszGenre;			// Song genre
		TCHAR* ptszGenre;
	};
	union {
		char* pszLength;		// Song length
		TCHAR* ptszLength;
	};
	union {
		char* pszPlayer;		// Player name
		TCHAR* ptszPlayer;
	};

	DWORD dwFlags;

} LISTENINGTOINFO;

#define LTI_UNICODE 1

#ifdef UNICODE
	#define LTI_TCHAR   LTI_UNICODE
#else
	#define LTI_TCHAR   0
#endif

// Set the listening info for the protocol.
// Pass NULL to remove it.
// wParam = NULL
// lParam = LISTENINGTOINFO *
#define PS_SET_LISTENINGTO	"/SetListeningTo"

// Get the listening info for the protocol
// wParam = NULL
// lParam = LISTENINGTOINFO *
// The strings inside the struct need to be free using miranda free.
#define PS_GET_LISTENINGTO	"/GetListeningTo"

// Also the protocol have to save a string with the text the other user is (probabily) 
// seeing under the main db key: <protocol>/ListeningTo

// For a contact, the protocol should store the listening info as an string inside
// the contact db key: <protocol>/ListeningTo


// ListeningTo configuration plugin //////////////////////////////////////////////////

// One plugin can be used to set some options relative to the listening to information.
// But protocols should not assume this plugin exists. If it does not exist, protocols
// have to decide what default to use.
// This plugin have to support the following services:

// Get the text format the user wants him / his contacts to see. Some strings represents
// the text information:
// %artist%, %album%, %title%, %track%, %year%, %genre%, %length%, %player%, %type%
// This service is optional
// wParam = TCHAR* - default text for this protocol
// lParam = 0
// Returns a TCHAR* containg the user setting. This need to be free using miranda free.
#define MS_LISTENINGTO_GETTEXTFORMAT	"ListeningTo/GetTextFormat"

// Get the text the user wants him / his contacts to see, parsed with the info sent to 
// this service.  Uses the same variables as the above service to the default text.
// wParam = TCHAR* - default text for this protocol
// lParam = LISTENINGTOINFO *
// Returns a TCHAR* containg the parsed text. This need to be free using miranda free.
#define MS_LISTENINGTO_GETPARSEDTEXT	"ListeningTo/GetParsedText"

// Get if the contact options about how to show the music info should be overriten or 
// not.
// wParam = NULL
// lParam = hContact
// Returns a BOOL
#define MS_LISTENINGTO_OVERRIDECONTACTOPTION	"ListeningTo/OverrideContactOption"

// Get the text to show if some info of the contact is empty.
// wParam = NULL
// lParam = NULL
// Returns a TCHAR *. Don't free
#define MS_LISTENINGTO_GETUNKNOWNTEXT	"ListeningTo/GetUnknownText"


#endif  // M_PROTO_LISTENINGTO_H__


