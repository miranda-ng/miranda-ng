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

#ifndef M_ADDCONTACT_H__
#define M_ADDCONTACT_H__ 1

typedef struct{
	int handleType;         // one of the HANDLE_ constants
	union {
		HANDLE hDbEvent;     // hDbEvent if acs.handleType == HANDLE_EVENT
		MCONTACT hContact;   // hContact if acs.handleType == HANDLE_CONTACT
		                     // ignored if acs.handleType == HANDLE_SEARCHRESULT
	};
	const char *szProto;    // ignored if acs.handleType != HANDLE_SEARCHRESULT
	PROTOSEARCHRESULT *psr; // ignored if acs.handleType != HANDLE_SEARCHRESULT
}ADDCONTACTSTRUCT;

/*
	Description: Open the add contact dialog, see notes

	wParam = [ (WPARAM)hWndParent ]
	lParam = (LPARAM)(ADDCONTACTSTRUCT*)&acs

	Notes:

	During 0.3.0.0 development (circa 2003/06/08)

	An entire copy of the ADDCONTACTSTRUCT structure is made,
	the PROTOSEARCHRESULT is also copied at psr->cbSize, all default
	strings are copied (even if they're not needed/used)
	if you pass a structure that is bigger than PROTOSEARCHRESULT
	then do not use pointers in the newly defined area because
	they will not be copied.

	passing wParam == NULL will result in a dialog that is created modeless

	Before the circa, a modal dialog was *always* created and
	strings within PROTOSEARCHRESULT were not copied.

*/

#define HANDLE_SEARCHRESULT		0
#define HANDLE_EVENT			1
#define HANDLE_CONTACT			2
#define MS_ADDCONTACT_SHOW		"AddContact/Show"

#endif // M_ADDCONTACT_H__
