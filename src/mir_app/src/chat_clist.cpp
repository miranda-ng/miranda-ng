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
	wchar_t pszGroup[50]; *pszGroup = '\0';
	ptrW groupName(db_get_wsa(0, CHAT_MODULE, "AddToGroup"));
	if (groupName)
		wcsncpy_s(pszGroup, groupName, _TRUNCATE);
	else
		mir_wstrcpy(pszGroup, L"Chat rooms");

	if (pszGroup[0])  {
		MGROUP hGroup = Clist_GroupExists(pszGroup);
		if (hGroup == 0) {
			hGroup = Clist_GroupCreate(0, pszGroup);
			if (hGroup)
				Clist_GroupSetExpanded(hGroup, 1);
		}
	}

	MCONTACT hContact = chatApi.FindRoom(pszModule, pszRoom);
	if (hContact) { //contact exist, make sure it is in the right group
		if (pszGroup[0]) {
			ptrW grpName(db_get_wsa(hContact, "CList", "Group"));
			if (!mir_wstrcmp(pszGroup, grpName))
				db_set_ws(hContact, "CList", "Group", pszGroup);
		}

		db_set_w(hContact, pszModule, "Status", ID_STATUS_OFFLINE);
		db_set_ws(hContact, pszModule, "Nick", pszDisplayName);
		return hContact;
	}

	// here we create a new one since no one is to be found
	if ((hContact = db_add_contact()) == 0)
		return 0;

	Proto_AddToContact(hContact, pszModule);
	if (pszGroup[0])
		db_set_ws(hContact, "CList", "Group", pszGroup);
	else
		db_unset(hContact, "CList", "Group");
	db_set_ws(hContact, pszModule, "Nick", pszDisplayName);
	db_set_ws(hContact, pszModule, "ChatRoomID", pszRoom);
	db_set_b(hContact, pszModule, "ChatRoom", (BYTE)iType);
	db_set_w(hContact, pszModule, "Status", ID_STATUS_OFFLINE);
	return hContact;
}

BOOL SetOffline(MCONTACT hContact, BOOL)
{
	if (hContact) {
		char *szProto = GetContactProto(hContact);
		db_set_w(hContact, szProto, "ApparentMode", 0);
		db_set_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
		return TRUE;
	}

	return FALSE;
}

BOOL SetAllOffline(BOOL, const char *pszModule)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		char *szProto = GetContactProto(hContact);
		if (!chatApi.MM_FindModule(szProto))
			continue;
		if (pszModule && mir_strcmp(pszModule, szProto))
			continue;
		int i = db_get_b(hContact, szProto, "ChatRoom", 0);
		if (i != 0) {
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

	char *szProto = GetContactProto(hContact);
	if (chatApi.MM_FindModule(szProto) == nullptr)
		return 0;
	if (db_get_b(hContact, szProto, "ChatRoom", 0) == 0)
		return 0;

	ptrW roomid(db_get_wsa(hContact, szProto, "ChatRoomID"));
	if (roomid == nullptr)
		return 0;

	SESSION_INFO *si = SM_FindSession(roomid, szProto);
	if (si) {
		// is the "toggle visibility option set, so we need to close the window?
		if (si->pDlg != nullptr && db_get_b(0, CHAT_MODULE, "ToggleVisibility", 0) == 1 && !cli.pfnGetEvent(hContact, 0) && IsWindowVisible(si->pDlg->GetHwnd()) && !IsIconic(si->pDlg->GetHwnd())) {
			si->pDlg->CloseTab();
			return 1;
		}
		chatApi.ShowRoom(si);
	}
	return 1;
}

static INT_PTR EventDoubleclicked(WPARAM,LPARAM lParam)
{
	return RoomDoubleclicked((WPARAM)((CLISTEVENT*)lParam)->hContact, 0);
}

INT_PTR JoinChat(WPARAM hContact, LPARAM lParam)
{
	if (hContact) {
		char *szProto = GetContactProto(hContact);
		if (szProto) {
			if (db_get_w(hContact, szProto, "Status", 0) == ID_STATUS_OFFLINE)
				CallProtoService(szProto, PS_JOINCHAT, hContact, lParam);
			else
				RoomDoubleclicked(hContact, 0);
		}
	}

	return 0;
}

INT_PTR LeaveChat(WPARAM hContact, LPARAM lParam)
{
	if (hContact) {
		char *szProto = GetContactProto(hContact);
		if (szProto)
			CallProtoService(szProto, PS_LEAVECHAT, hContact, lParam);
	}
	return 0;
}

int PrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (hContact == 0)
		return 0;

	bool bEnabledJoin = false, bEnabledLeave = false;
	char *szProto = GetContactProto(hContact);
	if (szProto) {
		// display this menu item only for chats
		if (db_get_b(hContact, szProto, "ChatRoom", 0)) {
			// still hide it for offline protos
			if (CallProtoService(szProto, PS_GETSTATUS, 0, 0) != ID_STATUS_OFFLINE) {
				if (db_get_w(hContact, szProto, "Status", 0) == ID_STATUS_OFFLINE) {
					if (ProtoServiceExists(szProto, PS_JOINCHAT)) {
						bEnabledJoin = true;
						Menu_ModifyItem(hJoinMenuItem, LPGENW("&Join chat"));
					}
				}
				else {
					bEnabledJoin = true;
					Menu_ModifyItem(hJoinMenuItem, LPGENW("&Open chat window"));
				}
			}
			bEnabledLeave = ProtoServiceExists(szProto, PS_LEAVECHAT) != 0;
		}
	}

	Menu_ShowItem(hJoinMenuItem, bEnabledJoin);
	Menu_ShowItem(hLeaveMenuItem, bEnabledLeave);
	return 0;
}

BOOL AddEvent(MCONTACT hContact, HICON hIcon, MEVENT hEvent, int type, wchar_t* fmt, ...)
{
	wchar_t szBuf[4096];

	if (!fmt || !fmt[0] || mir_wstrlen(fmt) > 2000)
		return FALSE;

	va_list marker;
	va_start(marker, fmt);
	mir_vsnwprintf(szBuf, _countof(szBuf), fmt, marker);
	va_end(marker);

	CLISTEVENT cle = {};
	cle.hContact = hContact;
	cle.hDbEvent = hEvent;
	cle.flags = type | CLEF_UNICODE;
	cle.hIcon = hIcon;
	cle.pszService = "GChat/DblClickEvent";
	cle.szTooltip.w = TranslateW(szBuf);

	if (!ServiceExists(cle.pszService))
		CreateServiceFunction(cle.pszService, &EventDoubleclicked);

	if (type) {
		if (!cli.pfnGetEvent(hContact, 0))
			cli.pfnAddEvent(&cle);
	}
	else {
		if (cli.pfnGetEvent(hContact, 0))
			cli.pfnRemoveEvent(hContact, GC_FAKE_EVENT);
		cli.pfnAddEvent(&cle);
	}
	return TRUE;
}

MCONTACT FindRoom(const char *pszModule, const wchar_t *pszRoom)
{
	for (MCONTACT hContact = db_find_first(pszModule); hContact; hContact = db_find_next(hContact, pszModule)) {
		if (!db_get_b(hContact, pszModule, "ChatRoom", 0))
			continue;

		ptrW roomid(db_get_wsa(hContact, pszModule, "ChatRoomID"));
		if (roomid != nullptr && !mir_wstrcmpi(roomid, pszRoom))
			return hContact;
	}

	return 0;
}
