/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson

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

#include "stdafx.h"

#include "chat.h"

MCONTACT AddRoom(const char *pszModule, const wchar_t *pszRoom, const wchar_t *pszDisplayName, int iType)
{
	ptrW wszGroup(Chat_GetGroup());
	if (mir_wstrlen(wszGroup)) {
		MGROUP hGroup = Clist_GroupExists(wszGroup);
		if (hGroup == 0) {
			hGroup = Clist_GroupCreate(0, wszGroup);
			if (hGroup)
				Clist_GroupSetExpanded(hGroup, 1);
		}
	}

	MCONTACT hContact = g_chatApi.FindRoom(pszModule, pszRoom);
	if (hContact) { // contact exist, make sure it is in the right group
		if (mir_wstrlen(wszGroup)) {
			ptrW grpName(Clist_GetGroup(hContact));
			if (!mir_wstrcmp(wszGroup, grpName))
				Clist_SetGroup(hContact, wszGroup);
		}

		db_set_w(hContact, pszModule, "Status", ID_STATUS_OFFLINE);
		db_set_ws(hContact, pszModule, "Nick", pszDisplayName);
		return hContact;
	}

	// here we create a new one since no one is to be found
	if ((hContact = db_add_contact()) == 0)
		return 0;

	Proto_AddToContact(hContact, pszModule);
	Clist_SetGroup(hContact, wszGroup);

	db_set_ws(hContact, pszModule, "Nick", pszDisplayName);
	db_set_ws(hContact, pszModule, "ChatRoomID", pszRoom);
	db_set_b(hContact, pszModule, "ChatRoom", (uint8_t)iType);
	db_set_w(hContact, pszModule, "Status", ID_STATUS_OFFLINE);
	return hContact;
}

BOOL SetOffline(MCONTACT hContact, BOOL)
{
	if (hContact) {
		char *szProto = Proto_GetBaseAccountName(hContact);
		db_set_w(hContact, szProto, "ApparentMode", 0);
		db_set_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
		return TRUE;
	}

	return FALSE;
}

BOOL SetAllOffline(BOOL, const char *pszModule)
{
	for (auto &hContact : Contacts(pszModule)) {
		char *szProto = Proto_GetBaseAccountName(hContact);
		if (!MM_FindModule(szProto))
			continue;
		
		if (Contact::IsGroupChat(hContact, szProto)) {
			db_set_w(hContact, szProto, "ApparentMode", 0);
			db_set_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
		}
	}

	return TRUE;
}

int RoomDoubleclicked(WPARAM hContact, LPARAM)
{
	if (!hContact)
		return 0;

	char *szProto = Proto_GetBaseAccountName(hContact);
	if (MM_FindModule(szProto) == nullptr)
		return 0;
	if (!Contact::IsGroupChat(hContact, szProto))
		return 0;

	ptrW roomid(db_get_wsa(hContact, szProto, "ChatRoomID"));
	if (roomid == nullptr)
		return 0;

	SESSION_INFO *si = SM_FindSession(roomid, szProto);
	if (si) {
		if (si->pDlg != nullptr && !g_clistApi.pfnGetEvent(hContact, 0) && IsWindowVisible(si->pDlg->GetHwnd()) && !IsIconic(si->pDlg->GetHwnd())) {
			si->pDlg->CloseTab();
			return 1;
		}
		g_chatApi.ShowRoom(si);
	}
	return 1;
}

MCONTACT FindRoom(const char *pszModule, const wchar_t *pszRoom)
{
	for (auto &hContact : Contacts(pszModule)) {
		if (!Contact::IsGroupChat(hContact, pszModule))
			continue;

		ptrW roomid(db_get_wsa(hContact, pszModule, "ChatRoomID"));
		if (roomid != nullptr && !mir_wstrcmpi(roomid, pszRoom))
			return hContact;
	}

	return 0;
}
