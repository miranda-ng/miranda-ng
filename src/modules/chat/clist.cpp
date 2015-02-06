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

#include "..\..\core\commonheaders.h"

#include "chat.h"

MCONTACT AddRoom(const char *pszModule, const TCHAR *pszRoom, const TCHAR *pszDisplayName, int iType)
{
	TCHAR pszGroup[50]; *pszGroup = '\0';
	ptrT groupName(db_get_tsa(NULL, CHAT_MODULE, "AddToGroup"));
	if (groupName)
		_tcsncpy_s(pszGroup, groupName, _TRUNCATE);
	else
		_tcscpy(pszGroup, _T("Chat rooms"));

	if (pszGroup[0])  {
		HANDLE hGroup = Clist_GroupExists(pszGroup);
		if (hGroup == 0) {
			hGroup = Clist_CreateGroup(0, pszGroup);
			if (hGroup) {
				CallService(MS_CLUI_GROUPADDED, (WPARAM)hGroup, 0);
				CallService(MS_CLIST_GROUPSETEXPANDED, (WPARAM)hGroup, 1);
			}
		}
	}

	MCONTACT hContact = ci.FindRoom(pszModule, pszRoom);
	if (hContact) { //contact exist, make sure it is in the right group
		if (pszGroup[0]) {
			ptrT grpName(db_get_tsa(hContact, "CList", "Group"));
			if (!mir_tstrcmp(pszGroup, grpName))
				db_set_ts(hContact, "CList", "Group", pszGroup);
		}

		db_set_w(hContact, pszModule, "Status", ID_STATUS_OFFLINE);
		db_set_ts(hContact, pszModule, "Nick", pszDisplayName);
		return hContact;
	}

	// here we create a new one since no one is to be found
	if ((hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0)) == NULL)
		return NULL;

	CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)pszModule);
	if (pszGroup[0])
		db_set_ts(hContact, "CList", "Group", pszGroup);
	else
		db_unset(hContact, "CList", "Group");
	db_set_ts(hContact, pszModule, "Nick", pszDisplayName);
	db_set_ts(hContact, pszModule, "ChatRoomID", pszRoom);
	db_set_b(hContact, pszModule, "ChatRoom", (BYTE)iType);
	db_set_w(hContact, pszModule, "Status", ID_STATUS_OFFLINE);
	return hContact;
}

BOOL SetOffline(MCONTACT hContact, BOOL bHide)
{
	if (hContact) {
		char *szProto = GetContactProto(hContact);
		db_set_w(hContact, szProto, "ApparentMode", 0);
		db_set_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
		return TRUE;
	}

	return FALSE;
}

BOOL SetAllOffline(BOOL bHide, const char *pszModule)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		char *szProto = GetContactProto(hContact);
		if (!ci.MM_FindModule(szProto))
			continue;
		if (pszModule && strcmp(pszModule, szProto))
			continue;
		int i = db_get_b(hContact, szProto, "ChatRoom", 0);
		if (i != 0) {
			db_set_w(hContact, szProto, "ApparentMode", (LPARAM)(WORD)0);
			db_set_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
		}
	}

	return TRUE;
}

int RoomDoubleclicked(WPARAM hContact, LPARAM lParam)
{
	if (!hContact)
		return 0;

	char *szProto = GetContactProto(hContact);
	if (ci.MM_FindModule(szProto) == NULL)
		return 0;
	if (db_get_b(hContact, szProto, "ChatRoom", 0) == 0)
		return 0;

	ptrT roomid(db_get_tsa(hContact, szProto, "ChatRoomID"));
	if (roomid == NULL)
		return 0;

	SESSION_INFO *si = ci.SM_FindSession(roomid, szProto);
	if (si) {
		// is the "toggle visibility option set, so we need to close the window?
		if (si->hWnd != NULL &&
			 db_get_b(NULL, CHAT_MODULE, "ToggleVisibility", 0) == 1 &&
			 !CallService(MS_CLIST_GETEVENT, hContact, 0) &&
			 IsWindowVisible(si->hWnd) && !IsIconic(si->hWnd))
		{
			if (ci.OnDblClickSession)
				ci.OnDblClickSession(si);
			return 1;
		}
		ci.ShowRoom(si, WINDOW_VISIBLE, TRUE);
	}
	return 1;
}

INT_PTR EventDoubleclicked(WPARAM wParam,LPARAM lParam)
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
				CLISTMENUITEM mi = { sizeof(mi) };
				mi.flags = CMIM_NAME;
				if (db_get_w(hContact, szProto, "Status", 0) == ID_STATUS_OFFLINE) {
					if (ProtoServiceExists(szProto, PS_JOINCHAT)) {
						bEnabledJoin = true;
						mi.pszName = LPGEN("&Join chat");
					}
				}
				else {
					bEnabledJoin = true;
					mi.pszName = LPGEN("&Open chat window");
				}
				Menu_ModifyItem(hJoinMenuItem, &mi);
			}
			bEnabledLeave = ProtoServiceExists(szProto, PS_LEAVECHAT) != 0;
		}
	}

	Menu_ShowItem(hJoinMenuItem, bEnabledJoin);
	Menu_ShowItem(hLeaveMenuItem, bEnabledLeave);
	return 0;
}

INT_PTR PrebuildContactMenuSvc(WPARAM wParam, LPARAM lParam)
{
	return PrebuildContactMenu(wParam, lParam);
}

BOOL AddEvent(MCONTACT hContact, HICON hIcon, MEVENT hEvent, int type, TCHAR* fmt, ...)
{
	TCHAR szBuf[4096];

	if (!fmt || !fmt[0] || _tcslen(fmt) > 2000)
		return FALSE;

	va_list marker;
	va_start(marker, fmt);
	mir_vsntprintf(szBuf, SIZEOF(szBuf), fmt, marker);
	va_end(marker);

	CLISTEVENT cle = { 0 };
	cle.cbSize = sizeof(cle);
	cle.hContact = hContact;
	cle.hDbEvent = hEvent;
	cle.flags = type | CLEF_TCHAR;
	cle.hIcon = hIcon;
	cle.pszService = "GChat/DblClickEvent" ;
	cle.ptszTooltip = TranslateTS(szBuf);
	if (type) {
		if (!CallService(MS_CLIST_GETEVENT, hContact, 0))
			CallService(MS_CLIST_ADDEVENT, hContact, (LPARAM)&cle);
	}
	else {
		if (CallService(MS_CLIST_GETEVENT, hContact, 0))
			CallService(MS_CLIST_REMOVEEVENT, hContact, (LPARAM)GC_FAKE_EVENT);
		CallService(MS_CLIST_ADDEVENT, hContact, (LPARAM)&cle);
	}
	return TRUE;
}

MCONTACT FindRoom(const char *pszModule, const TCHAR *pszRoom)
{
	for (MCONTACT hContact = db_find_first(pszModule); hContact; hContact = db_find_next(hContact, pszModule)) {
		if (!db_get_b(hContact, pszModule, "ChatRoom", 0))
			continue;

		ptrT roomid(db_get_tsa(hContact, pszModule, "ChatRoomID"));
		if (roomid != NULL && !mir_tstrcmpi(roomid, pszRoom))
			return hContact;
	}

	return 0;
}
