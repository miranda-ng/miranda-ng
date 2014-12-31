/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

#ifndef M_IGNORE_H__
#define M_IGNORE_H__ 1

//this module provides UI and storage for blocking only, protocol modules are
//responsible for implementing the block

#define IGNOREEVENT_ALL        (LPARAM)(-1)
#define IGNOREEVENT_MESSAGE       1
#define IGNOREEVENT_URL           2
#define IGNOREEVENT_FILE          3
#define IGNOREEVENT_USERONLINE    4
#define IGNOREEVENT_AUTHORIZATION 5
#define IGNOREEVENT_YOUWEREADDED  6 // 0.3.3a+
#define IGNOREEVENT_TYPINGNOTIFY  7 // 0.7+

//determines if a message type to a contact should be ignored  v0.1.0.1+
//wParam = (MCONTACT)hContact
//lParam = message type, an ignoreevent_ constant
//returns 0 if the message should be shown, or nonzero if it should be ignored
//Use hContact = NULL to retrieve the setting for unknown contacts (not on the
//contact list, as either permanent or temporary).
//don't use ignoreevent_all when calling this service
#define MS_IGNORE_ISIGNORED   "Ignore/IsIgnored"

//ignore future messages from a contact    v0.1.0.1+
//wParam = (MCONTACT)hContact
//lParam = message type, an ignoreevent_ constant
//returns 0 on success or nonzero on failure
//Use hContact = NULL to retrieve the setting for unknown contacts
#define MS_IGNORE_IGNORE      "Ignore/Ignore"

//receive future messages from a contact    v0.1.0.1+
//wParam = (MCONTACT)hContact
//lParam = message type, an ignoreevent_ constant
//returns 0 on success or nonzero on failure
//Use hContact = NULL to retrieve the setting for unknown contacts
#define MS_IGNORE_UNIGNORE    "Ignore/Unignore"


#endif // M_IGNORE_H__
