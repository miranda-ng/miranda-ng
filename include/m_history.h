/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)
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

#ifndef M_HISTORY_H__
#define M_HISTORY_H__ 1

/////////////////////////////////////////////////////////////////////////////////////////
// Called when server history load process is finished

namespace History
{
	MIR_APP_DLL(void) FinishLoad(MCONTACT hContact);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Shows the History dialog box for a contact
// wParam = (MCONTACT)hContact
// lParam = 0 (unused)
// hContact can be NULL to show system messages

#define MS_HISTORY_SHOWCONTACTHISTORY "History/ShowContactHistory"

/////////////////////////////////////////////////////////////////////////////////////////
// Empties contact's history
// wParam = (MCONTACT)hContact
// lParam = (BOOL)bForce - force history deletion, don't ask questions
// hContact can be NULL to wipe system history

#define MS_HISTORY_EMPTY "History/EmptyHistory"

/////////////////////////////////////////////////////////////////////////////////////////
// Fired when server history is loaded
// wParam = (MCONTACT)hContact
// lParam = 0 (unused)

#define ME_HISTORY_LOADED "History/Loaded"

#endif // M_HISTORY_H__
