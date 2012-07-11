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

// To be used by other plugins to send listening info to miranda
#define MIRANDA_WINDOWCLASS _T("Miranda.ListeningTo")
#define MIRANDA_DW_PROTECTION 0x8754


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


#endif // __M_LISTENINGTO_H__
