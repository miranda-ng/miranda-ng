/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
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

#ifndef M_CORE_H__
#include <m_core.h>
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// this module provides UI and storage for blocking only, protocol modules are
// responsible for implementing the block

#define IGNOREEVENT_ALL        (int)(-1)
#define IGNOREEVENT_MESSAGE       1
#define IGNOREEVENT_FILE          2
#define IGNOREEVENT_USERONLINE    3
#define IGNOREEVENT_AUTHORIZATION 4
#define IGNOREEVENT_TYPINGNOTIFY  5

// determines if a message type to a contact should be ignored
// mask is one of IGNOREEVENT_* constants
EXTERN_C MIR_APP_DLL(bool) Ignore_IsIgnored(MCONTACT hContact, int mask);

// ignores certain type of events from a contact
// returns 0 on success or nonzero on failure
// Use hContact = 0 to retrieve the setting for unknown contacts
EXTERN_C MIR_APP_DLL(int) Ignore_Ignore(MCONTACT hContact, int mask);

// allows certain type of future events from a contact
// returns 0 on success or nonzero on failure
// Use hContact = NULL to retrieve the setting for unknown contacts
EXTERN_C MIR_APP_DLL(int) Ignore_Allow(MCONTACT hContact, int mask);

#endif // M_IGNORE_H__
